/**
Copyright 2012 the Generic SIMD Intrinsic Library project authors. All rights reserved.

Copyright IBM Corp. 2013, 2013. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
   * Neither the name of IBM Corp. nor the names of its contributors may be
     used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * test_svec-vsx.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 */

#include <gtest/gtest.h>
#include <svec-vsx.h>

using namespace vsx;

#define EXPECT_VEC_EQ(v1, v2) EXPECT_TRUE(vec_all_eq(v1, v2))
#define DUMP(v) std::cout << #v << ":" << (v) << std::endl



TEST(svec_bool, ConstructorByScalars)
{

    __vector unsigned int t = { -1, 0, -1, 0};
    svec_bool<4> v1(1, 0, 1, 0);
    EXPECT_VEC_EQ(v1.reg(0), t);

    svec_bool<8> v2(1, 0, 1, 0, 1, 0, 1, 0);
    EXPECT_VEC_EQ(v2.reg(0), t);
    EXPECT_VEC_EQ(v2.reg(1), t);

    bool a[] = {1, 0, 1, 0};
    svec_bool<4> v3(a);
    EXPECT_VEC_EQ(v3.reg(0), t);

    __vector uint32_t va[] = { t, t };
    svec_bool<8> v4(va);
    EXPECT_VEC_EQ(v4.reg(0), t);
    EXPECT_VEC_EQ(v4.reg(1), t);
}

TEST(svec_8, ConstructorByScalars)
{

  svec_i8<4> v1(100,0,-50,1);
  __vector int8_t t = { 100, 0, -50, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0};
  DUMP(v1);
  EXPECT_VEC_EQ(v1.reg(0), t);

}


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
