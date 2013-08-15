/* ----------------------------------------------------------------------------
 *    IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011
    
   All Rights Reserved.

   File Name: flog.h

   Change Activity:
    
   Date             Description of Change                            By
   -----------      ---------------------                            --------
   

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _FLOG_H_
#define _FLOG_H_

#include <syslog.h>


#define LOG_MAX_LEN     1024	

void flog_init( const char *ident, int to_console, int to_syslog );
void flog_deinit( void );
void flog_err(const char *title, unsigned int pri, const char *file_name, const char *func_name, char *fmt, ...);
void flog_info(const char *title, unsigned int pri, const char *file_name, const char *func_name, char *fmt, ...);
void flog_debug(const char *title, unsigned int pri, const char *file_name, int line_num, const char *func_name, char *fmt, ...);

#define LOG_LEVEL_FATAL                   1
#define LOG_LEVEL_ERROR                   2
#define LOG_LEVEL_WARN                    3
#define LOG_LEVEL_INFO                    4
#define LOG_LEVEL_DEBUG                   5 

#ifndef LOG_LEVEL
    #define LOG_LEVEL    LOG_LEVEL_WARN 
#endif

#define LOG_INIT_SYSLOG_ONLY( ident )		flog_init( ident, 0, 1);
#define LOG_INIT_CONSOLE_ONLY( ident )		flog_init( ident, 1, 0);
#define LOG_INIT_CONSOLE_SYSLOG( ident )	flog_init( ident, 1, 1);

#define LOG_DEINIT				flog_deinit();

#if LOG_LEVEL >= LOG_LEVEL_FATAL
#define FLOG_FATAL(fmt, args...)    flog_err("FATAL", LOG_CRIT, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_FATAL(fmt, args...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define FLOG_ERROR(fmt, args...)    flog_err("ERROR", LOG_ERR, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_ERROR(fmt, args...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define FLOG_WARNING(fmt, args...)    flog_info("WARNING", LOG_WARNING, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_WARNING(fmt, args...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define FLOG_INFO(fmt, args...)    flog_info("INFO", LOG_INFO, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_INFO(fmt, args...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define FLOG_DEBUG(fmt, args...)    flog_debug("DEBUG", LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_DEBUG(fmt, args...)
#endif


#endif
