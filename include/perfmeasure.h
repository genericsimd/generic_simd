/*
 * perfmeasure.h
 *
 *  Created on: Jun 3, 2013
 *      Author: Haichuan Wang
 *
 *  Header file for call linux perf tool to measure HPM
 *  Reference: http://web.eece.maine.edu/~vweaver/projects/perf_events/perf_event_open.html
 */

#ifndef PERFMEASURE_H_
#define PERFMEASURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

typedef struct hpm_fds_t {
  int fd0;
  int fd1;
  int fd2;
  int fd3;
} hpm_fds_t;


void perf_events_create(hpm_fds_t* fds) {
  int i;
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.disabled = 1;
  //pe.exclude_kernel = 1;
  //pe.exclude_idle = 1;
  pe.exclude_hv = 1;

  //instrs
  pe.config = PERF_COUNT_HW_INSTRUCTIONS;
  fds->fd0 = perf_event_open(&pe, 0, -1, -1, 0);
  if (fds->fd0 == -1) {
         fprintf(stderr, "Error opening leader %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
  //cycles
  pe.config = PERF_COUNT_HW_CPU_CYCLES;
  fds->fd1 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd1 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
  //cache ref
  pe.config = PERF_COUNT_HW_CACHE_REFERENCES;
  fds->fd2 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd2 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }

  //cache ref
  pe.config = PERF_COUNT_HW_CACHE_MISSES;
  fds->fd3 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd3 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
}

void perf_events_create2(hpm_fds_t* fds) {
  int i;
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.size = sizeof(struct perf_event_attr);
  pe.disabled = 1;
  //pe.exclude_kernel = 1;
  //pe.exclude_idle = 1;
  pe.exclude_hv = 1;

  //instrs
  pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
  fds->fd0 = perf_event_open(&pe, 0, -1, -1, 0);
  if (fds->fd0 == -1) {
         fprintf(stderr, "Error opening leader %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
  //cycles
  pe.config = PERF_COUNT_HW_BRANCH_MISSES;
  fds->fd1 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd1 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
  //cache ref
  pe.config = PERF_COUNT_HW_INSTRUCTIONS; //PERF_COUNT_HW_STALLED_CYCLES_FRONTEND ;
  fds->fd2 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd2 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }

  //cache ref
  pe.config = PERF_COUNT_HW_CPU_CYCLES; //PERF_COUNT_HW_STALLED_CYCLES_BACKEND ;
  fds->fd3 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
  if (fds->fd3 == -1) {
         fprintf(stderr, "Error opening event %llx\n", pe.config);
         exit(EXIT_FAILURE);
  }
}

void perf_events_start(hpm_fds_t* fds) {
  ioctl(fds->fd0, PERF_EVENT_IOC_RESET, 0);
  ioctl(fds->fd1, PERF_EVENT_IOC_RESET, 0);
  ioctl(fds->fd2, PERF_EVENT_IOC_RESET, 0);
  ioctl(fds->fd3, PERF_EVENT_IOC_RESET, 0);

  ioctl(fds->fd0, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(fds->fd1, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(fds->fd2, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(fds->fd3, PERF_EVENT_IOC_ENABLE, 0);
}


void perf_events_stop_report(hpm_fds_t* fds) {
  long long c0,c1,c2,c3;
  ioctl(fds->fd0, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd0, &c0, sizeof(long long));
  ioctl(fds->fd1, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd1, &c1, sizeof(long long));
  ioctl(fds->fd2, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd2, &c2, sizeof(long long));
  ioctl(fds->fd3, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd3, &c3, sizeof(long long));

  printf("[HPM Perf]instr:%lld; cycles:%lld; cacheref:%lld; cachemiss:%lld\n",
      c0, c1, c2,c3);
}


void perf_events_stop_report2(hpm_fds_t* fds) {
  long long c0,c1,c2,c3;
  ioctl(fds->fd0, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd0, &c0, sizeof(long long));
  ioctl(fds->fd1, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd1, &c1, sizeof(long long));
  ioctl(fds->fd2, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd2, &c2, sizeof(long long));
  ioctl(fds->fd3, PERF_EVENT_IOC_DISABLE, 0);
  read(fds->fd3, &c3, sizeof(long long));

//  printf("[HPM Perf]Branch instrs:%lld; Misbranch instrs:%lld; Frontend Stall:%lld; Backend Stall:%lld\n",
//      c0, c1, c2,c3);
  printf("[HPM Perf]Branch instrs:%lld; Misbranch instrs:%lld; Instrs:%lld; Cycles:%lld\n",
      c0, c1, c2,c3);
}

void perf_events_close(hpm_fds_t* fds) {
  close(fds->fd0);
  close(fds->fd1);
  close(fds->fd2);
  close(fds->fd3);
}

/***** Macro Definition  *****/
#ifdef HPM_PERF
#define HPM_PERF_CREATE hpm_fds_t __hpm_fds; perf_events_create2(&__hpm_fds)
#define HPM_PERF_START perf_events_start(&__hpm_fds)
#define HPM_PERF_STOP perf_events_stop_report2(&__hpm_fds)
#define HPM_PERF_CLOSE perf_events_close(&__hpm_fds);
#else
#define HPM_PERF_CREATE
#define HPM_PERF_START
#define HPM_PERF_STOP
#define HPM_PERF_CLOSE
#endif

#endif /* PERFMEASURE_H_ */
