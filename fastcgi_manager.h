#ifndef __FASTCGI_MANAGER_H
#define __FASTCGI_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include "fcgiapp.h"


#define _NAME 0
#define _VALUE 1
#define BUFSIZE 1024
#define UPLOADDIR "/data/music_microservices/tmp/"


typedef struct _request
{
    FCGX_Request *frequest;
    hashmap get_arg;
    hashmap post_arg;
    hashmap cookie_arg;
	hashmap files_path;
	int header;           // whether header has been sent
} Request;


#define QUERY_STRING(r) fcgi_getenv(r,"QUERY_STRING")    // get parameter
#define REQUEST_METHOD(r) fcgi_getenv(r,"REQUEST_METHOD")
#define SCRIPT_NAME(r) fcgi_getenv(r,"SCRIPT_NAME")    //url
#define CONTENT_TYPE(r) fcgi_getenv(r,"CONTENT_TYPE")
#define CONTENT_LENGTH(r) fcgi_getenv(r,"CONTENT_LENGTH")
#define HTTP_USER_AGENT(r) fcgi_getenv(r,"HTTP_USER_AGENT")
#define HTTP_COOKIE(r) fcgi_getenv(r,"HTTP_COOKIE")

#define fcgi_printf(r,fmt,args...) do{ if(r->frequest!=NULL) FCGX_FPrintF(r->frequest->out,fmt,##args);\
                         else FCGI_printf(fmt,##args); }while(0)


int fcgi_fread(Request *r, char *str,int n);
int fcgi_write( const char *buf, size_t len,Request *r );
const char *fcgi_getenv(Request *r,const char *name);


void fcgi_request_init(Request * r);
void fcgi_request_free(Request * r);


char *fcgi_var( Request *r, const char *psz_name );
char* fcgi_cookievar(Request *r,char *psz_name);


void fcgi_header(Request *r);




#ifdef __cplusplus
}
#endif

#endif


