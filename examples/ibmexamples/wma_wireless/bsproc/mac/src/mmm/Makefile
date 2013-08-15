ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 

ENABLE_SYS_POWER    = yes

ifeq ($(ENABLE_SYS_POWER), yes)
CFLAGS      += -DSYS_POWER_COMPILE
endif

CFLAGS+=-c $(OPTFLAGS) -I$(INCDIR) -DTRACE_LEVEL=6 -DNODEBUG $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS)
LDFLAGS+=-pthread
INCDIR=../include/mac -I../include -I../include/hash
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
MMMDIR=$(SRCDIR)/mmm

## Include in SRCS any files that are part of other modules but are needed here in order to build an executable
## for testing
SRCS=$(SRCDIR)/common/memmgmt.c
OBJS=$(SRCDIR)/common/memmgmt.o

## Include in the following, the list of files and obj's that are part of this component
MMMSRCS=$(MMMDIR)/bs_ss_info.c $(MMMDIR)/mac_bs_ranging.c $(MMMDIR)/mac_ss_ranging.c $(MMMDIR)/ranging_mm.c $(MMMDIR)/mac_dsa_sm.c $(MMMDIR)/mac_dsc_sm.c $(MMMDIR)/mac_qos_mm.c $(MMMDIR)/mac_dsa_list.c $(MMMDIR)/mac_sf_sm.c $(MMMDIR)/mac_sf_api.c $(MMMDIR)/mac_dsd_processing.c $(MMMDIR)/mac_pkm_sm.c $(MMMDIR)/mac_auth_mm.c $(MMMDIR)/mac_br_api.c $(MMMDIR)/mac_acm_coding.c $(MMMDIR)/mac_rep_sm.c

MMMOBJS=$(MMMDIR)/bs_ss_info.o $(MMMDIR)/mac_bs_ranging.o $(MMMDIR)/mac_ss_ranging.o $(MMMDIR)/ranging_mm.o $(MMMDIR)/mac_dsa_sm.o $(MMMDIR)/mac_dsc_sm.o $(MMMDIR)/mac_qos_mm.o $(MMMDIR)/mac_dsa_list.o $(MMMDIR)/mac_sf_sm.o $(MMMDIR)/mac_sf_api.o $(MMMDIR)/mac_dsd_processing.o $(MMMDIR)/mac_pkm_sm.o $(MMMDIR)/mac_auth_mm.o $(MMMDIR)/mac_br_api.o $(MMMDIR)/mac_acm_coding.o $(MMMDIR)/mac_rep_sm.o

## Include in TESTSRC, any additional unit-test file needed to build an executable for unit testing
TESTSRC=
TESTOBJ=

EXECUTABLE=MMMWMAC
MACLIB=$(ROOT)/libmac.a
MMMLIB=$(MMMDIR)/mmm.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(MMMOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(MMMOBJS) $(TESTOBJ) -o $@

$(MMMLIB):  $(MMMOBJS)
	$(AR) crs $@ $(MMMOBJS)
	$(AR) rs $(MACLIB) $(MMMOBJS)

all: $(EXECUTABLE) $(MMMLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(MMMDIR)/$(EXECUTABLE)
	rm -rf $(MMMDIR)/*.o
	rm -rf $(MMMDIR)/*.a

