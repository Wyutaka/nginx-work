#include <stdio.h>
#include <unordered_map>
#include "ring_queue.h"

template<typename K,typename V>
using hashmap = std::unordered_map<K,V>;

hashmap<unsigned int,struct rte_ring*> g_queues;


//Aggr側で作成するときに使用
/*
 * rte_ring_*系の関数はrte_eal_initが呼ばれた後でないとセグメント例外が発生する
 * mtcpではmtcp_init内でrte_eal_initが呼ばれるので，mtcp_initの後でこの関数を呼び出す必要がある
 */
struct rte_ring* create_queue(unsigned int addr){
  char name[256];

  sprintf(name,"queue%x",addr);
  struct rte_ring* ring = rte_ring_lookup(name);
  
  if(ring != NULL){
    fprintf(stderr,"error: create_queue failed, the queue already exists\n");
    return NULL;
  }
  int ring_size = 16384;
  int flags = 0;
  ring = rte_ring_create(name, ring_size, rte_socket_id(), flags);
  fprintf(stderr,"hoge: create_ring_queue\n");
  g_queues.emplace(addr,ring);
  return ring;
}

void destroy_queue(){
  for(auto& it:g_queues){
    rte_ring_free(it.second);
  }
}


//整数化したip_addrからキューを取得
struct rte_ring* get_queue(unsigned int addr){
  try{
  return g_queues.at(addr);
  }catch(std::out_of_range& e){
    fprintf(stderr,"out of range: %8x\n",addr);
  }
}

struct rte_ring* search_queue_by_name(const char *name){
  return rte_ring_lookup(name);
}
