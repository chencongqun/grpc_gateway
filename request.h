#ifndef __REQUEST_H
#define __REQUEST_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <stddef.h>
#include "json-c/json.h"


#define CURL_TIMEOUT 5

typedef size_t (* pf_post_data_interator) (void *ptr, size_t size, size_t nmemb, void *data);



int curl_content(char* url,void *post, int postlen, char*result, int len, int multi_chunk, pf_post_data_interator func);


#ifdef __cplusplus
}
#endif

#endif

