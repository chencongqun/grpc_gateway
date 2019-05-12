#include <stdio.h>
#include <stdlib.h>
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include "fastcgi_manager.h"
#include "json-c/json.h"
#include "fcgiapp.h"
#include "log.h"
#include "handler_manager.h"



static void json_print_status( Request *p_r, int i_code, const char *psz_msg )
{
    struct json_object *p_obj = json_object_new_object();
    json_object_object_add( p_obj, "code", json_object_new_int( i_code ) );
    json_object_object_add( p_obj, "message", json_object_new_string( psz_msg));
    const char *psz_obj = json_object_to_json_string( p_obj );
    fcgi_header( p_r );
    fcgi_write( psz_obj, strlen( psz_obj ), p_r );
    json_object_put( p_obj );
}



int main(int argc, char ** argv)
{
	Request r;
	FCGX_Init();
    while (1)
    {
    	FCGX_Request *xRequest = (FCGX_Request *)malloc(sizeof(FCGX_Request));
		FCGX_InitRequest(xRequest, 0, 0);
		if ( FCGX_Accept_r(xRequest) <0 )
			continue;
		
		r.frequest = xRequest;
		fcgi_request_init(&r);

		system_handle(&r);
		//json_print_status(&r, 101, "sucess");

		//SLOG_INFO(fcgi_var(&r, "file"));
		//fcgi_write( fcgi_var(&r, "file"), strlen(fcgi_var(&r, "file")), &r );
		
		fcgi_request_free(&r);
		FCGX_Finish_r( xRequest );
    }
	return 0;
}





