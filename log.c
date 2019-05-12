#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "log.h"
#include <sys/types.h>
#include <unistd.h>


static int gs_i_log_level = LOG_Debug;
static int gs_i_log_type = LOGTYPE_SYSLOG;
static FILE *log_file_stream = NULL;
static char * log_filename = "/data/music_microservices/log/grpc_grateway.log";

int g_logsvctype = 0;
pid_t g_pid = 0;

static const char msg_type[4][9] = { "error", "warning", "info", "debug" };
static int syslog_type[4] = { 3, 4, 6, 7 };  // error, warning, info, debug in syslog



void simple_log( int i_loglevel, const char *psz_format, ... )
{
    if ( i_loglevel > gs_i_log_level )
        return;

    va_list args;
	FILE *stream = stderr;
	if ( gs_i_log_type == LOGTYPE_SYSLOG )
	{
		if ( log_file_stream == NULL )
			log_file_stream = fopen( log_filename, "a+" );
		if ( log_file_stream )
			stream = log_file_stream;
		g_pid = getpid();
	}

    va_start( args, psz_format );

    time_t now = time(NULL);
    char nowstr[32] = {0};
    strftime( nowstr, 32, "%Y-%m-%d %H:%M:%S", localtime(&now) );
    flockfile( stream );
    fprintf( stream, "[%s][%d] %s: ", nowstr, g_pid, msg_type[i_loglevel] );
    vfprintf( stream, psz_format, args );
    fprintf( stream, "\n" );
    fflush( stream );
    funlockfile( stream );

    va_end( args );
    
}

