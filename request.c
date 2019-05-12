#include "request.h"
#include "log.h"

static long writer(void *data, int size, int nmemb, char* content)
{
    int sizes;
    sizes = size *nmemb;

    memcpy(content,data,size * nmemb);
    return sizes;
}


int curl_content(char* url,void *post,int postlen, char*result, int len, int multi_chunk, pf_post_data_interator func)
{

    CURL *curl = NULL;
    struct curl_slist *headers = NULL;
    int ret = -1;

    do
    {
        curl = curl_easy_init();
        if (NULL == curl)
        {
            break;
        }

        //set timeout to 5seconds
        curl_easy_setopt(curl,CURLOPT_TIMEOUT,CURL_TIMEOUT);

        //no ssl verify
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);

        //define output function and buffer
        if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEDATA, result))
        {
            SLOG_ERROR("curl set write buffer failed");
            break;
        }
        if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer))
        {
            SLOG_ERROR("curl set writer function failed");
            break;
        }
        //define max buffer length ,current 1k
        if (CURLE_OK != curl_easy_setopt(curl,CURLOPT_BUFFERSIZE,len))
        {
            SLOG_ERROR("curl set buffer size failed");
            break;
        }

		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
		headers = curl_slist_append(headers, "Grpc-Encoding: identity");
		headers = curl_slist_append(headers, "Grpc-Accept-Encoding: identity,deflate,gzip");
		headers = curl_slist_append(headers, "Te: trailers");
		headers = curl_slist_append(headers, "Content-Type: application/grpc");
		headers = curl_slist_append(headers, "User-Agent: Python-gRPC-1.0.4 grpc-c/1.0.1 (manylinux; chttp2)");
		headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		
        //post mesage
        if (postlen > 0)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 0);
            
            if ( multi_chunk == 1 )
            {
            	curl_easy_setopt(curl, CURLOPT_READDATA, post);
        		curl_easy_setopt(curl, CURLOPT_READFUNCTION, func);
				curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

				
            }
			else
			{
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (char *)post);
            	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postlen);
			}
        }

        //set request url
        if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_URL, url))
        {
            SLOG_ERROR("set curl url failed");
            break;
        }

        //perform request
        if (CURLE_OK != curl_easy_perform(curl))
        {
            SLOG_ERROR("curl perform error");
            break;
        }

        //check retcode if 200 ok
        //不检测200返回值,有可能是一个跳转
        if (CURLE_OK != curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &ret))
        {
            SLOG_ERROR("curl retcode failed %d",ret);
            break;
        }

        ret = 0;
    }
    while(0);

    if (NULL != headers)
    {
        curl_slist_free_all(headers);
    }

    if (NULL != curl)
    {
        curl_easy_cleanup(curl);
    }
    return ret;
}



