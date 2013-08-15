export MY_WIMAX_ROOT=.
ROOT=$(MY_WIMAX_ROOT)
CC=gcc
MAKE=make
INCDIR=-I../include/mac -I../include/openssl  -I../include/bsl -I../include
CFLAGS  += -D_DUMP_UTIL_ENABLE_ -D_DUMP_BIN_RAW_DATA_
LOGFLAG=-g -Wall
#PERFFLAGS=-DPERFLOG
OPTFLAGS=-O3
DEBUGFLAGS="-DLOG_LEVEL=LOG_LEVEL_INFO"
CFLAGS+=-g -c $(OPTFLAGS) -I$(INCDIR) $(LOGFLAG) $(DEBUGFLAGS) $(PERFFLAGS) 
LDFLAGS=-lpthread -lm ../lib/openssl/libcrypto.a -ldl $(OPTFLAGS)
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
ARQDIR=$(SRCDIR)/arq
COMMONDIR=$(SRCDIR)/common
MMMDIR=$(SRCDIR)/mmm
CSDIR=$(SRCDIR)/cs
CPSDIR=$(SRCDIR)/cps
TBDIR=$(SRCDIR)/testbed

## TO DO: Prune the list of sources and obj's
SRCS=
OBJS=
MACLIB=$(ROOT)/libmac.a
ARQLIB=$(ARQDIR)/arq.a
COMMONLIB=$(COMMONDIR)/common.a
LOGLIB=$(COMMONDIR)/log.a
MMMLIB=$(MMMDIR)/mmm.a
CSLIB=$(CSDIR)/cs.a
CPSLIB=$(CPSDIR)/cps.a
TBLIB=$(TBDIR)/testbed.a
LOGXTOR=textlogger


EXECUTABLE=WMAC
PURIFY_EXECUTABLE=WMAC_PP


#ENABLE_DUMP_MAC = yes

#ifeq ($(ENABLE_DUMP_MAC), yes)
#CFLAGS         += -D_DUMP_MAC_
#endif

###---- make generates the exe file 

.PHONY: clean FORCE

$(MACLIB): $(OBJS) $(ARQLIB) $(COMMONLIB) $(LOGLIB) $(MMMLIB) $(CSLIB) $(CPSLIB) $(TBLIB)
	cp -pf libmac.a ../lib

$(EXECUTABLE): $(OBJS) $(ARQLIB) $(COMMONLIB) $(LOGLIB) $(MMMLIB) $(CSLIB) $(CPSLIB) $(TBLIB)
	$(CC)  $(OBJS) $(TBLIB) $(MMMLIB)  $(CSLIB) $(CPSLIB) $(COMMONLIB) $(ARQLIB) $(LOGLIB) $(LIBS) $(LDFLAGS) -o $@

$(PURIFY_EXECUTABLE): $(OBJS) $(ARQLIB) $(COMMONLIB) $(LOGLIB) $(MMMLIB) $(CSLIB) $(CPSLIB) $(TBLIB)
	purify $(CC) $(OBJS) $(TBLIB) $(MMMLIB)  $(CSLIB) $(CPSLIB) $(COMMONLIB) $(ARQLIB) $(LOGLIB) $(LIBS) $(LDFLAGS) -o $@

all: $(EXECUTABLE) $(LOGXTOR)

purify-all: $(PURIFY_EXECUTABLE) $(LOGXTOR)

$(SRCDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@


$(ARQLIB): FORCE
	$(MAKE) -f $(ARQDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS)  PERFFLAGS=$(PERFFLAGS) $(ARQLIB)

$(COMMONLIB): FORCE
	$(MAKE) -f $(COMMONDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(COMMONLIB)

$(LOGLIB): FORCE
	$(MAKE) -f $(COMMONDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(LOGLIB)

$(MMMLIB): FORCE
	$(MAKE) -f $(MMMDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(MMMLIB)
    
$(CSLIB): FORCE
	$(MAKE) -f $(CSDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(CSLIB)

$(CPSLIB): FORCE
	$(MAKE) -f $(CPSDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(CPSLIB)

$(TBLIB): FORCE
	$(MAKE) -f $(TBDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG) OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(TBLIB)

$(LOGXTOR): FORCE
	$(MAKE) -f $(COMMONDIR)/Makefile DEBUGFLAGS=$(DEBUGFLAGS) LOGFLAG=$(LOGFLAG)  OPTFLAGS=$(OPTFLAGS) PERFFLAGS=$(PERFFLAGS) $(COMMONDIR)/$(LOGXTOR)


FORCE:

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(SRCDIR)/*.o
	rm -rf $(ROOT)/WMAC
	rm -f `find . -name "*~"`
	rm -f `find . -name "*.o"`
	rm -f `find . -name "*.a"`
	$(MAKE) -f $(ARQDIR)/Makefile clean
	$(MAKE) -f $(COMMONDIR)/Makefile clean
	$(MAKE) -f $(MMMDIR)/Makefile clean
	$(MAKE) -f $(CSDIR)/Makefile clean
	$(MAKE) -f $(CPSDIR)/Makefile clean
	$(MAKE) -f $(TBDIR)/Makefile clean

