#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <fstream>
#include <string>
#include "c++test.h"

char* bridge_transaction_layer(ngx_event_t *ev) 
{
  // ngx_connection_t *c = static_cast<ngx_connection_t*>(ev->data);

  // ngx_stream_session_t *s = static_cast<ngx_stream_session_t*>(c->data);
  // ngx_log_debug0(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "\n\n\n Hello c++!! \n\n\n");
        // ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
        //             "proxy_ready... \n");

    std::ofstream writing_file;
    std::string filename = "/home/user1/nginx-work/nginx/nginx/1.21.6/nginx-1.21.6/src/stream/sample.txt";
    writing_file.open(filename, std::ios::out);
    std::string writing_text = "bridge_transaction_layer";
    writing_file << writing_text << std::endl;
    writing_file.close();

    return "bridge_transaction";
}