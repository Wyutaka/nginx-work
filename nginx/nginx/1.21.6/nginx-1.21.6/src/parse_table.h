//
// Created by miwa on 2020/02/25.
//

#ifndef NEW_MTCP_PARSE_TABLE_H
#define NEW_MTCP_PARSE_TABLE_H
#include <stddef.h>

enum TableType {
  kPeersTable,
  kLocalTable,
  VksTable,
  VtbTable,
  VclTable,
  kOtherTable,
};

#ifdef __cplusplus
extern "C" {
#endif
enum TableType parse_table(unsigned char *query_string, size_t length);
#ifdef __cplusplus
}
#endif
void send_result_peers(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);
void send_result_local(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);
void send_result_virtual_schema_ks(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);
void send_result_virtual_schema_tb(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);
void send_result_virtual_schema_cl(volatile Parser* parser_ctx, struct tcp_stream* cur_stream, short streamID);
#endif // NEW_MTCP_PARSE_TABLE_H

