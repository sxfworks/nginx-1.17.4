#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static ngx_int_t
ngx_http_conf_merge_handler(ngx_http_request_t *r);

static void* 
create_loc_ngx_merge_conf(ngx_conf_t *cf);

static char* 
merge_loc_ngx_merge_conf(ngx_conf_t *cf, void *prev, void *conf);

static ngx_int_t
ngx_http_merge_conf_init(ngx_conf_t *cf);

typedef struct {
    ngx_str_t conf_merge;
} ngx_conf_merge_t;

static ngx_command_t  ngx_http_conf_merge_commands[] = {
    {
        ngx_string("conf_merge"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t  ngx_http_conf_merge_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_http_merge_conf_init,           /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    create_loc_ngx_merge_conf,          /* create location configuration */
    merge_loc_ngx_merge_conf,            /* merge location configuration */
};

ngx_module_t  ngx_http_conf_merge_module = {
    NGX_MODULE_V1,
    &ngx_http_conf_merge_module_ctx,    /* module context */
    ngx_http_conf_merge_commands,       /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void* 
create_loc_ngx_merge_conf(ngx_conf_t *cf) {
    ngx_conf_merge_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_conf_merge_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

static char* 
merge_loc_ngx_merge_conf(ngx_conf_t *cf, void *prev, void *conf) {
    ngx_conf_merge_t  *pc = prev, *cc = conf;

    if (pc && pc->conf_merge.data) {
        cc->conf_merge = pc->conf_merge;
    }

    if (cc && cc->conf_merge.data) {
        ngx_conf_log_error(NGX_LOG_ERR, cf, ngx_errno, "conf data:%s, len:%d!", cc->conf_merge.data, cc->conf_merge.len);
    } else {
        ngx_conf_log_error(NGX_LOG_ERR, cf, ngx_errno, "cc conf is null", cc->conf_merge.data, cc->conf_merge.len);
    }

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_merge_conf_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_conf_merge_handler;

    return NGX_OK;
}

static ngx_int_t
ngx_http_conf_merge_handler(ngx_http_request_t *r)
{
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_conf_merge_t  *conf = ngx_http_get_module_loc_conf(r, ngx_http_conf_merge_module);

    ngx_str_t type = ngx_string("text/plain");
    ngx_str_t response = conf->conf_merge;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = response.len;
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    ngx_buf_t *b;
    b = ngx_create_temp_buf(r->pool, response.len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(b->pos, response.data, response.len);
    b->last = b->pos + response.len;
    b->last_buf = 1;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;
    return ngx_http_output_filter(r, &out);
}


