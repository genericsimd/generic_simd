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
 * test_lanes4.cpp
 *
 *  Created on: Jun 21, 2013
 *      Author: haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 */

#include <gtest/gtest.h>

#include <gsimd.h>
#define LANES 4
#include "test_utility.h"

#define EXPECT_VEC_EQ(v1, v2) EXPECT_TRUE(vec_all_eq(v1, v2))

typedef svec<4,void*> svec4_ptr;
typedef svec<4,bool> svec4_i1;
typedef svec<4,int8_t> svec4_i8;
typedef svec<4,uint8_t> svec4_u8;
typedef svec<4,int16_t> svec4_i16;
typedef svec<4,uint16_t> svec4_u16;
typedef svec<4,int32_t> svec4_i32;
typedef svec<4,uint32_t> svec4_u32;
typedef svec<4,int64_t> svec4_i64;
typedef svec<4,uint64_t> svec4_u64;
typedef svec<4,float> svec4_f;
typedef svec<4,double> svec4_d;


template<>
svec4_i1 random_vec<uint32_t, svec4_i1>(int maxValue) {
  svec4_i1 vec;
  for(int i = 0; i < LANES; i++) {
    uint32_t value = (rand() & 1) == 1 ? -1 : 0;
    vec[i] = value;
  }
  return vec;
}

/**
 * Test Constructors for all types
 */

#ifdef __ALTIVEC__ //these constructors uses power intrinsics

TEST(svec4_i1, ConstructorByScalars)
{
    svec4_i1 v1(1,0,1,0);
    __vector unsigned int t = { -1, 0, -1, 0};
    EXPECT_VEC_EQ(v1.v, t);
}
TEST(svec4_i8, ConstructorByScalars)
{
    svec4_i8 v1(100,0,-50,1);
    __vector signed char t = { 100, 0, -50, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    EXPECT_VEC_EQ(v1.v, t);
}
TEST(svec4_u8, ConstructorByScalars)
{
    svec4_u8 v1(100,0,150,1);
    __vector unsigned char t = { 100, 0, 150, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    EXPECT_VEC_EQ(v1.v, t);
}
TEST(svec4_i16, ConstructorByScalars)
{
    svec4_i16 v1(100,0,-50,1);
    __vector signed short t = { 100, 0, -50, 1, 0,0,0,0};
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_u16, ConstructorByScalars)
{
    svec4_u16 v1(100,0,150,1);
    __vector unsigned short t = { 100, 0, 150, 1, 0,0,0,0};
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i32, ConstructorByScalars)
{
    svec4_i32 v1(100,0,-50,1);
    __vector signed int t = { 100, 0, -50, 1};
    EXPECT_VEC_EQ(v1.v, t);
}


TEST(svec4_u32, ConstructorByScalars)
{
    svec4_u32 v1(100,0,150,1);
    __vector unsigned int t = { 100, 0, 150, 1};
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i64, ConstructorByScalars)
{
    svec4_i64 v1(100,0,-50,1);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_u64, ConstructorByScalars)
{
    svec4_u64 v1(100,0,150,1);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_f, ConstructorByScalars)
{
    svec4_f v1(100,0,-50,1.5);
    __vector float t = { 100, 0, -50, 1.5};
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_d, ConstructorByScalars)
{
    svec4_d v1(100,0,-50,1.5);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
}

TEST(svec4_i1, ConstructorByVector)
{
    __vector unsigned int t = { -1, 0, -1, 0};
    svec4_i1 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i8, ConstructorByVector)
{
    __vector signed char t = { 100, 0, -50, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    svec4_i8 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_u8, ConstructorByVector)
{
    __vector unsigned char t = { 100, 0, 150, 1, 0,0,0,0, 0,0,0,0, 0,0,0,0};
    svec4_u8 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i16, ConstructorByVector)
{
    __vector signed short t = { 100, 0, -50, 1, 0,0,0,0};
    svec4_i16 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_u16, ConstructorByVector)
{
    __vector unsigned short t = { 100, 0, 150, 1, 0,0,0,0};
    svec4_u16 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i32, ConstructorByVector)
{
    __vector signed int t = { 100, 0, -50, 1};
    svec4_i32 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_u32, ConstructorByVector)
{
    __vector unsigned int t = { 100, 0, 150, 1};
    svec4_u32 v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_i64, ConstructorByVector)
{
    __vector signed long long t1 = { 100, 0};
    __vector signed long long t2 = { -50, 1};
    svec4_i64 v1(t1, t2);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_u64, ConstructorByVector)
{
    __vector unsigned long long t1 = { 100, 0};
    __vector unsigned long long t2 = { 150, 1};
    svec4_u64 v1(t1, t2);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_f, ConstructorByVector)
{
    __vector float t = { 100, 0, -50, 1.5};
    svec4_f v1(t);
    EXPECT_VEC_EQ(v1.v, t);
}

TEST(svec4_d, ConstructorByVector)
{
    __vector double t1 = { 100, 0};
    __vector double t2 = { -50, 1.5};
    svec4_d v1(t1, t2);
    //no direct vector cmp available, had to use [] to do scalar cmp
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
}

#endif //VSX
/**
 * Test subscript for all types
 */

TEST(svec4_i1, SubScriptGet)
{
    svec4_i1 v1(1,0,1,0);
    EXPECT_TRUE(v1[0]);
    EXPECT_FALSE(v1[1]);
    EXPECT_TRUE(v1[2]);
    EXPECT_FALSE(v1[3]);
}

TEST(svec4_i1, SubScriptSet)
{
    svec4_i1 v1;
    v1[0] = 1;
    v1[1] = 0;
    v1[2] = 1;
    v1[3] = 0;
    EXPECT_TRUE(v1[0]);
    EXPECT_FALSE(v1[1]);
    EXPECT_TRUE(v1[2]);
    EXPECT_FALSE(v1[3]);
    svec_insert(&v1, 0, 0);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, 0);
    svec_insert(&v1, 3, 1);
    EXPECT_FALSE(v1[0]);
    EXPECT_TRUE(v1[1]);
    EXPECT_FALSE(v1[2]);
    EXPECT_TRUE(v1[3]);
}

TEST(svec4_i8, SubScriptGet)
{
    svec4_i8 v1(100,0,-50,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_i8, SubScriptSet)
{
    svec4_i8 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_u8, SubScriptGet)
{
    svec4_u8 v1(100,0,150,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_u8, SubScriptSet)
{
    svec4_u8 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = 150;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, 149);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), 149);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_i16, SubScriptGet)
{
    svec4_i16 v1(100,0,-50,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_i16, SubScriptSet)
{
    svec4_i16 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_u16, SubScriptGet)
{
    svec4_u16 v1(100,0,150,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_u16, SubScriptSet)
{
    svec4_u16 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = 150;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, 149);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), 149);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_i32, SubScriptGet)
{
    svec4_i32 v1(100,0,-50,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_i32, SubScriptSet)
{
    svec4_i32 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_u32, SubScriptGet)
{
    svec4_u32 v1(100,0,150,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, 149);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), 149);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_u32, SubScriptSet)
{
    svec4_u32 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = 150;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_i64, SubScriptGet)
{
    svec4_i64 v1(100,0,-50,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_i64, SubScriptSet)
{
    svec4_i64 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_u64, SubScriptGet)
{
    svec4_u64 v1(100,0,150,1);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
}

TEST(svec4_u64, SubScriptSet)
{
    svec4_u64 v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = 150;
    v1[3] = 1;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], 150);
    EXPECT_EQ(v1[3], 1);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, 149);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), 149);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_f, SubScriptGet)
{
    svec4_f v1(100,0,-50,1.5);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
}

