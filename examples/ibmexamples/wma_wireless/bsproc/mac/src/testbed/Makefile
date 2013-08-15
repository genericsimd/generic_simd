ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DTRACE_LEVEL=6 -DNODEBUG $(LOGFLAG) -D_GNU_SOURCE -std=c99 $(DEBUGFLAGS) $(PERFFLAGS)
LDFLAGS+=-pthread
INCDIR=../include/mac -I../include
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
TBDIR=$(SRCDIR)/testbed

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/common/memmgmt.c
OBJS=$(SRCDIR)/common/memmgmt.o
TBSRCS=$(TBDIR)/CPS.c $(TBDIR)/CS.c $(TBDIR)/dl_cbr_gen.c $(TBDIR)/ul_cs_consumer.c $(TBDIR)/dl_exp_params_init.c $(TBDIR)/dl_mme_gen.c \
       $(TBDIR)/mac_unit_test_normal.c $(TBDIR)/main.c $(TBDIR)/testCS.c $(TBDIR)/test_routines.c $(TBDIR)/mac_bs_ranging_utm.c $(TBDIR)/mac_ss_ranging_utm.c $(TBDIR)/mac_dsa_utm.c  $(TBDIR)/mac_dsc_utm.c $(TBDIR)/mac_dsd_test.c $(TBDIR)/mac_amc_test.c  

TBOBJS=$(TBDIR)/CPS.o $(TBDIR)/CS.o $(TBDIR)/dl_cbr_gen.o $(TBDIR)/ul_cs_consumer.o $(TBDIR)/dl_exp_params_init.o $(TBDIR)/dl_mme_gen.o \
       $(TBDIR)/mac_unit_test_normal.o $(TBDIR)/main.o  $(TBDIR)/testCS.o $(TBDIR)/test_routines.o $(TBDIR)/mac_bs_ranging_utm.o $(TBDIR)/mac_ss_ranging_utm.o $(TBDIR)/mac_dsa_utm.o $(TBDIR)/mac_dsc_utm.o $(TBDIR)/mac_dsd_test.o $(TBDIR)/mac_amc_test.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=TBWMAC
MACLIB=$(ROOT)/libmac.a
TBLIB=$(TBDIR)/testbed.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(TBOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(TBOBJS) $(TESTOBJ) -o $@

$(TBLIB):   $(TBOBJS)
	$(AR) crs $@ $(TBOBJS)
	$(AR) rs $(MACLIB) $(TBOBJS)

all: $(EXECUTABLE) $(TBLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(TBDIR)/$(EXECUTABLE)
	rm -rf $(TBDIR)/*.o
	rm -rf $(TBDIR)/*.a

