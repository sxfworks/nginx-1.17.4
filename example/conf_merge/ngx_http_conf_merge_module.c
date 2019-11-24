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
    r->headers_out.content_type = type;
    r->headers_out.content_length_n = 0;

    ngx_url_t u;
    ngx_memset(&u, 0, sizeof(u));
    u.url.data = ngx_palloc(r->pool, response.len + 1);
    u.url.len = response.len + 1;
    ngx_memset(u.url.data, 0, u.url.len);
    ngx_memcpy(u.url.data, response.data, response.len);
    u.default_port = 80;
    if (ngx_parse_url(r->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "url err:%s", u.err);
        }

        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_chain_t *out = NULL;
    ngx_chain_t *pout = out;

    ngx_chain_t *cl = ngx_alloc_chain_link(r->pool);
    cl->buf = ngx_create_temp_buf(r->pool, u.url.len + 2);
    ngx_memcpy(cl->buf->pos, u.url.data, u.url.len);
    cl->buf->last = cl->buf->pos + u.url.len;
    ngx_memcpy(cl->buf->last, "\r\n", 2);
    cl->buf->last += 2;

    out = cl;
    pout = cl;
    pout->next = NULL;

    u_char *tmp_buf = ngx_palloc(r->pool, 1024);
    ngx_memset(tmp_buf, 0, 1024);
    ngx_sprintf(tmp_buf, "default_port:%d\r\nnaddrs:%d\r\n", u.default_port, u.naddrs);
 
    cl = ngx_alloc_chain_link(r->pool);
    cl->buf = ngx_create_temp_buf(r->pool, ngx_strlen(tmp_buf));
    if (!cl->buf) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "alloc nil buf");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(cl->buf->pos, tmp_buf, ngx_strlen(tmp_buf));
    cl->buf->last = cl->buf->pos + ngx_strlen(tmp_buf);

    pout->next = cl;
    pout = cl;
    pout->next = NULL;
   
    ngx_uint_t i = 0;
    for (i = 0; i < u.naddrs && i < 5; i++) {
        ngx_addr_t pa = u.addrs[i];
        
        cl = ngx_alloc_chain_link(r->pool);
        cl->buf = ngx_create_temp_buf(r->pool, pa.name.len + 2);
        ngx_memcpy(cl->buf->pos, pa.name.data, pa.name.len);
        cl->buf->last = cl->buf->pos + pa.name.len;
        ngx_memcpy(cl->buf->last, "\r\n", 2);
        cl->buf->last += 2;
        
        pout->next = cl;
        pout = cl;
        pout->next = NULL;
    }

    if (pout && pout->buf) {
        pout->buf->last_buf = 1;
    }
    
    for (pout = out; pout; pout = pout->next) {
        ngx_str_t tmp;
        tmp.data = ngx_palloc(r->pool, pout->buf->last - pout->buf->pos + 1);
        ngx_memzero(tmp.data, pout->buf->last - pout->buf->pos + 1);
        tmp.len = pout->buf->last - pout->buf->pos;
        ngx_memcpy(tmp.data, pout->buf->pos, tmp.len);
        r->headers_out.content_length_n += tmp.len;
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "content-length [%d] data [%s] len [%d]", r->headers_out.content_length_n, tmp.data, tmp.len);
    }

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    return ngx_http_output_filter(r, out);
}