TEST(svec4_f, SubScriptSet)
{
    svec4_f v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1.5;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

TEST(svec4_d, SubScriptGet)
{
    svec4_d v1(100,0,-50,1.5);
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
}

TEST(svec4_d, SubScriptSet)
{
    svec4_d v1;
    v1[0] = 100;
    v1[1] = 0;
    v1[2] = -50;
    v1[3] = 1.5;
    EXPECT_EQ(v1[0], 100);
    EXPECT_EQ(v1[1], 0);
    EXPECT_EQ(v1[2], -50);
    EXPECT_EQ(v1[3], 1.5);
    svec_insert(&v1, 0, 99);
    svec_insert(&v1, 1, 1);
    svec_insert(&v1, 2, -49);
    svec_insert(&v1, 3, 0);
    EXPECT_EQ(svec_extract(v1, 0), 99);
    EXPECT_EQ(svec_extract(v1, 1), 1);
    EXPECT_EQ(svec_extract(v1, 2), -49);
    EXPECT_EQ(svec_extract(v1, 3), 0);
}

/**
 * Test load/store for all types
 */
#define ALIGN_VLENGTH 16
static char mem[40] POST_ALIGN(ALIGN_VLENGTH);
//const arrays, int/also used as unsigned
static const int8_t cint8[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2, 0, 0, 0, 0 };
static const int16_t cint16[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2, 0, 0, 0, 0 };
static const int32_t cint32[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2, 0, 0, 0, 0 };
static const int64_t cint64[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2, 0, 0, 0, 0 };
static const float cfloat[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2 };
static const double cdouble[] POST_ALIGN(ALIGN_VLENGTH) = { -1, 0, 1, 2 };

TEST(svec4_i1, load_store)
{
    svec4_i1* p = (svec4_i1*)mem;
    svec4_i1 v1(1,0,1,0);
    v1.store(p);
    svec4_i1 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_i8, load_store)
{
    svec4_i8* p = (svec4_i8*)mem;
    svec4_i8 v1(100,0,-50,1);
    v1.store(p);
    svec4_i8 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_u8, load_store)
{
    svec4_u8* p = (svec4_u8*)mem;
    svec4_u8 v1(100,0,150,1);
    v1.store(p);
    svec4_u8 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_i16, load_store)
{
    svec4_i16* p = (svec4_i16*)mem;
    svec4_i16 v1(100,0,-50,1);
    v1.store(p);
    svec4_i16 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_u16, load_store)
{
    svec4_u16* p = (svec4_u16*)mem;
    svec4_u16 v1(100,0,150,1);
    v1.store(p);
    svec4_u16 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_i32, load_store)
{
    svec4_i32* p = (svec4_i32*)mem;
    svec4_i32 v1(100,0,-50,1);
    v1.store(p);
    svec4_i32 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_u32, load_store)
{
    svec4_u32* p = (svec4_u32*)mem;
    svec4_u32 v1(100,0,150,1);
    v1.store(p);
    svec4_u32 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_i64, load_store)
{
    svec4_i64* p = (svec4_i64*)mem;
    svec4_i64 v1(100,0,-50,1);
    v1.store(p);
    svec4_i64 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_u64, load_store)
{
    svec4_u64* p = (svec4_u64*)mem;
    svec4_u64 v1(100,0,150,1);
    v1.store(p);
    svec4_u64 v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_f, load_store)
{
    svec4_f* p = (svec4_f*)mem;
    svec4_f v1(100,0,-50,1.5);
    v1.store(p);
    svec4_f v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

TEST(svec4_d, load_store)
{
    svec4_d* p = (svec4_d*)mem;
    svec4_d v1(100,0,-50,1.5);
    v1.store(p);
    svec4_d v2 = svec_load(p);
    EXPECT_SVEC_EQ(v1, v2);
}

/**
 * Test select
 */
TEST(svec4_i1, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_i1 v0(1, 0, 1, 0);
    svec4_i1 v1(0, 1, 0, 1);
    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_i1(0,0,0,0));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_i1(1,1,1,1));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}


