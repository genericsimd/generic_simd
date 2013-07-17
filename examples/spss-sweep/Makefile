
EXAMPLE=sweep
CPP_SRC=sweep.cpp


include ../common.mk

##XLC version
XLC_SRC=sweep_xlc.cpp
XLC_FLAGS=-O2 -qaltivec -qarch=pwr7 -I${BASE_DIR}/include
xlc: cmpxlc
	./${EXAMPLE}_xlc_orig
	./${EXAMPLE}_xlc_split_scalar
	./${EXAMPLE}_xlc_split_simd
	./${EXAMPLE}_xlc_mat_org
	./${EXAMPLE}_xlc_unrolled_scalar
	./${EXAMPLE}_xlc_unrolled_simd

cmpxlc: ${XLC_SRC}
	xlc ${XLC_FLAGS} -DORIG $< -o ${EXAMPLE}_xlc_orig
	xlc ${XLC_FLAGS} -DSPLIT_SCALAR $< -o ${EXAMPLE}_xlc_split_scalar
	xlc ${XLC_FLAGS} -DSPLIT_SIMD $< -o ${EXAMPLE}_xlc_split_simd
	xlc ${XLC_FLAGS} -DMAT_ORIG $< -o ${EXAMPLE}_xlc_mat_org
	xlc ${XLC_FLAGS} -DUNROLLED_SCALAR $< -o ${EXAMPLE}_xlc_unrolled_scalar
	xlc ${XLC_FLAGS} -DUNROLLED_SIMD $< -o ${EXAMPLE}_xlc_unrolled_simd	

INTRINSICS_SRC=sweep_intrinsics.cpp

intrinsics: cmpintrinsics
	./${EXAMPLE}_orig
	./${EXAMPLE}_split_scalar
	./${EXAMPLE}_split_simd
	./${EXAMPLE}_mat_org
	./${EXAMPLE}_unrolled_scalar
	./${EXAMPLE}_unrolled_simd

cmpintrinsics: ${INTRINSICS_SRC}
	${CXX} ${CXXFLAGS} -DORIG $< -o ${EXAMPLE}_orig
	${CXX} ${CXXFLAGS} -DSPLIT_SCALAR $< -o ${EXAMPLE}_split_scalar
	${CXX} ${CXXFLAGS} -DSPLIT_SIMD $< -o ${EXAMPLE}_split_simd
	${CXX} ${CXXFLAGS} -DMAT_ORIG $< -o ${EXAMPLE}_mat_org
	${CXX} ${CXXFLAGS} -DUNROLLED_SCALAR $< -o ${EXAMPLE}_unrolled_scalar
	${CXX} ${CXXFLAGS} -DUNROLLED_SIMD $< -o ${EXAMPLE}_unrolled_simd	



