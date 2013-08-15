/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_defines.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#include "binheap.h"
#include "arq_conn.h"
#include "arq_ds.h"
#include <time.h>


#ifndef _ARQ_DEFINES_H_
#define _ARQ_DEFINES_H_
#include "arq_ds.h"
extern ARQ_globals globals;


extern bin_heap_node_t *insert_dl_timer(dl_connection_t *conn,timer_type_t tt, int bsn, struct timespec delta, struct timeval curr_time, bin_heap_node_t **new_node);

# define timercmp_ts(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_nsec CMP (b)->tv_nsec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))
# define timeradd_ts(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;			      \
    if ((result)->tv_nsec >= 1000000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_nsec -= 1000000000;					      \
      }									      \
  } while (0)
# define timersub_ts(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;			      \
    if ((result)->tv_nsec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_nsec += 1000000000;					      \
    }									      \
  } while (0)


# define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
        (ts)->tv_sec = (tv)->tv_sec;                                    \
        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
# define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
        (tv)->tv_sec = (ts)->tv_sec;                                    \
        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}

#define set_itimer_spec(its, tv) 	{ 												\
	(its).it_interval.tv_sec = (its).it_interval.tv_nsec = 0; 						\
	(its).it_value.tv_sec = tv.tv_sec; (its).it_value.tv_nsec = tv.tv_usec*1000L; 	\
}

	
#define mod(a,b) ((((a) >= 0) || (-(a) == (b))) 	? (a)%(b) \
													: (-((a)/(b)-1)*(b)+(a)))

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

#define arq_ul_mac_mgmt_thread	(globals.ul_mac_mgmt_thread)
#define arq_ul_mac_mgmt_msg_q 	(globals.ul_mac_mgmt_msg_q)
#define arq_dl_timers			(globals.dl_timers)
#define arq_ul_timers			(globals.ul_timers)

#endif //_ARQ_DEFINES_H_
