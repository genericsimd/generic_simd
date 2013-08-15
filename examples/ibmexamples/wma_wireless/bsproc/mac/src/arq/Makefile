ROOT=$(MY_WIMAX_ROOT)
CC=gcc
AR=ar 
INCDIR=../include/mac
#LOGFLAG=-DLOG_METRICS
CFLAGS+=$(LOGFLAG) -c $(OPTFLAGS) -I$(INCDIR) -DQ_SIZE=20 -DTRACE_LEVEL=0 -Wall -std=c99 -D_GNU_SOURCE $(DEBUGFLAGS) $(PERFFLAGS) $(LOGFLAG) #-DDEBUG
LDFLAGS+=-pthread -lm
CPPDIR=$(ROOT)/src
SRCDIR=$(ROOT)/src
ARQDIR=$(SRCDIR)/arq
COMMONDIR=$(SRCDIR)/common

SRCS=$(SRCDIR)/common/memmgmt.c
OBJS=$(SRCDIR)/common/memmgmt.o

ARQSRCS=$(ARQDIR)/circq_array.c $(ARQDIR)/arq_init.c $(ARQDIR)/arq_conn.c $(ARQDIR)/rbtree.c \
                         $(ARQDIR)/arq_threads.c $(ARQDIR)/ll.c $(ARQDIR)/arq_tx_blocks_proc.c \
						 $(ARQDIR)/arq_dl_timeouts.c  $(ARQDIR)/arq_ifaces.c $(ARQDIR)/arq_rx_blocks_proc.c \
						 $(ARQDIR)/arq_ul_timeouts.c $(ARQDIR)/arq_ul_mgmt_msg.c $(ARQDIR)/binheap.c 
ARQOBJS=$(ARQDIR)/circq_array.o $(ARQDIR)/arq_init.o $(ARQDIR)/arq_conn.o $(ARQDIR)/rbtree.o \
                         $(ARQDIR)/arq_threads.o $(ARQDIR)/ll.o $(ARQDIR)/arq_tx_blocks_proc.o \
						 $(ARQDIR)/arq_dl_timeouts.o $(ARQDIR)/arq_ifaces.o $(ARQDIR)/arq_rx_blocks_proc.o \
						 $(ARQDIR)/arq_ul_timeouts.o $(ARQDIR)/arq_ul_mgmt_msg.o $(ARQDIR)/binheap.o 
#TESTSRC=$(ARQDIR)/ack_gen_test.c
#TESTSRC=$(ARQDIR)/ul_mgmt_msg_test.c
#TESTSRC=$(ARQDIR)/ul_conn_test.c
TESTSRC=  $(ARQDIR)/conns_init_test.c 
#TESTSRC=$(ARQDIR)/test_delta_list.c
#TESTSRC=$(ARQDIR)/test_linked_list.c 
#TESTSRC=$(ARQDIR)/signal_test.c 
		 #$(ARQDIR)/tx_blocks_q.c
				
#TESTOBJ=$(ARQDIR)/ack_gen_test.o
#TESTOBJ=$(ARQDIR)/ul_mgmt_msg_test.o
#TESTOBJ=$(ARQDIR)/ul_conn_test.o
TESTOBJ=  $(ARQDIR)/conns_init_test.o
#TESTOBJ=$(ARQDIR)/test_delta_list.o
#TESTOBJ=$(ARQDIR)/test_linked_list.o 
#TESTOBJ=$(ARQDIR)/signal_test.o 
		 #$(ARQDIR)/tx_blocks_q.o

TESTOBJ_UL=  $(ARQDIR)/ul_conn_test.o
TESTOBJ_DL=  $(ARQDIR)/conns_init_test.o
EXECUTABLE=ARQWMAC
EXECUTABLE_UL=ARQWMAC_UL
EXECUTABLE_DL=ARQWMAC_DL

MACLIB=$(ROOT)/libmac.a
ARQLIB=$(ARQDIR)/arq.a
LOGLIB=$(COMMONDIR)/log.a

#.SUFFIXES: .c .o

###---- make generates the exe file 

$(EXECUTABLE): $(OBJS) $(ARQOBJS) $(TESTOBJ)
	$(CC) $(LDFLAGS) $(OBJS) $(ARQOBJS) $(TESTOBJ) $(LOGLIB) -o $@
$(EXECUTABLE_UL): $(OBJS) $(ARQOBJS) $(TESTOBJ_UL)
	$(CC) $(LDFLAGS) $(OBJS) $(ARQOBJS) $(TESTOBJ_UL) $(LOGLIB)  -o $@
$(EXECUTABLE_DL): $(OBJS) $(ARQOBJS) $(TESTOBJ_DL)
	$(CC) $(LDFLAGS) $(OBJS) $(ARQOBJS) $(TESTOBJ_DL)  $(LOGLIB) -o $@

$(ARQLIB): 	$(ARQOBJS)
	@echo $(CFLAGS)
	$(AR) crs $@ $(ARQOBJS)
	$(AR) rs $(MACLIB) $(ARQOBJS)

all: $(EXECUTABLE) $(ARQLIB)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

###########--- Rule to clean object files and executable----
clean:
	rm -rf $(ARQDIR)/$(EXECUTABLE)
	rm -rf $(ARQDIR)/*.o
	rm -rf $(ARQDIR)/*.a
