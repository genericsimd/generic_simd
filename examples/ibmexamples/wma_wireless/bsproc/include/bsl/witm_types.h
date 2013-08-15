/**********************************************************
 * @file witm_types.h
 *      Defines types and global constants of wit-man
 *
 *  Change History:
 *  Date            Description
 *  -----------     ---------------
 *  04-Mar. 10      Initial create
 */

#ifndef _WIT_MANAGEMENT_H_INC_
#define _WIT_MANAGEMENT_H_INC_

#include <sys/types.h>
#include <stdint.h>

#ifdef __MINGW32__
#include <windows.h>
#include <tchar.h>
#endif

/* Constants for String Length */
#define WITM_STRLEN_HUGE		1024
#define WITM_STRLEN_LONG		256
#define WITM_STRLEN_SHORT		16

#ifdef __MINGW32__
#define WITM_AUTH_ALL           (0)
#else
#define WITM_AUTH_ALL           (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

/* xxx.xxx.xxx.xxx */
typedef uint8_t ipaddress_t[15];
/* Terminal, 32 bytes */
typedef uint8_t commterm_t[32];

/* Terminal, 32 bytes */
typedef uint8_t connection_t[32];

struct datetime
{
    int32_t tm_sec;         /* seconds */
    int32_t tm_min;         /* minutes */
    int32_t tm_hour;        /* hours */
    int32_t tm_mday;        /* day of the month */
    int32_t tm_mon;         /* month */
    int32_t tm_year;        /* year */
    int32_t tm_wday;        /* day of the week */
    int32_t tm_yday;        /* day in the year */
    int32_t tm_isdst;       /* daylight saving time */
};

#ifdef __cplusplus
extern C {
#endif

#ifdef __cplusplus
}
#endif

#endif
