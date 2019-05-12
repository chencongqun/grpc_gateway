#include "handler_manager.h"
#include "fastcgi_manager.h"
#include "request.h"
#include <dlfcn.h>
#include "log.h"

char * read_arg(void * ptr, char *key)
{
	SLOG_INFO("read_arg: %s", key);
	return fcgi_var((Request * )ptr, key);

}

void * get_function(char *func_name)
{
	char *path = "/data/music_microservices/files.so";
	void *dlhandler = dlopen(path, RTLD_LAZY);
    if (dlhandler == NULL) {
        SLOG_ERROR("dlopen %s failed:%s\n", func_name, dlerror());
		return NULL;
    }

    void *func = dlsym(dlhandler,func_name);
    if (NULL == func) {
        SLOG_ERROR("mod_handler not defined ignored\n");
        dlclose(dlhandler);
		return NULL;
    }

	return func;
}


static size_t post_read_callback(void *output, size_t size, size_t nmemb, void *instream)
{
	post_msg * msg = (post_msg *)instream;
	char *buffer = (char *)output;

	SLOG_INFO("get data: %s", msg->func_name );

	pf_request_to_protobuf func = (pf_request_to_protobuf)get_function(msg->func_name);
	if(func==NULL)
		return 0;
	int bytes = func(msg, read_arg, (void *)(buffer+5));

	
	msg->seq++;

	if (bytes==0)
	{
		if ( msg->fd > 0 )
			close(msg->fd);
		return 0;
	}
	SLOG_INFO("bytes: %d",bytes);

	buffer[0]=0;
	buffer[1]=(bytes>>24) % 0x100;
	buffer[2]=(bytes>>16) % 0x100;
	buffer[3]=(bytes>>8) % 0x100;
	buffer[4]=bytes % 0x100;
	
	return bytes+5;
}


#define HOST_IP "127.0.0.1"
#define HOST_PORT 50051

int system_handle(Request *r)
{
	int ret = -1;
	int post = 0;
	char result[1024] = {0};
	char url[256]={0};
	sprintf(url, "http://%s:%d/%s", HOST_IP, HOST_PORT, SCRIPT_NAME(r)+10);
	SLOG_INFO("grpc url: %s", url);

	post_msg msg;
	msg.r = r;
	msg.fd = 0;
	msg.seq = 0;

	//pf_post_data_interator read_request = NULL;
	pf_parse_response parse_func = (pf_parse_response)get_function("parse_response");
	if (parse_func == NULL )
	{
		SLOG_INFO("open parse_response failed");
		return 0;
	}
	
	char * method = rindex(SCRIPT_NAME(r), '/');

	SLOG_INFO("method: %s", method);


	if ( strncmp(method, "/getinfo", 7 )==0 )
	{
		msg.func_name = "audios_get_request_to_protobuf";
		char buffer[1024]={0};
		size_t length = post_read_callback((void *)buffer, 0, 0, (void *)&msg);
		ret = curl_content(url, (void *)buffer, length, result, 1024, 0, NULL);
	}
	else if ( strncmp(method, "/upload", 6 )==0 )
	{
		msg.func_name = "audios_upload_request_to_protobuf";
		ret = curl_content(url, (void *)&msg, sizeof(msg), result, 1024, 1, post_read_callback);
	}

	struct json_object * ret_obj = NULL;

	if ( ret == 0 )
		ret_obj = parse_func((void *)(result+5));
	else
		ret_obj = json_object_new_object();
	
	json_object_object_add( ret_obj, "ret", json_object_new_int(0) );
    json_object_object_add( ret_obj, "ret_msg", json_object_new_string(""));
	
	const char *psz_obj = json_object_to_json_string( ret_obj );
	fcgi_header( r );
    fcgi_write( psz_obj, strlen( psz_obj ), r );

	json_object_put( ret_obj );
	
	return 0;
}


