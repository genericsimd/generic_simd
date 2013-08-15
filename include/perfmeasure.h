/**
Copyright 2012 the Generic SIMD Intrinsic Library project authors. All rights reserved.

Copyright IBM Corp. 2013, 2013. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
   * Neither the name of IBM Corp. nor the names of its contributors may be
     used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * perfmeasure.h
 *
 *  Created on: Jun 3, 2013
 *  author: Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
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
  int fd[6]; //max 6 events
} hpm_fds_t;

typedef struct hpm_group_t {
  __u32 type;//type of events
  __u32 size;//4 or 6
  __u64 event[6];//max 6 events
  const char* event_name[6];//each event's name
} hpm_group_t;

static hpm_group_t hw_group = { PERF_TYPE_HARDWARE,
                        4,
                        {PERF_COUNT_HW_INSTRUCTIONS,
                         PERF_COUNT_HW_CPU_CYCLES,
                         PERF_COUNT_HW_CACHE_REFERENCES,
                         PERF_COUNT_HW_CACHE_MISSES,
                         0,
                         0},
                        {
                         "Instr","Cycles", "Cache Ref", "Cache Miss", "", ""
                        }
                       };

static hpm_group_t br_group = { PERF_TYPE_HARDWARE,
                        4,
                        {PERF_COUNT_HW_INSTRUCTIONS,
                         PERF_COUNT_HW_CPU_CYCLES,
                         PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
                         PERF_COUNT_HW_BRANCH_MISSES,
                         0,
                         0},
                        {
                         "Instr","Cycles", "Branch Instr", "Branch Miss", "", ""
                        }
                       };


static hpm_group_t sw_group = { PERF_TYPE_SOFTWARE,
                        5, //not support by power
                        {PERF_COUNT_SW_CPU_CLOCK,
                         PERF_COUNT_SW_TASK_CLOCK,
                         PERF_COUNT_SW_PAGE_FAULTS,
                         PERF_COUNT_SW_CONTEXT_SWITCHES,
                         PERF_COUNT_SW_CPU_MIGRATIONS,
                         0, /*PERF_COUNT_SW_ALIGNMENT_FAULTS*/},
                        {
                         "CPU clock","Task clock", "Page fault", "Context switch", "Migration", "Aligment fault"
                        }
                       };

#define CACHE_READ(name) (name | (PERF_COUNT_HW_CACHE_OP_READ << 8) | ( PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define CACHE_READ_MISS(name) (name | (PERF_COUNT_HW_CACHE_OP_READ << 8) | ( PERF_COUNT_HW_CACHE_RESULT_MISS << 16))
#define CACHE_WRITE(name) (name | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | ( PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16))
#define CACHE_WRITE_MISS(name) (name | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | ( PERF_COUNT_HW_CACHE_RESULT_MISS << 16))

static hpm_group_t icache_group = { PERF_TYPE_HW_CACHE,
                        2, //not support by power
                        {
                         CACHE_READ_MISS(PERF_COUNT_HW_CACHE_L1I),
                         CACHE_READ_MISS(PERF_COUNT_HW_CACHE_ITLB),
                         CACHE_READ(PERF_COUNT_HW_CACHE_L1I),
                         CACHE_READ(PERF_COUNT_HW_CACHE_ITLB),
                         0,
                         0, /*PERF_COUNT_SW_ALIGNMENT_FAULTS*/},
                        {
                         "L1I Miss", "ITLB Miss", "L1I Read", "ITLB Read", "", ""
                        }
                       };

static hpm_group_t dcache_group = { PERF_TYPE_HW_CACHE,
                        3, //not support by power
                        {
                            CACHE_READ(PERF_COUNT_HW_CACHE_L1D),
                            CACHE_READ_MISS(PERF_COUNT_HW_CACHE_L1D),
                            CACHE_WRITE_MISS(PERF_COUNT_HW_CACHE_L1D),
                            CACHE_WRITE(PERF_COUNT_HW_CACHE_L1D),
                            CACHE_READ(PERF_COUNT_HW_CACHE_DTLB),
                            CACHE_READ_MISS(PERF_COUNT_HW_CACHE_DTLB),
                         },
                        {
                         "L1D Read", "L1D Read Miss", "L1D Write Miss", "L1D Write", "L1D Write",  "DTLB Ref"
                        }
                       };

static hpm_group_t llc_group = { PERF_TYPE_HW_CACHE,
                        4, //not support by power
                        {
                         CACHE_READ(PERF_COUNT_HW_CACHE_LL),
                         CACHE_READ_MISS(PERF_COUNT_HW_CACHE_LL),
                         CACHE_WRITE(PERF_COUNT_HW_CACHE_LL),
                         CACHE_WRITE_MISS(PERF_COUNT_HW_CACHE_LL),
                         CACHE_WRITE(PERF_COUNT_HW_CACHE_L1D),
                         CACHE_READ(PERF_COUNT_HW_CACHE_DTLB),
                         },
                        {
                         "LLC Read", "LLC Read Miss", "LLC Write", "LLC Write Miss", "L1D Write",  "DTLB Ref"
                        }
                       };


void perf_events_create(hpm_fds_t* fds, hpm_group_t* egroup) {
  int i;
  int size = egroup->size;
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = egroup->type;
  pe.size = sizeof(struct perf_event_attr);
  pe.disabled = 1;
  //pe.exclude_kernel = 1;
  //pe.exclude_idle = 1;
  pe.exclude_hv = 1;

  for(i = 0; i < size; ++i) {
    pe.config = egroup->event[i];
    fds->fd[i] = perf_event_open(&pe, 0, -1, -1, 0);
    if (fds->fd[i] == -1) {
           fprintf(stderr, "Error opening leader %llx, %d, %s\n", pe.config, i, egroup->event_name[i]);
           exit(EXIT_FAILURE);
    }
  }
}

