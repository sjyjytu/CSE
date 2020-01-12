// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
  pthread_mutex_init(&lock, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab2 part2 code goes here
  std::map<lock_protocol::lockid_t, int>::iterator iter = lock_table.find(lid);
  pthread_mutex_lock(&lock);
  if (iter != lock_table.end() && iter->second != 0)
  {
    //not available
    while(lock_table[lid] != 0)
    {
      pthread_cond_wait(cond_table[lid], &lock);
    }
  }
  else if (iter == lock_table.end())
  {
    pthread_cond_t *init = new pthread_cond_t;
    pthread_cond_init(init, NULL);
    cond_table[lid] = init;
  }
  lock_table[lid] = 1;
  pthread_mutex_unlock(&lock);
  r = ret;
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab2 part2 code goes here
  pthread_mutex_lock(&lock);
  lock_table[lid] = 0;
  pthread_cond_signal(cond_table[lid]);
  pthread_mutex_unlock(&lock);
  r = ret;
  return ret;
}