TEST(svec4_i8, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_i8 v0(1, 2, 3, 4);
    svec4_i8 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_i8(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_i8(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_u8, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_u8 v0(1, 2, 3, 4);
    svec4_u8 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_u8(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_u8(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_i16, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_i16 v0(1, 2, 3, 4);
    svec4_i16 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_i16(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_i16(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_u16, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_u16 v0(1, 2, 3, 4);
    svec4_u16 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_u16(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_u16(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_i32, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_i32 v0(1, 2, 3, 4);
    svec4_i32 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_i32(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_i32(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_u32, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_u32 v0(1, 2, 3, 4);
    svec4_u32 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_u32(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_u32(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_i64, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_i64 v0(1, 2, 3, 4);
    svec4_i64 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_i64(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_i64(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_u64, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_u64 v0(1, 2, 3, 4);
    svec4_u64 v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_u64(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_u64(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_f, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_f v0(1, 2, 3, 4);
    svec4_f v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_f(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_f(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

TEST(svec4_d, select) {
    svec4_i1 mask(0, 1, 0, 1);
    svec4_d v0(1, 2, 3, 4);
    svec4_d v1(10, 20, 30, 40);

    EXPECT_SVEC_EQ(svec_select(mask, v0, v1), svec4_d(10,2,30,4));
    EXPECT_SVEC_EQ(svec_select(~mask, v0, v1), svec4_d(1,20,3,40));
    EXPECT_SVEC_EQ(svec_select(true, v0, v1), v0 );
    EXPECT_SVEC_EQ(svec_select(false, v0, v1), v1 );
}

/**
 * Test mask type's operations
 */

TEST(svec4_i1, movmsk)
{
    svec4_i1 v1(1,0,1,0);
    uint64_t m1 = (1 | 4);
    EXPECT_TRUE(svec_movmsk(v1) == m1);
    uint64_t m2 = (2 | 8);
    EXPECT_FALSE(svec_movmsk(v1) == m2);
}

TEST(svec4_i1, any)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(0,0,0,0);
    EXPECT_TRUE(v1.any_true());
    EXPECT_FALSE(v2.any_true());
}

TEST(svec4_i1, all)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,1,1,1);
    EXPECT_TRUE(v2.all_true());
    EXPECT_FALSE(v1.all_true());
}

TEST(svec4_i1, none_true)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(0,0,0,0);
    EXPECT_TRUE(v2.none_true());
    EXPECT_FALSE(v1.none_true());
}

TEST(svec4_i1, and)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,1,1,0);
    EXPECT_SVEC_EQ(v1 & v2, svec4_i1(1,0,1,0));
    EXPECT_SVEC_EQ(v1 && v2, svec4_i1(1,0,1,0));
}

TEST(svec4_i1, or)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,1,1,0);
    EXPECT_SVEC_EQ((v1 | v2), svec4_i1(1, 1, 1, 0));
    EXPECT_SVEC_EQ((v1 || v2), svec4_i1(1, 1, 1, 0));
}

TEST(svec4_i1, xor)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,1,1,0);
    EXPECT_SVEC_EQ((v1 ^ v2), svec4_i1(0, 1, 0, 0));
}

TEST(svec4_i1, not)
{
    svec4_i1 v1(1,0,1,0);
    EXPECT_SVEC_EQ((~v1), svec4_i1(0, 1, 0, 1));
    EXPECT_SVEC_EQ((!v1), svec4_i1(0, 1, 0, 1));
}



TEST(svec4_i1, EqOp)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,0,1,0);
    svec4_i1 v3(0,1,0,1);
    EXPECT_TRUE((v1 == v2).all_true());
    EXPECT_FALSE((v1 == v3).all_true());
    EXPECT_TRUE((v1 == v3).none_true());
}

TEST(svec4_i1, NeOp)
{
    svec4_i1 v1(1,0,1,0);
    svec4_i1 v2(1,0,1,0);
    svec4_i1 v3(0,1,0,1);
    EXPECT_FALSE((v1 != v2).any_true());
    EXPECT_TRUE((v1 != v3).any_true());
    EXPECT_TRUE((v1 != v3).all_true());
}

TEST(svec4_i8, CMP)
{
    svec4_i8 v1(1, 0, -50, 0);
    svec4_i8 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_u8, CMP)
{
    svec4_u8 v1(1, 0, 150, 0);
    svec4_u8 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 0, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 0, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 1, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 1, 0));
}

TEST(svec4_i16, CMP)
{
    svec4_i16 v1(1, 0, -50, 0);
    svec4_i16 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_u16, CMP)
{
    svec4_u16 v1(1, 0, 150, 0);
    svec4_u16 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 0, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 0, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 1, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 1, 0));
}

TEST(svec4_i32, CMP)
{
    svec4_i32 v1(1, 0, -50, 0);
    svec4_i32 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_u32, CMP)
{
    svec4_u32 v1(1, 0, 150, 0);
    svec4_u32 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 0, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 0, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 1, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 1, 0));
}

TEST(svec4_i64, CMP)
{
    svec4_i64 v1(1, 0, -50, 0);
    svec4_i64 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_u64, CMP)
{
    svec4_u64 v1(1, 0, 150, 0);
    svec4_u64 v2(0, 0, 1,   1);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 0, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 0, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 1, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 1, 0));
}

