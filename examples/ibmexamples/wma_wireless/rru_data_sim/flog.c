/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011
    
   All Rights Reserved.

   File Name: flog.c

   Change Activity:
    
   Date             Description of Change                            By
   -----------      ---------------------                            --------
   

   ---------------------------------------------------------------------------- */


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <pthread.h>
#include "flog.h"

static int print_to_console = 1;
static int print_to_syslog = 1;
static int flog_facility = LOG_SYSLOG;


/* ----------------------------------------------------------------------------
 *
 Function:    flog_init()

 Description: Logging system initialization

 Parameters:
     	ident		[IN] identify of log message, which is printed at the
			head of the whole message
	to_console	[IN] whether turn on console output
	to_syslog	[IN] whether turn on syslog output

 Return Value:
	None
 --------------------------------------------------------------------------- */
void flog_init( const char *ident, int to_console, int to_syslog )
{
	openlog (ident, LOG_NDELAY, flog_facility);

	print_to_console = to_console;
	print_to_syslog = to_syslog;
}

/* ----------------------------------------------------------------------------
 *
 Function:    flog_deinit()

 Description: Logging system de-initialization

 Parameters:
	None

 Return Value:
	None
 --------------------------------------------------------------------------- */
void flog_deinit( )
{
	closelog ();
}

/* ----------------------------------------------------------------------------
 *
 Function:    flog_err()

 Description: log error message
	title	[IN] message title: FATAL, ERROR, WARNING, INFO, DEBUG
	pri	[IN] output priority in syslog: LOG_CRIT, LOG_ERR, LOG_WARN,
		LOG_INFO, LOG_DEBUG
	file_name	[IN] source code file name 
	func_name       [IN] involved routin name
	fmt		[IN] message format string
        ...		[IN] tail variables 
 Parameters:
		
 Return Value:
	None
 --------------------------------------------------------------------------- */
void flog_err(const char *title, unsigned int pri, const char *file_name, const char *func_name, char *fmt, ...) 
{
	va_list args;
	char msg_buf[LOG_MAX_LEN];
	int n;

	if( errno )
	{
		n = snprintf( msg_buf, LOG_MAX_LEN, "[%3lx]%8s:%s:%s(): err_str: %s output_msg: ", 
				(unsigned long int)pthread_self(), title, file_name, func_name, strerror(errno));
		errno = 0; //FIXME: shall we clear it here
	}
	else
	{
		n = snprintf( msg_buf, LOG_MAX_LEN, "[%3lx]%8s:%s:%s(): ", 
				(unsigned long int)pthread_self(), title, file_name, func_name);
	}

	if( n < LOG_MAX_LEN)
	{ 
		va_start(args, fmt);
		vsnprintf( &msg_buf[n], LOG_MAX_LEN-n+1, fmt, args);
		va_end (args);
	}

	/* Print to stderr */
	if ( print_to_console ) {
		fprintf(stderr, "%s", msg_buf);
		fprintf(stderr, "\n");
		fflush(stderr);
	} 

	/* Print to syslog */
	if ( print_to_syslog ) {
		syslog (flog_facility | pri, "%s", msg_buf);
	}

}

/* ----------------------------------------------------------------------------
 *
 Function:    flog_info()

 Description: log normal information message
	title	[IN] message title: FATAL, ERROR, WARNING, INFO, DEBUG
	pri	[IN] output priority in syslog: LOG_CRIT, LOG_ERR, LOG_WARN,
		LOG_INFO, LOG_DEBUG
	file_name	[IN] source code file name 
	func_name       [IN] involved routin name
	fmt		[IN] message format string
        ...		[IN] tail variables 
 Parameters:
		
 Return Value:
	None
 --------------------------------------------------------------------------- */
