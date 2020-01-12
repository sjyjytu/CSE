// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
  ec = new extent_client(extent_dst);
  lc = new lock_client(lock_dst);
  lc->acquire(1);
  if (ec->put(1, "") != extent_protocol::OK)
      printf("error init root dir\n"); // XYB: init root dir
  lc->release(1);
}


yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool yfs_client::isfile(inum inum)
{
    //lc->acquire(inum);
    bool r = __isfile(inum);
    //lc->release(inum);
    return r;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool yfs_client::isdir(inum inum)
{
    //lc->acquire(inum);
    bool r = __isdir(inum);
    //lc->release(inum);
    return r;
}

int yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;
    lc->acquire(inum);
    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK)
    {
        lc->release(inum);
        return IOERR;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);
    lc->release(inum);
    return r;
}

int yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;
    lc->acquire(inum);
    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK)
    {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    lc->release(inum);
    return r;
}

#define EXT_RPC(xx)                                                \
    do                                                             \
    {                                                              \
        if ((xx) != extent_protocol::OK)                           \
        {                                                          \
            printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
            r = IOERR;                                             \
            goto release;                                          \
        }                                                          \
    } while (0)

// Only support set size of attr
int yfs_client::setattr(inum ino, size_t size)
{
    int r = OK;

    /*
     * your code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */
    extent_protocol::attr a;
    lc->acquire(ino);
    if (ec->getattr(ino, a) != extent_protocol::OK)
    {
        r = IOERR;
        lc->release(ino);
        return r;
    }
    if (a.size == size)
    {
        lc->release(ino);
        return r;
    }
    else
    {
        std::string buf;
        ec->get(ino, buf);
        buf.resize(size);
        ec->put(ino, buf);
        lc->release(ino);
    }
    return r;
}

int yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    lc->acquire(parent);
    bool found = 0;
    if ((r = __lookup(parent, name, found, ino_out)) != OK)
    {
        lc->release(parent);
        return r;
    }
    if (!found)
    {
        ec->create(extent_protocol::T_FILE, ino_out);
        std::string dir_content;
        __readdir(parent, dir_content);
        dir_content = dir_content + name + "/" + filename(ino_out) + "/";
        ec->put(parent, dir_content);
    }
    lc->release(parent);
    return r;
}

int yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    lc->acquire(parent);
    bool found = 0;
    if ((r = __lookup(parent, name, found, ino_out)) != OK)
    {
        lc->release(parent);
        return r;
    }
    if (!found)
    {
        ec->create(extent_protocol::T_DIR, ino_out);
        std::string dir_content;
        __readdir(parent, dir_content);
        dir_content = dir_content + name + "/" + filename(ino_out) + "/";
        ec->put(parent, dir_content);
    }
    lc->release(parent);
    return r;
}

int yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    //lc->acquire(parent);
    r = __lookup(parent, name, found, ino_out);
    //lc->release(parent);
    return r;
}

int yfs_client::readdir(inum dir, std::string &content)
{
    int r = OK;
    /*
     * your code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    //lc->acquire(dir);
    r = __readdir(dir, content);
    //lc->release(dir);
    return r;
}

int yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;

    /*
     * your code goes here.
     * note: read using ec->get().
     */
    lc->acquire(ino);
    std::string content;
    ec->get(ino, content);

    if ((unsigned int)off >= content.size())
    {
        data.erase();
        lc->release(ino);
        return r;
    }

    data = content.substr(off, size);
    lc->release(ino);
    return r;
}

