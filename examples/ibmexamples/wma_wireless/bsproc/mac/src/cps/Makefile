ROOT=$(MY_WIMAX_ROOT)
CC=gcc
MAKE=make
CFLAGS+=-g -pthread $(OPTFLAGS) -c -I$(INCDIR) $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS)
CFLAGS      += -D_DUMP_UTIL_ENABLE_ -D_DUMP_BIN_RAW_DATA_
LDFLAGS+=-pthread
INCDIR=../include/mac -I ../include -I ../include/dl_adapter
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
CPSDIR=$(SRCDIR)/cps
ULDIR=$(CPSDIR)/ul
DLDIR=$(CPSDIR)/dl
CPSCOMMONDIR=$(CPSDIR)/common
SCHEDULERDIR=$(CPSDIR)/scheduler

## TO DO: Add needed files to the list of sources and obj's
SRCS=
OBJS=
TESTSRC=
TESTOBJ=
MACLIB=$(ROOT)/libmac.a
CPSLIB=$(CPSDIR)/cps.a
ULLIB=$(ULDIR)/ul.a
DLLIB=$(DLDIR)/dl.a
CPSCOMMONLIB=$(CPSCOMMONDIR)/common.a
SCHEDULERLIB=$(SCHEDULERDIR)/scheduler.a


EXECUTABLE=CPSWMAC


###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(TESTOBJ) $(CPSLIB)
	$(CC) $(LDFLAGS) $(OBJS) $(TESTOBJ) $(CPSLIB) $(LIBS) -o $@

all: $(EXECUTABLE) $(CPSLIB)

$(SRCDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(CPSLIB): $(OBJS) $(CPSCOMMONLIB) $(ULLIB) $(DLLIB) $(SCHEDULERLIB)
#	$(AR) crs $@ $(OBJS) $(CPSCOMMONLIB) $(ULLIB) $(DLLIB) $(SCHEDULERLIB)
#	ranlib $@

$(MACLIB): $(OBJS) $(CPSCOMMONLIB) $(ULLIB) $(DLLIB) $(SCHEDULERLIB)
#	$(AR) crs $@ $(OBJS) $(CPSCOMMONLIB) $(ULLIB) $(DLLIB) $(SCHEDULERLIB)
#	ranlib $@

$(ULLIB): FORCE
	$(MAKE) -f $(ULDIR)/Makefile $(ULLIB) DEBUGFLAGS="$(DEBUGFLAGS)" LOGFLAG=$(LOGFLAG)

$(CPSCOMMONLIB): FORCE
	$(MAKE) -f $(CPSCOMMONDIR)/Makefile $(CPSCOMMONLIB) DEBUGFLAGS="$(DEBUGFLAGS)" LOGFLAG=$(LOGFLAG)

$(DLLIB): FORCE
	$(MAKE) -f $(DLDIR)/Makefile $(DLLIB) DEBUGFLAGS="$(DEBUGFLAGS)" LOGFLAG=$(LOGFLAG)

$(SCHEDULERLIB): FORCE
	$(MAKE) -f $(SCHEDULERDIR)/Makefile $(SCHEDULERLIB) DEBUGFLAGS="$(DEBUGFLAGS)" LOGFLAG=$(LOGFLAG)

FORCE:

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(CPSDIR)/*.o
	rm -rf $(CPSDIR)/*.a
	rm -rf $(CPSDIR)/$(EXECUTABLE)
	$(MAKE) -f $(ULDIR)/Makefile clean
	$(MAKE) -f $(DLDIR)/Makefile clean
	$(MAKE) -f $(SCHEDULERDIR)/Makefile clean
	$(MAKE) -f $(CPSCOMMONDIR)/Makefile clean


