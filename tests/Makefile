

GTEST_DIR=../gtest-1.6.0
TARGETS=vsx4 generic4 test_svec


CXXFLAGS=-I../include -Wno-int-to-pointer-cast -flax-vector-conversions -g
#disable assert failure
CXXFLAGS+= -DNDEBUG
########### The below section is used for new test on gtest framework

# Need build the libgtest.a
libgtest.a:
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv $@ gtest-all.o

vsx4: test_lanes4.cpp libgtest.a ../include/power_vsx4.h
	g++ -I${GTEST_DIR}/include -l pthread -mvsx -DVSX ${CXXFLAGS} $^ -o $@

generic4: test_lanes4.cpp libgtest.a ../include/generic4.h
	g++ -I${GTEST_DIR}/include -l pthread ${CXXFLAGS} $^ -o $@

sse4: test_lanes4.cpp libgtest.a ../include/sse4.h
	g++ -I${GTEST_DIR}/include -l pthread -msse4.2 -DSSE ${CXXFLAGS} $^ -o $@

test: vsx4 generic4 sse4
	./vsx4
	./sse4
	./generic4

test_svec: test_svec.cpp libgtest.a ../include/svec-vsx.h
	g++ -I${GTEST_DIR}/include -l pthread -mvsx ${CXXFLAGS} $^ -o $@


codegen: codegen.cpp
	g++ -mno-vrsave -mvsx ${CXXFLAGS} $< -O2 -S
#	g++ -mno-vrsave -mvsx ${CXXFLAGS} $< -O2 -c -g -Wa,-a,-ad > $@.cs

clean:
	rm -f *.o core *~ ${TARGETS} *.output *.a
