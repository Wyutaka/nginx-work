
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>


// char ascii[128][3] = {
// 	"00 ", "01 ", "02 ", "03 ", "04 ", "05 ", "06 ", "07 ", "08 ", "09 ", "0A ", "0B ", "0C ", "0D ", "0E ", "0F ", "10 ",
// 	"11 ", "12 ", "13 ", "14 ", "15 ", "16 ", "17 ", "18 ", "19 ", "1A ", "1B ", "1C ", "1D ", "1E ", "1F ", "20 ", "21 ",
// 	"22 ", "23 ", "24 ", "25 ", "26 ", "27 ", "28 ", "29 ", "2A ", "2B ", "2C ", "2D ", "2E ", "2F ", "30 ", "31 ", "32 ",
// 	"33 ", "34 ", "35 ", "36 ", "37 ", "38 ", "39 ", "3A ", "3B ", "3C ", "3D ", "3E ", "3F ", "40 ", "41 ", "42 ", "43 ",
// 	"44 ", "45 ", "46 ", "47 ", "48 ", "49 ", "4A ", "4B ", "4C ", "4D ", "4E ", "4F ", "50 ", "51 ", "52 ", "53 ", "54 ",
// 	"55 ", "56 ", "57 ", "58 ", "59 ", "5A ", "5B ", "5C ", "5D ", "5E ", "5F ", "60 ", "61 ", "62 ", "63 ", "64 ", "65 ",
// 	"66 ", "67 ", "68 ", "69 ", "6A ", "6B ", "6C ", "6D ", "6E ", "6F ", "70 ", "71 ", "72 ", "73 ", "74 ", "75 ", "76 ",
// 	"77 ", "78 ", "79 ", "7A ", "7B ", "7C ", "7D ", "7E ", "7F "};

// void dumpHex(const void *data, ngx_stream_session_t *s, size_t size)
// {
// 	int ls = size / 16;

// 	size_t i, j, k, l;
// 	for (l = 0; l < ls; l++)
// 	{
// 		char rdmsg[68];
// 		int sdmsg = 0; 
// 		int allnull = 0;
// 		for (i = 0; i < 16; ++i)
// 		{
// 			strncpy(rdmsg + sdmsg, ascii[((unsigned char *)data)[i + (l * 16)]], sizeof(ascii[0]));
// 			sdmsg += sizeof(ascii[0]);

// 			if ((i + 1) % 8 == 0)
// 			{
// 				strncpy(rdmsg + sdmsg, " ", 1);
// 				sdmsg += 1;
// 			}

// 			if (i + 1 == 16)
// 			{
// 				char *str = (char *)calloc(16, sizeof(char));
// 				strncpy(str, data + (l * 16), 16);
// 				for (k = 0; k < 16; k++)
// 				{
// 					if (((unsigned char *)data)[k + (l * 16)] <= 0x20 && ((unsigned char *)data)[k + (l * 16)] >= 0x128)
// 					{
// 						str[k] = 0x20;
// 					}
// 					else
// 					{
// 						allnull = 1;
// 						str[k] = ((unsigned char *)data)[k + (l * 16)];
// 					}
// 				}
// 				strncpy(rdmsg + sdmsg, "|  ", 3);
// 				sdmsg += 3;
// 				strncpy(rdmsg + sdmsg, str, 16);
// 				sdmsg += 16;
// 				// strncpy(rdmsg + sdmsg, " \n", 2);
// 				// sdmsg += 1;
// 			}
// 		}
// 		if (allnull != 0) 
//          ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log, 0,
//                            "packets | %s", rdmsg);
// 	}

// 	if (size % 16 > 0)
// 	{	
// 		char rdmsg[68];
// 		int sdmsg = 0; // sdmsg : dmsgの参照する位置
// 		int allnull = 0;
// 		for (i = 0; i < size % 16; i++)
// 		{
// 			strncpy(rdmsg + sdmsg, ascii[((unsigned char *)data)[i + (l * 16)]], sizeof(ascii[0]));
// 			sdmsg += sizeof(ascii[0]);

// 			if ((i + 1) % 8 == 0)
// 			{
// 				strncpy(rdmsg + sdmsg, " ", 1);
// 				sdmsg += 1;
// 			}
// 			if (i + 1 == size % 16)
// 			{
// 				// あまりの実装
// 				if (size % 16 <= 8)
// 				{
// 					strncpy(rdmsg + sdmsg, "  ", 2);
// 					sdmsg += 2;
// 				}
// 				for (j = (i + 1) % 16; j < 16; ++j)
// 				{
// 					strncpy(rdmsg + sdmsg, "   ", 3);
// 					sdmsg += 3;
// 				}
				
// 				char *str = (char *)calloc(size % 16, sizeof(char));
// 				strncpy(str, data + (l * 16), size % 16);
// 				for (k = 0; k < size % 16; k++)
// 				{
// 					if (((unsigned char *)data)[k + (l * 16)] <= 0x20 && ((unsigned char *)data)[k + (l * 16)] >= 0x128)
// 					{
// 						str[k] = 0x20;
// 					}
// 					else
// 					{
// 						allnull = 1;
// 						str[k] = ((unsigned char *)data)[k + (l * 16)];
// 					}
// 				}
// 				strncpy(rdmsg + sdmsg, "|  ", 3);
// 				sdmsg += 3;
// 				strncpy(rdmsg + sdmsg, str, size % 16);
// 				sdmsg += size % 16;
// 				strncpy(rdmsg + sdmsg, " \0", 2);
// 				sdmsg += 2;
// 			}
// 		}
// 		if (allnull != 0) ngx_log_debug1(NGX_LOG_DEBUG_STREAM, s->connection->log, 0,
//                            "packets | %s", rdmsg);
//     }
// }



