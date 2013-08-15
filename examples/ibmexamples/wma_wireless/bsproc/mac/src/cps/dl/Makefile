ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS+=$(LOGFLAG) $(OPTFLAGS) -c -g -I$(INCDIR) -DTRACE_LEVEL=6 -DNODEBUG $(DEBUGFLAGS) -std=c99 -D_GNU_SOURCE $(PERFFLAGS)
LDFLAGS+=-lpthread ../lib/openssl/libcrypto.a -ldl
INCDIR=../include/mac -I../include -I../include/bs_adapter -I../inlcude/bsl
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CPSDIR=$(SRCDIR)/cps
DLDIR=$(CPSDIR)/dl

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/memmgmt.c
OBJS=$(SRCDIR)/memmgmt.o

## Include in the following, the list of files and obj's that are part of this component
DLSRCS=$(DLDIR)/mac_dl_concatenation.c $(DLDIR)/mac_dl_cps_controller.c $(DLDIR)/mac_dl_frag_pack.c

DLOBJS=$(DLDIR)/mac_dl_concatenation.o $(DLDIR)/mac_dl_cps_controller.o $(DLDIR)/mac_dl_frag_pack.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=DLWMAC
MACLIB=$(ROOT)/libmac.a
DLLIB=$(DLDIR)/dl.a
CPSLIB=$(CPSDIR)/cps.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(DLOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(DLOBJS) $(TESTOBJ) -o $@

$(DLLIB):   $(DLOBJS)
	$(AR) crs $@ $(DLOBJS)
	$(AR) rs $(CPSLIB) $(DLOBJS)
	$(AR) rs $(MACLIB) $(DLOBJS)

all: $(EXECUTABLE) $(DLLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(DLDIR)/$(EXECUTABLE)
	rm -rf $(DLDIR)/*.o
	rm -rf $(DLDIR)/*.a

