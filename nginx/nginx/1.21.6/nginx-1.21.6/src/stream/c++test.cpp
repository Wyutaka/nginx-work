#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>
#include <fstream>
#include <string>
#include "c++test.h"

int bridge_transaction_layer(ngx_event_t *ev) 
{
  // ngx_connection_t *c = static_cast<ngx_connection_t*>(ev->data);

  // ngx_stream_session_t *s = static_cast<ngx_stream_session_t*>(c->data);
  // ngx_log_debug0(NGX_LOG_DEBUG_STREAM, s->connection->log, 0, "\n\n\n Hello c++!! \n\n\n");
        // ngx_log_debug0(NGX_LOG_DEBUG_STREAM, c->log, 0,
        //             "proxy_ready... \n");
  return 5;
}