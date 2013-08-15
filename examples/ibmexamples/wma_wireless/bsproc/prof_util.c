#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define __USE_GNU
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <limits.h>

#include "prof_util.h"

#if defined(ENABLE_PROF) 

int msr_fd ;
static prof_t g_prof __attribute__ ((aligned (128)));

static void prof_decre_init()
{
  int i;

  for (i = 0; i < PROFILE_TOKEN_MAX; ++i)
  {
    g_prof.records[i].count = 0;
    g_prof.records[i].start_time = 0;
    g_prof.records[i].max_time = 0;
    g_prof.records[i].min_time = ULONG_MAX;
    g_prof.records[i].total_time = 0;
  }

}
#ifndef VSXOPT
static int rdtsc_eventsel( unsigned int n, unsigned char event, unsigned char umask, int start )
{
	pmc_evtsel_t reg;   
	int ret;

	reg.fld.event_select= event;
	reg.fld.umask = umask;
	reg.fld.usr = 1;
	reg.fld.os = 0;
	reg.fld.e = 0;
	reg.fld.pc = 0;
	reg.fld.apic_int = 0;
	reg.fld.reserved1 = 0;
	reg.fld.en = start;
	reg.fld.inv = 0;
	reg.fld.cmask = 0;


	ret = pwrite( msr_fd, &reg.u64, sizeof(uint64_t), MSR_IA32_PERFEVTSEL(n)); 

	if( ret == 0 )
	{
		fprintf(stderr, "event select cycle: no data are written.\n");
	}
	else if( ret != sizeof(reg)  || ret < 0)
	{
		perror("event select cycle:");
	} 

	return ret;

}

int rdtsc_init( unsigned int cpu )
{
  
  char msr_file_name[64];

  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  msr_fd = open(msr_file_name, O_RDWR);
  if ( msr_fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", cpu);
      return -2;
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", cpu);
      return -3;
    } else {
      perror("rdmsr:open");
      return -127;
    }
  }

  rdtsc_eventsel( MSC_COUNTER1, MSC_EVENT_CYCLE, 0, 1 );
  rdtsc_eventsel( MSC_COUNTER2, MSC_EVENT_INSTR, 0, 1 );
  prof_decre_init();

  return 0;
}

int rdtsc_deinit(  )
{

  close(msr_fd);
  return 0;
}
#else

int rdtsc_init( unsigned int cpu )
{
  return 0;
}

int rdtsc_deinit(  )
{

  return 0;
}
#endif

void prof_start_timer(int token)
{
  g_prof.records[token].count++;
  g_prof.records[token].start_time = rdtsc_ticks();
}

void prof_stop_timer(int token)
{
  unsigned long long elapse;

  elapse = rdtsc_ticks() - g_prof.records[token].start_time;

  if (elapse < g_prof.records[token].min_time)
    g_prof.records[token].min_time = elapse;
  if (elapse > g_prof.records[token].max_time)
    g_prof.records[token].max_time = elapse;

  g_prof.records[token].total_time += elapse;
}


void prof_start_counter(int n, int token)
{
  g_prof.records[token].count++;
  g_prof.records[token].start_time = rdtsc_counts( n );
}

void prof_stop_counter(int n, int token)
{
  unsigned long long elapse;

  elapse = rdtsc_counts( n ) - g_prof.records[token].start_time;

  if (elapse < g_prof.records[token].min_time)
    g_prof.records[token].min_time = elapse;
  if (elapse > g_prof.records[token].max_time)
    g_prof.records[token].max_time = elapse;

  g_prof.records[token].total_time += elapse;
}


