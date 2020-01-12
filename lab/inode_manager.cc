#include "inode_manager.h"

// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void disk::read_block(blockid_t id, char *buf)
{
  if (id < 0 || id >= BLOCK_NUM || buf == NULL)
    return;

  memcpy(buf, blocks[id], BLOCK_SIZE);
  buf[BLOCK_SIZE] = '\0';
}

void disk::write_block(blockid_t id, const char *buf)
{
  if (id < 0 || id >= BLOCK_NUM || buf == NULL)
    return;

  memcpy(blocks[id], buf, BLOCK_SIZE);
}

// block layer -----------------------------------------

// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /*
   * your code goes here.
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.
   */
  pthread_mutex_lock(&block_lock);
  for (uint32_t i = IBLOCK(INODE_NUM, sb.nblocks) + 1; i < sb.nblocks; i++)
  {
    if (using_blocks.count(i) <= 0 || using_blocks[i] == 0)
    {
      using_blocks[i] = 1;
      pthread_mutex_unlock(&block_lock);
      return i;
    }
  }
  pthread_mutex_unlock(&block_lock);
  return 0;
}

void block_manager::free_block(uint32_t id)
{
  /* 
   * your code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  using_blocks[id] = 0;
  return;
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager()
{
  d = new disk();
  pthread_mutex_init(&block_lock, NULL);
  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;
}

void block_manager::read_block(uint32_t id, char *buf)
{
  d->read_block(id, buf);
}

void block_manager::write_block(uint32_t id, const char *buf)
{
  d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1)
  {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
  pthread_mutex_init(&ino_lock, NULL);
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* 
   * your code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().
   */
  pthread_mutex_lock(&ino_lock);
  inode_t new_inode;
  new_inode.type = type;
  time_t now = time(NULL);
  new_inode.size = 0;
  new_inode.atime = now;
  new_inode.mtime = now;
  new_inode.ctime = now;
  // find an inode num to use
  struct inode *ino;
  char buf[BLOCK_SIZE];
  for (uint32_t i = 1; i < INODE_NUM; i++)
  {
    bm->read_block(IBLOCK(i, bm->sb.nblocks), buf);
    ino = (struct inode *)buf + i % IPB;
    if (ino->type == 0)
    {
      put_inode(i, &new_inode);
      pthread_mutex_unlock(&ino_lock);
      return i;
    }
  }
  pthread_mutex_unlock(&ino_lock);
  return 0;
}

