#ifndef __HANDLER_H
#define __HANDLER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include "fastcgi_manager.h"
#include "json-c/json.h"
#include "hashmap.h"


typedef struct handler_funcs_t
{
	int stream_data;
	size_t (* pf_read_post_data) (void *ptr, size_t size, size_t nmemb, void *data);
}handler_funcs;


typedef struct post_msg_t
{
	Request *r;
	int seq;
	int fd;
	char *func_name;
}post_msg;

typedef char * (* pf_read_arg) (void *ptr, char *key);
typedef int (* pf_request_to_protobuf) (post_msg *msg, pf_read_arg read_msg, void *output);
typedef struct json_object * (* pf_parse_response)(void *response);






//void handlers_init();
int system_handle(Request *r);



#ifdef __cplusplus
}
#endif

#endif
