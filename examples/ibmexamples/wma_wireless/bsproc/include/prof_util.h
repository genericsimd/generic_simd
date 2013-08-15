#ifndef __PROF_UTIL_H__
#define __PROF_UTIL_H__

#ifdef ENABLE_PROF 

//#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <inttypes.h>

#define REG_TSC  0x10

#define MSC_EVENT_CYCLE  0x3c
#define MSC_EVENT_INSTR  0xc0

#define MSC_COUNTER1  0
#define MSC_COUNTER2  1

#define MSC_CPU  1

#define MSR_IA32_PMC(n)		(0x0c1 + (n))
#define MSR_IA32_PERFEVTSEL(n)	(0x186 + (n))

typedef union {
    struct {
	uint64_t	event_select	: 8;
	uint64_t	umask		: 8;
	uint64_t	usr		: 1;
	uint64_t	os		: 1;
	uint64_t	e		: 1;
	uint64_t	pc		: 1;
	uint64_t	apic_int	: 1;
	uint64_t	reserved1	: 1;
	uint64_t	en		: 1;
	uint64_t	inv		: 1;
	uint64_t	cmask		: 8;
     }		fld;
     uint64_t	u64;
} pmc_evtsel_t;
#define PMC_EVTSEL_ZERO	{ .u64 = 0ULL }



extern int msr_fd;

int rdtsc_init( unsigned int cpu );
int rdtsc_deinit( void );

#ifndef VSXOPT
static inline uint64_t rdtsc_ticks()
{
  uint64_t data;
  pread( msr_fd, &data, sizeof data, REG_TSC); 
  return data;
}

static inline uint64_t rdtsc_counts( int n )
{
  uint64_t data;
  pread( msr_fd, &data, sizeof data, MSR_IA32_PMC(n)); 
  return data;
}
#else
static inline uint64_t rdtsc_ticks()
{
    unsigned long long val;
    __asm__ __volatile__(
        "mftb %[val]"
        : [val] "=r" (val));
    return val;
}   

static inline uint64_t rdtsc_counts( int n )
{
	return 0;
}

#endif

#define PROF_TIMER_TOKEN(s) PROF_TIMER_##s
#define PROF_TIMER_TOKEN_STR(s) #s


// profile timer token
typedef enum _PROF_TIMER_TOKEN_T_
{
    PROF_TIMER_TOKEN( PHY_UL_NO_DEQUE ) = 0,
    PROF_TIMER_TOKEN( PHY_UL_PREPROC ),
    PROF_TIMER_TOKEN( PHY_UL_BURST_DEQUE ),
    PROF_TIMER_TOKEN( PHY_UL_PWR_ADJUST ),
    PROF_TIMER_TOKEN( PHY_UL_OFDM_DEMOD ),
    PROF_TIMER_TOKEN( PHY_UL_SUBCAR_DEMOD ),
    PROF_TIMER_TOKEN( PHY_UL_CHAN_EST ),
    PROF_TIMER_TOKEN( PHY_UL_DEMAPPING ),
    PROF_TIMER_TOKEN( PHY_UL_BURST_FORMING ),
    PROF_TIMER_TOKEN( PHY_UL_DEMOD ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_DECODE ),
    PROF_TIMER_TOKEN( PHY_UL_OTHER ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_DEINTER ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_CLIP ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_VIT ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_DERAN ),
    PROF_TIMER_TOKEN( PHY_UL_FEC_OTHER ),
    PROF_TIMER_TOKEN( PHY_UL_VITERBI ),
    PROF_TIMER_TOKEN( PHY_UL_VITERBI_C ),
    PROF_TIMER_TOKEN( PHY_UL_VITERBI_I ),
    PROF_TIMER_TOKEN( PHY_UL_OFDM_DEMOD_IFFT ),
    PROF_TIMER_TOKEN( PHY_DL ),
    PROF_TIMER_TOKEN( PHY_DL_SYM1 ),
    PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ),
    PROF_TIMER_TOKEN( PHY_DL_ZONEPERM ),
    PROF_TIMER_TOKEN( PHY_DL_OTHER ),
    PROF_TIMER_TOKEN( PHY_RANGING ),
    PROF_TIMER_TOKEN( PHY_RANGING_ZPERM ),
    PROF_TIMER_TOKEN( PHY_RANGING_IR ),
    PROF_TIMER_TOKEN( PHY_RANGING_PR ),
    PROF_TIMER_TOKEN( PHY_RANGING_IR_IFFT ),
    PROF_TIMER_TOKEN( PHY_RANGING_PR_IFFT ),
    PROFILE_TOKEN_MAX

} PROF_TIMER_TOKEN_T;

typedef struct prof_record
{
  unsigned long long count;
  unsigned long long max_time;
  unsigned long long min_time;
  unsigned long long total_time;
  unsigned long long start_time;
} prof_record_t;

typedef struct prof
{
  prof_record_t records[PROFILE_TOKEN_MAX];
} prof_t;

void prof_start_timer(int token);
void prof_stop_timer(int token);
void prof_start_counter(int n, int token);
void prof_stop_counter(int n, int token);
void dump_single_prof();
int bind_self( int n );

#ifdef  PROF_TIMER_ENABLE
#define PROF_START_TIMER(token) prof_start_timer(token)
#define PROF_STOP_TIMER(token)  prof_stop_timer(token)
#else
#define PROF_START_TIMER(token)
#define PROF_STOP_TIMER(token)
#endif

#ifdef  PROF_COUNTER_ENABLE
#define PROF_START_COUNTER(n, token) prof_start_counter(n, token)
#define PROF_STOP_COUNTER(n, token)  prof_stop_counter(n, token)
#else
#define PROF_START_COUNTER(n, token)
#define PROF_STOP_COUNTER(n, token)
#endif

#define PROF_INIT( cpu )  	rdtsc_init( cpu )
#define PROF_TIMER_DUMP	        dump_single_prof()
#define PROF_DEINIT  		rdtsc_deinit()
#define PROF_BIND_SELF( n )     bind_self( n )

#else
#define PROF_INIT( cpu )
#define PROF_TIMER_DUMP
#define PROF_DEINIT
#define PROF_START_COUNTER(n, token)
#define PROF_STOP_COUNTER(n, token)
#define PROF_START_TIMER(token)
#define PROF_STOP_TIMER(token)

#define PROF_BIND_SELF( n )

#endif //PROF_ENABLE

#endif //__PROF_UTIL_H__
