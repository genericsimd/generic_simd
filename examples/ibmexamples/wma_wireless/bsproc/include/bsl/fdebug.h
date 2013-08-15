#ifndef _FDEBUG_H_INC_
#define _FDEBUG_H_INC_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define WRITE_RAW_INFO(fmt, args...) \
	do { fprintf(stdout, "[%d]"fmt"\n", getpid(), ##args); fflush(stdout); } while(0)
#define WRITE_INDENT_INFO(fmt, args...) \
	do { fprintf(stdout, "\t[%d]"fmt"\n", getpid(), ##args); fflush(stdout); } while(0)
#define WRITE_DEBUG_INFO(fmt, args...) \
	do { fprintf(stdout, "[%d]%s:\t "fmt"\n", getpid(), __FUNCTION__, ##args); fflush(stdout); } while(0)
#define WRITE_ERROR_INFO(fmt, args...) \
	do { fprintf(stderr, "[%d]%s:\t "fmt": %s\n", getpid(), __FUNCTION__, ##args, strerror(errno)); fflush(stderr); } while(0)

#define DEBUG_LEVEL_NONE                    0
#define DEBUG_LEVEL_FATAL                   1
#define DEBUG_LEVEL_SYSTEM                  2
#define DEBUG_LEVEL_USER                    4

#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL     DEBUG_LEVEL_FATAL
#endif

#if DEBUG_LEVEL > DEBUG_LEVEL_NONE
#define FERROR(fmt, args...)    WRITE_ERROR_INFO(fmt, ##args)
#else
#define FERROR(fmt, args...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_FATAL
#define FFATAL(fmt, args...)    WRITE_DEBUG_INFO(fmt, ##args)
#else
#define FFATAL(fmt, args...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_SYSTEM
#define FSYSTEM(fmt, args...)    WRITE_RAW_INFO(fmt, ##args)
#else
#define FSYSTEM(fmt, args...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_USER
#define FUSER(fmt, args...)    WRITE_DEBUG_INFO(fmt, ##args)
#else
#define FUSER(fmt, args...)
#endif

#ifndef DEBUG_FILE
#define DEBUG_FILE		"/tmp/wnt.log"
#endif

inline static int daemonize()
{
	pid_t pid, sid;

	/* already a daemon */
	if ( getppid() == 1 ) return 0;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
	{
		FERROR("Fork:");
		exit(-1);
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		exit(0);
	}

	/* At this point we are executing as the child process */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0)
	{
		FERROR("Set session id");
		exit(-1);
	}

/*
	if ((chdir("/")) < 0)
	{
		FERROR("Chdir");
		exit(-1);
	}
*/

	FSYSTEM("Start as daemon, log to %s", DEBUG_FILE);

	if (freopen(DEBUG_FILE, "a+", stdout) == NULL)
	{
		FERROR("reopen stdout");
	}

	if (freopen(DEBUG_FILE, "a+", stderr) == NULL)
	{
		FERROR("reopen stderr");
	}

	if (freopen(DEBUG_FILE, "r", stdin) == NULL)
	{
		FERROR("reopen stdin");
	}

	return 0;
}

inline static void show_buffer(char *buf, int total, int line)
{
	int i = 0;
	while (i < total)
	{
		i ++;
		printf("%02x%c", (unsigned char)(buf[i - 1]), (i % line) ? ' ':'\n');
	}

	if (i % line)
	{
		printf("\n");
	}
}

#endif