int yfs_client::write(inum ino, size_t size, off_t off, const char *data,
                      size_t &bytes_written)
{
    int r = OK;

    /*
     * your code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    lc->acquire(ino);
    std::string content;
    ec->get(ino, content);
    std::string buf;
    buf.assign(data, size);
    if ((unsigned int)off <= content.size())
    {
        content.replace(off, size, buf);
        bytes_written = size;
    }
    else
    {
        size_t old_size = content.size();
        content.resize(size + off, '\0');
        content.replace(off, size, buf);
        bytes_written = size + off - old_size;
    }
    ec->put(ino, content);
    lc->release(ino);
    return r;
}

int yfs_client::unlink(inum parent, const char *name)
{
    int r = OK;

    /*
     * your code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
    lc->acquire(parent);
    bool found = 0;
    inum ino;
    if ((r = __lookup(parent, name, found, ino)) != OK)
    {
        lc->release(parent);
        return r;
    }
    if (found)
    {
        std::string dir_content;
        __readdir(parent, dir_content); // example:  a.c/123/b.o/145/c.d/135/

        std::string file_to_delete = std::string(name) + "/";
        int pos = dir_content.find(file_to_delete);
        if (pos != -1)
        {
            std::string left_dir_content = dir_content.substr(0, pos);           // a.c/123/
            std::string right_dir_content = dir_content.substr(pos);             // b.o/145/c.d/135/
            right_dir_content = right_dir_content.substr(file_to_delete.size()); // 145/c.d/135/
            int num_end_pos = right_dir_content.find('/');
            if (num_end_pos == -1)
            {
                lc->release(parent);
                return ENOENT;
            }
            right_dir_content = (unsigned int)num_end_pos == right_dir_content.size() - 1 ? "" : right_dir_content.substr(num_end_pos + 1);
            dir_content = left_dir_content + right_dir_content;
        }

        ec->put(parent, dir_content);
        ec->remove(ino);
    }
    lc->release(parent);
    return r;
}

int yfs_client::readlink(inum ino, std::string &data)
{
    int r = OK;
    lc->acquire(ino);
    std::string buf;
    r = ec->get(ino, buf);
    data = buf;
    lc->release(ino);
    return r;
}

int yfs_client::symlink(inum parent, const char *name, const char *content, inum &ino_out)
{
    lc->acquire(parent);
    int r = OK;
    bool found = 0;
    if ((r = __lookup(parent, name, found, ino_out)) != OK)
    {
        lc->release(parent);
        return r;
    }
    if (!found)
    {
        r = ec->create(extent_protocol::T_SYMLINK, ino_out);
        r = ec->put(ino_out, std::string(content));
        std::string dir_content;
        __readdir(parent, dir_content);
        dir_content = dir_content + name + "/" + filename(ino_out) + "/";
        ec->put(parent, dir_content);
    }
    else
    {
        r = EEXIST;
    }
    lc->release(parent);
    return r;
}

bool yfs_client::__isfile(inum inum)
{
    extent_protocol::attr a;
    // lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK)
    {
        printf("error getting attr\n");
        // lc->release(inum);
        return false;
    }
    // lc->release(inum);
    if (a.type == extent_protocol::T_FILE)
    {
        printf("isfile: %lld is a file\n", inum);
        return true;
    }
    printf("isfile: %lld is not a file\n", inum);
    return false;
}

bool yfs_client::__isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK)
    {
        printf("error getting attr\n");
        return false;
    }
    if (a.type == extent_protocol::T_DIR)
    {
        printf("isdir: %lld is a dir\n", inum);
        return true;
    }
    printf("isdir: %lld is not a dir\n", inum);
    return false;
}

int yfs_client::__readdir(inum dir, std::string &content)
{
    int r = OK;
    /*
     * your code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    if (!__isdir(dir))
    {
        r = NOENT;
    }
    else
    {
        // example:  a.c/123/b.o/145/
        ec->get(dir, content);
    }
    return r;
}

int yfs_client::__lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    /*
     * your code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    std::string content;
    if ((r = __readdir(parent, content)) == OK)
    {
        // content example:  a.c/123/b.o/145/c.d/156/
        std::string to_find = std::string(name) + "/"; // b.o/
        int pos = content.find(to_find);
        if (pos != -1)
        {
            found = 1;
            content = content.substr(pos + to_find.size());
            ino_out = n2i(content.substr(0, content.find_first_of('/')));
        }
    }
    return r;
}