typedef struct {
    ngx_chain_t  *from_upstream;
    ngx_chain_t  *from_downstream;
} ngx_stream_write_filter_ctx_t;


static ngx_int_t ngx_stream_write_filter(ngx_stream_session_t *s,
    ngx_chain_t *in, ngx_uint_t from_upstream);
static ngx_int_t ngx_stream_write_filter_init(ngx_conf_t *cf);


static ngx_stream_module_t  ngx_stream_write_filter_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_stream_write_filter_init,          /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL                                   /* merge server configuration */
};


ngx_module_t  ngx_stream_write_filter_module = {
    NGX_MODULE_V1,
    &ngx_stream_write_filter_module_ctx,   /* module context */
    NULL,                                  /* module directives */
    NGX_STREAM_MODULE,                     /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_stream_write_filter(ngx_stream_session_t *s, ngx_chain_t *in,
    ngx_uint_t from_upstream)
{
    off_t                           size;
    ngx_uint_t                      last, flush, sync;
    ngx_chain_t                    *cl, *ln, **ll, **out, *chain;
    ngx_connection_t               *c;
    ngx_stream_write_filter_ctx_t  *ctx;

    ctx = ngx_stream_get_module_ctx(s, ngx_stream_write_filter_module);

    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->connection->pool,
                          sizeof(ngx_stream_write_filter_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_stream_set_ctx(s, ctx, ngx_stream_write_filter_module);
    }

    if (from_upstream) {
        c = s->connection;
        out = &ctx->from_upstream;

    } else {
        c = s->upstream->peer.connection;
        out = &ctx->from_downstream;
    }

    if (c->error) {
        return NGX_ERROR;
    }

    size = 0;
    flush = 0;
    sync = 0;
    last = 0;
    ll = out;

    /* find the size, the flush point and the last link of the saved chain */

    for (cl = *out; cl; cl = cl->next) {
        ll = &cl->next;

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "write old buf t:%d f:%d %p, pos %p, size: %z "
                       "file: %O, size: %O",
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);

        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        if (ngx_buf_size(cl->buf) < 0) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "negative size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        size += ngx_buf_size(cl->buf);

        if (cl->buf->flush || cl->buf->recycled) {
            flush = 1;
        }

        if (cl->buf->sync) {
            sync = 1;
        }

        if (cl->buf->last_buf) {
            last = 1;
        }
    }

    /* add the new chain to the existent one */

    for (ln = in; ln; ln = ln->next) {
        cl = ngx_alloc_chain_link(c->pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

        cl->buf = ln->buf;
        *ll = cl;
        ll = &cl->next;

        // ngx_log_debug7(NGX_LOG_DEBUG_EVENT, c->log, 0,
        //                "write new buf t:%d f:%d %p, pos %p, size: %z "
        //                "file: %O, size: %O",
        //                cl->buf->temporary, cl->buf->in_file,
        //                cl->buf->start, cl->buf->pos,
        //                cl->buf->last - cl->buf->pos,
        //                cl->buf->file_pos,
        //                cl->buf->file_last - cl->buf->file_pos);

        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        if (ngx_buf_size(cl->buf) < 0) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "negative size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }

        size += ngx_buf_size(cl->buf);

        if (cl->buf->flush || cl->buf->recycled) {
            flush = 1;
        }

        if (cl->buf->sync) {
            sync = 1;
        }

        if (cl->buf->last_buf) {
            last = 1;
        }
    }

    *ll = NULL;


    
    // ngx_log_debug1(NGX_LOG_INFO, c->log, 0, "buf size is: %u\n\n", cl->buf);
    // dumpHex(cl->buf->last, s, 100);
    // ngx_log_debug0(NGX_LOG_INFO, c->log, 0, "\n\n");

    // ngx_log_debug3(NGX_LOG_DEBUG_STREAM, c->log, 0,
    //                "stream write filter: l:%ui f:%ui s:%O", last, flush, size);

    if (size == 0
        && !(c->buffered & NGX_LOWLEVEL_BUFFERED)
        && !(last && c->need_last_buf)
        && !(c->type == SOCK_DGRAM && flush))
    {
        if (last || flush || sync) {
            for (cl = *out; cl; /* void */) {
                ln = cl;
                cl = cl->next;
                ngx_free_chain(c->pool, ln);
            }

            *out = NULL;
            c->buffered &= ~NGX_STREAM_WRITE_BUFFERED;

            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                      "the stream output chain is empty");

        ngx_debug_point();

        return NGX_ERROR;
    }

    chain = c->send_chain(c, *out, 0);

    // ngx_log_debug1(NGX_LOG_DEBUG_STREAM, c->log, 0,
    //                "stream write filter %p", chain);

    if (chain == NGX_CHAIN_ERROR) {
        c->error = 1;
        return NGX_ERROR;
    }

    for (cl = *out; cl && cl != chain; /* void */) {
        ln = cl;
        cl = cl->next;
        ngx_free_chain(c->pool, ln);
    }

    *out = chain;

    if (chain) {
        if (c->shared) {
            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "shared connection is busy");
            return NGX_ERROR;
        }

        c->buffered |= NGX_STREAM_WRITE_BUFFERED;
        return NGX_AGAIN;
    }

    c->buffered &= ~NGX_STREAM_WRITE_BUFFERED;

    if (c->buffered & NGX_LOWLEVEL_BUFFERED) {
        return NGX_AGAIN;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_stream_write_filter_init(ngx_conf_t *cf)
{
    ngx_stream_top_filter = ngx_stream_write_filter;

    return NGX_OK;
}
