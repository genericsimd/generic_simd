

ifeq (${BASE_DIR},)
  BASE_DIR=../..
endif


BITS=64
CC=gcc
CXX=g++ 
CCFLAGS+= -I${BASE_DIR}/include -O2 -m$(BITS)
CXXFLAGS+= -I${BASE_DIR}/include -O2  -m$(BITS)

#compile power vsx
CXXFLAGS+= -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast


default: ${EXAMPLE}


${EXAMPLE}: ${EXAMPLE}.cpp
	${CXX} ${CXXFLAGS} $< -o $@

	
run: ${EXAMPLE}
	./$< ${RUN_ARGS}
