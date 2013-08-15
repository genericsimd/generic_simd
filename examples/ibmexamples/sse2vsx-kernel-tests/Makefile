TARGETS = chanest_test demod-test fft viterbi

all: 
	for p in ${TARGETS}; do \
	  make -C $$p; \
	done

test:
	for p in ${TARGETS}; do \
	  make -C $$p test; \
	done

clean:
	for p in ${TARGETS}; do \
	  make -C $$p clean; \
	done
