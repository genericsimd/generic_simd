TARGET = chk_demod
OBJS =  chk_demod.o phy_ul_demodulation.o flog.o

include ../makefile.footer

# why using -O0 in the options?

chk: chk.o viterbicore.o flog.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^
