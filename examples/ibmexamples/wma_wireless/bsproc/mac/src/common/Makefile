ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
#LOGFLAG=-DLOG_METRICS
CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DNODEBUG -DTRACE_LEVEL=0  -std=c99 -D_GNU_SOURCE $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS) #-DDDEBUG
LDFLAGS+=-pthread
INCDIR=../include/mac
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
COMMONDIR=$(SRCDIR)/common

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=
OBJS=

## Include in the following, the list of files and obj's that are part of this component
COMMONSRCS=$(COMMONDIR)/memmgmt.c  $(COMMONDIR)/logical_packet.c $(COMMONDIR)/perf_log.c $(COMMONDIR)/thread_sync.c $(COMMONDIR)/app_timer.c $(COMMONDIR)/dll_ordered_list.c $(COMMONDIR)/sll_fifo_q.c
LOGSRCS=$(COMMONDIR)/log.c $(COMMONDIR)/logreader.c 

COMMONOBJS=$(COMMONDIR)/memmgmt.o  $(COMMONDIR)/logical_packet.o $(COMMONDIR)/perf_log.o $(COMMONDIR)/thread_sync.o $(COMMONDIR)/app_timer.o $(COMMONDIR)/dll_ordered_list.o $(COMMONDIR)/sll_fifo_q.o
LOGOBJS=$(COMMONDIR)/log.o $(COMMONDIR)/logreader.o

XTOROBJS=$(COMMONDIR)/textlogger.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=CMNWMAC

MACLIB=$(ROOT)/libmac.a
COMMONLIB=$(COMMONDIR)/common.a
LOGLIB=$(COMMONDIR)/log.a
LOGXTOR=$(COMMONDIR)/textlogger

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(COMMONOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(COMMONOBJS) $(TESTOBJ) -o $@

$(COMMONLIB): $(COMMONOBJS)
	$(AR) crs $@ $(COMMONOBJS)
	$(AR) rs $(MACLIB) $(COMMONOBJS)

$(LOGLIB): $(LOGOBJS)
	$(AR) crs $@ $(LOGOBJS)
	$(AR) rs $(MACLIB) $(LOGOBJS)

$(LOGXTOR): $(XTOROBJS)
	$(CC) $(LDFLAGS) $(XTOROBJS) -o $@


all: $(COMMONLIB) $(LOGLIB) $(LOGXTOR)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(COMMONDIR)/$(EXECUTABLE)
	rm -rf $(COMMONDIR)/*.o
	rm -rf $(COMMONDIR)/*.a
	rm -rf $(COMMONDIR)/$(LOGXTOR)
