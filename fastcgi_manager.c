#include "fastcgi_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int fcgi_fread(Request *r, char *str,int n)
{
    if(r->frequest==NULL)
        return fread(str,1,n,stdin);
    else
        return FCGX_GetStr(str,n,r->frequest->in);
}

int fcgi_write( const char *buf, size_t len,Request *r )
{

    if(r->frequest==NULL)
        return fwrite((void*)buf,1,len,stdout);
	else
        return FCGX_PutStr(buf,len, r->frequest->out);
}

const char *fcgi_getenv(Request *r,const char *name)
{
    if(r->frequest==NULL)
        return getenv(name);
    else
        return FCGX_GetParam(name, r->frequest->envp);
}

int fcgi_getchar(void *p_ctx)
{
    Request *r = (Request *)p_ctx;
    return FCGX_GetChar(r->frequest->in);
}


void fcgi_header(Request *r)
{
    if(r->header==0)
    {
        fcgi_printf(r, "%s", "Content-type: text/html\n");
        fcgi_printf(r, "%s", "Cache-Control: no-cache\n");
        fcgi_printf(r, "%s", "Expires: 0\n");
        fcgi_printf(r, "%s", "P3P: CP=CAO PSA OUR\r\n\r\n");
        r->header =1;
    }
}


void fcgi_SetHeader_int64( Request *r, const char *psz_field, int64_t i_val )
{
    char psz_val[128] = {0};
#ifdef __LP64__
    sprintf( psz_val, "%jd", i_val );
#else
    sprintf( psz_val, "%lld", i_val );
#endif
    fcgi_printf( r, "%s: %s\r\n", psz_field, psz_val );
}

void fcgi_SetHeader_uint64( Request *r, const char *psz_field, uint64_t i_val )
{
    char psz_val[128] = {0};
#ifdef __LP64__
    sprintf( psz_val, "%ju", i_val );
#else
    sprintf( psz_val, "%llu", i_val );
#endif
    fcgi_printf( r, "%s: %s\r\n", psz_field, psz_val );
}

void fcgi_EndHeader( Request *r )
{
    fcgi_printf( r, "%s", "\r\n" );
}

void fcgi_status(Request *r,char *status)
{
    if (status)
    {
        fcgi_printf(r, "Status: %s\r\n\r\n",status);
        fcgi_printf(r, "%s", status);
    }
}

char *fcgi_var( Request *r, const char *psz_name )
{
    char *psz_value = NULL;
    psz_value = hashmap_get( r->get_arg, MAKE_STRING_KEY(psz_name));
    if ( !psz_value )
        psz_value = hashmap_get( r->post_arg, MAKE_STRING_KEY(psz_name));
    return psz_value;
}

char* fcgi_cookievar( Request *r,char* psz_name )
{
    return hashmap_get( r->cookie_arg, MAKE_STRING_KEY(psz_name));
}

