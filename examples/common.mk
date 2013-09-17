.PONY: %.perfexe %.perfcollect %.perfbr %.perhw


ifeq (${BASE_DIR},)
  BASE_DIR=../..
endif


BITS=64
CC=gcc
CXX=g++ 
CCFLAGS+= -I${BASE_DIR}/include -O2 -m$(BITS)
CXXFLAGS+= -I${BASE_DIR}/include -O2  -m$(BITS)

CCFLAGS+= -Wno-int-to-pointer-cast
CXXFLAGS+= -Wno-int-to-pointer-cast
##########################################################
# Platform specific options
# ppc64 or intel
##########################################################

MACHINE=$(shell uname -m)
ifeq ($(firstword $(filter ppc64,$(MACHINE))),ppc64)
  CXXFLAGS += -mno-vrsave -mvsx -flax-vector-conversions -mcpu=power7
  CCFLAGS += -mno-vrsave -mvsx -flax-vector-conversions -mcpu=power7
  ifeq (${PPC_ISA}, P8)
    PLATFORM = ppc64_P8
    CCFLAGS += -D__POWER8
    CXXFLAGS += -D__POWER8
  else
    PLATFORM = ppc64_P7
  endif
else
  PLATFORM=x86-64
  CCFLAGS += -msse4.2
  CXXFLAGS += -msse4.2
endif


default: ${EXAMPLE}


${EXAMPLE}: ${EXAMPLE}.cpp
	${CXX} ${CXXFLAGS} $< -o $@

	
run: ${EXAMPLE}
	./$< ${RUN_ARGS}
	
${EXAMPLE}_tune: ${EXAMPLE}_tune.cpp
	${CXX} ${CXXFLAGS} $< -o $@

tune: ${EXAMPLE}_tune
	./$< ${RUN_ARGS}

TMP=__perf.tmp

#special for collecting all perf data
%.perf: %.perfbr %.perficache %.perfdcache %.perfllc
	@echo "end"
	@rm -f ${TMP}




%.perfhw: CXXFLAGS+= -DPERF_HW
%.perfhw: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@
	./$@ | tee ${TMP}
	@grep "HPM Event" ${TMP} | tail -1
	@grep "HPM Values" ${TMP}


%.perfbr: CXXFLAGS+= -DPERF_BR
%.perfbr: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@
	./$@ | tee ${TMP}
	@grep "HPM Event" ${TMP} | tail -1
	@grep "HPM Values" ${TMP}
	
%.perficache: CXXFLAGS+= -DPERF_ICACHE
%.perficache: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@
	./$@ | tee ${TMP}
	@grep "HPM Event" ${TMP} | tail -1
	@grep "HPM Values" ${TMP}


%.perfdcache: CXXFLAGS+= -DPERF_DCACHE
%.perfdcache: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@
	./$@ | tee ${TMP}
	@grep "HPM Event" ${TMP} | tail -1
	@grep "HPM Values" ${TMP}

%.perfllc: CXXFLAGS+= -DPERF_LLC
%.perfllc: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@
	./$@ | tee ${TMP}
	@grep "HPM Event" ${TMP} | tail -1
	@grep "HPM Values" ${TMP}


clean:
	rm -f ${EXAMPLE} ${EXAMPLE}_tune
	
