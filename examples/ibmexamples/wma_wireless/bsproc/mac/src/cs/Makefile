ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DTRACE_LEVEL=0 $(LOGFLAG) $(DEBUGFLAGS) -DNODEBUG $(PERFFLAGS)
LDFLAGS+=-pthread
INCDIR=../include/mac
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CSDIR=$(SRCDIR)/cs

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/common/memmgmt.c
OBJS=$(SRCDIR)/common/memmgmt.o
CSSRCS=$(CSDIR)/mac_hash.c $(CSDIR)/mac_sdu_queue.c $(CSDIR)/sdu_cid_queue.c  $(CSDIR)/CS.c $(CSDIR)/classification_function.c

CSOBJS=$(CSDIR)/mac_hash.o $(CSDIR)/mac_sdu_queue.o $(CSDIR)/sdu_cid_queue.o $(CSDIR)/CS.o $(CSDIR)/classification_function.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=CSWMAC
MACLIB=$(ROOT)/libmac.a
CSLIB=$(CSDIR)/cs.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(CSOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(CSOBJS) $(TESTOBJ) -o $@

$(CSLIB):   $(CSOBJS)
	$(AR) crs $@ $(CSOBJS)
	$(AR) rs $(MACLIB) $(CSOBJS)

all: $(EXECUTABLE) $(CSLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(CSDIR)/$(EXECUTABLE)
	rm -rf $(CSDIR)/*.o
	rm -rf $(CSDIR)/*.a

