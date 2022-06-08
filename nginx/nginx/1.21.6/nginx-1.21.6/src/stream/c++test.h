#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>

#ifndef __CPPSUB_H__
#define __CPPSUB_H__ 1
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  extern int bridge_transaction_layer(ngx_event_t *ev);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CPPSUB_H__ */