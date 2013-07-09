/*
 * test_svec-vsx.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: Haichuan Wang
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
