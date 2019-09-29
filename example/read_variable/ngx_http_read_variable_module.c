#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *
ngx_http_read_variable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
ngx_http_read_variable_handler(ngx_http_request_t *r);

static ngx_command_t  ngx_http_read_variable_commands[] = {
    {
        ngx_string("read_variable"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_read_variable,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t  ngx_http_read_variable_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                  		        /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,       			            /* create location configuration */
    NULL         			            /* merge location configuration */
};

ngx_module_t  ngx_http_read_variable_module = {
    NGX_MODULE_V1,
    &ngx_http_read_variable_module_ctx,    /* module context */
    ngx_http_read_variable_commands,       /* module directives */
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

static char *
ngx_http_read_variable(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t  *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    if (clcf) {
        clcf->handler = ngx_http_read_variable_handler;
        ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno, "clcf %s is not null", clcf->name.data);
    } else {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno, "clcf %s is null");
    }
    

    if (conf) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno, "cmd name:%s, conf:%d, conf not null", cmd->name.data, cmd->conf);
    } else {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno, "cmd name:%s, conf:%d, conf null", cmd->name.data, cmd->conf);
    }
    
    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_read_variable_handler(ngx_http_request_t *r)
{
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_t type = ngx_string("text/plain");
    ngx_str_t response = ngx_string("Hello World!");

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


