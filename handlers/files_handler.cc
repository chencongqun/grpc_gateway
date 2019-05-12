#include "files_handler.h"
#include "files.pb.h"


using namespace files;

struct json_object * parse_response( void *response)
{
	struct json_object *p_obj = json_object_new_object();

	FilesInfo fileinfo_ret;
	fileinfo_ret.ParseFromString((char *)response);

	json_object_object_add( p_obj, "file_id", json_object_new_string(fileinfo_ret.file_id().c_str()) );
    json_object_object_add( p_obj, "url", json_object_new_string(fileinfo_ret.url().c_str()) );

	return p_obj;
}


int file_get_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output)
{
	if ( msg->seq > 0 )
		return 0;
	GetRequest request;
	if(read_msg(msg->r, "biz_name")==NULL || read_msg(msg->r, "file_id")==NULL)
		return 0;
	request.set_biz_name(read_msg(msg->r,"biz_name"));
	request.set_file_id(read_msg(msg->r, "file_id"));
	request.SerializeWithCachedSizesToArray((::google::protobuf::uint8 *)output);
	return request.ByteSize();
}


int file_post_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output)
{
	PostRequest request;
	char buffer[BUFFERSIZE];
	if(read_msg(msg->r, "biz_name")==NULL || read_msg(msg->r, "file")==NULL)
	{
		return 0;
	}
	request.set_biz_name(read_msg(msg->r,"biz_name"));
	request.set_filename(rindex(read_msg(msg->r, "file"), '/')+1);
	request.set_seq(msg->seq);
	
	if ( msg->fd <= 0 )
		msg->fd = open(read_msg(msg->r,"file"), O_RDONLY);

	int read_bytes = read(msg->fd, buffer, BUFFERSIZE);
	if (read_bytes==0)
		return 0;

	request.set_data(buffer, read_bytes);
	request.set_length(read_bytes);

	request.SerializeWithCachedSizesToArray((::google::protobuf::uint8 *)output);
	
	return request.ByteSize();
	
}


int audios_get_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output)
{
	if ( msg->seq > 0 )
		return 0;
	KsongAudiosGetRequest request;
	if(read_msg(msg->r, "file_id")==NULL)
		return 0;
	request.set_file_id(read_msg(msg->r, "file_id"));
	request.SerializeWithCachedSizesToArray((::google::protobuf::uint8 *)output);
	return request.ByteSize();
}


int audios_upload_request_to_protobuf(post_msg *msg, pf_read_arg read_msg, void *output)
{
	KsongAudiosUploadRequest request;
	char buffer[BUFFERSIZE];
	if(read_msg(msg->r, "song_id")==NULL || read_msg(msg->r, "file")==NULL)
	{
		return 0;
	}
	request.set_song_id(atoi(read_msg(msg->r,"song_id")));
	request.set_filename(rindex(read_msg(msg->r, "file"), '/')+1);
	
	if ( msg->fd <= 0 )
		msg->fd = open(read_msg(msg->r,"file"), O_RDONLY);

	int read_bytes = read(msg->fd, buffer, BUFFERSIZE);
	if (read_bytes==0)
		return 0;

	request.set_data(buffer, read_bytes);

	request.SerializeWithCachedSizesToArray((::google::protobuf::uint8 *)output);
	
	return request.ByteSize();
	
}




