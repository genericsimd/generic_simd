

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
  ifeq (${PPC_ISA}, P7)
    PLATFORM = ppc64_P7
  else
  ifeq (${PPC_ISA}, P8)
    PLATFORM = ppc64_P9
    CCFLAGS += -D__POWER8
    CXXFLAGS += -D__POWER8
  else
    PLATFORM = ppc64_P9
    CCFLAGS += -D__POWER8 -D__POWER9
    CXXFLAGS += -D__POWER8 -D__POWER9
  endif
  endif
else
  PLATFORM=x86-64
endif


default: ${EXAMPLE}


${EXAMPLE}: ${EXAMPLE}.cpp
	${CXX} ${CXXFLAGS} $< -o $@

	
run: ${EXAMPLE}
	./$< ${RUN_ARGS}

clean:
	rm -f ${EXAMPLE}
	