TEST(svec4_f, CMP)
{
    svec4_f v1(1, 0, -50, 1.5);
    svec4_f v2(0, 0, 1,   1.51);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_d, CMP)
{
    svec4_d v1(1, 0, -50, 1.5);
    svec4_d v2(0, 0, 1,   1.51);
    EXPECT_SVEC_EQ(v1 == v2, svec4_i1(0, 1, 0, 0));
    EXPECT_SVEC_EQ(v1 != v2, svec4_i1(1, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 < v2, svec4_i1(0, 0, 1, 1));
    EXPECT_SVEC_EQ(v1 <= v2, svec4_i1(0, 1, 1, 1));
    EXPECT_SVEC_EQ(v1 > v2, svec4_i1(1, 0, 0, 0));
    EXPECT_SVEC_EQ(v1 >= v2, svec4_i1(1, 1, 0, 0));
}

TEST(svec4_i8, broadcast_rotate_shuffle)
{
    svec4_i8 v1(1, 0, -50, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_i8(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_i8(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_i8(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_i8(0, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_i8(0, 1, 0, -50));
}

TEST(svec4_u8, broadcast_rotate_shuffle)
{
    svec4_u8 v1(1, 0, 150, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_u8(150, 150, 150, 150));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_u8(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_u8(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_u8(0, 1, 0, 150));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_u8(0, 1, 0, 150));
}

TEST(svec4_i16, broadcast_rotate_shuffle)
{
    svec4_i16 v1(1, 0, -50, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_i16(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_i16(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_i16(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_i16(0, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_i16(0, 1, 0, -50));
}

TEST(svec4_u16, broadcast_rotate_shuffle)
{
    svec4_u16 v1(1, 0, 150, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_u16(150, 150, 150, 150));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_u16(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_u16(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_u16(0, 1, 0, 150));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_u16(0, 1, 0, 150));
}

TEST(svec4_i32, broadcast_rotate_shuffle)
{
    svec4_i32 v1(1, 0, -50, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_i32(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_i32(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_i32(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_i32(0, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_i32(0, 1, 0, -50));
}

TEST(svec4_u32, broadcast_rotate_shuffle)
{
    svec4_u32 v1(1, 0, 150, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_u32(150, 150, 150, 150));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_u32(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_u32(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_u32(0, 1, 0, 150));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_u32(0, 1, 0, 150));
}
TEST(svec4_i64, broadcast_rotate_shuffle)
{
    svec4_i64 v1(1, 0, -50, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_i64(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_i64(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_i64(0, -50, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_i64(0, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_i64(0, 1, 0, -50));
}

TEST(svec4_u64, broadcast_rotate_shuffle)
{
    svec4_u64 v1(1, 0, 150, 0);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_u64(150, 150, 150, 150));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_u64(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_u64(0, 150, 0, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_u64(0, 1, 0, 150));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_u64(0, 1, 0, 150));
}

TEST(svec4_f, broadcast_rotate_shuffle)
{
    svec4_f v1(1, 0, -50, 1.5);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_f(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_f(0, -50, 1.5, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_f(0, -50, 1.5, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_f(1.5, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_f(1.5, 1, 0, -50));
}

TEST(svec4_d, broadcast_rotate_shuffle)
{
    svec4_d v1(1, 0, -50, 1.5);
    EXPECT_SVEC_EQ(svec_broadcast(v1, 2), svec4_d(-50, -50, -50, -50));
    EXPECT_SVEC_EQ(svec_rotate(v1, 1), svec4_d(0, -50, 1.5, 1));
    EXPECT_SVEC_EQ(svec_rotate(v1, 5), svec4_d(0, -50, 1.5, 1));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(3,0,1,2)), svec4_d(1.5, 1, 0, -50));
    EXPECT_SVEC_EQ(svec_shuffle(v1, svec4_i32(7,4,5,6)), svec4_d(1.5, 1, 0, -50));
}

TEST(svec4_i1, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_i1(0), svec4_i1(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i1(1), svec4_i1(1, 1, 1, 1));
    EXPECT_SVEC_EQ(svec4_i1(0), svec4_i1(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i1(1), svec4_i1(1, 1, 1, 1));
}

TEST(svec4_i8, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_i8(0), svec4_i8(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i8(-15), svec4_i8(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i8(-15), svec4_i8(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i8(100), svec4_i8(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_i8::load_const(cint8), svec4_i8(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_i8::load_and_splat((int8_t*)cint8), svec4_i8(-1, -1, -1, -1));
}

TEST(svec4_u8, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_u8(0), svec4_u8(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_u8(30), svec4_u8(30, 30, 30, 30));
    EXPECT_SVEC_EQ(svec4_u8(15), svec4_u8(15, 15, 15, 15));
    EXPECT_SVEC_EQ(svec4_u8(228), svec4_u8(228, 228, 228, 228));
    EXPECT_SVEC_EQ(svec4_u8::load_const((const uint8_t*)cint8), svec4_u8(255, 255, 255, 255));
    EXPECT_SVEC_EQ(svec4_u8::load_and_splat((uint8_t*)cint8), svec4_u8(255, 255, 255, 255));
}

TEST(svec4_i16, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_i16(0), svec4_i16(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i16(-15), svec4_i16(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i16(-15), svec4_i16(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i16(100), svec4_i16(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_i16::load_const(cint16), svec4_i16(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_i16::load_and_splat((int16_t*)cint16), svec4_i16(-1, -1, -1, -1));
}

TEST(svec4_u16, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_u16(0), svec4_u16(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_u16(30), svec4_u16(30, 30, 30, 30));
    EXPECT_SVEC_EQ(svec4_u16(15), svec4_u16(15, 15, 15, 15));
    EXPECT_SVEC_EQ(svec4_u16(228), svec4_u16(228, 228, 228, 228));
    EXPECT_SVEC_EQ(svec4_u16::load_const((const uint16_t*)cint16), svec4_u16(65535, 65535, 65535, 65535));
    EXPECT_SVEC_EQ(svec4_u16::load_and_splat((uint16_t*)cint16), svec4_u16(65535, 65535, 65535, 65535));
}

TEST(svec4_i32, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_i32(0), svec4_i32(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i32(-15), svec4_i32(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i32(-15), svec4_i32(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i32(100), svec4_i32(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_i32::load_const(cint32), svec4_i32(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_i32::load_and_splat((int32_t*)cint32), svec4_i32(-1, -1, -1, -1));
}

TEST(svec4_u32, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_u32(0), svec4_u32(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_u32(30), svec4_u32(30, 30, 30, 30));
    EXPECT_SVEC_EQ(svec4_u32(15), svec4_u32(15, 15, 15, 15));
    EXPECT_SVEC_EQ(svec4_u32(228), svec4_u32(228, 228, 228, 228));
    EXPECT_SVEC_EQ(svec4_u32::load_const((const uint32_t*)cint32), svec4_u32(4294967295u, 4294967295u, 4294967295u, 4294967295u));
    EXPECT_SVEC_EQ(svec4_u32::load_and_splat((uint32_t*)cint32), svec4_u32(4294967295u, 4294967295u, 4294967295u, 4294967295u));
}

TEST(svec4_i64, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_i64(0), svec4_i64(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_i64(-15), svec4_i64(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i64(-15), svec4_i64(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_i64(100), svec4_i64(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_i64::load_const(cint64), svec4_i64(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_i64::load_and_splat((int64_t*)cint64), svec4_i64(-1, -1, -1, -1));
}

TEST(svec4_u64, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_u64(0), svec4_u64(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_u64(30), svec4_u64(30, 30, 30, 30));
    EXPECT_SVEC_EQ(svec4_u64(15), svec4_u64(15, 15, 15, 15));
    EXPECT_SVEC_EQ(svec4_u64(228), svec4_u64(228, 228, 228, 228));
    EXPECT_SVEC_EQ(svec4_u64::load_const((const uint64_t*)cint64), svec4_u64(18446744073709551615ul, 18446744073709551615ul, 18446744073709551615ul, 18446744073709551615ul));
    EXPECT_SVEC_EQ(svec4_u64::load_and_splat((uint64_t*)cint64), svec4_u64(18446744073709551615ul, 18446744073709551615ul, 18446744073709551615ul, 18446744073709551615ul));
}

TEST(svec4_f, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_f(0), svec4_f(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_f(-15), svec4_f(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_f(-8), svec4_f(-8, -8, -8, -8));
    EXPECT_SVEC_EQ(svec4_f(100), svec4_f(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_f::load_const(cfloat), svec4_f(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_f::load_and_splat((float*)cfloat), svec4_f(-1, -1, -1, -1));
}

TEST(svec4_d, zero_smear_load_const)
{
    EXPECT_SVEC_EQ(svec4_d(0), svec4_d(0, 0, 0, 0));
    EXPECT_SVEC_EQ(svec4_d(-15), svec4_d(-15, -15, -15, -15));
    EXPECT_SVEC_EQ(svec4_d(-8), svec4_d(-8, -8, -8, -8));
    EXPECT_SVEC_EQ(svec4_d(100), svec4_d(100, 100, 100, 100));
    EXPECT_SVEC_EQ(svec4_d::load_const(cdouble), svec4_d(-1, -1, -1, -1));
    EXPECT_SVEC_EQ(svec4_d::load_and_splat((double*)cdouble), svec4_d(-1, -1, -1, -1));
}


TEST(svec4_i8, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_i8 v0(0, 0, 1, 2);
    svec4_i8 v1(10, 20, 30, 40);
    svec4_i8 v2(0, 20, 30, 40);

    int8_t* src = (int8_t*)cint8;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_i8::gather(ptrs, mask), v0, mask);

    int8_t* dst = (int8_t*)mem;
    v0.store((svec4_i8*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i8*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_i8::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_i8::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,1,2,3);
    svec4_i64 off64(0,1,2,3);

    EXPECT_SVEC_MASKED_EQ(svec4_i8::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_i8::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_i8*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i8*)mem), v2);

    v0.store((svec4_i8*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i8*)mem), v2);

    //masked load/store
    svec4_i8* pmem = (svec4_i8*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_i8::masked_load((svec4_i8*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);

}

TEST(svec4_u8, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_u8 v0(0, 0, 1, 2);
    svec4_u8 v1(10, 20, 30, 40);
    svec4_u8 v2(0, 20, 30, 40);

    uint8_t* src = (uint8_t*)cint8;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_u8::gather(ptrs, mask), v0, mask);

    uint8_t* dst = (uint8_t*)mem;
    v0.store((svec4_u8*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u8*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_u8::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_u8::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,1,2,3);
    svec4_i64 off64(0,1,2,3);

    EXPECT_SVEC_MASKED_EQ(svec4_u8::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_u8::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_u8*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u8*)mem), v2);

    v0.store((svec4_u8*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u8*)mem), v2);

    //masked load/store
    svec4_u8* pmem = (svec4_u8*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_u8::masked_load((svec4_u8*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}


TEST(svec4_i16, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_i16 v0(0, 0, 1, 2);
    svec4_i16 v1(10, 20, 30, 40);
    svec4_i16 v2(0, 20, 30, 40);

    int16_t* src = (int16_t*)cint16;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_i16::gather(ptrs, mask), v0, mask);

    int16_t* dst = (int16_t*)mem;
    v0.store((svec4_i16*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i16*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_i16::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_i16::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,2,4,6);
    svec4_i64 off64(0,2,4,6);

    EXPECT_SVEC_MASKED_EQ(svec4_i16::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_i16::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_i16*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i16*)mem), v2);

    v0.store((svec4_i16*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i16*)mem), v2);

    //masked load/store
    svec4_i16* pmem = (svec4_i16*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_i16::masked_load((svec4_i16*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_u16, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_u16 v0(0, 0, 1, 2);
    svec4_u16 v1(10, 20, 30, 40);
    svec4_u16 v2(0, 20, 30, 40);

    uint16_t* src = (uint16_t*)cint16;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_u16::gather(ptrs, mask), v0, mask);

    uint16_t* dst = (uint16_t*)mem;
    v0.store((svec4_u16*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u16*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_u16::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_u16::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,2,4,6);
    svec4_i64 off64(0,2,4,6);

    EXPECT_SVEC_MASKED_EQ(svec4_u16::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_u16::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_u16*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u16*)mem), v2);

    v0.store((svec4_u16*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u16*)mem), v2);

    //masked load/store
    svec4_u16* pmem = (svec4_u16*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_u16::masked_load((svec4_u16*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}


TEST(svec4_i32, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_i32 v0(0, 0, 1, 2);
    svec4_i32 v1(10, 20, 30, 40);
    svec4_i32 v2(0, 20, 30, 40);

    int32_t* src = (int32_t*)cint32;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_i32::gather(ptrs, mask), v0, mask);

    int32_t* dst = (int32_t*)mem;
    v0.store((svec4_i32*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i32*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_i32::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_i32::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,4,8,12);
    svec4_i64 off64(0,4,8,12);

    EXPECT_SVEC_MASKED_EQ(svec4_i32::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_i32::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_i32*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i32*)mem), v2);

    v0.store((svec4_i32*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i32*)mem), v2);

    //masked load/store
    svec4_i32* pmem = (svec4_i32*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_i32::masked_load((svec4_i32*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_u32, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_u32 v0(0, 0, 1, 2);
    svec4_u32 v1(10, 20, 30, 40);
    svec4_u32 v2(0, 20, 30, 40);

    uint32_t* src = (uint32_t*)cint32;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_u32::gather(ptrs, mask), v0, mask);

    uint32_t* dst = (uint32_t*)mem;
    v0.store((svec4_u32*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u32*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_u32::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_u32::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,4,8,12);
    svec4_i64 off64(0,4,8,12);

    EXPECT_SVEC_MASKED_EQ(svec4_u32::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_u32::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_u32*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u32*)mem), v2);

    v0.store((svec4_u32*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u32*)mem), v2);

    //masked load/store
    svec4_u32* pmem = (svec4_u32*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_u32::masked_load((svec4_u32*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_i64, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_i64 v0(0, 0, 1, 2);
    svec4_i64 v1(10, 20, 30, 40);
    svec4_i64 v2(0, 20, 30, 40);

    int64_t* src = (int64_t*)cint64;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_i64::gather(ptrs, mask), v0, mask);

    int64_t* dst = (int64_t*)mem;
    v0.store((svec4_i64*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i64*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_i64::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_i64::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,8,16,24);
    svec4_i64 off64(0,8,16,24);

    EXPECT_SVEC_MASKED_EQ(svec4_i64::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_i64::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_i64*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i64*)mem), v2);

    v0.store((svec4_i64*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_i64*)mem), v2);

    //masked load/store
    svec4_i64* pmem = (svec4_i64*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_i64::masked_load((svec4_i64*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);

}

TEST(svec4_u64, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_u64 v0(0, 0, 1, 2);
    svec4_u64 v1(10, 20, 30, 40);
    svec4_u64 v2(0, 20, 30, 40);

    uint64_t* src = (uint64_t*)cint64;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_u64::gather(ptrs, mask), v0, mask);

    uint64_t* dst = (uint64_t*)mem;
    v0.store((svec4_u64*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u64*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_u64::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_u64::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,8,16,24);
    svec4_i64 off64(0,8,16,24);

    EXPECT_SVEC_MASKED_EQ(svec4_u64::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_u64::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_u64*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u64*)mem), v2);

    v0.store((svec4_u64*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_u64*)mem), v2);

    //masked load/store
    svec4_u64* pmem = (svec4_u64*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_u64::masked_load((svec4_u64*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_f, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_f v0(0, 0, 1, 2);
    svec4_f v1(10, 20, 30, 40);
    svec4_f v2(0, 20, 30, 40);

    float* src = (float*)cfloat;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_f::gather(ptrs, mask), v0, mask);

    float* dst = (float*)mem;
    v0.store((svec4_f*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_f*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_f::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_f::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,4,8,12);
    svec4_i64 off64(0,4,8,12);

    EXPECT_SVEC_MASKED_EQ(svec4_f::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_f::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_f*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_f*)mem), v2);

    v0.store((svec4_f*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_f*)mem), v2);

    //masked load/store
    svec4_f* pmem = (svec4_f*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_f::masked_load((svec4_f*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_d, gather_scatter)
{
    svec4_i1 mask(0,1,1,1);
    svec4_d v0(0, 0, 1, 2);
    svec4_d v1(10, 20, 30, 40);
    svec4_d v2(0, 20, 30, 40);

    double* src = (double*)cdouble;
    svec4_ptr ptrs(src, src+1, src+2, src+3);
    EXPECT_SVEC_MASKED_EQ(svec4_d::gather(ptrs, mask), v0, mask);

    double* dst = (double*)mem;
    v0.store((svec4_d*)mem); //org data
    svec4_ptr ptrs2(dst, dst+1, dst+2, dst+3);
    v1.scatter(ptrs2, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_d*)mem), v2);

    v0.scatter_stride(dst, (int64_t)1, (int64_t)1);
    EXPECT_SVEC_EQ(svec4_d::gather_stride(dst, 1, 1), v0);
    v1.scatter_stride(dst, 0, 1);
    EXPECT_SVEC_EQ(svec4_d::gather_stride(dst, (int64_t)0, (int64_t)1), v1);

    svec4_i32 off32(0,8,16,24);
    svec4_i64 off64(0,8,16,24);

    EXPECT_SVEC_MASKED_EQ(svec4_d::gather_base_offsets(src, 1, off32, mask), v0, mask);
    EXPECT_SVEC_MASKED_EQ(svec4_d::gather_base_offsets(src, 1, off64, mask), v0, mask);

    v0.store((svec4_d*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off32, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_d*)mem), v2);

    v0.store((svec4_d*)mem); //org data
    v1.scatter_base_offsets(dst, 1, off64, mask);
    EXPECT_SVEC_EQ(svec_load((svec4_d*)mem), v2);

    //masked load/store
    svec4_d* pmem = (svec4_d*) mem;
    EXPECT_SVEC_MASKED_EQ(svec4_d::masked_load((svec4_d*)src, mask), v0, mask);
    v0.store(pmem); //org data
    v1.masked_store(pmem, mask);
    EXPECT_SVEC_EQ(svec_load(pmem), v2);
}

TEST(svec4_i8, unary){
    svec4_i8 v0(2, -4, 8, -16);
    svec4_i8 v1(-2, 4, -8, 16);
    svec4_i8 v3(2, 4, 8, 16);
    EXPECT_SVEC_EQ(-v0, v1);
    EXPECT_SVEC_EQ(v0.abs(), v3);


}

TEST(svec4_i16, unary){
    svec4_i16 v0(2, -4, 8, -16);
    svec4_i16 v1(-2, 4, -8, 16);
    svec4_i16 v3(2, 4, 8, 16);
    EXPECT_SVEC_EQ(-v0, v1);
    EXPECT_SVEC_EQ(v0.abs(), v3);
}

TEST(svec4_i32, unary){
    svec4_i32 v0(2, -4, 8, -16);
    svec4_i32 v1(-2, 4, -8, 16);
    svec4_i32 v3(2, 4, 8, 16);
    EXPECT_SVEC_EQ(-v0, v1);
    EXPECT_SVEC_EQ(v0.abs(), v3);
}

TEST(svec4_i64, unary){
    svec4_i64 v0(2, -4, 8, -16);
    svec4_i64 v1(-2, 4, -8, 16);
    svec4_i64 v3(2, 4, 8, 16);
    EXPECT_SVEC_EQ(-v0, v1);
    EXPECT_SVEC_EQ(v0.abs(), v3);
}

TEST(svec4_f, unary){
    svec4_f v0(2.3, -4.6, 8.7, -16.2);
    svec4_f v1(-2.3, 4.6, -8.7, 16.2);
    svec4_f v3(2.3, 4.6, 8.7, 16.2);
    EXPECT_SVEC_EQ(-v0, v1);
    //EXPECT_SVEC_EQ(v0.abs(), v3);

    svec4_f v_round(roundf(2.3), roundf(-4.6), roundf(8.7), roundf(-16.2));
    svec4_f v_ceil(ceilf(2.3), ceilf(-4.6), ceilf(8.7), ceilf(-16.2));
    svec4_f v_floor(floorf(2.3), floorf(-4.6), floorf(8.7), floorf(-16.2));
    svec4_f v_sqrt(sqrtf(2.3), sqrtf(4.6), sqrtf(8.7), sqrtf(16.2));
    svec4_f v_rcp(1.0f/(2.3), 1.0f/(-4.6), 1.0f/(8.7), 1.0f/(-16.2));
    svec4_f v_rsqrt(1.0f/sqrtf(2.3), 1.0f/sqrtf(4.6), 1.0f/sqrtf(8.7), 1.0f/sqrtf(16.2));
    svec4_f v_log(logf(2.3), logf(4.6), logf(8.7), logf(16.2));

    EXPECT_SVEC_EQ(v0.round(), v_round);
    EXPECT_SVEC_EQ(v0.ceil(), v_ceil);
    EXPECT_SVEC_EQ(v0.floor(), v_floor);
    EXPECT_SVEC_FEQ(v3.sqrt(), v_sqrt);
    EXPECT_SVEC_FEQ(v0.rcp(), v_rcp);
    EXPECT_SVEC_FEQ(v3.rsqrt(), v_rsqrt);
    EXPECT_SVEC_FEQ(v3.log(), v_log);
}

TEST(svec4_d, unary){
    svec4_d v0(2.3, -4.6, 8.7, -16.2);
    svec4_d v1(-2.3, 4.6, -8.7, 16.2);
    svec4_d v3(2.3, 4.6, 8.7, 16.2);
    EXPECT_SVEC_EQ(-v0, v1);
    //EXPECT_SVEC_EQ(v0.abs(), v3);

    svec4_d v_round(round(2.3), round(-4.6), round(8.7), round(-16.2));
    svec4_d v_ceil(ceil(2.3), ceil(-4.6), ceil(8.7), ceil(-16.2));
    svec4_d v_floor(floor(2.3), floor(-4.6), floor(8.7), floor(-16.2));
    svec4_d v_sqrt(sqrt(2.3), sqrt(4.6), sqrt(8.7), sqrt(16.2));
    svec4_d v_rcp(1.0/(2.3), 1.0/(-4.6), 1.0/(8.7), 1.0/(-16.2));
    svec4_d v_rsqrt(1.0/sqrt(2.3), 1.0/sqrt(4.6), 1.0/sqrt(8.7), 1.0f/sqrtf(16.2));
    svec4_d v_log(log(2.3), log(4.6), log(8.7), log(16.2));

    EXPECT_SVEC_EQ(v0.round(), v_round);
    EXPECT_SVEC_EQ(v0.ceil(), v_ceil);
    EXPECT_SVEC_EQ(v0.floor(), v_floor);
    EXPECT_SVEC_FEQ(v3.sqrt(), v_sqrt);
    EXPECT_SVEC_FEQ(v0.rcp(), v_rcp);
    EXPECT_SVEC_FEQ(v3.rsqrt(), v_rsqrt);
    EXPECT_SVEC_FEQ(v3.log(), v_log);
}

TEST(svec4_i8, binary)
{
    svec4_i8 v0(2, 4, 8, 16);
    svec4_i8 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_i8 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_i8 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_i8 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_i8 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_i8(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_u8, binary)
{
    svec4_u8 v0(2, 4, 8, 16);
    svec4_u8 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_u8 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_u8 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_u8 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_u8 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_u8(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_i16, binary)
{
    svec4_i16 v0(2, 4, 8, 16);
    svec4_i16 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_i16 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_i16 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_i16 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_i16 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_i16(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_u16, binary)
{
    svec4_u16 v0(2, 4, 8, 16);
    svec4_u16 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_u16 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_u16 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_u16 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_u16 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_u16(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_i32, binary)
{
    svec4_i32 v0(2, 4, 8, 16);
    svec4_i32 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_i32 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_i32 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_i32 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_i32 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_i32(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_u32, binary)
{
    svec4_u32 v0(2, 4, 8, 16);
    svec4_u32 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_u32 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_u32 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_u32 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_u32 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_u32(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_i64, binary)
{
    svec4_i64 v0(2, 4, 8, 16);
    svec4_i64 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_i64 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_i64 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_i64 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_i64 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_i64(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_u64, binary)
{
    svec4_u64 v0(2, 4, 8, 16);
    svec4_u64 v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_u64 v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_u64 v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_u64 v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_u64 v_div(2/1, 4/2, 8/3, 16/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_u64(2/2, 2/4, 2/8, 2/16));
}

TEST(svec4_f, binary)
{
    svec4_f v0(2, 4, 8, 16);
    svec4_f v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_f v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_f v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_f v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_f v_div(2.0/1, 4.0/2, 8.0/3, 16.0/4), v_div_s(2.0/2, 4.0/2, 8.0/2, 16.0/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_f(2/2.0, 2/4.0, 2/8.0, 2/16.0));
}

TEST(svec4_d, binary)
{
    svec4_d v0(2, 4, 8, 16);
    svec4_d v1(1, 2, 3, 4);

    int8_t s = 2;

    svec4_d v_add(3, 6, 11, 20), v_add_s(4, 6, 10, 18);
    svec4_d v_sub(1, 2, 5, 12), v_sub_s(0, 2, 6, 14);
    svec4_d v_mul(2, 8, 24, 64), v_mul_s(4, 8, 16, 32);
    svec4_d v_div(2.0/1, 4.0/2, 8.0/3, 16.0/4), v_div_s(2/2, 4/2, 8/2, 16/2);

    EXPECT_SVEC_EQ(v0 + v1, v_add);
    EXPECT_SVEC_EQ(v0 + s, v_add_s);
    EXPECT_SVEC_EQ(s + v0, v_add_s);
    EXPECT_SVEC_EQ(v0 * v1, v_mul);
    EXPECT_SVEC_EQ(v0 * s, v_mul_s);
    EXPECT_SVEC_EQ(s * v0, v_mul_s);
    EXPECT_SVEC_EQ(v0 - v1, v_sub);
    EXPECT_SVEC_EQ(v0 - s, v_sub_s);
    EXPECT_SVEC_EQ(s - v0, -v_sub_s);
    EXPECT_SVEC_EQ(v0 / v1, v_div);
    EXPECT_SVEC_EQ(v0 / s, v_div_s);
    EXPECT_SVEC_EQ(s / v0, svec4_d(2/2.0, 2/4.0, 2/8.0, 2/16.0));
}

TEST(svec4_f, reduce)
{
  svec4_f v0(1, 2, 3, 4);
  svec4_f v1(10, 20, 30, 40);
  svec4_f v2(100, 200, 300, 400);
  svec4_f v3(1000, 2000, 3000, 4000);

  svec4_f sum = svec_preduce_add(v0, v1, v2, v3);
  EXPECT_SVEC_EQ(sum, svec4_f(10, 100, 1000, 10000));
}

TEST(svec4_d, reduce)
{
  svec4_d v0(1, 2, 3, 4);
  svec4_d v1(10, 20, 30, 40);
  svec4_d v2(100, 200, 300, 400);
  svec4_d v3(1000, 2000, 3000, 4000);

  svec4_d sum = svec_preduce_add(v0, v1, v2, v3);
  EXPECT_SVEC_EQ(sum, svec4_d(10, 100, 1000, 10000));
}


TEST(svec4_i8, shift)
{
  svec4_i8 v = random_vec<int8_t, svec4_i8>();
  svec4_u8 sv = random_vec<uint8_t, svec4_u8>(8);
  int s = random() % 8;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_i8, svec4_u8>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_i8, svec4_u8>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_i8>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_i8>(v, s)));
}

TEST(svec4_u8, shift)
{
  svec4_u8 v = random_vec<uint8_t, svec4_u8>();
  svec4_u8 sv = random_vec<uint8_t, svec4_u8>(8);
  int s = random() % 8;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_u8, svec4_u8>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_u8, svec4_u8>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_u8>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_u8>(v, s)));
}

TEST(svec4_i16, shift)
{
  svec4_i16 v = random_vec<int16_t, svec4_i16>();
  svec4_u16 sv = random_vec<uint16_t, svec4_u16>(16);
  int s = random() % 16;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_i16, svec4_u16>(v, sv)));
//  DUMP(v);
//  DUMP(sv);
//  DUMP(svec_shr(v, sv));
//  DUMP((ref_shr<svec4_i16, svec4_u16>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_i16, svec4_u16>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_i16>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_i16>(v, s)));
}

TEST(svec4_u16, shift)
{
  svec4_u16 v = random_vec<uint16_t, svec4_u16>();
  svec4_u16 sv = random_vec<uint16_t, svec4_u16>(16);
  int s = random() % 16;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_u16, svec4_u16>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_u16, svec4_u16>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_u16>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_u16>(v, s)));
}

TEST(svec4_i32, shift)
{
  svec4_i32 v = random_vec<int32_t, svec4_i32>();
  svec4_u32 sv = random_vec<uint32_t, svec4_u32>(32);
  int s = random() % 32;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_i32, svec4_u32>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_i32, svec4_u32>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_i32>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_i32>(v, s)));
}

TEST(svec4_u32, shift)
{
  svec4_u32 v = random_vec<uint32_t, svec4_u32>();
  svec4_u32 sv = random_vec<uint32_t, svec4_u32>(32);
  int s = random() % 32;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_u32, svec4_u32>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_u32, svec4_u32>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_u32>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_u32>(v, s)));
}

TEST(svec4_i64, shift)
{
  svec4_i64 v = random_vec<int64_t, svec4_i64>();
  svec4_u64 sv = random_vec<uint64_t, svec4_u64>(64);
  int s = random() % 64;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_i64, svec4_u64>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_i64, svec4_u64>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_i64>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_i64>(v, s)));
}

TEST(svec4_u64, shift)
{
  svec4_u64 v = random_vec<uint64_t, svec4_u64>();
  svec4_u64 sv = random_vec<uint64_t, svec4_u64>(64);
  int s = random() % 64;
  EXPECT_SVEC_EQ(svec_shl(v, sv), (ref_shl<svec4_u64, svec4_u64>(v, sv)));
  EXPECT_SVEC_EQ(svec_shr(v, sv), (ref_shr<svec4_u64, svec4_u64>(v, sv)));
  EXPECT_SVEC_EQ(svec_shl(v, s), (ref_shl<svec4_u64>(v, s)));
  EXPECT_SVEC_EQ(svec_shr(v, s), (ref_shr<svec4_u64>(v, s)));
}

TEST(svec4_i1, cast)
{
  svec4_i1 v = random_vec<uint32_t, svec4_i1>();
//  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_i1, svec4_ii, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_i1, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_i1, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_i1, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_i1, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_i1, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_i1, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_i1, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_i1, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_i1, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_i1, svec4_d, double>(v)));
}

TEST(svec4_i8, cast)
{
  svec4_i8 v = random_vec<int8_t, svec4_i8>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_i8, svec4_i1, uint32_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_i8, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_i8, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_i8, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_i8, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_i8, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_i8, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_i8, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_i8, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_i8, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_i8, svec4_d, double>(v)));
}

TEST(svec4_u8, cast)
{
  svec4_u8 v = random_vec<uint8_t, svec4_u8>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_u8, svec4_i1, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_u8, svec4_i8, int8_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_u8, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_u8, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_u8, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_u8, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_u8, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_u8, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_u8, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_u8, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_u8, svec4_d, double>(v)));
}


TEST(svec4_i16, cast)
{
  svec4_i16 v = random_vec<int16_t, svec4_i16>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_i16, svec4_i1, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_i16, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_i16, svec4_u8, uint8_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_i16, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_i16, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_i16, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_i16, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_i16, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_i16, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_i16, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_i16, svec4_d, double>(v)));
}

TEST(svec4_u16, cast)
{
  svec4_u16 v = random_vec<uint16_t, svec4_u16>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_u16, svec4_i1, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_u16, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_u16, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_u16, svec4_i16, int16_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_u16, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_u16, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_u16, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_u16, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_u16, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_u16, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_u16, svec4_d, double>(v)));
}

TEST(svec4_i32, cast)
{
  svec4_i32 v = random_vec<int32_t, svec4_i32>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_i32, svec4_i1, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_i32, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_i32, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_i32, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_i32, svec4_u16, uint16_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_i32, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_i32, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_i32, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_i32, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_i32, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_i32, svec4_d, double>(v)));
}

TEST(svec4_u32, cast)
{
  svec4_u32 v = random_vec<uint32_t, svec4_u32>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_u32, svec4_i1, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_u32, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_u32, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_u32, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_u32, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_u32, svec4_i32, int32_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_u32, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_u32, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_u32, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_u32, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_u32, svec4_d, double>(v)));
}


TEST(svec4_i64, cast)
{
  svec4_i64 v = random_vec<int64_t, svec4_i64>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_i64, svec4_i1, bool>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_i64, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_i64, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_i64, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_i64, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_i64, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_i64, svec4_u32, uint32_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_i64, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_i64, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_i64, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_i64, svec4_d, double>(v)));
}

TEST(svec4_u64, cast)
{
  svec4_u64 v = random_vec<uint64_t, svec4_u64>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_u64, svec4_i1, bool>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_u64, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_u64, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_u64, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_u64, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_u64, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_u64, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_u64, svec4_i64, int64_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_u64, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_u64, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_u64, svec4_d, double>(v)));
}


TEST(svec4_f, cast)
{
  svec4_f v = random_vec<float, svec4_f>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_f, svec4_i1, bool>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_f, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_f, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_f, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_f, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_f, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_f, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_f, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_f, svec4_u64, uint64_t>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_f, svec4_f, float>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_f, svec4_d, double>(v)));
}

TEST(svec4_d, cast)
{
  svec4_d v = random_vec<double, svec4_d>();
  EXPECT_SVEC_EQ(svec_cast<svec4_i1>(v), (ref_cast<svec4_d, svec4_i1, bool>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i8>(v), (ref_cast<svec4_d, svec4_i8, int8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u8>(v), (ref_cast<svec4_d, svec4_u8, uint8_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i16>(v), (ref_cast<svec4_d, svec4_i16, int16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u16>(v), (ref_cast<svec4_d, svec4_u16, uint16_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i32>(v), (ref_cast<svec4_d, svec4_i32, int32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u32>(v), (ref_cast<svec4_d, svec4_u32, uint32_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_i64>(v), (ref_cast<svec4_d, svec4_i64, int64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_u64>(v), (ref_cast<svec4_d, svec4_u64, uint64_t>(v)));
  EXPECT_SVEC_EQ(svec_cast<svec4_f>(v), (ref_cast<svec4_d, svec4_f, float>(v)));
//  EXPECT_SVEC_EQ(svec_cast<svec4_d>(v), (ref_cast<svec4_d, svec4_d, double>(v)));
}


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
