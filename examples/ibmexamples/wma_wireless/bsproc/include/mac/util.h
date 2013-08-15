/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: util.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <sys/time.h>

#define __X86_ATO__

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#define mod(a,b) ((((a) >= 0) || (-(a) == (b))) 	? (a)%(b) \
													: (-((a)/(b)-1)*(b)+(a)))

#define ONE_MILLION_LL 1000000LL

static inline unsigned long long readtsc(void)
{
/* 	unsigned long long curr_tsc;
 	__asm__ __volatile__("cpuid; rdtsc" : "=A" (curr_tsc));
 	__asm__ __volatile__("cpuid" : : );
 	return curr_tsc; */

 
	unsigned long long curr_tsc;
        struct timeval curr_time;
        gettimeofday(&curr_time, NULL);
        curr_tsc = curr_time.tv_sec*ONE_MILLION_LL;
        curr_tsc += curr_time.tv_usec;
        return curr_tsc;
}

#ifdef __X86_ATO__
static inline int  fetch_and_incr(int *val)
{
    int ret = 1;
	__asm__ __volatile__("lock; xaddl %0, %1" : "+r" (ret), "+m" (*val) : : "memory" );
	return ret;
}

static inline int atomic_incr(int *val)
{
	int ret = *val;
	__asm__ __volatile__("lock; incl %0" : "+m"(*val) : : "memory");
	return ret;
}
#endif

#ifdef __PPC_ATO__

static inline int  fetch_and_incr(int *val)
{
        int t;
        int ret;

        __asm__ __volatile__(
"1:     lwarx   %0,0,%2         # atomic_inc_return\n\
        addic   %1,%0,1\n"
"       stwcx.  %1,0,%2 \n\
        bne-    1b"

        : "=&r" (t), "=&r" (ret)
        : "r" (val)
        : "cc", "memory");

        return t;
}

static inline int atomic_incr(int *val)
{
        int t = *val;
        int ret;

        __asm__ __volatile__(
"1:     lwarx   %0,0,%1         # atomic_inc_return\n\
        addic   %0,%0,1\n"
"       stwcx.  %0,0,%1 \n\
        bne-    1b"
        : "=&r" (ret)
        : "r" (val)
        : "cc", "memory");

        return t;
}


#endif


#endif // _UTIL_H_
