
EXAMPLE=dgemm
CPP_SRC=dgemm.cpp
RUN_ARGS=10
ITER=10

BASE_DIR=../../..
include ${BASE_DIR}/examples/common.mk

##XLC version
XLC_SRC=dgemm_intrinsics.cpp
XLC_FLAGS=-O2 -qaltivec -qarch=pwr7 -I${BASE_DIR}/include
xlc: cmpxlc
	./${EXAMPLE}_xlc_orig ${ITER}
	./${EXAMPLE}_xlc_intrinsics_simd ${ITER}

cmpxlc: ${XLC_SRC}
	xlc ${XLC_FLAGS} $< -o ${EXAMPLE}_xlc_orig
	xlc ${XLC_FLAGS} -DSIMD $< -o ${EXAMPLE}_xlc_intrinsics_simd

INTRINSICS_SRC=dgemm_intrinsics.cpp

intrinsics: cmpintrinsics
	./${EXAMPLE}_orig ${ITER}
	./${EXAMPLE}_intrinsics_simd ${ITER}

cmpintrinsics: ${INTRINSICS_SRC}
	${CXX} ${CXXFLAGS} $< -o ${EXAMPLE}_orig 
	${CXX} ${CXXFLAGS} -DSIMD $< -o ${EXAMPLE}_intrinsics_simd 



clean:
	rm -rf ${EXAMPLE} ${EXAMPLE}_tune \
	${EXAMPLE}_xlc_orig ${EXAMPLE}_xlc_intrinsics_simd \
	${EXAMPLE}_orig ${EXAMPLE}_intrinsics_simd


