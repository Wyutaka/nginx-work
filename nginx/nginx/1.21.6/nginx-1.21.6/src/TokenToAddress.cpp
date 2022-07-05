#include <iostream>
#include <fstream>
#include <string>

#include <algorithm>
#include <vector>
#include <unordered_set>

#include <cassert>

#include <arpa/inet.h>
#include "TokenToAddress.h"

struct TokenRange{
  long start_token;
  long end_token;
  unsigned int n_address;
};


static std::vector<TokenRange> g_vec;
static std::vector<unsigned int> g_addrList;

struct Comparator{
  bool operator() (const TokenRange& lhs,const TokenRange& rhs) const {
  return lhs.start_token < rhs.start_token;
  }
};

int init_token(const char* name){
  //TODO:仮の実装としてファイルから読み込んでいる
  std::ifstream ifs(name);
  if(ifs.fail() == true)return -1;

  TokenRange range;
  char comma;
  std::string address;
  std::unordered_set<unsigned int> g_addrSet;
  while(!ifs.eof()){
    ifs >> range.start_token;
    ifs >> comma;
    ifs >> range.end_token;
    ifs >> comma;
    ifs >> address;

    struct in_addr addr; 
    int ret = inet_aton(address.c_str(), &addr);
    if(ret == 0){
      assert(0);
    }
    range.n_address = addr.s_addr;

    g_vec.push_back(range);
    g_addrSet.insert(range.n_address);
  }


  for(auto& item:g_addrSet){
    union addr_union {
      unsigned int addr;
      unsigned char af[4];
    } au;
    au.addr = item;
#define C(n) (int)au.af[n]
    //printf("----------------%d.%d.%d.%d\n", C(0), C(1), C(2), C(3));
#undef C

    g_addrList.push_back(item);
  }

  std::sort(g_vec.begin(),g_vec.end(), Comparator());
  return 0;
}

unsigned int* get_address_list(int* n){
  *n = g_addrList.size();
  return &g_addrList.at(0);
}

unsigned int token2address(long token){

  TokenRange t;
  t.start_token = token;
  auto& vec = g_vec;
  auto it = std::upper_bound(vec.begin(),vec.end(), t,Comparator());
  /*
    1-10,11-20,21-30の範囲が登録されている時，5が指定されたとき必要なのは1-10である
    <1,1-10>,<11,11-20>,<21,21-20>のように keyをstart,valueをrangeとした時
    upper_bound(5)は 5 > key となる <11,11-20>を返す
    そのため，デクリメントにより一つ前を取得する
  */
  
  if(it == vec.begin()){
    it = vec.end();
    --it;
  }else{
    --it;
  }
  return it->n_address;
}

