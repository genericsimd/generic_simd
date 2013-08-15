/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: debug.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifndef _DEBUG_H
#define _DEBUG_H
#include <stdio.h>
#include "flog.h"

#ifdef DDEBUG
#define TRACE(n, trace_str)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str);	\
		fflush (stdout); \
	}

#define TRACE1(n, trace_str, arg1)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1);	\
		fflush (stdout); \
	}

#define TRACE2(n, trace_str, arg1, arg2)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1, arg2);	\
		fflush (stdout); \
	}

#define TRACE3(n, trace_str, arg1, arg2, arg3)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1, arg2, arg3);	\
		fflush (stdout); \
	}

#define TRACE4(n, trace_str, arg1, arg2, arg3, arg4)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1, arg2, arg3, arg4);	\
		fflush (stdout); \
	}

#define TRACE5(n, trace_str, arg1, arg2, arg3, arg4, arg5)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1, arg2, arg3, arg4, arg5);	\
		fflush (stdout); \
	}

#define TRACE6(n, trace_str, arg1, arg2, arg3, arg4, arg5, arg6)  \
	if (n <= TRACE_LEVEL) {  \
		printf (trace_str, arg1, arg2, arg3, arg4, arg5, arg6);	\
		fflush (stdout); \
	}

#else
#define TRACE(n, trace_str) ;
#define TRACE1(n, trace_str, arg1) ;
#define TRACE2(n, trace_str, arg1, arg2) ;
#define TRACE3(n, trace_str, arg1, arg2, arg3) ;
#define TRACE4(n, trace_str, arg1, arg2, arg3, arg4) ;
#define TRACE5(n, trace_str, arg1, arg2, arg3, arg4, arg5) ;
#define TRACE6(n, trace_str, arg1, arg2, arg3, arg4, arg5, arg6) ;
#endif


#endif //_DEBUG_H
