GXX = g++
#GXX = /opt/at5.0/bin/g++

GTEST_DIR=gtest-1.6.0
TARGETS=vsx4 generic4 test_svec


CXXFLAGS=-I../include -Wno-int-to-pointer-cast -flax-vector-conversions -g
#disable assert failure
CXXFLAGS+= -DNDEBUG
########### The below section is used for new test on gtest framework

# Need build the libgtest.a
libgtest.a:
	${GXX} -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv $@ gtest-all.o

vsx4: test_lanes4.cpp libgtest.a ../include/power_vsx4.h
	${GXX} -I${GTEST_DIR}/include -l pthread -mvsx ${CXXFLAGS} $^ -o $@

generic4: test_lanes4.cpp libgtest.a ../include/generic4.h
	${GXX} -I${GTEST_DIR}/include -l pthread ${CXXFLAGS} $^ -o $@
	
generic8: test_lanes8.cpp libgtest.a ../include/generic8.h
	${GXX} -I${GTEST_DIR}/include -l pthread ${CXXFLAGS} $^ -o $@

sse4: test_lanes4.cpp libgtest.a ../include/sse4.h
	${GXX} -I${GTEST_DIR}/include -l pthread -msse4.2 ${CXXFLAGS} $^ -o $@

test_svec: test_svec.cpp libgtest.a ../include/svec-vsx.h
	${GXX} -I${GTEST_DIR}/include -l pthread -mvsx ${CXXFLAGS} $^ -o $@


codegen: codegen.cpp
	${GXX} -mno-vrsave -mvsx ${CXXFLAGS} $< -O2 -S
#	${GXX} -mno-vrsave -mvsx ${CXXFLAGS} $< -O2 -c -g -Wa,-a,-ad > $@.cs

clean:
	rm -f *.o *.exe core *~ ${TARGETS} *.output *.a
