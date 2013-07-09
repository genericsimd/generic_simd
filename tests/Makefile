

GTEST_DIR=../gtest-1.6.0
TARGETS=power_vsx4 test_svec


CXXFLAGS=-I../include -Wno-int-to-pointer-cast -flax-vector-conversions -g
#disable assert failure
CXXFLAGS+= -DNDEBUG
########### The below section is used for new test on gtest framework

# Need build the libgtest.a
libgtest.a:
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	ar -rv $@ gtest-all.o

power_vsx4: power_vsx4.cpp libgtest.a ../include/power_vsx4.h
	g++ -I${GTEST_DIR}/include -l pthread -mvsx ${CXXFLAGS} $^ -o $@


power_vsx4_test: power_vsx4
	./$<

test_svec: test_svec.cpp libgtest.a ../include/svec-vsx.h
	g++ -I${GTEST_DIR}/include -l pthread -mvsx ${CXXFLAGS} $^ -o $@

clean:
	rm -f *.o core *~ ${TARGETS} *.output *.a