void dump_single_prof()
{
  prof_t *prof_ptr =  &g_prof;

  unsigned int i = 0;
  const char *token_str[] = {
    PROF_TIMER_TOKEN_STR( PHY_UL_NO_DEQUE ),
    PROF_TIMER_TOKEN_STR( PHY_UL_PREPROC ),
    PROF_TIMER_TOKEN_STR( PHY_UL_BURST_DEQUE ),
    PROF_TIMER_TOKEN_STR( PHY_UL_PWR_ADJUST ),
    PROF_TIMER_TOKEN_STR( PHY_UL_OFDM_DEMOD ),
    PROF_TIMER_TOKEN_STR( PHY_UL_SUBCAR_DEMOD ),
    PROF_TIMER_TOKEN_STR( PHY_UL_CHAN_EST ),
    PROF_TIMER_TOKEN_STR( PHY_UL_DEMAPPING ),
    PROF_TIMER_TOKEN_STR( PHY_UL_BURST_FORMING ),
    PROF_TIMER_TOKEN_STR( PHY_UL_DEMOD ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_DECODE ),
    PROF_TIMER_TOKEN_STR( PHY_UL_OTHER ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_DEINTER ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_CLIP ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_VIT ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_DERAN ),
    PROF_TIMER_TOKEN_STR( PHY_UL_FEC_OTHER ),
    PROF_TIMER_TOKEN_STR( PHY_UL_VITERBI ),
    PROF_TIMER_TOKEN_STR( PHY_UL_VITERBI_C ),
    PROF_TIMER_TOKEN_STR( PHY_UL_VITERBI_I ),
    PROF_TIMER_TOKEN_STR( PHY_UL_OFDM_DEMOD_IFFT ),
    PROF_TIMER_TOKEN_STR( PHY_DL ),
    PROF_TIMER_TOKEN_STR( PHY_DL_SYM1 ),
    PROF_TIMER_TOKEN_STR( PHY_DL_FECENCODE ),
    PROF_TIMER_TOKEN_STR( PHY_DL_ZONEPERM ),
    PROF_TIMER_TOKEN_STR( PHY_DL_OTHER ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING_ZPERM ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING_IR ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING_PR ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING_IR_IFFT ),
    PROF_TIMER_TOKEN_STR( PHY_RANGING_PR_IFFT )
  };

  printf("PROF DATA=================>\n");
  printf("     Count\t     Total\t       Max\t       Min\t    Average\t(Token)\n");

  for (i = 0; i < sizeof(token_str)/sizeof(const char *); ++i)
  {
    double avg = 0;
    unsigned long long min = 0;

    if(prof_ptr->records[i].count != 0) {
      avg = (double)prof_ptr->records[i].total_time 
            / (double)prof_ptr->records[i].count;
      min = prof_ptr->records[i].min_time;
    }

    printf("%10llu\t%10llu\t%10llu\t%10llu\t%11.2lf\t(%s)\n",
          prof_ptr->records[i].count, 
          prof_ptr->records[i].total_time, 
          prof_ptr->records[i].max_time, 
          min, 
          avg,
          token_str[i]); 

  }
}

int bind_self( int n )
{
#if 1
	int s, j;
	cpu_set_t cpuset;
	pthread_t thread;

	thread = pthread_self();

	CPU_ZERO(&cpuset);
	CPU_SET(n, &cpuset);

	s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
	if (s != 0)
		printf("pthread_setaffinity_np %d\n", s);

	usleep(10);

	/* Check the actual affinity mask assigned to the thread */

	s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
	if (s != 0)
		printf("pthread_getaffinity_np %d\n", s);

	printf("Set returned by pthread_getaffinity_np() contained:\n");
	for (j = 0; j < CPU_SETSIZE; j++)
		if (CPU_ISSET(j, &cpuset))
			printf("    CPU %d\n", j);
#endif
	return 0;

}


#endif

//#define TEST
#ifdef TEST

int main( int argc, char **argv )
{
	int i;

	PROF_INIT( 0 );

	for( i = 0; i < 5; i++ ) {
		PROF_START_TIMER( PROF_TIMER_TOKEN( MAC ));

		PROF_START_TIMER( PROF_TIMER_TOKEN( PHY ));
		//rdtsc_ticks();
		PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY ));

		sleep(5);
		PROF_STOP_TIMER( PROF_TIMER_TOKEN( MAC ));
	}

	PROF_TIMER_DUMP;
	PROF_DEINIT;

	return 0;

}

#endif