//void perf_events_create2(hpm_fds_t* fds) {
//  int i;
//  struct perf_event_attr pe;
//  memset(&pe, 0, sizeof(struct perf_event_attr));
//  pe.type = PERF_TYPE_HARDWARE;
//  pe.size = sizeof(struct perf_event_attr);
//  pe.disabled = 1;
//  //pe.exclude_kernel = 1;
//  //pe.exclude_idle = 1;
//  pe.exclude_hv = 1;
//
//  //instrs
//  pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
//  fds->fd0 = perf_event_open(&pe, 0, -1, -1, 0);
//  if (fds->fd0 == -1) {
//         fprintf(stderr, "Error opening leader %llx\n", pe.config);
//         exit(EXIT_FAILURE);
//  }
//  //cycles
//  pe.config = PERF_COUNT_HW_BRANCH_MISSES;
//  fds->fd1 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
//  if (fds->fd1 == -1) {
//         fprintf(stderr, "Error opening event %llx\n", pe.config);
//         exit(EXIT_FAILURE);
//  }
//  //cache ref
//  pe.config = PERF_COUNT_HW_INSTRUCTIONS; //PERF_COUNT_HW_STALLED_CYCLES_FRONTEND ;
//  fds->fd2 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
//  if (fds->fd2 == -1) {
//         fprintf(stderr, "Error opening event %llx\n", pe.config);
//         exit(EXIT_FAILURE);
//  }
//
//  //cache ref
//  pe.config = PERF_COUNT_HW_CPU_CYCLES; //PERF_COUNT_HW_STALLED_CYCLES_BACKEND ;
//  fds->fd3 = perf_event_open(&pe, 0, -1, fds->fd0, 0);
//  if (fds->fd3 == -1) {
//         fprintf(stderr, "Error opening event %llx\n", pe.config);
//         exit(EXIT_FAILURE);
//  }
//}

void perf_events_start(hpm_fds_t* fds, hpm_group_t* egroup) {
  int i;
  int size = egroup->size;
  for(i = 0; i < size; ++i) {
    ioctl(fds->fd[i], PERF_EVENT_IOC_RESET, 0);
  }
  for(int i = 0; i < egroup->size; ++i) {
    ioctl(fds->fd[i], PERF_EVENT_IOC_ENABLE, 0);
  }
}


void perf_events_stop_report(hpm_fds_t* fds, hpm_group_t* egroup) {
  int size = egroup->size;
  int i;
  for(i = 0; i < size; ++i) {
    ioctl(fds->fd[i], PERF_EVENT_IOC_DISABLE, 0);
  }
  printf("[HPM Event]");
  //title
  for(i = 0; i < size; ++i) {
    printf("%s,", egroup->event_name[i]);
  }
  printf("\n[HPM Values]");
  long long c;
  for(i = 0; i < size; ++i) {
    read(fds->fd[i], &c, sizeof(long long));
    printf("%lld,", c);
  }
  printf("\n");
}


//void perf_events_stop_report2(hpm_fds_t* fds) {
//  long long c0,c1,c2,c3;
//  ioctl(fds->fd0, PERF_EVENT_IOC_DISABLE, 0);
//  read(fds->fd0, &c0, sizeof(long long));
//  ioctl(fds->fd1, PERF_EVENT_IOC_DISABLE, 0);
//  read(fds->fd1, &c1, sizeof(long long));
//  ioctl(fds->fd2, PERF_EVENT_IOC_DISABLE, 0);
//  read(fds->fd2, &c2, sizeof(long long));
//  ioctl(fds->fd3, PERF_EVENT_IOC_DISABLE, 0);
//  read(fds->fd3, &c3, sizeof(long long));
//
////  printf("[HPM Perf]Branch instrs:%lld; Misbranch instrs:%lld; Frontend Stall:%lld; Backend Stall:%lld\n",
////      c0, c1, c2,c3);
//  printf("[HPM Perf]Branch instrs:%lld; Misbranch instrs:%lld; Instrs:%lld; Cycles:%lld\n",
//      c0, c1, c2,c3);
//}

void perf_events_close(hpm_fds_t* fds, hpm_group_t* egroup) {
  int i;
  int size = egroup->size;
  for(i = 0; i < size; ++i) {
    close(fds->fd[i]);
  }
}

/***** Macro Definition  *****/
#if (defined PERF_HW) || (defined PERF_BR) || (defined PERF_SW) || (defined PERF_ICACHE) || (defined PERF_DCACHE) || (defined PERF_LLC)

#ifdef PERF_HW
#define GNAME hw_group
#endif

#ifdef PERF_BR
#define GNAME br_group
#endif

#ifdef PERF_SW
#define GNAME sw_group
#endif

#ifdef PERF_ICACHE
#define GNAME icache_group
#endif

#ifdef PERF_DCACHE
#define GNAME dcache_group
#endif

#ifdef PERF_LLC
#define GNAME llc_group
#endif

#define HPM_PERF_CREATE hpm_fds_t __hpm_fds; perf_events_create(&__hpm_fds, &GNAME)
#define HPM_PERF_START perf_events_start(&__hpm_fds, &GNAME)
#define HPM_PERF_STOP perf_events_stop_report(&__hpm_fds, &GNAME)
#define HPM_PERF_CLOSE perf_events_close(&__hpm_fds, &GNAME);
#else
#define HPM_PERF_CREATE
#define HPM_PERF_START
#define HPM_PERF_STOP
#define HPM_PERF_CLOSE
#endif

#endif /* PERFMEASURE_H_ */
