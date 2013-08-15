ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DTRACE_LEVEL=6 $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS) -I ../include 
CFLAGS      += -D_DUMP_UTIL_ENABLE_ -D_DUMP_BIN_RAW_DATA_
LDFLAGS+=-pthread
INCDIR=../include/mac
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CPSDIR=$(SRCDIR)/cps
CPSCOMMONDIR=$(CPSDIR)/common

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/memmgmt.c
OBJS=$(SRCDIR)/memmgmt.o

## Include in the following, the list of files and obj's that are part of this component
CPSCOMMONSRCS= $(CPSCOMMONDIR)/mac_arq.c $(CPSCOMMONDIR)/mac_br_queue.c $(CPSCOMMONDIR)/mac_common_data.c \
	       $(CPSCOMMONDIR)/mac_connection.c $(CPSCOMMONDIR)/mac_cps.c $(CPSCOMMONDIR)/mac_crc.c $(CPSCOMMONDIR)/mac_frame.c \
	       $(CPSCOMMONDIR)/mac_hcs.c $(CPSCOMMONDIR)/mac_headermsg_builder.c $(CPSCOMMONDIR)/mac_headermsg_parser.c \
	       $(CPSCOMMONDIR)/mac_subframe_queue.c $(CPSCOMMONDIR)/mac_serviceflow.c \
	$(CPSCOMMONDIR)/mac_amc.c $(CPSCOMMONDIR)/ucd_handler.c $(CPSCOMMONDIR)/dcd_handler.c $(CPSCOMMONDIR)/ucd_dcd_parser.c $(CPSCOMMONDIR)/mac_auth_functions.c

CPSCOMMONOBJS= $(CPSCOMMONDIR)/mac_arq.o $(CPSCOMMONDIR)/mac_br_queue.o $(CPSCOMMONDIR)/mac_common_data.o \
	       $(CPSCOMMONDIR)/mac_connection.o $(CPSCOMMONDIR)/mac_cps.o $(CPSCOMMONDIR)/mac_crc.o $(CPSCOMMONDIR)/mac_frame.o \
	       $(CPSCOMMONDIR)/mac_hcs.o $(CPSCOMMONDIR)/mac_headermsg_builder.o $(CPSCOMMONDIR)/mac_headermsg_parser.o \
	       $(CPSCOMMONDIR)/mac_subframe_queue.o $(CPSCOMMONDIR)/mac_serviceflow.o \
	$(CPSCOMMONDIR)/mac_amc.o $(CPSCOMMONDIR)/ucd_handler.o $(CPSCOMMONDIR)/dcd_handler.o $(CPSCOMMONDIR)/ucd_dcd_parser.o $(CPSCOMMONDIR)/mac_auth_functions.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=CPSCMNWMAC
MACLIB=$(ROOT)/libmac.a
CPSCOMMONLIB=$(CPSCOMMONDIR)/common.a
CPSLIB=$(CPSDIR)/cps.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(CPSCOMMONOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(CPSCOMMONOBJS) $(TESTOBJ) -o $@

$(CPSCOMMONLIB):   $(CPSCOMMONOBJS)
	$(AR) crs $@ $(CPSCOMMONOBJS)
	$(AR) rs $(CPSLIB) $(CPSCOMMONOBJS)
	$(AR) rs $(MACLIB) $(CPSCOMMONOBJS)

all: $(EXECUTABLE) $(CPSCOMMONLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(CPSCOMMONDIR)/$(EXECUTABLE)
	rm -rf $(CPSCOMMONDIR)/*.o
	rm -rf $(CPSCOMMONDIR)/*.a

