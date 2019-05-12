#include "fastcgi_manager.h"
#include "log.h"
#include <unistd.h>

static char x2c(char *what)
{
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void unescape_url(char *url)
{
    register int x,y;

    for (x=0,y=0; url[y]; ++x,++y)
    {
        if((url[x] = url[y]) == '%')
        {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
        else if(url[x]=='+')
        {
            url[x]=' ';
        }
    }
    url[x] = '\0';
}

static int nextline(Request *r,char s[], int lim)
{
    int c=0, i=0, num;

    for (i=0; (i<lim) && ((c=fcgi_getchar(r))!=EOF) && (c!='\n'); i++)
    {
        s[i] = c;
    }
    if (c == '\n')
    {
        s[i] = c;
    }
    if ((i==0) && (c!='\n'))
        num = 0;
    else if (i == lim)
        num = i;
    else
        num = i+1;
    return num;
}

static char *lower_case( char *buffer )
{
    char *tempstr = buffer;

    while (*buffer != '\0')
    {
        if ( isupper(*buffer) )
            *buffer = tolower(*buffer);
        buffer++;
    }
    return tempstr;
}


static int parse_cgi_argument(hashmap args_map, const char *buffer)
{
    if (buffer==NULL || args_map == NULL )
        return -1;
    int i=0, j=0, num=0, token=_NAME;
    int len = (int)strlen(buffer);
    char *lexeme = (char *)malloc(sizeof(char) * len + 1);
    char *name = NULL;
    char *value = NULL;
    while (i < len)
    {
        j = 0;
        while ( (buffer[i] != '=') && (buffer[i] != '&') && (i < len) )
        {
            lexeme[j] = (buffer[i] == '+') ? ' ' : buffer[i];
            i++;
            j++;
        }
        lexeme[j] = '\0';
        if (token == _NAME)
        {
            name = strdup(lexeme);
            unescape_url(name);
            if ( (buffer[i] != '=') || (i == len - 1) )
            {
                value = (char *)malloc(sizeof(char));
                strcpy(value,"");
                hashmap_put(args_map, MAKE_STRING_KEY(name), value);
                name = NULL;
                value = NULL;
                if (i == len - 1) /* null value at end of expression */
                    num++;
                else   /* error in expression */
                {
                    free(lexeme);
                    return -1;
                }
            }
            else
                token = _VALUE;
        }
        else
        {
            value = strdup(lexeme);
            unescape_url(value);
            hashmap_put(args_map, MAKE_STRING_KEY(name), value);
            name = NULL;
            value = NULL;
            token = _NAME;
            num++;
        }
        i++;
        j = 0;
    }
    free(lexeme);
    return num;
}

static int parse_form_encoded(Request *r, hashmap args_map, hashmap files_path)
{
    char *uploadfname, *tempstr, *boundary;
    char *buffer = (char *)malloc(sizeof(char) * BUFSIZE + 1);
    char *prevbuf = (char *)malloc(sizeof(char) * BUFSIZE + 1);
    int i,j,bytes_read, pre_bytes_read, buffer_size, start, isfile, num_forms=0;
    char *name;
    char *value;
    FILE *uploadfile = NULL;

    if ( !CONTENT_LENGTH(r) )
        return 0;

    boundary = strstr(CONTENT_TYPE(r),"boundary=");
    boundary += (sizeof(char) * 9);
    char *psz_ua = (char *)HTTP_USER_AGENT(r);

    nextline(r, buffer, BUFSIZE);

    while ((bytes_read=nextline( r, buffer, BUFSIZE)) != 0)
    {
        /* this assumes that buffer contains no binary characters.
                if the buffer contains the first valid header, then this
                is a safe assumption.  however, it should be improved for
                robustness sake. */
        buffer[bytes_read] = '\0';
        tempstr = buffer;
        tempstr += (sizeof(char) * 38); /* 38 is header up to name */
        name = strdup(tempstr);
        tempstr = name;
        while (*tempstr != '"')
            tempstr++;
        *tempstr = '\0';

        if (strstr(buffer,"filename=\"") != NULL)
        {
            isfile = 1;

            tempstr = buffer;
            tempstr = strstr(tempstr,"filename=\"");
            tempstr += (sizeof(char) * 10);
            value = strdup(tempstr);
            tempstr = value;
            while (*tempstr != '"')
                tempstr++;
            *tempstr = '\0';

            /* Netscape's Windows browsers handle paths differently from its
                        UNIX and Mac browsers.  It delivers a full path for the uploaded
                     file (which it shouldn't do), and it uses backslashes rather than
                        forward slashes.  No need to worry about Internet Explorer, since
                        it doesn't support HTTP File Upload at all. */
            if (psz_ua && strstr(lower_case(psz_ua),"win") != 0)
            {
                tempstr = strrchr(value, '\\');
                if (tempstr)
                {
                    tempstr++;
                    value = tempstr;
                }
                else
                {
                    tempstr = strrchr(value, '/');
                    if (tempstr)
                    {
                        tempstr++;
                        value = tempstr;
                    }
                }
            }
            uploadfname = (char *)malloc(strlen(UPLOADDIR)+strlen(value)+10);
            sprintf(uploadfname,"%s/%d_%s", UPLOADDIR, getpid(), value);
            if ( (uploadfile = fopen(uploadfname,"w")) == NULL)
            {
                /* null filename; for now, just don't save info.  later, save to default file */
                isfile = 0;
				free(uploadfname);
            }
			else
				hashmap_put( files_path, MAKE_STRING_KEY(name), uploadfname );
			hashmap_put( args_map, MAKE_STRING_KEY(name), uploadfname );
			
            num_forms++;
        }
        else
            isfile = 0;

        /* ignore rest of headers and first blank line */
        while (nextline(r,buffer, BUFSIZE) > 1)
        {
            /* DOS style blank line? */
            if ((buffer[0] == '\r') && (buffer[1] == '\n'))
                break;
        }
        int done = 0;
        j = 0;
        start = 1;
        value = (char *)malloc(sizeof(char) * BUFSIZE + 1);
        buffer_size = BUFSIZE;
        strcpy(value,"");
        while (!done)
        {
            bytes_read = nextline( r, buffer, BUFSIZE );
            buffer[bytes_read] = '\0';
            if (bytes_read && strstr(buffer, boundary) == NULL)
            {
                if (start)
                {
                    i = 0;
                    while (i < bytes_read)
                    {
                        prevbuf[i] = buffer[i];
                        i++;
                    }
                    pre_bytes_read = bytes_read;
                    start = 0;
                }
                else
                {
                    /* flush buffer */
                    i = 0;
                    while (i < pre_bytes_read)
                    {
                        if (isfile)
                            fputc(prevbuf[i],uploadfile);
                        else
                        {
                            if (j > buffer_size)
                            {
                                buffer_size += BUFSIZE;
                                value = (char *) realloc(value, sizeof(char) * buffer_size+1);
                            }
                            value[j] = prevbuf[i];
                            j++;
                        }
                        i++;
                    }
                    /* buffer new input */
                    i = 0;
                    while (i < bytes_read)
                    {
                        prevbuf[i] = buffer[i];
                        i++;
                    }
                    pre_bytes_read = bytes_read;
                }
            }
            else
            {
                done = 1;
                /* flush buffer except last two characters */
                i = 0;
                while (i < pre_bytes_read - 2)
                {
                    if (isfile)
                        fputc(prevbuf[i],uploadfile);
                    else
                    {
                        if (j > buffer_size)
                        {
                            buffer_size += BUFSIZE;
                            value = (char *) realloc(value, sizeof(char) * buffer_size+1);
                        }
                        value[j] = prevbuf[i];
                        j++;
                    }
                    i++;
                }
            }
        }
        if (isfile)
            fclose(uploadfile);
        else
        {
            value[j] = '\0';
            hashmap_put( args_map, MAKE_STRING_KEY(name), value );
            num_forms++;
            j = 0;
        }
    }

}

static int parse_cookies( Request * r, hashmap cookies_map )
{
    int i=0, j=0, len=0, num_cookies=0;
    short NM = 1;

    const char *cookies = HTTP_COOKIE(r);
    if (cookies == NULL)
        return 0;
    len = (int)strlen(cookies);
    char *name = (char *)malloc(sizeof(char) * len + 1);
    char *value = (char *)malloc(sizeof(char) * len + 1);

    for (i = 0; i < len; i++)
    {
        if (cookies[i] == '=')
        {
            name[j] = '\0';
            if (i == len - 1)
            {
                strcpy(value,"");
                hashmap_put( cookies_map, MAKE_STRING_KEY(name), value );
                num_cookies++;
            }
            j = 0;
            NM = 0;
        }
        else if ( (cookies[i] == '&') || (i == len - 1) )
        {
            if (!NM)
            {
                if (i == len - 1)
                {
                    value[j] = cookies[i];
                    j++;
                }
                value[j] = '\0';
                hashmap_put( cookies_map, MAKE_STRING_KEY(name), value );
                num_cookies++;
                j = 0;
                NM = 1;
            }
        }
        else if ( (cookies[i] == ';') || (i == len - 1) )
        {
            if (!NM)
            {
                if (i == len - 1)
                {
                    value[j] = cookies[i];
                    j++;
                }
                value[j] = '\0';
                hashmap_put( cookies_map, MAKE_STRING_KEY(name), value );
                num_cookies++;
                i++;   /* erases trailing space */
                j = 0;
                NM = 1;
            }
        }
        else if (NM)
        {
            name[j] = cookies[i];
            j++;
        }
        else if (!NM)
        {
            value[j] = cookies[i];
            j++;
        }
    }
    return num_cookies;
}

static char *read_post_data(Request * r)
{
    unsigned int content_length;
    char *buffer = NULL;

    if (CONTENT_LENGTH(r) != NULL)
    {
        if (atoi(CONTENT_LENGTH(r)) < 0)
        {
            fprintf(stderr,"caught by cgihtml: CONTENT_LENGTH < 0\n");
            return NULL;
        }

        content_length = atoi(CONTENT_LENGTH(r));
        buffer = (char *)malloc(sizeof(char) * content_length + 1);

        if (fcgi_fread(r, buffer, content_length) != content_length)
        {
            /* consistency error. */
            fprintf(stderr,"caught by cgihtml: input length < CONTENT_LENGTH\n");
            return NULL;
        }
        buffer[content_length] = '\0';
    }
    return buffer;
}

void fcgi_request_init(Request * r)
{
    r->get_arg = hashmap_create(100);
    r->post_arg = hashmap_create(100);
    r->cookie_arg = hashmap_create(100);
	r->files_path = hashmap_create(100);
	
    // read and parse get arguments
    parse_cgi_argument(r->get_arg, QUERY_STRING(r));

    // read and parse post arguments
    if ((CONTENT_TYPE(r) != NULL) && (strstr(CONTENT_TYPE(r),"multipart/form-data") != NULL))
        parse_form_encoded(r, r->post_arg, r->files_path);
    else
        parse_cgi_argument( r->post_arg, read_post_data(r) );

    parse_cookies( r, r->cookie_arg );

	r->header = 0;
}

void fcgi_request_free(Request * r)
{
    hashmap_free(r->get_arg);
    hashmap_free(r->post_arg);
    hashmap_free(r->cookie_arg);

	hashmap_iterator it = hashmap_iterate(r->files_path);

	hashmap_iterator* next = NULL;

	while( (next = hashmap_next(&it))!=NULL )
	{
		SLOG_INFO("free file: %s", next->p_val);
		unlink(next->p_val);
	}

	hashmap_free(r->files_path);
}