void flog_info(const char *title, unsigned int pri, const char *file_name, const char *func_name, char *fmt, ...)
{
        va_list args;
        char msg_buf[LOG_MAX_LEN];
        int n;

        n = snprintf( msg_buf, LOG_MAX_LEN, "[%3lx]%8s:%s:%s(): ",
                      (unsigned long int)pthread_self(), title, file_name, func_name);

        if( n < LOG_MAX_LEN)
        {
                va_start(args, fmt);
                vsnprintf( &msg_buf[n], LOG_MAX_LEN-n+1, fmt, args);
                va_end (args);
        }

        /* Print to stderr */
        if ( print_to_console ) {
                fprintf(stdout, "%s", msg_buf);
                fprintf(stdout, "\n");
                fflush(stdout);
        }

        /* Print to syslog */
        if ( print_to_syslog ) {
                syslog (flog_facility | pri, "%s", msg_buf);
        }

}

/* ----------------------------------------------------------------------------
 *
 Function:    flog_debug()

 Description: log debug information message
	title	[IN] message title: FATAL, ERROR, WARNING, INFO, DEBUG
	pri	[IN] output priority in syslog: LOG_CRIT, LOG_ERR, LOG_WARN,
		LOG_INFO, LOG_DEBUG
	file_name	[IN] source code file name 
	line_num	[IN] line number where to log the message 
	func_name       [IN] involved routin name
	fmt		[IN] message format string
        ...		[IN] tail variables 
 Parameters:
		
 Return Value:
	None
 --------------------------------------------------------------------------- */
void flog_debug(const char *title, unsigned int pri, const char *file_name, int line_num, const char *func_name, char *fmt, ...) 
{
	va_list args;
	char msg_buf[LOG_MAX_LEN];
	int n;

	if( errno )
	{
		n = snprintf( msg_buf, LOG_MAX_LEN, "[%3lx]%8s:%s:%s():%d: err_str: %s output_msg: ", 
				(unsigned long int)pthread_self(), title, file_name, func_name, line_num, strerror(errno));
		errno = 0; //FIXME: shall we clear it here
	}
	else
	{
		n = snprintf( msg_buf, LOG_MAX_LEN, "[%3lx]%8s:%s:%s():%d: ", 
				(unsigned long int)pthread_self(), title, file_name, func_name, line_num);
	}

	if( n < LOG_MAX_LEN)
	{ 
		va_start(args, fmt);
		vsnprintf( &msg_buf[n], LOG_MAX_LEN-n+1, fmt, args);
		va_end (args);
	}

	/* Print to stderr */
	if ( print_to_console ) {
		fprintf(stdout, "%s", msg_buf);
		fprintf(stdout, "\n");
		fflush(stdout);
	} 

	/* Print to syslog */
	if ( print_to_syslog ) {
		syslog (flog_facility | pri, "%s", msg_buf);
	}

}


//#define FLOG_TEST
#ifdef FLOG_TEST
int test()
{

	char buf[LOG_MAX_LEN * 2];
        FILE *f;
	int i;

        //init a long message
	for( i = 0; i < LOG_MAX_LEN * 2; i++)
		buf[i] = '0' + i % 10;
        buf[i-1] = '\0';
 
	//no error
	FLOG_ERROR( "========test 1(no err): %s %d============", "hello", 100 );
       
        //error         
        f = fopen("/dev/nofile", "rb");
	FLOG_ERROR( "========test 1(err): %s %d============", "hello", 100 );

        //long message
	FLOG_ERROR( "========test 2: %s ============", buf);
        
	//empty message
	FLOG_ERROR( "" );

        //FATAL
	FLOG_FATAL( "========test 1(no err): %s %d============", "hello", 100 );
       
        
        //WARNING
	FLOG_WARNING( "========test 1(no err): %s %d============", "hello", 100 );
      
 
        //INFO
	FLOG_INFO( "========test 1(no err): %s %d============", "hello", 100 );
      
        //DEBUG
	FLOG_DEBUG( "========test 1(no err): %s %d============", "hello", 100 );
      

	return 0;
	
}


int main( int argc, char *argv[])
{
        LOG_INIT_CONSOLE_SYSLOG( "flog" )
	test();
	LOG_DEINIT

        LOG_INIT_CONSOLE_ONLY( "flog" )
	test();
	LOG_DEINIT

        LOG_INIT_SYSLOG_ONLY( "flog" )
	test();
	LOG_DEINIT

	return 0;
}
#endif
