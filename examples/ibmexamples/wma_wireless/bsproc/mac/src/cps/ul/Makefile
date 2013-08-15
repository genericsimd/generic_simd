ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DTRACE_LEVEL=6 -DNODEBUG $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS)
LDFLAGS+=-lpthread ../lib/openssl/libcrypto.a -ldl
INCDIR=../include/mac -I../include/phy -I ../include/bs_adapter -I ../include -I ../include/bsl
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CPSDIR=$(SRCDIR)/cps
ULDIR=$(CPSDIR)/ul

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/memmgmt.c
OBJS=$(SRCDIR)/memmgmt.o

## Include in the following, the list of files and obj's that are part of this component
ULSRCS=$(ULDIR)/mac_ul_cps_controller.c $(ULDIR)/mac_ul_frag_queue.c $(ULDIR)/mac_ul_pdu_parser.c $(ULDIR)/mac_ul_pdu_queue.c $(ULDIR)/mac_ul_reassembly.c

ULOBJS=$(ULDIR)/mac_ul_cps_controller.o $(ULDIR)/mac_ul_frag_queue.o $(ULDIR)/mac_ul_pdu_parser.o $(ULDIR)/mac_ul_pdu_queue.o $(ULDIR)/mac_ul_reassembly.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=ULWMAC
MACLIB=$(ROOT)/libmac.a
ULLIB=$(ULDIR)/ul.a
CPSLIB=$(CPSDIR)/cps.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(ULOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(ULOBJS) $(TESTOBJ) -o $@

$(ULLIB):   $(ULOBJS)
	$(AR) crs $@ $(ULOBJS)
	$(AR) rs $(CPSLIB) $(ULOBJS)
	$(AR) rs $(MACLIB) $(ULOBJS)

all: $(EXECUTABLE) $(ULLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(ULDIR)/$(EXECUTABLE)
	rm -rf $(ULDIR)/*.o
	rm -rf $(ULDIR)/*.a

