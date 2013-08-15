TARGET = chk
OBJS = chk.o viterbicore.o flog.o

include ../makefile.footer

test: chk
	@./chk
	@od -v -t x1 bbu8_fecdecoding_vit.out > bbu8_fecdecoding_vit.data
	diff -urN bbu8_fecdecoding_vit.sample bbu8_fecdecoding_vit.data | less
