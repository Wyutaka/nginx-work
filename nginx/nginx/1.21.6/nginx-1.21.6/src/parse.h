#pragma once

// #include <mtcp_api.h>
//#include "../transaction/include/proxy/types/detail/aggregator_types.h"
//#include <proxy/types/detail/aggregator_types.h>
//#include "../transaction/include/proxy/types/detail/aggregator.hpp"
#include <proxy/types/detail/aggregator_types.h>

#ifdef __cplusplus
#define IN_PROXY(name) proxy::name
#else
#define IN_PROXY(name) name
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct {
  mctx_t mctx;
  int listener;

  struct {
    int n_cl_recv;
    int n_cl_send;
    int n_ag_recv;
    int n_ag_send;
    int n_cl2ag_drop;
    int n_ag2cl_drop;
    int n_cl_drop;
  } stat;

} Parser;

typedef struct {
  unsigned char version;
  unsigned char flags;
  short streamID;
  unsigned char opcode;
  long length;
} cql_header;




void print_buffer(uchar* buf,int len);

uint process_header(uchar* buffer, int len);
int process_body(uchar* buffer, int len);
uchar* process_query(uchar* buf,int len, int* key_len, int *type);
uint query_length(uchar* buf, int len);

void parse_key_value(char *body, char *key, char *value, int *key_len, int *value_len);
void parse_id(char *body, char *id, int *id_len);

void return_ready_query(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);

void* init_parser(mctx_t mctx);
void destroy_parser(void* arg);
int print_cql(char* buffer, int len, IN_PROXY(Aggregator)* aggr);

void print_parser_stat();

#ifdef __cplusplus
}
#endif

#undef IN_PROXY

