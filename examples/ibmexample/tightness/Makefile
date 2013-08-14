
EXAMPLE=tightness
CPP_SRC=tightness.cpp
RUN_ARGS=-repeats 10
#MASS dir
MASS_DIR=/opt/ibm/mass/6.0/sles10/include
MASS_LIB_PATH=/opt/ibm/mass/6.0/sles10/lib64

include ../../common.mk

##XLC version
XLC_SRC=tightness_intrinsics.c
XLC_FLAGS=-O2 -qaltivec -qarch=pwr7 -q64 -I${BASE_DIR}/include -I${MASS_DIR} -lmass_simdp7_64 -L${MASS_LIB_PATH}
xlc: cmpxlc
	./${EXAMPLE}1_xlc_orig ${RUN_ARGS}
	./${EXAMPLE}2_xlc_orig ${RUN_ARGS}
	./${EXAMPLE}3_xlc_orig ${RUN_ARGS}
	./${EXAMPLE}4_xlc_orig ${RUN_ARGS}
	./${EXAMPLE}1_xlc_intrinsics_simd ${RUN_ARGS}
	./${EXAMPLE}2_xlc_intrinsics_simd ${RUN_ARGS}
	./${EXAMPLE}3_xlc_intrinsics_simd ${RUN_ARGS}
	./${EXAMPLE}4_xlc_intrinsics_simd ${RUN_ARGS}

cmpxlc: ${XLC_SRC}
	xlc ${XLC_FLAGS} -DTIGHTNESS_1 $< -o ${EXAMPLE}1_xlc_orig
	xlc ${XLC_FLAGS} -DTIGHTNESS_2 $< -o ${EXAMPLE}2_xlc_orig
	xlc ${XLC_FLAGS} -DTIGHTNESS_3 $< -o ${EXAMPLE}3_xlc_orig
	xlc ${XLC_FLAGS} -DTIGHTNESS_4 $< -o ${EXAMPLE}4_xlc_orig
	xlc ${XLC_FLAGS} -DSIMD -DTIGHTNESS_1 $< -o ${EXAMPLE}1_xlc_intrinsics_simd
	xlc ${XLC_FLAGS} -DSIMD -DTIGHTNESS_2 $< -o ${EXAMPLE}2_xlc_intrinsics_simd
	xlc ${XLC_FLAGS} -DSIMD -DTIGHTNESS_3 $< -o ${EXAMPLE}3_xlc_intrinsics_simd
	xlc ${XLC_FLAGS} -DSIMD -DTIGHTNESS_4 $< -o ${EXAMPLE}4_xlc_intrinsics_simd

INTRINSICS_SRC=tightness_intrinsics.c
CXXFLAGS+= -I${MASS_DIR} -lmass_simdp7_64 -L${MASS_LIB_PATH}

intrinsics: cmpintrinsics
	./${EXAMPLE}1_orig ${RUN_ARGS}
	./${EXAMPLE}2_orig ${RUN_ARGS}
	./${EXAMPLE}3_orig ${RUN_ARGS}
	./${EXAMPLE}4_orig ${RUN_ARGS}
# Cannot compile due to the different interfaces for xlc/gcc
#	./${EXAMPLE}1_intrinsics_simd ${RUN_ARGS}
#	./${EXAMPLE}2_intrinsics_simd ${RUN_ARGS}
#	./${EXAMPLE}3_intrinsics_simd ${RUN_ARGS}
#	./${EXAMPLE}4_intrinsics_simd ${RUN_ARGS}

cmpintrinsics: ${INTRINSICS_SRC}
	${CXX} ${CXXFLAGS} -DTIGHTNESS_1 $< -o ${EXAMPLE}1_orig 
	${CXX} ${CXXFLAGS} -DTIGHTNESS_2 $< -o ${EXAMPLE}2_orig 
	${CXX} ${CXXFLAGS} -DTIGHTNESS_3 $< -o ${EXAMPLE}3_orig 
	${CXX} ${CXXFLAGS} -DTIGHTNESS_4 $< -o ${EXAMPLE}4_orig 
# Cannot compile due to the different interfaces for xlc/gcc
#	${CXX} ${CXXFLAGS} -DSIMD -DTIGHTNESS_1 $< -o ${EXAMPLE}1_intrinsics_simd 
#	${CXX} ${CXXFLAGS} -DSIMD -DTIGHTNESS_2 $< -o ${EXAMPLE}2_intrinsics_simd 
#	${CXX} ${CXXFLAGS} -DSIMD -DTIGHTNESS_3 $< -o ${EXAMPLE}3_intrinsics_simd 
#	${CXX} ${CXXFLAGS} -DSIMD -DTIGHTNESS_4 $< -o ${EXAMPLE}4_intrinsics_simd 

tightness1:${CPP_SRC}
	${CXX} -DTIGHTNESS_1 ${CXXFLAGS} $< -o $@

tightness2:${CPP_SRC}
	${CXX} -DTIGHTNESS_2 ${CXXFLAGS} $< -o $@

tightness3:${CPP_SRC}
	${CXX} -DTIGHTNESS_3 ${CXXFLAGS} $< -o $@

tightness4:${CPP_SRC}
	${CXX} -DTIGHTNESS_4 ${CXXFLAGS} $< -o $@

runall: tightness1 tightness2 tightness3 tightness4
	./tightness1 ${RUN_ARGS}
	./tightness2 ${RUN_ARGS}
	./tightness3 ${RUN_ARGS}
	./tightness4 ${RUN_ARGS}

clean:
	rm -rf ${EXAMPLE} ${EXAMPLE}_tune \
	${EXAMPLE}1_xlc_orig ${EXAMPLE}1_xlc_intrinsics_simd \
	${EXAMPLE}2_xlc_orig ${EXAMPLE}2_xlc_intrinsics_simd \
	${EXAMPLE}3_xlc_orig ${EXAMPLE}3_xlc_intrinsics_simd \
	${EXAMPLE}4_xlc_orig ${EXAMPLE}4_xlc_intrinsics_simd \
	${EXAMPLE}1_orig ${EXAMPLE}1_intrinsics_simd \
	${EXAMPLE}2_orig ${EXAMPLE}2_intrinsics_simd \
	${EXAMPLE}3_orig ${EXAMPLE}3_intrinsics_simd \
	${EXAMPLE}4_orig ${EXAMPLE}4_intrinsics_simd \
	tightness1 \
	tightness2 \
	tightness3 \
	tightness4