void inode_manager::free_inode(uint32_t inum)
{
  /* 
   * your code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   */
  inode_t *ino = get_inode(inum);
  if (ino != NULL)
  {
    int block_num = (ino->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (block_num > NDIRECT)
    {
      int i = 0;
      for (; i < NDIRECT; i++)
      {
        bm->free_block(ino->blocks[i]);
      }
      char indirect_block[BLOCK_SIZE];
      bm->read_block(ino->blocks[NDIRECT], indirect_block);
      for (; i < block_num; i++)
      {
        blockid_t bid = ((int *)indirect_block)[i - NDIRECT];
        bm->free_block(bid);
      }
      bm->free_block(ino->blocks[NDIRECT]);
    }
    else
    {
      for (int i = 0; i < block_num; i++)
      {
        bm->free_block(ino->blocks[i]);
      }
    }

    ino->type = 0;
    ino->size = 0;
    ino->atime = 0;
    ino->mtime = 0;
    ino->ctime = 0;
    // memset(ino->blocks, 0, sizeof(ino->blocks));
    put_inode(inum, ino);
    free(ino);
  }
  return;
}

/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode *
inode_manager::get_inode(uint32_t inum)
{
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];

  printf("\tim: get_inode %d\n", inum);

  if (inum < 0 || inum >= INODE_NUM)
  {
    printf("\tim: inum out of range\n");
    return NULL;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  // printf("%s:%d\n", __FILE__, __LINE__);

  ino_disk = (struct inode *)buf + inum % IPB;
  if (ino_disk->type == 0)
  {
    printf("\tim: inode not exist\n");
    return NULL;
  }

  ino = (struct inode *)malloc(sizeof(struct inode));
  *ino = *ino_disk;

  return ino;
}

void inode_manager::put_inode(uint32_t inum, struct inode *ino)
{
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;

  printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode *)buf + inum % IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void inode_manager::read_file(uint32_t inum, char **buf_out, int *size)
{
  /*
   * your code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_Out
   */
  inode_t *ino = get_inode(inum);
  if (ino != NULL)
  {
    *size = ino->size;
    int block_num = (ino->size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    char buf[BLOCK_SIZE];
    char *full_buf = (char *)malloc(block_num * BLOCK_SIZE);
    memset(full_buf, 0, *size + 1);
    int i = 0;
    for (; i < MIN(block_num, NDIRECT); i++)
    {
      blockid_t bid = ino->blocks[i];
      bm->read_block(bid, buf);
      memcpy(full_buf + i * BLOCK_SIZE, buf, BLOCK_SIZE);
      // strncat(full_buf, buf, strlen(buf));
    }
    char indirect_block[BLOCK_SIZE];
    if (block_num > NDIRECT)
    {
      bm->read_block(ino->blocks[NDIRECT], indirect_block);
      for (; i < block_num; i++)
      {
        blockid_t bid = ((int *)indirect_block)[i - NDIRECT];
        bm->read_block(bid, buf);
        memcpy(full_buf + i * BLOCK_SIZE, buf, BLOCK_SIZE);
        // strncat(full_buf, buf, strlen(buf));
      }
    }
    *buf_out = full_buf;

    time_t now = time(NULL);
    ino->atime = now;
    put_inode(inum, ino);
    free(ino);
  }
  return;
}

/* alloc/free blocks if needed */
void inode_manager::write_file(uint32_t inum, const char *buf, int size)
{
  /*
   * your code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode
   */
  inode_t *ino = get_inode(inum);
  if (ino != NULL)
  {
    int origin_size = ino->size;
    int origin_block_num = (origin_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int need_block_num = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    blockid_t blocks[MAXFILE];
    //expand ino->blocks
    char indirect_block[BLOCK_SIZE];
    if (origin_block_num > NDIRECT)
    {
      bm->read_block(ino->blocks[NDIRECT], indirect_block);
    }
    for (int i = 0; i < origin_block_num; i++)
    {
      if (i < NDIRECT)
      {
        blocks[i] = ino->blocks[i];
      }
      else
      {
        // get indirect block content
        blocks[i] = ((uint32_t *)indirect_block)[i - NDIRECT];
      }
    }

    // consider alloc more or free some
    if (origin_size < size)
    {
      int i = 0;
      for (; i < origin_block_num; i++)
      {
        blockid_t bid = blocks[i];
        bm->write_block(bid, &buf[i * BLOCK_SIZE]);
      }
      for (; i < need_block_num; i++)
      {
        blockid_t bid = bm->alloc_block();
        blocks[i] = bid;
        bm->write_block(bid, &buf[i * BLOCK_SIZE]);
      }
    }
    else
    {
      int i = 0;
      for (; i < need_block_num; i++)
      {
        bm->write_block(blocks[i], &buf[i * BLOCK_SIZE]);
      }
      for (; i < origin_block_num; i++)
      {
        bm->free_block(blocks[i]);
        blocks[i] = 0;
      }
    }
    // update ino->blocks with blocks
    int min_one = MIN(need_block_num, origin_block_num);
    for (int i = min_one; i < NDIRECT; i++)
    {
      ino->blocks[i] = blocks[i];
    }
    if (need_block_num > NDIRECT)
    {
      if (origin_block_num <= NDIRECT)
      {
        ino->blocks[NDIRECT] = bm->alloc_block();
      }
      blockid_t *indirect_block_begin = &blocks[NDIRECT];
      bm->write_block(ino->blocks[NDIRECT], (char *)indirect_block_begin);
    }
    else if (origin_block_num > NDIRECT)
    {
      bm->free_block(ino->blocks[NDIRECT]);
      ino->blocks[NDIRECT] = 0;
    }

    ino->size = size;
    time_t now = time(NULL);
    ino->mtime = now;
    ino->ctime = now;
    // if (ino->type == extent_protocol::T_DIR)
    // {
    //   ino->ctime = now;
    // }
    put_inode(inum, ino);
    free(ino);
  }
  return;
}

void inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /*
   * your code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
  struct inode *ino;
  char buf[BLOCK_SIZE];
  if (inum < 0 || inum >= INODE_NUM)
  {
    printf("\tim: inum out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);

  ino = (struct inode *)buf + inum % IPB;

  a.type = ino->type;
  a.atime = ino->atime;
  a.mtime = ino->mtime;
  a.ctime = ino->ctime;
  a.size = ino->size;
  return;
}

void inode_manager::remove_file(uint32_t inum)
{
  /*
   * your code goes here
   * note: you need to consider about both the data block and inode of the file
   */
  free_inode(inum);
  return;
}
