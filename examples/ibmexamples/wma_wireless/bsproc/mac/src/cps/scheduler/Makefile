ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS=-c $(OPTFLAGS) -I$(INCDIR) -DNODEBUG $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS) -I ../include
CFLAGS  += -D_DUMP_UTIL_ENABLE_ -D_DUMP_BIN_RAW_DATA_
LDFLAGS=-pthread
INCDIR=../include/mac
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CPSDIR=$(SRCDIR)/cps
SCHEDULERDIR=$(CPSDIR)/scheduler

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=
OBJS=

## Include in the following, the list of files and obj's that are part of this component
SCHEDULERSRCS=$(SCHEDULERDIR)/br_queue.c $(SCHEDULERDIR)/dlmap_builder.c $(SCHEDULERDIR)/dl_scheduler.c $(SCHEDULERDIR)/free_maps.c \
	      $(SCHEDULERDIR)/init_maps.c $(SCHEDULERDIR)/mac_scheduling.c $(SCHEDULERDIR)/scheduler.c \
	      $(SCHEDULERDIR)/ul_mgt_msg_queue.c $(SCHEDULERDIR)/ul_scheduler.c $(SCHEDULERDIR)/scheduler_utils.c \
              $(SCHEDULERDIR)/ss_scheduler.c

SCHEDULEROBJS=$(SCHEDULERDIR)/br_queue.o $(SCHEDULERDIR)/dlmap_builder.o $(SCHEDULERDIR)/dl_scheduler.o $(SCHEDULERDIR)/free_maps.o \
	      $(SCHEDULERDIR)/init_maps.o $(SCHEDULERDIR)/mac_scheduling.o $(SCHEDULERDIR)/scheduler.o \
	      $(SCHEDULERDIR)/ul_mgt_msg_queue.o $(SCHEDULERDIR)/ul_scheduler.o $(SCHEDULERDIR)/scheduler_utils.o \
              $(SCHEDULERDIR)/ss_scheduler.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=SCHEDULERWMAC
MACLIB=$(ROOT)/libmac.a
SCHEDULERLIB=$(SCHEDULERDIR)/scheduler.a
CPSLIB=$(CPSDIR)/cps.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(SCHEDULEROBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(SCHEDULEROBJS) $(TESTOBJ) -o $@

$(SCHEDULERLIB):   $(SCHEDULEROBJS)
	$(AR) crs $@ $(SCHEDULEROBJS)
	$(AR) rs $(CPSLIB) $(SCHEDULEROBJS)
	$(AR) rs $(MACLIB) $(SCHEDULEROBJS)

all: $(EXECUTABLE) $(SCHEDULERLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(SCHEDULERDIR)/$(EXECUTABLE)
	rm -rf $(SCHEDULERDIR)/*.o
	rm -rf $(SCHEDULERDIR)/*.a

