#ifndef __FILES_HANDLER_H
#define __FILES_HANDLER_H

#ifdef __cplusplus
extern "C"
{
#endif



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "json-c/json.h"



#define BUFFERSIZE 10240

typedef char * (* pf_read_arg) (void *ptr, char *key);

typedef struct post_msg_t
{
	void *r;
	int seq;
	int fd;
}post_msg;



int file_get_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output);
struct json_object * parse_response( void *response);
int file_post_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output);

int audios_get_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output);
int audios_upload_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output);



#ifdef __cplusplus
}
#endif




#endif



