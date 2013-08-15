TARGET = chk_fft
OBJS = chk_fft.o fft.o

#IPPROOT         = /opt/intel/ipp/6.1.1.042/em64t
#CFLAGS          += -I $(IPPROOT)/include
#LDFLAGS         += $(IPPROOT)/sharedlib/libippsem64t.so $(IPPROOT)/sharedlib/libippcoreem64t.so -lpthread

include ../makefile.footer

chk: chk.o viterbicore.o flog.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^
