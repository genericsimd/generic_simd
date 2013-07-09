/**
 * @file power_vsx4.h
 * @date  Jun 21, 2013
 * @author Peng Wu (pengwu@us.ibm.com)
 * @author Ilie G Tanase (igtanase@us.ibm.com)
 * @author Mauricio J Serrano (mserrano@us.ibm.com)
 * @author Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 * @brief SIMD LANES=4 interfaces implemented by Power vsx
 *
 * The source file is organized as follows
 * I. all types definition, with class function incorporated
 * II. data operation interfaces
 *   1. load/store
 *   2. select
 *   3. broadcast/rotate/shuffle/smear/setzero
 *   4. gather/scatter
 *   5. load const, smear const, load and splat
 *   6. masked load/masked store
 * III. Mask type (i1) interfaces
 *   1. mask construction
 *   2. bit operations
 * IV. General data operation interfaces
 *   1. Unary
 *   2. Math unary
 *   3. Binary
 *   4. Ternary
 *   5. Compare
 *   6. Max/Min
 *   7. Reduce
 *   8. Cast
 *
 *  The current implementation requires gcc to compile. Reasons
 *  1) In order to implement the splat interface efficiently, the code uses
 *     gcc built-in function  __builtin_constant_p(exp).
 */


/*
  SIMD Generic Interface incorporates code from the Intel ISPC intrinsics implementation,
   which is covered by the following license:

  --------------------------------------------------------------------------------------
  Copyright (c) 2010-2012, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POWER_VSX4_H_
#define POWER_VSX4_H_

#include <stdint.h>
#include <math.h>
#include <altivec.h>
#include <assert.h>
#include <iostream>

#include "intrinsics_utility.h"
#include "power9_intrinsics.h"

namespace vsx {

#define LANES 4

//////////////////////////////////////////////////////////////
//
// Constructor Section
//
//////////////////////////////////////////////////////////////


/**
 * @brief macros to define compare methods
 * == and != are available for all the types.
 */
#define VEC_CMP_EQ_DECL(VTYPE, MTYPE)     \
  FORCEINLINE MTYPE operator==(VTYPE a); \
  FORCEINLINE MTYPE operator!=(VTYPE a);

#define VEC_CMP_DECL(VTYPE, MTYPE)     \
  FORCEINLINE MTYPE operator==(VTYPE a); \
  FORCEINLINE MTYPE operator!=(VTYPE a); \
  FORCEINLINE MTYPE operator<(VTYPE a); \
  FORCEINLINE MTYPE operator<=(VTYPE a); \
  FORCEINLINE MTYPE operator>(VTYPE a); \
  FORCEINLINE MTYPE operator>=(VTYPE a); \

/**
 * @brief macros for unary. note "-" means neg or complement
 */
#define VEC_UNARY_DECL(VTYPE, STYPE) \
  FORCEINLINE VTYPE operator-(); \
  FORCEINLINE STYPE reduce_add(); \
  FORCEINLINE STYPE reduce_max(); \
  FORCEINLINE STYPE reduce_min();

/**
 * @brief macros for binary operations.
 */
#define VEC_BIN_DECL(VTYPE, STYPE)    \
  FORCEINLINE VTYPE operator+(VTYPE a); \
  FORCEINLINE VTYPE operator+(STYPE s); \
  FORCEINLINE VTYPE operator-(VTYPE a); \
  FORCEINLINE VTYPE operator-(STYPE s); \
  FORCEINLINE VTYPE operator*(VTYPE a); \
  FORCEINLINE VTYPE operator*(STYPE s); \
  FORCEINLINE VTYPE operator/(VTYPE a); \
  FORCEINLINE VTYPE operator/(STYPE s);


/**
 * @brief macros for mask vector class's class method
 */
#define MVEC_CLASS_METHOD_DECL(VTYPE, STYPE) \
  static FORCEINLINE VTYPE load(VTYPE* p); \
  FORCEINLINE void store(VTYPE* p);


/**
 * @brief macros for non-mask i8 - double types's method
 */
#define VEC_CLASS_METHOD_DECL(VTYPE, STYPE) \
  MVEC_CLASS_METHOD_DECL(VTYPE, STYPE); \
  VEC_UNARY_DECL(VTYPE, STYPE);\
  VEC_BIN_DECL(VTYPE, STYPE);\
  static FORCEINLINE VTYPE masked_load(VTYPE* p, svec4_i1 mask); \
  FORCEINLINE void masked_store(VTYPE* p, svec4_i1 mask); \
  static FORCEINLINE VTYPE load_const(const STYPE* p); \
  static FORCEINLINE VTYPE load_and_splat(STYPE* p); \
  static FORCEINLINE VTYPE gather(svec4_ptr ptrs, svec4_i1 mask);\
  FORCEINLINE void scatter(svec4_ptr ptrs, svec4_i1 mask); \
  static FORCEINLINE VTYPE gather_base_offsets(STYPE* b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask);\
  static FORCEINLINE VTYPE gather_base_offsets(STYPE* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask);\
  FORCEINLINE void scatter_base_offsets(STYPE* b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask); \
  FORCEINLINE void scatter_base_offsets(STYPE* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask); \
  FORCEINLINE VTYPE broadcast(int32_t index); \
  FORCEINLINE VTYPE rotate(int32_t index); \
  FORCEINLINE VTYPE shuffle(svec4_i32 index);

/**
 * @brief macros method definition for integer vector only
 * Note: shift's operator can only be unsigned vector
 */
#define VEC_INT_CLASS_METHOD_DECL(VTYPE, VTYPE_B, STYPE) \
  FORCEINLINE VTYPE operator|(VTYPE a); \
  FORCEINLINE VTYPE operator&(VTYPE a); \
  FORCEINLINE VTYPE operator^(VTYPE a); \
  FORCEINLINE VTYPE operator<<(VTYPE_B a); \
  FORCEINLINE VTYPE operator<<(STYPE s); \
  FORCEINLINE VTYPE operator>>(VTYPE_B a); \
  FORCEINLINE VTYPE operator>>(STYPE s); \
  FORCEINLINE VTYPE operator%(VTYPE a); \
  FORCEINLINE VTYPE operator%(STYPE s);

/**
 * brief macros for float/double math unary operations
 */
#define VEC_FLOAT_CLASS_METHOD_DECL(VTYPE) \
  FORCEINLINE VTYPE round(); \
  FORCEINLINE VTYPE floor(); \
  FORCEINLINE VTYPE ceil(); \
  FORCEINLINE VTYPE sqrt(); \
  FORCEINLINE VTYPE rcp(); \
  FORCEINLINE VTYPE rsqrt(); \
  FORCEINLINE VTYPE exp(); \
  FORCEINLINE VTYPE log(); \
  FORCEINLINE VTYPE pow(VTYPE a);

struct svec4_ptr;
struct svec4_i8;
struct svec4_u8;
struct svec4_i16;
struct svec4_u16;
struct svec4_i32;
struct svec4_u32;
struct svec4_i64;
struct svec4_u64;
struct svec4_f;
struct svec4_d;


/**
 * @brief i1 vector type, typically as a mask vector, or bool vector.
 * The back-end storage is a __vector unsigned int. So one __vector can
 * hold 4 masks. The reason to choose  __vector unsigned int as back storage is that the bool
 * vector is frequently used in i32/u32/float vector operations.
 * In these cases, the mask vector doesn't need pack or unpack operation.
 */
struct svec4_i1 {

    __vector unsigned int v; //!< use __vector unsigned int v for storage

    /**
     * @brief default constructor. Return an undefined svec4_i1.
     */
    FORCEINLINE svec4_i1() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector unsigned int. Each scalar in the vector should be either 0 or -1(0XFFFFFFFF).
     * @return a mask vector whose value is from the vv.
     */
    FORCEINLINE svec4_i1(__vector unsigned int vv) : v(vv) { }
    /**
     * @brief Construct a vector mask with four scalar values.
     * @return a mask vector with the specified scalar values.
     */
    FORCEINLINE svec4_i1(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        __vector unsigned int t = { a ? -1 : 0, b ? -1 : 0, c ? -1 : 0, d ? -1 : 0 };
        v = t;
    }
    /**
     * @brief Construct a vector mask with one scalar value. All the elements are the same as the input value.
     * @return a mask vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_i1( uint32_t a) {
      if(__builtin_constant_p(a)){
        v = (a!=0) ? vec_splat_s32(-1) : vec_splat_s32(0);
      } else {
        INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear i1");
        __vector unsigned int t = { a ? -1 : 0, a ? -1 : 0, a ? -1 : 0, a ? -1 : 0 };
        v = t;
      }
    }

    /**
     * @brief [] subscript to directly access one element in the vector
     * Note: For svec4_i1 [] operator in the left(assign case), -1 must be used to represent true in the right.
     * \code{.cpp}
     *   svec4_i1 a;
     *   a[0] = 0; //assign the 1st element as false;
     *   a[1] = -1; //assign the 2nd element as true.
     * \endcode
     */
    SUBSCRIPT_FUNC(uint32_t);
    COUT_FUNC_I1(svec4_i1, LANES);

    VEC_CMP_EQ_DECL(svec4_i1, svec4_i1);
    MVEC_CLASS_METHOD_DECL(svec4_i1, uint32_t);

    FORCEINLINE bool any_true();
    FORCEINLINE bool all_true();
    FORCEINLINE bool none_true();

    FORCEINLINE svec4_i1 operator|(svec4_i1);
    FORCEINLINE svec4_i1 operator&(svec4_i1 a);
    FORCEINLINE svec4_i1 operator^(svec4_i1 a);
    FORCEINLINE svec4_i1 operator~();

};

/**
 * @brief signed char vector, with 4 elements in the vector.
 */
struct svec4_i8 {
    __vector signed char v;

    /**
     * @brief Default constructor, return an undefined svec4_i8.
     */
    FORCEINLINE svec4_i8() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector signed char.
     * @return a signed char vector, whose value is from the vv.
     */
    FORCEINLINE svec4_i8(__vector signed char vv) : v(vv) {  }
    /**
     * @brief Construct a signed char vector with four scalar values.
     * @return a signed char vector with the specified scalar values.
     */
    FORCEINLINE svec4_i8(int8_t a, int8_t b, int8_t c, int8_t d) {
        __vector signed char t = {a,b,c,d,0,0,0,0,
                                  0,0,0,0,0,0,0,0};
        v = t;
    }
    /**
     * @brief Construct a signed char vector with one scalar value. All the elements are the same as the input value.
     * @return a signed char vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_i8( int8_t a) {
      if(__builtin_constant_p(a) && (a <= 15) && (a >= -16)){
         v = vec_splat_s8(a); //will gen one instr.vspltisb
      } else {
        INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear i8");
        __vector signed char t = {a,a,a,a,0,0,0,0,
                                  0,0,0,0,0,0,0,0};
        v = t;
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(int8_t);
    COUT_FUNC_I8(svec4_i8, LANES);

    VEC_CMP_DECL(svec4_i8, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_i8, int8_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_i8, svec4_u8, int8_t);

};

/**
 * @brief unsigned char vector, with 4 elements in the vector
 */
struct svec4_u8 {
    __vector unsigned char v;
    /**
     * @brief Default constructor, return an undefined svec4_u8.
     */
    FORCEINLINE svec4_u8() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector unsigned char.
     * @return an unsigned char vector, whose value is from the vv.
     */
    FORCEINLINE svec4_u8(__vector unsigned char vv) : v(vv) {  }
    /**
     * @brief Construct an unsigned char vector with four scalar values.
     * @return an unsigned char vector with the specified scalar values.
     */
    FORCEINLINE svec4_u8(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
        __vector unsigned char t = {a,b,c,d,0,0,0,0,
                                    0,0,0,0,0,0,0,0};
        v = t;
    }
    /**
     * @brief Construct a unsigned char vector with one scalar value. All the elements are the same as the input value.
     * @return an unsigned char vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_u8(uint8_t a) {
      if(__builtin_constant_p(a) && (a <= 15)){
         v = vec_splat_u8(a); //will gen one instr.vspltisb
      } else {
        INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear u8");
        __vector unsigned char t = {a,a,a,a,0,0,0,0,
                                    0,0,0,0,0,0,0,0};
        v = t;
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(uint8_t);
    COUT_FUNC_I8(svec4_u8, LANES);

    VEC_CMP_DECL(svec4_u8, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_u8, uint8_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_u8, svec4_u8, uint8_t);
};

struct svec4_i16 {
    __vector signed short v;
    /**
     * @brief Default constructor, return a undefined svec4_i16.
     */
    FORCEINLINE svec4_i16() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector signed short.
     * @return a signed short vector, whose value is from the vv.
     */
    FORCEINLINE svec4_i16(__vector signed short vv) : v(vv) {  }
    /**
     * @brief Construct a signed short vector with four scalar values.
     * @return a signed short vector with the specified scalar values.
     */
    FORCEINLINE svec4_i16(int16_t a, int16_t b, int16_t c, int16_t d) {
        __vector signed short t = {a,b,c,d, 0,0,0,0};
        v = t;
    }
    /**
     * @brief Construct a signed short vector with one scalar value. All the elements are the same as the input value.
     * @return a signed short vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_i16( int16_t a) {
      if(__builtin_constant_p(a) && (a <= 15) && (a >= -16)){
         v = vec_splat_s16(a); //will gen one instr.vspltisb
      } else {
        INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear i16");
        __vector signed short t = {a,a,a,a, 0,0,0,0};
        v = t;
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(int16_t);
    COUT_FUNC(svec4_i16, LANES);

    VEC_CMP_DECL(svec4_i16, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_i16, int16_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_i16, svec4_u16, int16_t);

};

struct svec4_u16 {
    __vector unsigned short v;
    /**
     * @brief Default constructor, return a undefined svec4_u16.
     */
    FORCEINLINE svec4_u16() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector signed char.
     * @return an unsigned char vector, whose value is from the vv.
     */
    FORCEINLINE svec4_u16(__vector unsigned short vv) : v(vv) {  }
    /**
     * @brief Construct an unsigned short vector with four scalar values.
     * @return an unsigned short vector with the specified scalar values.
     */
    FORCEINLINE svec4_u16(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
        __vector unsigned short t = {a,b,c,d, 0,0,0,0};
        v = t;
    }
    /**
     * @brief Construct an unsigned char vector with one scalar value. All the elements are the same as the input value.
     * @return an unsigned short vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_u16( uint16_t a) {
      if(__builtin_constant_p(a) && (a <= 15)){
         v = vec_splat_u16(a); //will gen one instr.vspltisb
      } else {
        INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear u16");
        __vector unsigned short t = {a,a,a,a, 0,0,0,0};
        v = t;
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(uint16_t);
    COUT_FUNC(svec4_u16, LANES);

    VEC_CMP_DECL(svec4_u16, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_u16, uint16_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_u16, svec4_u16, uint16_t);

};



struct svec4_i32 {
    __vector signed int v;
    /**
     * @brief Default constructor, return a undefined svec4_i32.
     */
    FORCEINLINE svec4_i32() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector signed int.
     * @return a signed int vector, whose value is from the vv.
     */
    FORCEINLINE svec4_i32(__vector signed int vv) : v(vv) {  }
    /**
     * @brief Construct a signed int vector with four scalar values.
     * @return a signed int vector with the specified scalar values.
     */
    FORCEINLINE svec4_i32(int a, int b, int c, int d) {
      __vector signed int t = {a,b,c,d};
        v = t;
    }
    /**
     * @brief Construct a signed int vector with one scalar value. All the elements are the same as the input value.
     * @return a signed int vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_i32(int32_t a) {
      if(__builtin_constant_p(a)){
        if((a <= 15) && (a >= -16)) {
          v = vec_splat_s32(a); //will gen one instr.vspltisb
        } else {
          INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear i32");
          __vector signed int t = {a,a,a,a};
            v = t;
        }
      } else { //non-const
#ifdef __POWER8
        v = vec_smear_p8(a);
#else
        int32_t* p = &a;
        __vector signed int register x = vec_vsx_ld(0, p);
        v = vec_splat_p7(x, 0);
#endif
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(int32_t);
    COUT_FUNC(svec4_i32, LANES);

    VEC_CMP_DECL(svec4_i32, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_i32, int32_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_i32, svec4_u32, int32_t);

};

struct svec4_u32 {
    __vector unsigned int v;
    /**
     * @brief Default constructor, return a undefined svec4_u32.
     */
    FORCEINLINE svec4_u32() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector unsigned int.
     * @return an unsigned int vector, whose value is from the vv.
     */
    FORCEINLINE svec4_u32(__vector unsigned int vv) : v(vv) {  }
    /**
     * @brief Construct an unsigned int vector with four scalar values.
     * @return an unsigned int vector with the specified scalar values.
     */
    FORCEINLINE svec4_u32(int a, int b, int c, int d) {
      __vector unsigned int t = {a,b,c,d};
        v = t;
    }
    /**
     * @brief Construct an unsigned int vector with one scalar value. All the elements are the same as the input value.
     * @return an unsigned int vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_u32( uint32_t a) {
      if(__builtin_constant_p(a)){
        if((a <= 15)) {
          v = vec_splat_u32(a); //will gen one instr.vspltisb
        } else {
          INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear u32");
          __vector unsigned int t = {a,a,a,a};
          v = t;
        }
      } else { //non-const
#ifdef __POWER8
        v = vec_smear_p8(a);
#else
        uint32_t* p = &a;
        __vector unsigned int register x = vec_vsx_ld(0, p);
        v = vec_splat_p7((__vector signed)x, 0);
#endif
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(uint32_t);
    COUT_FUNC(svec4_u32, LANES);

    VEC_CMP_DECL(svec4_u32, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_u32, uint32_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_u32, svec4_u32, uint32_t);
};


struct svec4_i64 {
    __vector signed long long v[2];
    /**
     * @brief Default constructor, return a undefined svec4_i64.
     */
    FORCEINLINE svec4_i64() { }
    /**
     * @brief For internal use only. Construct svec4_i64 with two __vector signed long long values
     * @return a signed long long vector, whose value is from the vv.
     */
    FORCEINLINE svec4_i64(__vector signed long long a, __vector signed long long b){
        v[0] = a;
        v[1] = b;
    }
    /**
     * @brief Construct a signed long long vector with four scalar values.
     * @return a signed long long vector with the specified scalar values.
     */
    FORCEINLINE svec4_i64(int64_t a, int64_t b, int64_t c, int64_t d) {
      __vector signed long long t1 = {a,b};
      __vector signed long long t2 = {c,d};
        v[0] = t1;
        v[1] = t2;
    }
    /**
     * @brief Construct a signed long long vector with one scalar value. All the elements are the same as the input value.
     * @return a signed long long vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_i64( int64_t a) {
      if(__builtin_constant_p(a)){
#ifdef __POWER8
        if ((a >= -16l) && (a <= 15l)) {
          const int iv = (int)a;
          __vector signed int x = {iv,iv,iv,iv};
          __vector signed long long t = vec_unpackh_p8(x);
          v[0] = v[1] = t;
        } else
#endif
        if(a == 0) {
          __vector signed long long r1 = (__vector signed long long)vec_splat_s32(0);
          v[0] = v[1] = r1;
        } else {
          __vector long long x = {a,a};
          v[0] = v[1] = x;
        }
      } else {
#if 0 // not accepted def __POWER9
        __vector signed long long r =  vec_smear_i64_p9(a);
        v[0] = v[1] =r;
#else
#ifdef __POWER8
        __vector unsigned long long r = vec_smear_i64_p8(a);
        v[0] = v[1] = r;
#else
        int64_t* p = &a;
        __vector signed long long r = vec_smear_i64_p7((long long*)p);
        v[0] = v[1] = r;
#endif // __POWER8
#endif // __POWER9
      } //non const
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(int64_t);
    COUT_FUNC(svec4_i64, LANES);

    VEC_CMP_DECL(svec4_i64, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_i64, int64_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_i64, svec4_u64, int64_t);
};

struct svec4_u64 {
    __vector unsigned long long v[2];
    /**
     * @brief Default constructor, return a undefined svec4_u64.
     */
    FORCEINLINE svec4_u64() { }
    /**
     * @brief For internal use only. Construct svec4_u64 with two __vector unsigned long long values
     * @return an unsigned long long vector, whose value is from a and b.
     */
    FORCEINLINE svec4_u64(__vector unsigned long long a, __vector unsigned long long b){
        v[0] = a;
        v[1] = b;
    }
    /**
     * @brief Construct an unsigned long long vector with four scalar values.
     * @return an unsigned long long vector with the specified scalar values.
     */
    FORCEINLINE svec4_u64(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
      __vector unsigned long long t1 = {a,b};
      __vector unsigned long long t2 = {c,d};
        v[0] = t1;
        v[1] = t2;
    }
    /**
     * @brief Construct an unsigned long long vector with one scalar value. All the elements are the same as the input value.
     * @return an unsigned long long vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_u64( uint64_t a) {
      if(__builtin_constant_p(a)){
#ifdef __POWER8
        if ((a >= 0ul) && (a <= 31ul)) {
          const int iv = (int)v;
          __vector signed int x = {iv,iv,iv,iv};
          __vector unsigned long long t = vec_unpackh_p8(x);
          v[0] = v[1] = t;
        } else
#endif
        if(a == 0) {
          __vector unsigned long long r1 = (__vector unsigned long long)vec_splat_u32(0);
          v[0] = v[1] = r1, r1;
        } else {
          __vector unsigned long long x = {a,a};
          v[0] = v[1] = x;
        }
      } else {
#if 0 // not accepted def __POWER9
        __vector unsigned long long r =  vec_smear_i64_p9(a);
        v[0] = v[1] = r;
#else
#ifdef __POWER8
        __vector unsigned long long r = vec_smear_i64_p8(a);
        v[0] = v[1] = r;
#else
        uint64_t* p = &a;
        __vector unsigned long long r = vec_smear_i64_p7((long long*)p);
        v[0] = v[1] = r;
#endif // __POWER8
#endif // __POWER9
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(uint64_t);
    COUT_FUNC(svec4_u64, LANES);

    VEC_CMP_DECL(svec4_u64, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_u64, uint64_t);
    VEC_INT_CLASS_METHOD_DECL(svec4_u64, svec4_u64, uint64_t);
};


struct svec4_f {
    __vector float v;
    /**
     * @brief Default constructor, return a undefined svec4_f.
     */
    FORCEINLINE svec4_f() { }
    /**
     * @brief For internal use only.
     * @param vv a __vector float.
     * @return a float vector, whose value is from the vv.
     */
    FORCEINLINE svec4_f(__vector float vv) : v(vv) {  }
    /**
     * @brief Construct a float vector with four scalar values.
     * @return a float vector with the specified scalar values.
     */
    FORCEINLINE svec4_f(float a, float b, float c, float d) {
      __vector float t = {a,b,c,d};
        v = t;
    }
    /**
     * @brief Construct a float  vector with one scalar value. All the elements are the same as the input value.
     * @return a float vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_f( float a) {
      if(__builtin_constant_p(a)){
        if(a == 0) {
          v = (__vector float) vec_splat_s32(0);
        } else {
          float p; int iv;
          p = 1.0; iv = (int)(p*a);
          if (( (((float)iv)/p) == a ) && (iv >= -16) && (iv <= 15)) {
            v = vec_ctf(vec_splat_s32(iv),0);
          } else {
            p = 2.0; iv = (int)(p*a);
            if (( (((float)iv)/p) == a ) && (iv >= -16) && (iv <= 15)) {
              v = vec_ctf(vec_splat_s32(iv),1);
            } else {
              p = 4.0; iv = (int)(p*a);
              if (( (((float)iv)/p) == a ) && (iv >= -16) && (iv <= 15)) {
                v = vec_ctf(vec_splat_s32(iv),2);
              } else {
                //no one instr solution.
                __vector float t = {a,a,a,a};
                v = t;
              }
            }
          } //non zero const
        }
      } else { //none const
#ifdef __POWER8
        v = vec_smear_p8(a);
#else
        float* p = &a;
        __vector float register x = vec_vsx_ld(0, p);
        v = vec_splat_p7(x, 0);
#endif
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(float);
    COUT_FUNC(svec4_f, LANES);

    VEC_CMP_DECL(svec4_f, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_f, float);
    VEC_FLOAT_CLASS_METHOD_DECL(svec4_f);

};

struct svec4_d {
    __vector double v[2];
    /**
     * @brief Default constructor, return a undefined svec4_d.
     */
    FORCEINLINE svec4_d() { }
    /**
     * @brief For internal use only. Construct svec4_d with two __vector double values
     * @return a double vector, whose value is from a and b.
     */
    FORCEINLINE svec4_d(__vector double a, __vector double b){
        v[0] = a;
        v[1] = b;
    }
    /**
     * @brief Construct a double  vector with four scalar values.
     * @return a double vector with the specified scalar values.
     */
    FORCEINLINE svec4_d(double a, double b, double c, double d) {
      __vector double t1 = {a,b};
      __vector double t2 = {c,d};
        v[0] = t1;
        v[1] = t2;
    }
    /**
     * @brief Construct a double  vector with one scalar value. All the elements are the same as the input value.
     * @return a double vector. Each element is the same as the input a.
     */
    FORCEINLINE svec4_d( double a) {
      if(__builtin_constant_p(a)){
        if(a == 0) {
          __vector double r1 = (__vector double)vec_splat_s32(0);
          v[0] = v[1] = r1;
        } else {
          __vector double t = vec_smear_p7(a);
          v[0] = v[1] = t;
        }
      } else {
        __vector double t = vec_smear_p7(a);
        v[0] = v[1] = t;
      }
    }
    /**
     * @brief [] subscript to directly access (set or get) one element in the vector.
     */
    SUBSCRIPT_FUNC(double);
    COUT_FUNC(svec4_d, LANES);

    VEC_CMP_DECL(svec4_d, svec4_i1);
    VEC_CLASS_METHOD_DECL(svec4_d, double);
    VEC_FLOAT_CLASS_METHOD_DECL(svec4_d);
};


//////////////////////////////////////////////////////////////
//
// Data operation interfaces
//
//////////////////////////////////////////////////////////////

//
//// 1. Extract / Insert
//
///**
// * @brief macros for svec's insert extract method implementation
// * The implementation is based on vector type's subscript operator
// */
//#define INSERT_EXTRACT(VTYPE, STYPE)                                  \
//  static FORCEINLINE STYPE svec_extract_element(VTYPE v, int index) {    \
//    return v[index];                      \
//  }                                                                     \
//  static FORCEINLINE void svec_insert_element(VTYPE *v, int index, STYPE val) { \
//    (*v)[index] = val;                      \
//  }
//
///**
// * @brief macros for fixed index (0,1,2,3) insert extract method implementation
// */
//#define INSERT_EXTRACT_INDEX(VTYPE, STYPE)                \
//  static FORCEINLINE STYPE svec_extract_element0(VTYPE v) {          \
//    INC_STATS_NAME(STATS_EXTRACT, 1, "extract0");                      \
//    return ((STYPE *)&v)[0];                          \
//  }                                   \
//  static FORCEINLINE STYPE svec_extract_element1(VTYPE v) {          \
//    INC_STATS_NAME(STATS_EXTRACT, 1, "extract1");                      \
//    return ((STYPE *)&v)[1];                          \
//  }                                   \
//  static FORCEINLINE STYPE svec_extract_element2(VTYPE v) {          \
//    INC_STATS_NAME(STATS_EXTRACT, 1, "extract2");                      \
//    return ((STYPE *)&v)[2];                          \
//  }                                   \
//  static FORCEINLINE STYPE svec_extract_element3(VTYPE v) {          \
//    INC_STATS_NAME(STATS_EXTRACT, 1, "extract3");                      \
//    return ((STYPE *)&v)[3];                          \
//  }
//
//INSERT_EXTRACT(svec4_i1, uint32_t);
//INSERT_EXTRACT(svec4_i8, int8_t);
//INSERT_EXTRACT(svec4_u8, uint8_t);
//INSERT_EXTRACT(svec4_i16, int16_t);
//INSERT_EXTRACT(svec4_u16, uint16_t);
//INSERT_EXTRACT(svec4_i32, int32_t);
//INSERT_EXTRACT(svec4_u32, uint32_t);
//INSERT_EXTRACT(svec4_i64, int64_t);
//INSERT_EXTRACT(svec4_u64, uint64_t);
//INSERT_EXTRACT(svec4_f, float);
//INSERT_EXTRACT(svec4_d, double);

// 1. Load / Store

/**
 * @brief macros for svec's load and store method implementation
 */
#define LOAD_STORE(VTYPE, STYPE)                       \
static FORCEINLINE VTYPE svec_load(const VTYPE *p) {      \
    STYPE *ptr = (STYPE *)p;                           \
    VTYPE ret;                                         \
    INC_STATS_NAME(STATS_LOAD_SLOW, 1, #VTYPE);             \
    for (int i = 0; i < 4; ++i)                \
      ret[i] = ptr[i];             \
    return ret;                                        \
}                                                      \
static FORCEINLINE void svec_store(VTYPE *p, VTYPE v) {   \
  STYPE *ptr = (STYPE *)p;                 \
  INC_STATS_NAME(STATS_STORE_SLOW, 1, #VTYPE);              \
  for (int i = 0; i < 4; ++i)                  \
    ptr[i] = v[i];               \
}

/**
 * @brief load a vector from an address
 */
static FORCEINLINE svec4_i1 svec_load(const svec4_i1 *p) {
  return *((__vector unsigned int *)p);
}

/**
 * @brief store a vector to an address
 */
static FORCEINLINE void svec_store(svec4_i1 *p, svec4_i1 v) {
  *((__vector unsigned int*)p) = v.v;
}

static FORCEINLINE svec4_i8 svec_load(const svec4_i8 *p) {
  return vec_vsx_ld(0, (signed int*)p);
}

static FORCEINLINE void svec_store(svec4_i8 *p, svec4_i8 v) {
  vec_vsx_st(v.v, 0, (signed char*)p);
}

static FORCEINLINE svec4_u8 svec_load(const svec4_u8 *p) {
  return vec_vsx_ld(0, (signed int*)p);
}

static FORCEINLINE void svec_store(svec4_u8 *p, svec4_u8 v) {
  vec_vsx_st(v.v, 0, (unsigned char*)p);
}

/**
 * @brief load and store for svec4_i16/svec4_u16.
 * Generic implementation. Should be slow
 */
LOAD_STORE(svec4_i16, int16_t);

LOAD_STORE(svec4_u16, uint16_t);

static FORCEINLINE svec4_i32 svec_load(const svec4_i32 *p) {
  return *((__vector signed int *)p);
}

static FORCEINLINE void svec_store(svec4_i32 *p, svec4_i32 v) {
  *((__vector signed int*)p) = v.v;
}

static FORCEINLINE svec4_u32 svec_load(const svec4_u32 *p) {
  return *((__vector unsigned int *)p);
}

static FORCEINLINE void svec_store(svec4_u32 *p, svec4_u32 v) {
  *((__vector unsigned int*)p) = v.v;
}

static FORCEINLINE svec4_i64 svec_load(const svec4_i64 *p) {
  __vector signed long long v0 = *(((__vector signed long long *)p)+0);
  __vector signed long long v1 = *(((__vector signed long long *)p)+1);
  return svec4_i64(v0,v1);
}

static FORCEINLINE void svec_store(svec4_i64 *p, svec4_i64 v) {
  *(((__vector signed long long *)p)+0) = v.v[0];
  *(((__vector signed long long *)p)+1) = v.v[1];
}

static FORCEINLINE svec4_u64 svec_load(const svec4_u64 *p) {
  __vector unsigned long long v0 = *(((__vector unsigned long long *)p)+0);
  __vector unsigned long long v1 = *(((__vector unsigned long long *)p)+1);
  return svec4_u64(v0,v1);
}

static FORCEINLINE svec4_f svec_load(const svec4_f *p) {
  return *((__vector float *)p);
}

static FORCEINLINE void svec_store(svec4_f *p, svec4_f v) {
  *((__vector float*)p) = v.v;
}

static FORCEINLINE void svec_store(svec4_u64 *p, svec4_u64 v) {
  *(((__vector unsigned long long *)p)+0) = v.v[0];
  *(((__vector unsigned long long *)p)+1) = v.v[1];
}


static FORCEINLINE svec4_d svec_load(const svec4_d *p) {
  __vector double v0 = *(((__vector double *)p)+0);
  __vector double v1 = *(((__vector double *)p)+1);
  return svec4_d(v0,v1);
}

static FORCEINLINE void svec_store(svec4_d *p, svec4_d v) {
  *(((__vector double *)p)+0) = v.v[0];
  *(((__vector double *)p)+1) = v.v[1];
}

// 3. Select

/**
 * @brief macros for svec's select by mask vector method generic implementation
 */
#define SELECT(TYPE)                                                \
static FORCEINLINE TYPE svec_select(svec4_i1 mask, TYPE a, TYPE b) {  \
    TYPE ret;                                                       \
    NOT_IMPLEMENTED("select");                      \
    return ret;                                                     \
}

/**
 * @brief construct c by selecting elements from two input vectors according to the mask
 * @param mask the mask vector. True: select elements from a; False: select elements from b.
 * @param a the 1st vector. The elements will be selected if the corresponding mask element is true.
 * @param b the 2nd vector. The elements will be selected if the corresponding mask element is true.
 * @return A new vector whose elements is selected from a or b
 */
FORCEINLINE svec4_i1 svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b) {
    return vec_sel(b.v, a.v, mask.v);
}

/**
 * @brief select of svec4_i8 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_i8 svec_select(svec4_i1 mask, svec4_i8 a, svec4_i8 b) {
    __vector unsigned int tsi=vec_splat_s32(0);//{0,0,0,0};
    __vector unsigned char t = vec_pack(vec_pack(mask.v,tsi),(vector unsigned short)tsi);
    return vec_sel(b.v, a.v, t);
}

/**
 * @brief select of svec4_u8 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_u8 svec_select(svec4_i1 mask, svec4_u8 a, svec4_u8 b) {
    __vector unsigned int tsi=vec_splat_u32(0);//{0,0,0,0};
    __vector unsigned char t = vec_pack(vec_pack(mask.v,tsi),(vector unsigned short)tsi);
    return vec_sel(b.v, a.v, t);
}

/**
 * @brief select of svec4_i16 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_i16 svec_select(svec4_i1 mask, svec4_i16 a, svec4_i16 b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i16");
    int16_t v0 = mask[0] ? a[0] : b[0];
    int16_t v1 = mask[1] ? a[1] : b[1];
    int16_t v2 = mask[2] ? a[2] : b[2];
    int16_t v3 = mask[3] ? a[3] : b[3];
    return svec4_i16(v0, v1, v2, v3);
}

/**
 * @brief select of svec4_u16 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_u16 svec_select(svec4_i1 mask, svec4_u16 a, svec4_u16 b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u16");
    uint16_t v0 = mask[0] ? a[0] : b[0];
    uint16_t v1 = mask[1] ? a[1] : b[1];
    uint16_t v2 = mask[2] ? a[2] : b[2];
    uint16_t v3 = mask[3] ? a[3] : b[3];
    return svec4_u16(v0, v1, v2, v3);
}

/**
 * @brief select of svec4_i32 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_i32 svec_select(svec4_i1 mask, svec4_i32 a, svec4_i32 b) {
    return vec_sel(b.v, a.v, mask.v);
}

/**
 * @brief select of svec4_u32 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_u32 svec_select(svec4_i1 mask, svec4_u32 a, svec4_u32 b) {
    return vec_sel(b.v, a.v, mask.v);
}

/**
 * @brief select of svec4_i64 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_i64 svec_select(svec4_i1 mask, svec4_i64 a, svec4_i64 b) {

#ifdef __POWER8
   __vector signed long long t1 = vec_sel(b.v[0],a.v[0],vec_unpackh_p8(mask.v));
   __vector signed long long t2 = vec_sel(b.v[1],a.v[1],vec_unpackl_p8(mask.v));
   svec4_i64 res2 = svec4_i64(t1,t2);
  return res2;
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i64");
  int64_t v0 = mask[0] ? a[0] : b[0];
  int64_t v1 = mask[1] ? a[1] : b[1];
  int64_t v2 = mask[2] ? a[2] : b[2];
  int64_t v3 = mask[3] ? a[3] : b[3];
  return svec4_i64(v0,v1,v2,v3);
#endif
}

/**
 * @brief select of svec4_u64 vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_u64 svec_select(svec4_i1 mask, svec4_u64 a, svec4_u64 b) {

#ifdef __POWER8
   __vector unsigned long long t1 = vec_sel(b.v[0],a.v[0],vec_unpackh_p8(mask.v));
   __vector unsigned long long t2 = vec_sel(b.v[1],a.v[1],vec_unpackl_p8(mask.v));
   svec4_u64 res2 = svec4_u64(t1,t2);
  return res2;
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u64");
  uint64_t v0 = mask[0] ? a[0] : b[0];
  uint64_t v1 = mask[1] ? a[1] : b[1];
  uint64_t v2 = mask[2] ? a[2] : b[2];
  uint64_t v3 = mask[3] ? a[3] : b[3];
  return svec4_u64(v0,v1,v2,v3);
#endif
}

/**
 * @brief select of svec4_f vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_f svec_select(svec4_i1 mask, svec4_f a, svec4_f b) {
    return vec_sel(b.v, a.v, mask.v);
}

/**
 * @brief select of svec4_d vectors by a mask vector
 * see svec_select(svec4_i1 mask, svec4_i1 a, svec4_i1 b)
 */
FORCEINLINE svec4_d svec_select(svec4_i1 mask, svec4_d a, svec4_d b) {
#ifdef __POWER8
  __vector double t1 = vec_sel(b.v[0],a.v[0],vec_unpackh_p8(mask.v));
  __vector double t2 = vec_sel(b.v[1],a.v[1],vec_unpackl_p8(mask.v));
  svec4_d res2 = svec4_d(t1,t2);
  return res2;
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select_double");
  double v0 = mask[0] ? a[0] : b[0];
  double v1 = mask[1] ? a[1] : b[1];
  double v2 = mask[2] ? a[2] : b[2];
  double v3 = mask[3] ? a[3] : b[3];
  return svec4_d(v0,v1,v2,v3);
#endif
}


/**
 * @brief macros for svec's select by bool scalar method implementation
 */
#define SELECT_BOOLCOND(TYPE)                       \
/**
 * @brief Select two TYPE vectors by a bool scalar. The same as cond ? a: b
 */\
FORCEINLINE TYPE svec_select(bool cond, TYPE a, TYPE b) {       \
    return cond ? a : b;                                            \
}

SELECT_BOOLCOND(svec4_i1);
SELECT_BOOLCOND(svec4_i8);
SELECT_BOOLCOND(svec4_u8);
SELECT_BOOLCOND(svec4_i16);
SELECT_BOOLCOND(svec4_u16);
SELECT_BOOLCOND(svec4_i32);
SELECT_BOOLCOND(svec4_u32);
SELECT_BOOLCOND(svec4_i64);
SELECT_BOOLCOND(svec4_u64);
SELECT_BOOLCOND(svec4_f);
SELECT_BOOLCOND(svec4_d);


// 4. broadcast/rotate/shuffle/smear/setzero

/**
 * @brief macro for broadcast method implementation
 * All broadcast are slow implementation
 */
#define BROADCAST(VTYPE, STYPE)                   \
  static FORCEINLINE VTYPE svec_broadcast(VTYPE v, int index) { \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "broadcast");           \
    STYPE bval = v[index];                \
    VTYPE ret(bval,bval,bval,bval);                 \
    return ret;                             \
  }

BROADCAST(svec4_i8, int8_t);
BROADCAST(svec4_u8, uint8_t);
BROADCAST(svec4_i16, int16_t);
BROADCAST(svec4_u16, uint16_t);
BROADCAST(svec4_i32, int32_t);
BROADCAST(svec4_u32, uint32_t);
BROADCAST(svec4_i64, int64_t);
BROADCAST(svec4_u64, uint64_t);
BROADCAST(svec4_f, float);
BROADCAST(svec4_d, double);

/**
 * @brief macro for rotate method implementation
 */
#define ROTATE(VTYPE, STYPE)                  \
  static FORCEINLINE VTYPE svec_rotate(VTYPE v, int index) {    \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "rotate");          \
    VTYPE ret (v[(0+index) & 0x3],            \
               v[(1+index) & 0x3],            \
               v[(2+index) & 0x3],            \
               v[(3+index) & 0x3]);           \
    return ret;                             \
  }

ROTATE(svec4_i8, int8_t);
ROTATE(svec4_u8, uint8_t);
ROTATE(svec4_i16, int16_t);
ROTATE(svec4_u16, uint16_t);
ROTATE(svec4_i32, int32_t);
ROTATE(svec4_u32, uint32_t);
ROTATE(svec4_i64, int64_t);
ROTATE(svec4_u64, uint64_t);
ROTATE(svec4_f, float);
ROTATE(svec4_d, double);

/**
 * @brief macro for shuffle/shuffle2 methods implementation
 */
#define SHUFFLES(VTYPE, STYPE)                 \
  static FORCEINLINE VTYPE svec_shuffle(VTYPE v, svec4_i32 index) {   \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "shuffle");           \
    VTYPE ret (v[index[0] & 0x3],    \
               v[index[1] & 0x3],    \
               v[index[2] & 0x3],    \
               v[index[3] & 0x3] );  \
    return ret;                               \
  }                                   \
  static FORCEINLINE VTYPE svec_shuffle2(VTYPE v0, VTYPE v1, svec4_i32 index) { \
    VTYPE ret;                              \
    NOT_IMPLEMENTED("shuffle 2");                   \
    return ret;                             \
}

SHUFFLES(svec4_i8, int8_t);
SHUFFLES(svec4_u8, uint8_t);
SHUFFLES(svec4_i16, int16_t);
SHUFFLES(svec4_u16, uint16_t);
SHUFFLES(svec4_i32, int32_t);
SHUFFLES(svec4_u32, uint32_t);
SHUFFLES(svec4_i64, int64_t);
SHUFFLES(svec4_u64, uint64_t);
SHUFFLES(svec4_f, float);
SHUFFLES(svec4_d, double);

/**
 * @brief macro for setzero method implementation
 */
#define ZERO(VTYPE, NAME)                   \
  static FORCEINLINE VTYPE svec_zero(VTYPE) {  \
    VTYPE ret(0,0,0,0);                        \
    return ret;                                \
  }
/*

template <class RetVecType> static RetVecType svec_zero();

template<>
FORCEINLINE svec4_i1 svec_zero<svec4_i1>() {
  return vec_splat_u32(0);
}

template<>
FORCEINLINE svec4_i8 svec_zero<svec4_i8>() {
  return vec_splat_s8(0);
}

template<>
FORCEINLINE svec4_u8 svec_zero<svec4_u8>() {
  return vec_splat_u8(0);
}

template<>
FORCEINLINE svec4_i16 svec_zero<svec4_i16>() {
  return vec_splat_s16(0);
}

template<>
FORCEINLINE svec4_u16 svec_zero<svec4_u16>() {
  return vec_splat_u16(0);
}

template<>
FORCEINLINE svec4_i32 svec_zero<svec4_i32>() {
  return vec_splat_s32(0);
}

template<>
FORCEINLINE svec4_u32 svec_zero<svec4_u32>() {
  return vec_splat_u32(0);
}

template<>
FORCEINLINE svec4_i64 svec_zero<svec4_i64>() {
  __vector signed long long r1 = (__vector signed long long)vec_splat_s32(0);
  return svec4_i64(r1,r1);
}

template<>
FORCEINLINE svec4_u64 svec_zero<svec4_u64>() {
  __vector unsigned long long r1 = (__vector unsigned long long)vec_splat_u32(0);
  return svec4_u64(r1,r1);
}

template<>
FORCEINLINE svec4_f svec_zero<svec4_f>() {
  return (__vector float) vec_splat_s32(0);
}

template<>
FORCEINLINE svec4_d svec_zero<svec4_d>() {
    __vector double r1 = (__vector double)vec_splat_s32(0);
    return svec4_d(r1,r1);
}
*/
/*
//smear const

template <class RetVecType> static RetVecType svec_smear_const(const uint32_t v);
template<>
FORCEINLINE svec4_i1 svec_smear_const<svec4_i1>(const uint32_t v) {
    return (v!=0) ? vec_splat_s32(-1) : vec_splat_s32(0);
}

template <class RetVecType> static RetVecType svec_smear_const(const int8_t v);
template<>
FORCEINLINE svec4_i8 svec_smear_const<svec4_i8>(const int8_t v) {
    if ((v <= 15) && (v >= -16)) {
       return vec_splat_s8(v); //will gen one instr.vspltisb
    } else {
       INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear const i8");
       return svec4_i8(v,v,v,v);
    }
}

template <class RetVecType> static RetVecType svec_smear_const(const uint8_t v);
template<>
FORCEINLINE svec4_u8 svec_smear_const<svec4_u8>(const uint8_t v) {
    if ((v <= 15)) { //>15 causes 1st bit 1, and signed ext cause wrong
       return vec_splat_u8(v); //will gen one instr.vspltisb
    } else {
       INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear const u8");
       return svec4_u8(v,v,v,v);
    }
}

template <class RetVecType> static RetVecType svec_smear_const(const int16_t v);
template<>
FORCEINLINE svec4_i16 svec_smear_const<svec4_i16>(const int16_t v) {
    if ((v <= 15) && (v >= -16)) {
       return vec_splat_s16(v); //will gen one instr.vspltish
    } else {
       INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear const i16");
       return svec4_i16(v,v,v,v);
    }
}

template <class RetVecType> static RetVecType svec_smear_const(const uint16_t v);
template<>
FORCEINLINE svec4_u16 svec_smear_const<svec4_u16>(const uint16_t v) {
    if ((v <= 15)) {
       return vec_splat_u16(v); //will gen one instr.vspltish
    } else {
       INC_STATS_NAME(STATS_SMEAR_SLOW,1, "smear const u16");
       return svec4_u16(v,v,v,v);
    }
}

template <class RetVecType> static RetVecType svec_smear_const(const int32_t v);
template<>
FORCEINLINE svec4_i32 svec_smear_const<svec4_i32>(const int32_t v) {
  if ((v <= 15) && (v >= -16)) {
     return vec_splat_s32(v); //will gen one instr.vspltisw
  } else {
      __vector signed int x = {v,v,v,v};
      return svec4_i32(x);
  }
}

template <class RetVecType> static RetVecType svec_smear_const(const uint32_t v);
template<>
FORCEINLINE svec4_u32 svec_smear_const<svec4_u32>(const uint32_t v) {
    if ((v <= 15)) {
       return vec_splat_u32(v); ////will gen one instr.vspltisw
    } else {
        __vector unsigned int x = {v,v,v,v};
        return svec4_u32(x);
    }
}

template <class RetVecType> static RetVecType svec_smear_const(const int64_t v);
template<>
FORCEINLINE svec4_i64 svec_smear_const<svec4_i64>(const int64_t v) {
#ifdef __POWER8
  if ((v >= -16l) && (v <= 15l)) {
    const int iv = (int)v;
    __vector signed int x = {iv,iv,iv,iv};
    __vector signed long long t = vec_unpackh_p8(x);
    return svec4_i64(t,t);
  } else
#endif
  {
    __vector long long x = {v,v};
    return svec4_i64(x,x);
  }
}

template <class RetVecType> static RetVecType svec_smear_const(const uint64_t v);
template<>
FORCEINLINE svec4_u64 svec_smear_const<svec4_u64>(const uint64_t v) {
#ifdef __POWER8
  if ((v >= 0ul) && (v <= 31ul)) {
    const int iv = (int)v;
    __vector signed int x = {iv,iv,iv,iv};
    __vector unsigned long long t = vec_unpackh_p8(x);
    return svec4_u64(t,t);
  } else
#endif
  {
    __vector unsigned long long x = {v,v};
    return svec4_u64(x,x);
  }
}

template <class RetVecType> static RetVecType svec_smear_const(const float v);
template<>
FORCEINLINE svec4_f svec_smear_const<svec4_f>(const float v) {
    float p; int iv;
    p = 1.0; iv = (int)(p*v);
    if (( (((float)iv)/p) == v ) && (iv >= -16) && (iv <= 15)) return vec_ctf(vec_splat_s32(iv),0);
    p = 2.0; iv = (int)(p*v);
    if (( (((float)iv)/p) == v ) && (iv >= -16) && (iv <= 15)) return vec_ctf(vec_splat_s32(iv),1);
    p = 4.0; iv = (int)(p*v);
    if (( (((float)iv)/p) == v ) && (iv >= -16) && (iv <= 15)) return vec_ctf(vec_splat_s32(iv),2);

    __vector float x = {v,v,v,v};
    return svec4_f(x);
    //Some older impl
    //way 1: svec4_f(v,v,v,v)
    //way 2: __vector float x = {v,v,v,v}; svec4_f(x);
}

template <class RetVecType> static RetVecType svec_smear_const(const double v);
template<>
FORCEINLINE svec4_d svec_smear_const<svec4_d>(const double v) {
    __vector double t = vec_smear_p7(v);
    return svec4_d(t,t);
}

template <class RetVecType> static RetVecType svec_smear(uint32_t v);
template<>
FORCEINLINE svec4_i1 svec_smear<svec4_i1>(uint32_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_i1>(v);
    }
    return svec4_i1(v,v,v,v);

}

template <class RetVecType> static RetVecType svec_smear(int8_t v);
template<>
FORCEINLINE svec4_i8 svec_smear<svec4_i8>(int8_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_i8>(v);
    }
    return svec4_i8(v,v,v,v);
}

template <class RetVecType> static RetVecType svec_smear(uint8_t v);
template<>
FORCEINLINE svec4_u8 svec_smear<svec4_u8>(uint8_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_u8>(v);
    }
    return svec4_u8(v,v,v,v);
}

template <class RetVecType> static RetVecType svec_smear(int16_t v);
template<>
FORCEINLINE svec4_i16 svec_smear<svec4_i16>(int16_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_i16>(v);
    }
    return svec4_i16(v,v,v,v);
}

template <class RetVecType> static RetVecType svec_smear(uint16_t v);
template<>
FORCEINLINE svec4_u16 svec_smear<svec4_u16>(uint16_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_u16>(v);
    }
    return svec4_u16(v,v,v,v);
}

template <class RetVecType> static RetVecType svec_smear(int32_t v);
template<>
FORCEINLINE svec4_i32 svec_smear<svec4_i32>(int32_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_i32>(v);
    }
#ifdef __POWER8
    return vec_smear_p8(v);
#else
    int32_t* p = &v;
    __vector signed int register x = vec_vsx_ld(0, p);
    return svec4_i32(vec_splat_p7(x, 0));
#endif
}

template<>
FORCEINLINE svec4_u32 svec_smear<svec4_u32>(uint32_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_u32>(v);
    }
#ifdef __POWER8
    return vec_smear_p8(v);
#else
    uint32_t* p = &v;
    __vector unsigned int register x = vec_vsx_ld(0, p);
    return svec4_u32(vec_splat_p7((__vector signed)x, 0));
#endif
}

template <class RetVecType> static RetVecType svec_smear(int64_t v);
template<>
FORCEINLINE svec4_i64 svec_smear<svec4_i64>(int64_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_i64>(v);
    }
#if 0 // not accepted def __POWER9
  __vector signed long long r =  vec_smear_i64_p9(v);
  return svec4_i64(r,r);
#else
#ifdef __POWER8
  __vector unsigned long long r = vec_smear_i64_p8(v);
  return svec4_i64(r,r);
#else
  int64_t* p = &v;
  __vector signed long long r = vec_smear_i64_p7((long long*)p);
  return svec4_i64(r,r);
#endif // __POWER8
#endif // __POWER9
}

template <class RetVecType> static RetVecType svec_smear(uint64_t v);
template<>
FORCEINLINE svec4_u64 svec_smear<svec4_u64>(uint64_t v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_u64>(v);
    }
#if 0 // not accepted def __POWER9
  __vector unsigned long long r =  vec_smear_i64_p9(v);
  return svec4_i64(r,r);
#else
#ifdef __POWER8
  __vector unsigned long long r = vec_smear_i64_p8(v);
  return svec4_u64(r,r);
#else
  uint64_t* p = &v;
  __vector unsigned long long r = vec_smear_i64_p7((long long*)p);
  return svec4_u64(r,r);
#endif // __POWER8
#endif // __POWER9
}

template <class RetVecType> static RetVecType svec_smear(float v);
template<>
FORCEINLINE svec4_f svec_smear<svec4_f>(float v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_f>(v);
    }
#ifdef __POWER8
  return vec_smear_p8(v);
#else
  float* p = &v;
  __vector float register x = vec_vsx_ld(0, p);
  return svec4_f(vec_splat_p7(x, 0));
#endif
}

template <class RetVecType> static RetVecType svec_smear(double v);
template<>
FORCEINLINE svec4_d svec_smear<svec4_d>(double v) {
    if(__builtin_constant_p(v)){
      return svec_smear_const<svec4_d>(v);
    }
    __vector double t =vec_smear_p7(v);
    return svec4_d(t,t);
}

*/

//load const

static FORCEINLINE svec4_i8 svec_load_const(const int8_t* p) {
    return svec4_i8(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_u8 svec_load_const(const uint8_t* p) {
    return svec4_u8(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_i16 svec_load_const(const int16_t* p) {
    return svec4_i16(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_u16 svec_load_const(const uint16_t* p) {
    return svec4_u16(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_i32 svec_load_const(const int32_t* p) {
    return svec4_i32(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_u32 svec_load_const(const uint32_t* p) {
    return svec4_u32(p[0], p[0], p[0], p[0]);
}

static FORCEINLINE svec4_i64 svec_load_const(const int64_t* p) {
    __vector signed long long t= vec_smear_const_i64_p7((const long long *)p);
    return svec4_i64(t,t);
}

static FORCEINLINE svec4_u64 svec_load_const(const uint64_t* p) {
    __vector unsigned long long t= vec_smear_const_i64_p7((const long long *)p);
    return svec4_u64(t,t);
}

static FORCEINLINE svec4_f svec_load_const(const float* p) {
#ifdef __POWER9
  return vec_smear_const_float_p9(p);
#else
  return vec_smear_const_float_p7((const __vector float *)p);
#endif
}

static FORCEINLINE svec4_d svec_load_const(const double* p) {
    __vector double t= vec_smear_const_double_p7(p);
    return svec4_d(t,t);
}

//load and splat

static FORCEINLINE svec4_i8 svec_load_and_splat(int8_t* p) {
    INC_STATS_NAME(STATS_SMEAR_SLOW,1, "load_and_splat i8");
    int8_t v = *p;
    return svec4_i8(v,v,v,v);
}

static FORCEINLINE svec4_u8 svec_load_and_splat(uint8_t* p) {
    INC_STATS_NAME(STATS_SMEAR_SLOW,1,"load_and_splat u8");
    uint8_t v = *p;
    return svec4_u8(v,v,v,v);
}

static FORCEINLINE svec4_i16 svec_load_and_splat(int16_t* p) {
    INC_STATS_NAME(STATS_SMEAR_SLOW,1,"load_and_splat i16");
    int16_t v = *p;
    return svec4_i16(v,v,v,v);
}

static FORCEINLINE svec4_u16 svec_load_and_splat(uint16_t* p) {
    INC_STATS_NAME(STATS_SMEAR_SLOW,1,"load_and_splat u16");
    uint16_t v = *p;
    return svec4_u16(v,v,v,v);
}

static FORCEINLINE svec4_i32 svec_load_and_splat(int32_t* p) {
#ifdef __POWER9
  return (__vector signed int)vec_smear_i32_p9(p);
#else
#ifdef __POWER8
  return vec_smear_i32_p8(p);
#else
  __vector signed int register x = vec_vsx_ld(0, p);
   return svec4_i32(vec_splat_p7(x,0));
#endif //__POWER8
#endif //__POWER9
}

static FORCEINLINE svec4_u32 svec_load_and_splat(uint32_t* p) {
#ifdef __POWER9
  return (__vector unsigned int)vec_smear_i32_p9(p);
#else
#ifdef __POWER8
  return vec_smear_i32_p8(p);
#else
  __vector unsigned int register x = vec_vsx_ld(0, p);
   return svec4_u32(vec_splat_p7((__vector signed)x,0));
#endif //__POWER8
#endif //__POWER9
}

static FORCEINLINE svec4_i64 svec_load_and_splat(int64_t* p) {
    __vector signed long long r = vec_smear_i64_p7((signed long long*)p);
    return svec4_i64(r,r);
}

static FORCEINLINE svec4_u64 svec_load_and_splat(uint64_t* p) {
    __vector unsigned long long r = vec_smear_i64_p7((unsigned long long*)p);
    return svec4_u64(r,r);
}

static FORCEINLINE svec4_f svec_load_and_splat(float* p) {
#ifdef __POWER9
  return vec_smear_float_p9(p);
#else
#ifdef __POWER8
  return vec_smear_float_p8(p);
#else
   __vector float register x = vec_vsx_ld(0, p);
  return svec4_f(vec_splat_p7(x, 0));
#endif //__POWER8
#endif //__POWRE9
}

static FORCEINLINE svec4_d svec_load_and_splat(double* p) {
    __vector double t= vec_smear_double_p7(p);
    return svec4_d(t,t);
}


// 5. Gather / Scatter
/**
 * including gather general/gather base offsets, scatter general, scatter base offsets
 *
 * Here, we will define a special compile time dependent type for ptrs vector
 *
 */

#define IS64BIT

#ifdef IS64BIT
/**
 * @brief svec4_ptr on 64bit platform. It inherits svec4_u64.
 * Only used in gather and scatter to store addresses.
 * @see gather(
 */
struct svec4_ptr : public svec4_u64{
    /**
     * @brief Construct a svec4_ptr from four addresses.
     * @return a svec4_ptr vector with the four addresses
     */
    FORCEINLINE svec4_ptr(void* p0, void* p1, void* p2, void* p3):
        svec4_u64((uint64_t)(p0),(uint64_t)(p1),(uint64_t)(p2),(uint64_t)(p3)){}
};
#else //IS32BIT
/**
 * @brief svec4_ptr on 32bit platform. It inherits svec4_u32.
 * Only used in gather and scatter to store addresses.
 */
struct svec4_ptr: public svec4_u32{
    /**
     * @brief Construct a svec4_ptr from four addresses.
     * @return a svec4_ptr vector with the four addresses
     */
    FORCEINLINE svec4_ptr(void* p0, void* p1, void* p2, void* p3):
        svec4_u32((uint32_t)(p0),(uint32_t)(p1),(uint32_t)(p2),(uint32_t)(p3)){}
};
#endif //IS64BIT

#ifndef DOXYGEN_SHOULD_SKIP_THIS //not want generate svec_gather*/svec_scatter methods

/**
 * @brief utilities for fast gather general impl
 */
template<typename RetVec, typename RetScalar, typename PTRS, typename MSK>
static FORCEINLINE RetVec
lGatherGeneral(PTRS ptrs, MSK mask) {
  RetScalar r[4];
  if(mask[0]) { r[0] = *((RetScalar*)ptrs[0]);}
  if(mask[1]) { r[1] = *((RetScalar*)ptrs[1]);}
  if(mask[2]) { r[2] = *((RetScalar*)ptrs[2]);}
  if(mask[3]) { r[3] = *((RetScalar*)ptrs[3]);}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return RetVec(r[0],r[1],r[2],r[3]);
}

/**
 * @brief slow implementation of gather general
 * Must use template to specify the return type
 * @param mask
 * @return
 */
#define GATHER_GENERAL(VTYPE, STYPE, PTRTYPE)         \
template<> \
FORCEINLINE VTYPE svec_gather<VTYPE>(PTRTYPE ptrs, svec4_i1 mask) {   \
  return lGatherGeneral<VTYPE, STYPE, PTRTYPE, svec4_i1>(ptrs, mask);                                \
}


template <class RetVecType> static RetVecType svec_gather(svec4_u32 ptrs, svec4_i1 mask);
template <class RetVecType> static RetVecType svec_gather(svec4_u64 ptrs, svec4_i1 mask);

//There is a fast impl for gather addr64 on i8/u8 types
//But it is commented out. So I didn't move the code to here
//Please see vsx4.h __gather64_i8
GATHER_GENERAL(svec4_i8, int8_t, svec4_u32);
GATHER_GENERAL(svec4_i8, int8_t, svec4_u64);
GATHER_GENERAL(svec4_u8, uint8_t, svec4_u32);
GATHER_GENERAL(svec4_u8, uint8_t, svec4_u64);
GATHER_GENERAL(svec4_i16, int16_t, svec4_u32);
GATHER_GENERAL(svec4_i16, int16_t, svec4_u64);
GATHER_GENERAL(svec4_u16, uint16_t, svec4_u32);
GATHER_GENERAL(svec4_u16, uint16_t, svec4_u64);
GATHER_GENERAL(svec4_i32, int32_t, svec4_u32);

//GATHER_GENERAL(svec4_i32, int32_t, svec4_u64);
template<>
FORCEINLINE svec4_i32 svec_gather<svec4_i32>(svec4_u64 ptrs, svec4_i1 mask) {
#ifdef __POWER9
  return vec_gather(ptrs.v[0], ptrs.v[1], mask.v);
#else
  typedef svec4_i32 RetVec;
  return lGatherGeneral<RetVec,int32_t,svec4_u64,svec4_i1>(ptrs,mask);
#endif
}

GATHER_GENERAL(svec4_u32, uint32_t, svec4_u32);

//GATHER_GENERAL(svec4_u32, uint32_t, svec4_u64);
template<>
FORCEINLINE svec4_u32 svec_gather<svec4_u32>(svec4_u64 ptrs, svec4_i1 mask) {
#ifdef __POWER9
  return vec_gather(ptrs.v[0], ptrs.v[1], mask.v);
#else
  typedef svec4_u32 RetVec;
  return lGatherGeneral<RetVec,uint32_t,svec4_u64,svec4_i1>(ptrs,mask);
#endif
}




GATHER_GENERAL(svec4_i64, int64_t, svec4_u32);

//GATHER_GENERAL(svec4_i64, int64_t, svec4_u64);
template<>
FORCEINLINE svec4_i64 svec_gather<svec4_i64>(svec4_u64 ptrs, svec4_i1 mask) {
#ifdef __POWER9
    __vector signed long long r1 = vec_gather(ptrs.v[0], mask.v);
    //shift right
    __vector unsigned int s_mask = vec_sld(mask.v, mask.v,8);
    __vector signed long long r2 = vec_gather(ptrs.v[1],s_mask);
    return svec4_i64(r1,r2);
#else
  typedef svec4_i64 RetVec;
  return lGatherGeneral<RetVec,int64_t, svec4_u64,svec4_i1>(ptrs,mask);
#endif
}

GATHER_GENERAL(svec4_u64, uint64_t, svec4_u32);

//GATHER_GENERAL(svec4_u64, uint64_t, svec4_u64);
template<>
FORCEINLINE svec4_u64 svec_gather<svec4_u64>(svec4_u64 ptrs, svec4_i1 mask) {
#ifdef __POWER9
    __vector signed long long r1 = vec_gather(ptrs.v[0], mask.v);
    //shift right
    __vector unsigned int s_mask = vec_sld(mask.v, mask.v,8);
    __vector signed long long r2 = vec_gather(ptrs.v[1],s_mask);
    return svec4_u64(r1,r2);
#else
  typedef svec4_u64 RetVec;
  return lGatherGeneral<RetVec,uint64_t, svec4_u64,svec4_i1>(ptrs,mask);
#endif
}


GATHER_GENERAL(svec4_f, float, svec4_u32);

//GATHER_GENERAL(svec4_f, float, svec4_u64);
template<>
FORCEINLINE svec4_f svec_gather<svec4_f>(svec4_u64 ptrs, svec4_i1 mask) {
#ifdef __POWER9
  return vec_gather_float(ptrs.v[0], ptrs.v[1], mask.v);
#else
  typedef svec4_f RetVec;
  return lGatherGeneral<RetVec,float,svec4_u64,svec4_i1>(ptrs,mask);
#endif
}

GATHER_GENERAL(svec4_d, double, svec4_u32);
GATHER_GENERAL(svec4_d, double, svec4_u64);

//Utility functions for gather base off sets
/**
 * @brief P7 impl for gather base offsets
 * @param p
 * @param scale
 * @param offsets
 * @param mask
 * @return
 */
template<typename RetVec, typename RetScalar, typename OFF, typename MSK>
static FORCEINLINE RetVec
lGatherBaseOffsets(unsigned char *p, uint32_t scale,
                     OFF offsets, MSK mask) {
  RetScalar r[4];
  OFF vzero(0,0,0,0);

  offsets = svec_select(mask.v, offsets, vzero);
  r[0] = *(RetScalar *)(p + scale * offsets[0]);
  r[1] = *(RetScalar *)(p + scale * offsets[1]);
  r[2] = *(RetScalar *)(p + scale * offsets[2]);
  r[3] = *(RetScalar *)(p + scale * offsets[3]);
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather offset with select");
  return RetVec(r[0], r[1], r[2], r[3]);
}

/**
 * @ macros for generic impl of gather base offsets
 */
#define GATHER_BASE_OFFSETS(VTYPE, STYPE, OTYPE)         \
FORCEINLINE VTYPE svec_gather_base_offsets(STYPE* b, uint32_t scale, OTYPE offsets, svec4_i1 mask) {   \
  return lGatherBaseOffsets<VTYPE, STYPE, OTYPE, svec4_i1>((uint8_t*)b, scale, offsets, mask);                                \
}


//////////////////////////////////////POWER8
#ifdef __POWER8

//                 Gather 32 bit data with 32 bit offset
template<typename RetVec, typename RetScalar, typename OFF, typename MSK>
static FORCEINLINE RetVec
lGatherBaseOffsets32_32P8(unsigned char *p, uint32_t scale,
                     OFF offsets, MSK mask) {
    RetScalar r[4];
    OFF vzero(0,0,0,0);
    //if mask is not set we still read from p+0 to avoid the if
    offsets = svec_select(mask, offsets, vzero);
    int offset;
    RetScalar *ptr;
    //extract individual offsets
    uint64_t doff1 = vec_extract_l(offsets.v);
    uint64_t doff2 = vec_extract_r(offsets.v);
    //split them in two
    uint32_t o1=(uint32_t) doff1;
    uint32_t o0=(uint32_t)(doff1 >> 32);
    uint32_t o3=(uint32_t) doff2;
    uint32_t o2=(uint32_t)(doff2 >> 32);
#ifdef CORRECTNESS_CHECK
    if(o0 != offsets[0] ||
       o1 != offsets[1] ||
       o2 != offsets[2] ||
       o3 != offsets[3]) {
      printf("Error while extracting for gather\n");
    }
#endif
    return vec_gather_p8((RetScalar*)(p + (scale*o0)),
             (RetScalar*)(p+(scale*o1)),
             (RetScalar*)(p+(scale*o2)),
             (RetScalar*)(p+(scale*o3)) );
}

//                 Gather 64 bit data with 32 bit offset
template<typename RetVec, typename RetScalar, typename OFF, typename MSK>
static FORCEINLINE RetVec
lGatherBaseOffsets32_64P8(unsigned char *p, uint32_t scale,
                     OFF offsets, MSK mask) {
    RetScalar r[4];
    OFF vzero(0,0,0,0);
    //if mask is not set we still read from p+0 to avoid the if
    offsets = svec_select(mask, offsets, vzero);
    int offset;
    RetScalar *ptr;
    //extract individual offsets
    uint64_t doff1 = vec_extract_l(offsets.v);
    uint64_t doff2 = vec_extract_r(offsets.v);
    //split them in two
    uint32_t o1=(uint32_t) doff1;
    uint32_t o0=(uint32_t)(doff1 >> 32);
    uint32_t o3=(uint32_t) doff2;
    uint32_t o2=(uint32_t)(doff2 >> 32);
#ifdef CORRECTNESS_CHECK
    if(o0 != offsets[0] ||
       o1 != offsets[1] ||
       o2 != offsets[2] ||
       o3 != offsets[3]) {
      printf("Error while extracting for gather\n");
    }
#endif
    return RetVec(vec_gather_p8((RetScalar*)(p + (scale*o0)),
                (RetScalar*)(p+(scale*o1)))  ,
          vec_gather_p8((RetScalar*)(p+(scale*o2)),
                (RetScalar*)(p+(scale*o3))) );
}


//                 Gather 32 bit data with 64 bit offset
template<typename RetVec, typename RetScalar, typename OFF, typename MSK>
static FORCEINLINE RetVec
lGatherBaseOffsets64_32P8(unsigned char *p, uint32_t scale,
                     OFF offsets, MSK mask) {
    RetScalar r[4];
    OFF vzero(0,0,0,0);
    //if mask is not set we still read from p+0 to avoid the if
    offsets = svec_select(mask, offsets, vzero);
    int offset;
    RetScalar *ptr;
    //extract individual offsets
    uint64_t o0 = vec_extract_l(offsets.v[0]);
    uint64_t o1 = vec_extract_r(offsets.v[0]);
    uint64_t o2 = vec_extract_l(offsets.v[1]);
    uint64_t o3 = vec_extract_r(offsets.v[1]);

#ifdef CORRECTNESS_CHECK
    if(o0 != offsets[0] ||
       o1 != offsets[1] ||
       o2 != offsets[2] ||
       o3 != offsets[3]) {
      printf("Error while extracting for gather\n");
    }
#endif
    return vec_gather_p8((RetScalar*)(p+(scale*o0)),
             (RetScalar*)(p+(scale*o1)),
             (RetScalar*)(p+(scale*o2)),
             (RetScalar*)(p+(scale*o3)) );
}

//                 Gather 64 bit data with 64 bit offset
template<typename RetVec, typename RetScalar, typename OFF, typename MSK>
static FORCEINLINE RetVec
lGatherBaseOffsets64_64P8(unsigned char *p, uint32_t scale,
                     OFF offsets, MSK mask) {
    RetScalar r[4];
    OFF vzero(0,0,0,0);
    //if mask is not set we still read from p+0 to avoid the if
    offsets = svec_select(mask.v, offsets, vzero);
    int offset;
    RetScalar *ptr;
    //extract individual offsets
    uint64_t o0 = vec_extract_l(offsets.v[0]);
    uint64_t o1 = vec_extract_r(offsets.v[0]);
    uint64_t o2 = vec_extract_l(offsets.v[1]);
    uint64_t o3 = vec_extract_r(offsets.v[1]);

#ifdef CORRECTNESS_CHECK
    if(o0 != offsets[0] ||
       o1 != offsets[1] ||
       o2 != offsets[2] ||
       o3 != offsets[3]) {
      printf("Error while extracting for gather\n");
    }
#endif
    return RetVec(vec_gather_p8((RetScalar*)(p + (scale*o0)),
                (RetScalar*)(p+(scale*o1)))  ,
          vec_gather_p8((RetScalar*)(p+(scale*o2)),
                (RetScalar*)(p+(scale*o3))) );
}

///////////////////////////////////////////////// NEW
#endif //endif __POWER8


GATHER_BASE_OFFSETS(svec4_i8, int8_t, svec4_i32);
GATHER_BASE_OFFSETS(svec4_i8, int8_t, svec4_i64);
GATHER_BASE_OFFSETS(svec4_u8, uint8_t, svec4_i32);
GATHER_BASE_OFFSETS(svec4_u8, uint8_t, svec4_i64);
GATHER_BASE_OFFSETS(svec4_i16, int16_t, svec4_i32);
GATHER_BASE_OFFSETS(svec4_i16, int16_t, svec4_i64);
GATHER_BASE_OFFSETS(svec4_u16, uint16_t, svec4_i32);
GATHER_BASE_OFFSETS(svec4_u16, uint16_t, svec4_i64);

//GATHER_BASE_OFFSETS(svec4_i32, int32_t, svec4_i32);
static FORCEINLINE svec4_i32
svec_gather_base_offsets(int32_t *b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask){
#ifdef __POWER9
  return vec_gather((signed int*)p, (__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
#else
  #ifdef __POWER8
  return lGatherBaseOffsets32_32P8<svec4_i32,int32_t,svec4_i32,svec4_i1>((uint8_t*)b,scale,offsets,mask);
  #else
  return lGatherBaseOffsets<svec4_i32, int32_t, svec4_i32,svec4_i1>((uint8_t*)b,scale,offsets,mask);
  #endif
#endif
}

//GATHER_BASE_OFFSETS(svec4_i32, int32_t, svec4_i64);
static FORCEINLINE svec4_i32
svec_gather_base_offsets(int32_t* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask){
    uint8_t *p = (uint8_t*)b;
    typedef svec4_i32 RetVec;
#ifdef __POWER9
  return vec_gather((signed int*)p, offsets.v[0], offsets.v[1], mask.v);
#else
  #ifdef __POWER8
  RetVec r1=lGatherBaseOffsets64_32P8<svec4_i32,int32_t,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  return r1;
  #else
  return lGatherBaseOffsets<svec4_i32, int32_t,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}

//GATHER_BASE_OFFSETS(svec4_u32, uint32_t, svec4_i32);
static FORCEINLINE svec4_u32
svec_gather_base_offsets(uint32_t *b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask){
#ifdef __POWER9
  return vec_gather((signed int*)p, (__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
#else
  #ifdef __POWER8
  return lGatherBaseOffsets32_32P8<svec4_u32,uint32_t,svec4_i32,svec4_i1>((uint8_t*)b,scale,offsets,mask);
  #else
  return lGatherBaseOffsets<svec4_u32, uint32_t, svec4_i32,svec4_i1>((uint8_t*)b,scale,offsets,mask);
  #endif
#endif
}

//GATHER_BASE_OFFSETS(svec4_u32, uint32_t, svec4_i64);
static FORCEINLINE svec4_u32
svec_gather_base_offsets(uint32_t* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask){
    uint8_t *p = (uint8_t*)b;
    typedef svec4_i32 RetVec;
#ifdef __POWER9
  return vec_gather((signed int*)p, offsets.v[0], offsets.v[1], mask.v);
#else
  #ifdef __POWER8
  RetVec r1=lGatherBaseOffsets64_32P8<svec4_u32,uint32_t,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  return r1;
  #else
  return lGatherBaseOffsets<svec4_u32, uint32_t,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}

//GATHER_BASE_OFFSETS(svec4_i64, int64_t, svec4_i32);
static FORCEINLINE svec4_i64
svec_gather_base_offsets(int64_t *b, uint32_t scale, svec4_i32 offsets,svec4_i1 mask){
  uint8_t *p = (uint8_t *)b;
  typedef svec4_i64 RetVec;
#ifdef __POWER9
    __vector signed long long r1 = vec_gather_l((signed long long*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    __vector signed long long r2 = vec_gather_r((signed long long*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    return svec4_i64(r1,r2);
#else
  #ifdef __POWER8
    svec4_i64 r2 = lGatherBaseOffsets32_64P8<RetVec,int64_t,svec4_i32,svec4_i1>(p,scale,offsets,mask);
    return r2;
  #else
    return lGatherBaseOffsets<RetVec,int64_t, svec4_i32,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}



GATHER_BASE_OFFSETS(svec4_i64, int64_t, svec4_i64);

//GATHER_BASE_OFFSETS(svec4_u64, uint64_t, svec4_i32);
static FORCEINLINE svec4_u64
svec_gather_base_offsets(uint64_t *b, uint32_t scale, svec4_i32 offsets,svec4_i1 mask){
  uint8_t *p = (uint8_t *)b;
  typedef svec4_u64 RetVec;
#ifdef __POWER9
    __vector signed long long r1 = vec_gather_l((signed long long*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    __vector signed long long r2 = vec_gather_r((signed long long*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    return svec4_i64(r1,r2);
#else
  #ifdef __POWER8
    svec4_u64 r2 = lGatherBaseOffsets32_64P8<RetVec,uint64_t,svec4_i32,svec4_i1>(p,scale,offsets,mask);
    return r2;
  #else
    return lGatherBaseOffsets<RetVec,uint64_t, svec4_i32,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}

GATHER_BASE_OFFSETS(svec4_u64, uint64_t, svec4_i64);


//GATHER_BASE_OFFSETS(svec4_f, float, svec4_i32);
static FORCEINLINE svec4_f
svec_gather_base_offsets(float *b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask){
    uint8_t *p = (uint8_t*)b;
#ifdef __POWER9
  return vec_gather((float*)p, (__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
#else
  #ifdef __POWER8
  return  lGatherBaseOffsets32_32P8<svec4_f,float,svec4_i32,svec4_i1>(p,scale,offsets,mask);
  #else
  return  lGatherBaseOffsets<svec4_f,float, svec4_i32,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}

//GATHER_BASE_OFFSETS(svec4_f, float, svec4_i64);
static FORCEINLINE svec4_f
svec_gather_base_offsets(float* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask){
  uint8_t *p = (uint8_t*)b;
  typedef svec4_f RetVec;
#ifdef __POWER9
    return vec_gather((float*)p, offsets.v[0], offsets.v[1], mask.v);
#else
  #ifdef __POWER8
  RetVec r1=lGatherBaseOffsets64_32P8<RetVec,float,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  return r1;
  #else
  return lGatherBaseOffsets<RetVec,float,svec4_i64,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}


//GATHER_BASE_OFFSETS(svec4_d, double, svec4_i32);
static FORCEINLINE svec4_d
svec_gather_base_offsets(double* b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask){
  typedef svec4_d RetVec;
  uint8_t* p = (uint8_t*)b;
#ifdef __POWER9
    __vector double r1 = vec_gather_l((double*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    __vector double r2 = vec_gather_r((double*)p,(__vector unsigned int)offsets.v, (__vector unsigned int)mask.v);
    return svec4_d(r1,r2);
#else
  #ifdef __POWER8
    svec4_d r2 = lGatherBaseOffsets32_64P8<RetVec,double,svec4_i32,svec4_i1>(p,scale,offsets,mask);
    return r2;
  #else
    return lGatherBaseOffsets<svec4_d,double,svec4_i32,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}


//GATHER_BASE_OFFSETS(svec4_d, double, svec4_i64);
static FORCEINLINE svec4_d
svec_gather_base_offsets(double* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask){
    uint8_t *p = (uint8_t*)b;
    typedef svec4_d RetVec;
#ifdef __POWER9
    __vector double r1 = vec_gather((double*)p, offsets.v[0], mask.v);
    //shift right
    __vector unsigned int s_mask = vec_sld(mask.v, mask.v,8);
    __vector double r2 = vec_gather((double*)p, offsets.v[1], s_mask);
    return svec4_d(r1,r2);
#else
  #ifdef __POWER8
    RetVec r1=lGatherBaseOffsets64_64P8<RetVec,double,svec4_i64,svec4_i1>(p,scale,offsets,mask);
    return r1;
  #else
    return lGatherBaseOffsets<svec4_d, double, svec4_i64,svec4_i1>(p,scale,offsets,mask);
  #endif
#endif
}


#define SCATTER_GENERAL(VTYPE, STYPE, PTRTYPE)            \
static FORCEINLINE void svec_scatter(PTRTYPE ptrs, VTYPE val, svec4_i1 mask) { \
    if(mask[0]) { *((STYPE*)ptrs[0]) = val[0]; }\
    if(mask[1]) { *((STYPE*)ptrs[1]) = val[1]; }\
    if(mask[2]) { *((STYPE*)ptrs[2]) = val[2]; }\
    if(mask[3]) { *((STYPE*)ptrs[3]) = val[3]; }\
    INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter general "#VTYPE);          \
}

/**
 * @brief Utility functions for scatter general fast impl
 */
template<typename STYPE, typename PTRTYPE, typename VTYPE>
static FORCEINLINE void lScatterGeneral(PTRTYPE ptrs,
                        VTYPE val, svec4_i1 mask) {
  if(mask[0]) { *((STYPE*)ptrs[0]) = val[0]; }
  if(mask[1]) { *((STYPE*)ptrs[1]) = val[1]; }
  if(mask[2]) { *((STYPE*)ptrs[2]) = val[2]; }
  if(mask[3]) { *((STYPE*)ptrs[3]) = val[3]; }
  INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter general");
}

#ifdef __POWER8

template<typename STYPE, typename PTRTYPE, typename VTYPE>
static FORCEINLINE void lScatter64_32(PTRTYPE ptrs,
                      VTYPE val, svec4_i1 mask) {

  uint64_t p0 = vec_extract_l(ptrs.v[0]);
  uint64_t p1 = vec_extract_r(ptrs.v[0]);
  uint64_t p2 = vec_extract_l(ptrs.v[1]);
  uint64_t p3 = vec_extract_r(ptrs.v[1]);

  //extract mask
  uint64_t doff1 = vec_extract_l(mask.v);
  uint64_t doff2 = vec_extract_r(mask.v);
  //split them in four
  uint32_t m1=(uint32_t) doff1;
  uint32_t m0=(uint32_t)(doff1 >> 32);
  uint32_t m3=(uint32_t) doff2;
  uint32_t m2=(uint32_t)(doff2 >> 32);

  //check correctness
  /*
  if(p0 != __extract_element(ptrs,0) ||
     p1 != __extract_element(ptrs,1) ||
     p2 != __extract_element(ptrs,2) ||
     p3 != __extract_element(ptrs,3)) {
    printf("Error while extracting ptrs for scatter\n");
  }

  if(m0 != __extract_element(mask,0) ||
     m1 != __extract_element(mask,1) ||
     m2 != __extract_element(mask,2) ||
     m3 != __extract_element(mask,3)) {
    printf("Error while extracting mask for scatter\n");
  }
  */

  if(m0)
    vec_scatter_step_12((STYPE*)p0, val.v);
  if(m1)
    vec_scatter_step_0((STYPE*)p1, val.v);
  if(m2)
    vec_scatter_step_4((STYPE*)p2, val.v);
  if(m3)
    vec_scatter_step_8((STYPE*)p3, val.v);
}
#endif


SCATTER_GENERAL(svec4_i8, int8_t, svec4_u32);
SCATTER_GENERAL(svec4_i8, int8_t, svec4_u64);
SCATTER_GENERAL(svec4_u8, uint8_t, svec4_u32);
SCATTER_GENERAL(svec4_u8, uint8_t, svec4_u64);
SCATTER_GENERAL(svec4_i16, int16_t, svec4_u32);
SCATTER_GENERAL(svec4_i16, int16_t, svec4_u64);
SCATTER_GENERAL(svec4_u16, uint16_t, svec4_u32);
SCATTER_GENERAL(svec4_u16, uint16_t, svec4_u64);
SCATTER_GENERAL(svec4_i32, int32_t, svec4_u32);

//SCATTER_GENERAL(svec4_i32, int32_t, svec4_u64);
static FORCEINLINE void svec_scatter(svec4_u64 ptrs, svec4_i32 val, svec4_i1 mask) {
#ifdef __POWER9
  //lScatter64_32<int32_t, __vec4_i64, __vec4_i32>(ptrs,val,mask);
  vec_scatter(val.v,(int32_t *)0, ptrs.v[0], ptrs.v[1], mask.v);
#else
 #ifdef __POWER8
  lScatter64_32<int32_t, svec4_u64, svec4_i32>(ptrs,val,mask);
 #else
  lScatterGeneral<int32_t, svec4_u64, svec4_i32>(ptrs,val,mask);
 #endif
#endif
}

SCATTER_GENERAL(svec4_u32, uint32_t, svec4_u32);

//SCATTER_GENERAL(svec4_u32, uint32_t, svec4_u64);
static FORCEINLINE void svec_scatter(svec4_u64 ptrs, svec4_u32 val, svec4_i1 mask) {
#ifdef __POWER9
  //lScatter64_32<int32_t, __vec4_i64, __vec4_i32>(ptrs,val,mask);
  vec_scatter(val.v,(int32_t *)0, ptrs.v[0], ptrs.v[1], mask.v);
#else
 #ifdef __POWER8
  lScatter64_32<uint32_t, svec4_u64, svec4_u32>(ptrs,val,mask);
 #else
  lScatterGeneral<uint32_t, svec4_u64, svec4_u32>(ptrs,val,mask);
 #endif
#endif
}

SCATTER_GENERAL(svec4_i64, int64_t, svec4_u32);
SCATTER_GENERAL(svec4_i64, int64_t, svec4_u64);
SCATTER_GENERAL(svec4_u64, uint64_t, svec4_u32);
SCATTER_GENERAL(svec4_u64, uint64_t, svec4_u64);
SCATTER_GENERAL(svec4_f, float, svec4_u32);

//SCATTER_GENERAL(svec4_f, float, svec4_u64);
static FORCEINLINE void svec_scatter (svec4_u64 ptrs,svec4_f val,svec4_i1 mask) {
#ifdef __POWER9
  //lScatter64_32<float, __vec4_i64, __vec4_f>(ptrs,val,mask);
  vec_scatter(val.v,(float *)0, ptrs.v[0], ptrs.v[1], mask.v);
#else
 #ifdef __POWER8
  lScatter64_32<float, svec4_u64, svec4_f>(ptrs,val,mask);
 #else
  lScatterGeneral<float, svec4_u64, svec4_f>(ptrs,val,mask);
 #endif
#endif
}

SCATTER_GENERAL(svec4_d, double, svec4_u32);
SCATTER_GENERAL(svec4_d, double, svec4_u64);


/**
 * @ macros for generic impl of scatter base offsets
 */
#define SCATTER_BASE_OFFSETS(VTYPE, STYPE, OTYPE)         \
FORCEINLINE void svec_scatter_base_offsets(STYPE* b, uint32_t scale, OTYPE offsets, VTYPE val, svec4_i1 mask) {   \
  if(mask[0]) { *(STYPE*)((uint8_t*)b + scale * offsets[0]) = val[0]; }\
  if(mask[1]) { *(STYPE*)((uint8_t*)b + scale * offsets[1]) = val[1]; }\
  if(mask[2]) { *(STYPE*)((uint8_t*)b + scale * offsets[2]) = val[2]; }\
  if(mask[3]) { *(STYPE*)((uint8_t*)b + scale * offsets[3]) = val[3]; }\
  INC_STATS_NAME(STATS_SCATTER_SLOW,1,"scatter offset "#VTYPE);          \
}

/**
 * @brief Utility functions for scatter base offsets fast impl
 */
template<typename STYPE, typename OTYPE, typename VTYPE>
static FORCEINLINE void lScatterBaseOffsets(unsigned char *b,
                        uint32_t scale, OTYPE offsets,
                        VTYPE val, svec4_i1 mask) {
  unsigned char *base = b;
  if(mask[0]) { *(STYPE*)(b + scale * offsets[0]) = val[0]; }
  if(mask[1]) { *(STYPE*)(b + scale * offsets[1]) = val[1]; }
  if(mask[2]) { *(STYPE*)(b + scale * offsets[2]) = val[2]; }
  if(mask[3]) { *(STYPE*)(b + scale * offsets[3]) = val[3]; }
  INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter offset");
}

#ifdef __POWER8
template<typename STYPE, typename OTYPE, typename VTYPE>
static FORCEINLINE void lScatterBaseOffsets32_32(unsigned char *b,
                        uint32_t scale, OTYPE offsets,
                        VTYPE val, svec4_i1 mask) {
  //data is 32; offset is 32
  unsigned char *base = b;
  //extract offsets
  uint64_t doff1 = vec_extract_l(offsets.v);
  uint64_t doff2 = vec_extract_r(offsets.v);
  //split them in four
  uint32_t o1=(uint32_t) doff1;
  uint32_t o0=(uint32_t)(doff1 >> 32);
  uint32_t o3=(uint32_t) doff2;
  uint32_t o2=(uint32_t)(doff2 >> 32);

  //extract mask
  doff1 = vec_extract_l(mask.v);
  doff2 = vec_extract_r(mask.v);
  //split them in four
  uint32_t m1=(uint32_t) doff1;
  uint32_t m0=(uint32_t)(doff1 >> 32);
  uint32_t m3=(uint32_t) doff2;
  uint32_t m2=(uint32_t)(doff2 >> 32);

  //check correctness
  /*
  if(o0 != __extract_element(offsets,0) ||
     o1 != __extract_element(offsets,1) ||
     o2 != __extract_element(offsets,2) ||
     o3 != __extract_element(offsets,3)) {
    printf("Error while extracting offsets for scatter\n");
  }

  if(m0 != __extract_element(mask,0) ||
     m1 != __extract_element(mask,1) ||
     m2 != __extract_element(mask,2) ||
     m3 != __extract_element(mask,3)) {
    printf("Error while extracting mask for scatter\n");
  }
  */

  STYPE *ptr0 = (STYPE *)(base + scale * o0);
  STYPE *ptr1 = (STYPE *)(base + scale * o1);
  STYPE *ptr2 = (STYPE *)(base + scale * o2);
  STYPE *ptr3 = (STYPE *)(base + scale * o3);

  if(m0)
    vec_scatter_step_12(ptr0, val.v);
  if(m1)
    vec_scatter_step_0(ptr1, val.v);
  if(m2)
    vec_scatter_step_4(ptr2, val.v);
  if(m3)
    vec_scatter_step_8(ptr3, val.v);
}


template<typename STYPE, typename OTYPE, typename VTYPE>
static FORCEINLINE void lScatterBaseOffsets64_32(unsigned char *b,
                        uint32_t scale, OTYPE offsets,
                        VTYPE val, svec4_i1 mask) {
  //data is 32; offset is 64
  unsigned char *base = b;

  uint64_t o0 = vec_extract_l(offsets.v[0]);
  uint64_t o1 = vec_extract_r(offsets.v[0]);
  uint64_t o2 = vec_extract_l(offsets.v[1]);
  uint64_t o3 = vec_extract_r(offsets.v[1]);

  //extract mask
  uint64_t doff1 = vec_extract_l(mask.v);
  uint64_t doff2 = vec_extract_r(mask.v);
  //split them in four
  uint32_t m1=(uint32_t) doff1;
  uint32_t m0=(uint32_t)(doff1 >> 32);
  uint32_t m3=(uint32_t) doff2;
  uint32_t m2=(uint32_t)(doff2 >> 32);

  //check correctness
  /*
  if(o0 != __extract_element(offsets,0) ||
     o1 != __extract_element(offsets,1) ||
     o2 != __extract_element(offsets,2) ||
     o3 != __extract_element(offsets,3)) {
    printf("Error while extracting offsets for scatter\n");
  }

  if(m0 != __extract_element(mask,0) ||
     m1 != __extract_element(mask,1) ||
     m2 != __extract_element(mask,2) ||
     m3 != __extract_element(mask,3)) {
    printf("Error while extracting mask for scatter\n");
  }
  */

  STYPE *ptr0 = (STYPE *)(base + scale * o0);
  STYPE *ptr1 = (STYPE *)(base + scale * o1);
  STYPE *ptr2 = (STYPE *)(base + scale * o2);
  STYPE *ptr3 = (STYPE *)(base + scale * o3);

  if(m0)
    vec_scatter_step_12(ptr0, val.v);
  if(m1)
    vec_scatter_step_0(ptr1, val.v);
  if(m2)
    vec_scatter_step_4(ptr2, val.v);
  if(m3)
    vec_scatter_step_8(ptr3, val.v);
}
#endif



SCATTER_BASE_OFFSETS(svec4_i8, int8_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_i8, int8_t, svec4_i64);
SCATTER_BASE_OFFSETS(svec4_u8, uint8_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_u8, uint8_t, svec4_i64);
SCATTER_BASE_OFFSETS(svec4_i16, int16_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_i16, int16_t, svec4_i64);
SCATTER_BASE_OFFSETS(svec4_u16, uint16_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_u16, uint16_t, svec4_i64);

//SCATTER_BASE_OFFSETS(svec4_i32, int32_t, svec4_i32);
static FORCEINLINE void
svec_scatter_base_offsets(int32_t* p, uint32_t scale, svec4_i32 offsets,
                          svec4_i32 val, svec4_i1 mask){
    uint8_t* b = (uint8_t*) p;
#ifdef __POWER9
    vec_scatter(val.v,(int *)b,offsets.v, mask.v);
#else
 #ifdef __POWER8
    lScatterBaseOffsets32_32<int32_t, svec4_i32, svec4_i32>(b,scale,offsets,val,mask);
 #else
    lScatterBaseOffsets<int32_t, svec4_i32, svec4_i32>(b,scale,offsets,val,mask);
 #endif
#endif
}


//SCATTER_BASE_OFFSETS(svec4_i32, int32_t, svec4_i64);
static FORCEINLINE void
svec_scatter_base_offsets(int32_t* p, uint32_t scale, svec4_i64 offsets,
                          svec4_i32 val, svec4_i1 mask){
    uint8_t* b = (uint8_t*) p;
#ifdef __POWER9
  vec_scatter(val.v,(int32_t *)b, offsets.v[0], offsets.v[1], mask.v);
#else
  #ifdef __POWER8
   lScatterBaseOffsets64_32<int32_t, svec4_i64, svec4_i32>(b,scale,offsets,val,mask);
  #else
   lScatterBaseOffsets<int32_t,svec4_i64, svec4_i32>(b,scale,offsets,val,mask);
  #endif
#endif
}

//SCATTER_BASE_OFFSETS(svec4_u32, uint32_t, svec4_i32);
static FORCEINLINE void
svec_scatter_base_offsets(uint32_t* p, uint32_t scale, svec4_i32 offsets,
                          svec4_u32 val, svec4_i1 mask){
    uint8_t* b = (uint8_t*) p;
#ifdef __POWER9
    vec_scatter(val.v,(int *)b,offsets.v, mask.v);
#else
 #ifdef __POWER8
    lScatterBaseOffsets32_32<uint32_t, svec4_i32, svec4_u32>(b,scale,offsets,val,mask);
 #else
    lScatterBaseOffsets<uint32_t, svec4_i32, svec4_u32>(b,scale,offsets,val,mask);
 #endif
#endif
}

//SCATTER_BASE_OFFSETS(svec4_u32, uint32_t, svec4_i64);
static FORCEINLINE void
svec_scatter_base_offsets(uint32_t* p, uint32_t scale, svec4_i64 offsets,
                          svec4_u32 val, svec4_i1 mask){
    uint8_t* b = (uint8_t*) p;
#ifdef __POWER9
  vec_scatter(val.v,(int32_t *)b, offsets.v[0], offsets.v[1], mask.v);
#else
  #ifdef __POWER8
   lScatterBaseOffsets64_32<uint32_t, svec4_i64, svec4_u32>(b,scale,offsets,val,mask);
  #else
   lScatterBaseOffsets<uint32_t,svec4_i64, svec4_u32>(b,scale,offsets,val,mask);
  #endif
#endif
}

SCATTER_BASE_OFFSETS(svec4_i64, int64_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_i64, int64_t, svec4_i64);
SCATTER_BASE_OFFSETS(svec4_u64, uint64_t, svec4_i32);
SCATTER_BASE_OFFSETS(svec4_u64, uint64_t, svec4_i64);

//SCATTER_BASE_OFFSETS(svec4_f, float, svec4_i32);
static FORCEINLINE void
svec_scatter_base_offsets(float* p, uint32_t scale, svec4_i32 offsets,
                                  svec4_f val,svec4_i1 mask){
    uint8_t* b = (uint8_t*)p;
#ifdef __POWER9
    vec_scatter(val.v,(float *)b,offsets.v, mask.v);
#else
 #ifdef __POWER8
    lScatterBaseOffsets32_32<float, svec4_i32, svec4_f>(b,scale,offsets,val,mask);
 #else
    lScatterBaseOffsets<float, svec4_i32, svec4_f>(b,scale,offsets,val,mask);
 #endif
#endif
}

//SCATTER_BASE_OFFSETS(svec4_f, float, svec4_i64);
static FORCEINLINE void
svec_scatter_base_offsets(float* p,uint32_t scale, svec4_i64 offsets,
                          svec4_f val, svec4_i1 mask){
    uint8_t* b = (uint8_t*)p;
#ifdef __POWER9
  vec_scatter(val.v,(float *)b,offsets.v[0], offsets.v[1], mask.v);
#else
 #ifdef __POWER8
  lScatterBaseOffsets64_32<float, svec4_i64, svec4_f>(b,scale,offsets,val,mask);
 #else
  lScatterBaseOffsets<float, svec4_i64, svec4_f>(b,scale,offsets,val,mask);
 #endif
#endif
}


SCATTER_BASE_OFFSETS(svec4_d, double, svec4_i32);

//SCATTER_BASE_OFFSETS(svec4_d, double, svec4_i64);
static FORCEINLINE void
svec_scatter_base_offsets(double* p, uint32_t scale, svec4_i64 offsets,
                               svec4_d val, svec4_i1 mask){
    uint8_t* b = (uint8_t*)p;
#ifdef __POWER9
  vec_scatter(val.v[0],(double *)b, offsets.v[0], (__vector long long)vec_unpackh_p8(mask.v));
  vec_scatter(val.v[1],(double *)b, offsets.v[1], (__vector long long)vec_unpackl_p8(mask.v));
#else
  lScatterBaseOffsets<double, svec4_i64, svec4_d>(b,scale,offsets,val,mask);
#endif
}

#endif //DOXYGEN_SHOULD_SKIP_THIS


//  5. masked load/masked store

//Masked load/store is implemented based on gather_base_offsets/scatter_base_offsets
//Here we only use offsets with 32bit

#define MASKED_LOAD_STORE(VTYPE, STYPE, SCALE, MTYPE)                       \
static FORCEINLINE VTYPE svec_masked_load(VTYPE *p, MTYPE mask) { \
    return svec_gather_base_offsets((STYPE*)p, SCALE, svec4_i32(0,1,2,3), mask);  \
}                                                      \
static FORCEINLINE void svec_masked_store(VTYPE *p, VTYPE v, MTYPE mask) { \
    svec_scatter_base_offsets((STYPE*)p, SCALE, svec4_i32(0,1,2,3), v, mask); \
}

MASKED_LOAD_STORE(svec4_i8, int8_t, 1, svec4_i1);
MASKED_LOAD_STORE(svec4_u8, uint8_t, 1, svec4_i1);
MASKED_LOAD_STORE(svec4_i16, int16_t, 2, svec4_i1);
MASKED_LOAD_STORE(svec4_u16, uint16_t, 2, svec4_i1);
MASKED_LOAD_STORE(svec4_i32, int32_t, 4, svec4_i1);
MASKED_LOAD_STORE(svec4_u32, uint32_t, 4, svec4_i1);
MASKED_LOAD_STORE(svec4_i64, int64_t, 8, svec4_i1);
MASKED_LOAD_STORE(svec4_u64, uint64_t, 8, svec4_i1);
MASKED_LOAD_STORE(svec4_f, float, 4, svec4_i1);
MASKED_LOAD_STORE(svec4_d, double, 8, svec4_i1);

//////////////////////////////////////////////////////////////
//
// Mask type (i1) interfaces
//
//////////////////////////////////////////////////////////////

// 1. mask construction
/**
 * @brief Check any element of the mask is non-zero
 * @param mask the svec_i1 type vector
 * @return true is at least one element in the mask is true
 */
static FORCEINLINE bool svec_any_true(svec4_i1& mask) {
    return vec_any_ne(mask.v, vec_splat_u32(0));
}

/**
 * @brief Check all elements of the mask are non-zero
 * @param mask the svec_i1 type vector
 * @return true is all elements in the mask are true
 */
static FORCEINLINE bool svec_all_true(svec4_i1& mask) {
    return vec_all_ne(mask.v, vec_splat_u32(0));
}


/**
 * @brief Check none elements of the mask are zero
 * @param mask the svec_i1 type vector
 * @return true is all elements in the mask are false
 */
static FORCEINLINE bool svec_none_true(svec4_i1& mask) {
    return vec_all_eq(mask.v, vec_splat_u32(0));
}

// 2. bit operations

/**
 * @brief return a & b
 */
static FORCEINLINE svec4_i1 svec_and(svec4_i1 a, svec4_i1 b) {
  return a.v & b.v;
}


/**
 * @brief return a | b
 */
static FORCEINLINE svec4_i1 svec_or(svec4_i1 a, svec4_i1 b) {
  return a.v | b.v;
}

/**
 * @brief return a ^ b
 */
static FORCEINLINE svec4_i1 svec_xor(svec4_i1 a, svec4_i1 b) {
  return a.v ^ b.v;
}

/**
 * @brief return ~a
 */
static FORCEINLINE svec4_i1 svec_not(svec4_i1 a) {
  return ~a.v;
}


//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////


// 1. Unary
#define UNARY_OP(TYPE, NAME, OP)            \
static FORCEINLINE TYPE NAME(TYPE v) {      \
  INC_STATS_NAME(STATS_UNARY_SLOW, 1, #OP);           \
  return TYPE(OP(v[0]), OP(v[1]), OP(v[2]), OP(v[3]));\
}

#define UNARY_OP_OPT(TYPE, NAME, OP)\
static FORCEINLINE TYPE NAME(TYPE a) { \
  return OP(a.v); \
}

/**
 * @brief macros for 64bit object, i64/u64/double
 */
#define UNARY_OP_OPT64(TYPE, NAME, OP)\
static FORCEINLINE TYPE NAME(TYPE a) { \
  return  TYPE(OP(a.v[0]), OP(a.v[1]));  \
}

// neg operation
UNARY_OP_OPT(svec4_i8, svec_neg, -);
UNARY_OP_OPT(svec4_u8, svec_neg, -);
UNARY_OP_OPT(svec4_i16, svec_neg, -);
UNARY_OP_OPT(svec4_u16, svec_neg, -);
UNARY_OP_OPT(svec4_i32, svec_neg, -);
UNARY_OP_OPT(svec4_u32, svec_neg, -);
UNARY_OP_OPT64(svec4_i64, svec_neg, -);
UNARY_OP_OPT64(svec4_u64, svec_neg, -);
UNARY_OP_OPT(svec4_f, svec_neg, -);
UNARY_OP_OPT64(svec4_d, svec_neg, -);

//  2. Math unary
//round
UNARY_OP(svec4_f, svec_round, roundf);
UNARY_OP(svec4_d, svec_round, round);
//floor
UNARY_OP_OPT(svec4_f, svec_floor, vec_floor);
UNARY_OP(svec4_d, svec_floor, floor);
//ceil
UNARY_OP_OPT(svec4_f, svec_ceil, vec_ceil);
UNARY_OP(svec4_d, svec_ceil, ceil);
//reverse 1/
static FORCEINLINE svec4_f svec_rcp(svec4_f v) {
  //return vec_re(v);//Get the reciprocal estimate
  __vector float estimate = vec_re( v.v );
  //One round of Newton-Raphson refinement
  __vector float r = vec_madd( vec_nmsub(estimate, v.v, (__vector float){1.0,1.0,1.0,1.0} ), estimate, estimate);
  return svec4_f(r);
}

UNARY_OP(svec4_d, svec_rcp, 1.0/);
//reverse sqrt
static FORCEINLINE svec4_f svec_rsqrt(svec4_f v) {
    //return vec_rsqrte(v);
    //Get the square root reciprocal estimate
    __vector float zero = (__vector float){0,0,0,0};
    __vector float oneHalf = (__vector float){0.5,0.5,0.5,0.5};
    __vector float one = (__vector float){1.0,1.0,1.0,1.0};
    __vector float estimate = vec_rsqrte( v.v );
    //One round of Newton-Raphson refinement
    __vector float estimateSquared = vec_madd( estimate, estimate, zero );
    __vector float halfEstimate = vec_madd( estimate, oneHalf, zero );
    __vector float r = vec_madd( vec_nmsub( v.v, estimateSquared, one ), halfEstimate, estimate );
    return svec4_f(r);

}

UNARY_OP(svec4_d, svec_rsqrt, 1.0/sqrt);
//sqrt
static FORCEINLINE svec4_f svec_sqrt(svec4_f v) {
    __vector float r = vec_madd( v.v, svec_rsqrt(v).v, (__vector float){0,0,0,0} );
    return svec4_f(r);
}

UNARY_OP(svec4_d, svec_sqrt, sqrt);

//exp
static FORCEINLINE svec4_f svec_exp(svec4_f v) {
  return vec_expte(v.v);
}
UNARY_OP(svec4_d, svec_exp, exp);


//log
static FORCEINLINE svec4_f svec_log(svec4_f v) {
  return vec_loge(v.v);
}
UNARY_OP(svec4_d, svec_log, log);



//  3. Binary
/**
 * @brief macros for generic slow imple of binary operation
 */
#define BINARY_OP(TYPE, NAME, OP)                \
static FORCEINLINE TYPE NAME(TYPE a, TYPE b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  TYPE ret(a[0] OP b[0], a[1] OP b[1], a[2] OP b[2], a[3] OP b[3]); \
  return ret;                            \
}

#define BINARY_OP_FUNC(TYPE, NAME, FUNC)                \
static FORCEINLINE TYPE NAME(TYPE a, TYPE b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  TYPE ret(FUNC(a[0], b[0]), FUNC(a[1], b[1]), FUNC(a[2], b[2]), FUNC(a[3], b[3])); \
  return ret;                            \
}

/**
 * @brief macros based on __vector type's operator overload
 */

#define BINARY_OP_OPT(TYPE, NAME, OP) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE b) { \
  return TYPE(a.v OP b.v); \
}

#define BINARY_OP_OPT64(TYPE, NAME, OP) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE b) { \
  return TYPE(a.v[0] OP b.v[0], a.v[1] OP b.v[1]); \
}

#define BINARY_OP_OPT_FUNC(TYPE, TYPE_B, NAME, FUNC) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE_B b) { \
  return TYPE(FUNC(a.v, b.v)); \
}

#define BINARY_OP_OPT_FUNC64(TYPE, TYPE_B, NAME, FUNC) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE_B b) { \
  return TYPE(FUNC(a.v[0], b.v[0]), FUNC(a.v[1], b.v[1])); \
}

/**
 * @brief macros for binary: vector op scalar
 */
#define BINARY_OP_SCALAR(VTYPE, STYPE, NAME, OP)                \
static FORCEINLINE VTYPE NAME(VTYPE a, STYPE s) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  VTYPE ret(a[0] OP s, a[1] OP s, a[2] OP s, a[3] OP s); \
  return ret;                            \
}


// add

static FORCEINLINE svec4_i8 svec_add (svec4_i8 a, svec4_i8 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_u8 svec_add(svec4_u8 a, svec4_u8 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_i16 svec_add (svec4_i16 a, svec4_i16 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_u16 svec_add(svec4_u16 a, svec4_u16 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_i32 svec_add (svec4_i32 a, svec4_i32 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_u32 svec_add(svec4_u32 a, svec4_u32 b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_i64 svec_add (svec4_i64 a, svec4_i64 b) {
#ifdef __POWER8
  return svec4_i64(vec_add_p8(a.v[0],b.v[0]),vec_add_p8(a.v[1],b.v[1]) );
#else
  return svec4_i64(a.v[0] + b.v[0],  a.v[1] + b.v[1]);
#endif
}

static FORCEINLINE svec4_u64 svec_add(svec4_u64 a, svec4_u64 b) {
#ifdef __POWER8
  return svec4_u64(vec_add_p8(a.v[0],b.v[0]),vec_add_p8(a.v[1],b.v[1]) );
#else
  return svec4_u64(a.v[0] + b.v[0],  a.v[1] + b.v[1]);
#endif
}

static FORCEINLINE svec4_f svec_add (svec4_f a, svec4_f b) {
  return vec_add(a.v,b.v);
}

static FORCEINLINE svec4_d svec_add(svec4_d a, svec4_d b) {
    return svec4_d(a.v[0] + b.v[0],  a.v[1] + b.v[1]);
}

//sub
static FORCEINLINE svec4_i8 svec_sub (svec4_i8 a, svec4_i8 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_u8 svec_sub(svec4_u8 a, svec4_u8 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_i16 svec_sub (svec4_i16 a, svec4_i16 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_u16 svec_sub(svec4_u16 a, svec4_u16 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_i32 svec_sub (svec4_i32 a, svec4_i32 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_u32 svec_sub(svec4_u32 a, svec4_u32 b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_i64 svec_sub (svec4_i64 a, svec4_i64 b) {
#ifdef __POWER8
  return svec4_i64(vec_sub_p8(a.v[0],b.v[0]),vec_sub_p8(a.v[1],b.v[1]) );
#else
  return svec4_i64(a.v[0] - b.v[0],  a.v[1] - b.v[1]);
#endif
}

static FORCEINLINE svec4_u64 svec_sub(svec4_u64 a, svec4_u64 b) {
#ifdef __POWER8
  return svec4_u64(vec_sub_p8(a.v[0],b.v[0]),vec_sub_p8(a.v[1],b.v[1]) );
#else
  return svec4_u64(a.v[0] - b.v[0],  a.v[1] - b.v[1]);
#endif
}

static FORCEINLINE svec4_f svec_sub (svec4_f a, svec4_f b) {
  return vec_sub(a.v,b.v);
}

static FORCEINLINE svec4_d svec_sub(svec4_d a, svec4_d b) {
    return svec4_d(a.v[0] - b.v[0],  a.v[1] - b.v[1]);
}



//mul
static FORCEINLINE svec4_i8 svec_mul (svec4_i8 a, svec4_i8 b) {
  return a.v * b.v;
}

static FORCEINLINE svec4_u8 svec_mul(svec4_u8 a, svec4_u8 b) {
  return a.v * b.v;
}

static FORCEINLINE svec4_i16 svec_mul (svec4_i16 a, svec4_i16 b) {
  return a.v * b.v;
}

static FORCEINLINE svec4_u16 svec_mul(svec4_u16 a, svec4_u16 b) {
  return a.v * b.v;
}

static FORCEINLINE svec4_i32 svec_mul (svec4_i32 a, svec4_i32 b) {
#ifdef __POWER8
  return ((__vector signed int)vec_mul_p8((vector unsigned int)a.v,(vector unsigned int)b.v));
#else
  //return vec_mulo((__vector signed short)a.v, (__vector signed short)(b.v));
  //adapted from apple web site
  __vector unsigned int bSwapped, BD, AD_plus_BC;
  __vector unsigned int sixteen = vec_splat_u32(-16 ); //only low 5 bits important here
  __vector unsigned int zero = vec_splat_u32(0);
  bSwapped = vec_rl( b.v, sixteen );
  //Calculate A*D + B*C, and B*D
  BD = vec_mulo( (__vector unsigned short) a.v, (__vector unsigned short) b.v );
  AD_plus_BC = vec_msum( (__vector unsigned short) a.v, (__vector unsigned short) bSwapped, zero );

  //Left shift the high results by 16 bits
  AD_plus_BC = vec_sl( AD_plus_BC, sixteen );

  //Add in the BD component
  return vec_add( AD_plus_BC, BD );
#endif
}

static FORCEINLINE svec4_u32 svec_mul(svec4_u32 a, svec4_u32 b) {
#ifdef __POWER8
  return ((__vector signed int)vec_mul_p8((vector unsigned int)a.v,(vector unsigned int)b.v));
#else
  //return vec_mulo((__vector signed short)a.v, (__vector signed short)(b.v));
  //adapted from apple web site
  __vector unsigned int bSwapped, BD, AD_plus_BC;
  __vector unsigned int sixteen = vec_splat_u32(-16 ); //only low 5 bits important here
  __vector unsigned int zero = vec_splat_u32(0);
  bSwapped = vec_rl( b.v, sixteen );
  //Calculate A*D + B*C, and B*D
  BD = vec_mulo( (__vector unsigned short) a.v, (__vector unsigned short) b.v );
  AD_plus_BC = vec_msum( (__vector unsigned short) a.v, (__vector unsigned short) bSwapped, zero );

  //Left shift the high results by 16 bits
  AD_plus_BC = vec_sl( AD_plus_BC, sixteen );

  //Add in the BD component
  return vec_add( AD_plus_BC, BD );
#endif
}

static FORCEINLINE svec4_i64 svec_mul (svec4_i64 a, svec4_i64 b) {
  return svec4_i64(a.v[0] * b.v[0],  a.v[1] * b.v[1]);
}

static FORCEINLINE svec4_u64 svec_mul(svec4_u64 a, svec4_u64 b) {
    return svec4_u64(a.v[0] * b.v[0],  a.v[1] * b.v[1]);
}

static FORCEINLINE svec4_f svec_mul (svec4_f a, svec4_f b) {
  return vec_mul(a.v,b.v);
}

static FORCEINLINE svec4_d svec_mul(svec4_d a, svec4_d b) {
    return svec4_d(a.v[0] * b.v[0],  a.v[1] * b.v[1]);
}

//div

BINARY_OP_OPT(svec4_i8, svec_div, /);
BINARY_OP_OPT(svec4_u8, svec_div, /);
BINARY_OP_OPT(svec4_i16, svec_div, /);
BINARY_OP_OPT(svec4_u16, svec_div, /);
BINARY_OP_OPT(svec4_i32, svec_div, /);
BINARY_OP_OPT(svec4_u32, svec_div, /);
BINARY_OP_OPT64(svec4_i64, svec_div, /);
BINARY_OP_OPT64(svec4_u64, svec_div, /);
BINARY_OP_OPT(svec4_f, svec_div, /);
BINARY_OP_OPT64(svec4_d, svec_div, /);


//power only for float
BINARY_OP_FUNC(svec4_f, svec_pow, powf);
BINARY_OP_FUNC(svec4_d, svec_pow, pow);

//or
BINARY_OP_OPT(svec4_i8, svec_or, |);
BINARY_OP_OPT(svec4_u8, svec_or, |);
BINARY_OP_OPT(svec4_i16, svec_or, |);
BINARY_OP_OPT(svec4_u16, svec_or, |);
BINARY_OP_OPT(svec4_i32, svec_or, |);
BINARY_OP_OPT(svec4_u32, svec_or, |);
BINARY_OP_OPT64(svec4_i64, svec_or, |);
BINARY_OP_OPT64(svec4_u64, svec_or, |);
//and
BINARY_OP_OPT(svec4_i8, svec_and, &);
BINARY_OP_OPT(svec4_u8, svec_and, &);
BINARY_OP_OPT(svec4_i16, svec_and, &);
BINARY_OP_OPT(svec4_u16, svec_and, &);
BINARY_OP_OPT(svec4_i32, svec_and, &);
BINARY_OP_OPT(svec4_u32, svec_and, &);
BINARY_OP_OPT64(svec4_i64, svec_and, &);
BINARY_OP_OPT64(svec4_u64, svec_and, &);

//xor
BINARY_OP_OPT(svec4_i8, svec_xor, ^);
BINARY_OP_OPT(svec4_u8, svec_xor, ^);
BINARY_OP_OPT(svec4_i16, svec_xor, ^);
BINARY_OP_OPT(svec4_u16, svec_xor, ^);
BINARY_OP_OPT(svec4_i32, svec_xor, ^);
BINARY_OP_OPT(svec4_u32, svec_xor, ^);
BINARY_OP_OPT64(svec4_i64, svec_xor, ^);
BINARY_OP_OPT64(svec4_u64, svec_xor, ^);

#define BIN_VEC_SCAL(VTYPE, STYPE) \
static FORCEINLINE VTYPE svec_add_scalar(VTYPE a, STYPE s) { \
  return svec_add(a, VTYPE(s)); \
} \
static FORCEINLINE VTYPE svec_scalar_add(STYPE s, VTYPE a) { \
  return svec_add(VTYPE(s), a); \
} \
static FORCEINLINE VTYPE svec_sub_scalar(VTYPE a, STYPE s) { \
  return svec_sub(a, VTYPE(s)); \
} \
static FORCEINLINE VTYPE svec_scalar_sub(STYPE s, VTYPE a) { \
  return svec_sub(VTYPE(s), a); \
} \
static FORCEINLINE VTYPE svec_mul_scalar(VTYPE a, STYPE s) { \
  return svec_mul(a, VTYPE(s)); \
} \
static FORCEINLINE VTYPE svec_scalar_mul(STYPE s, VTYPE a) { \
  return svec_mul(VTYPE(s), a); \
} \
static FORCEINLINE VTYPE svec_div_scalar(VTYPE a, STYPE s) { \
  return svec_div(a, VTYPE(s)); \
} \
static FORCEINLINE VTYPE svec_scalar_div(STYPE s, VTYPE a) { \
  return svec_div(VTYPE(s), a); \
} \

BIN_VEC_SCAL(svec4_i8, int8_t);
BIN_VEC_SCAL(svec4_u8, uint8_t);
BIN_VEC_SCAL(svec4_i16, int16_t);
BIN_VEC_SCAL(svec4_u16, uint16_t);
BIN_VEC_SCAL(svec4_i32, int32_t);
BIN_VEC_SCAL(svec4_u32, uint32_t);
BIN_VEC_SCAL(svec4_i64, int64_t);
BIN_VEC_SCAL(svec4_u64, uint64_t);
BIN_VEC_SCAL(svec4_f, float);
BIN_VEC_SCAL(svec4_d, double);


//shift left
BINARY_OP_OPT_FUNC(svec4_i8, svec4_u8, svec_shl, vec_sl);
BINARY_OP_OPT_FUNC(svec4_u8, svec4_u8, svec_shl, vec_sl);
BINARY_OP_OPT_FUNC(svec4_i16, svec4_u16, svec_shl, vec_sl);
BINARY_OP_OPT_FUNC(svec4_u16, svec4_u16, svec_shl, vec_sl);
BINARY_OP_OPT_FUNC(svec4_i32, svec4_u32, svec_shl, vec_sl);
BINARY_OP_OPT_FUNC(svec4_u32, svec4_u32, svec_shl, vec_sl);

//BINARY_OP_OPT_FUNC64(svec4_i64, svec4_u64, svec_shl, vec_sl);
static FORCEINLINE svec4_i64  svec_shl(svec4_i64 a, svec4_u64 b) {
  INC_STATS_NAME(STATS_BINARY_SLOW,1, "shl i64"); \
  return svec4_i64(a[0] << b[0], a[1] << b[1], a[2] << b[2], a[3] << b[3]);
}

//BINARY_OP_OPT_FUNC64(svec4_u64, svec4_u64, svec_shl, vec_sl);
static FORCEINLINE svec4_u64  svec_shl(svec4_u64 a, svec4_u64 b) {
  INC_STATS_NAME(STATS_BINARY_SLOW,1, "shl u64"); \
  return svec4_u64(a[0] << b[0], a[1] << b[1], a[2] << b[2], a[3] << b[3]);
}
//shift right
BINARY_OP_OPT_FUNC(svec4_i8, svec4_u8, svec_shr, vec_sr);
BINARY_OP_OPT_FUNC(svec4_u8, svec4_u8, svec_shr, vec_sr);
BINARY_OP_OPT_FUNC(svec4_i16, svec4_u16, svec_shr, vec_sr);
BINARY_OP_OPT_FUNC(svec4_u16, svec4_u16, svec_shr, vec_sr);
BINARY_OP_OPT_FUNC(svec4_i32, svec4_u32, svec_shr, vec_sr);
BINARY_OP_OPT_FUNC(svec4_u32, svec4_u32, svec_shr, vec_sr);

//BINARY_OP_OPT_FUNC64(svec4_i64, svec4_u64, svec_shr, vec_sr);
static FORCEINLINE svec4_i64  svec_shr(svec4_i64 a, svec4_u64 b) {
  INC_STATS_NAME(STATS_BINARY_SLOW,1, "shr i64"); \
  return svec4_i64(a[0] >> b[0], a[1] >> b[1], a[2] >> b[2], a[3] >> b[3]);
}

//BINARY_OP_OPT_FUNC64(svec4_u64, svec4_u64, svec_shr, vec_sr);
static FORCEINLINE svec4_u64  svec_shr(svec4_u64 a, svec4_u64 b) {
  INC_STATS_NAME(STATS_BINARY_SLOW,1, "shr u64"); \
  return svec4_u64(a[0] >> b[0], a[1] >> b[1], a[2] >> b[2], a[3] >> b[3]);
}

//uniform shift left

// a better impl may be by smear and vector shift
BINARY_OP_SCALAR(svec4_i8, int8_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_u8, uint8_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_i16, int16_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_u16, uint16_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_i32, int32_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_u32, uint16_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_i64, int64_t, svec_shl, <<);
BINARY_OP_SCALAR(svec4_u64, uint64_t, svec_shl, <<);
//shift right
BINARY_OP_SCALAR(svec4_i8, int8_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_u8, uint8_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_i16, int16_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_u16, uint16_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_i32, int32_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_u32, uint16_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_i64, int64_t, svec_shr, >>);
BINARY_OP_SCALAR(svec4_u64, uint64_t, svec_shr, >>);

//remainder %

/**
 * @brief remainder impl uses generic one
 */
BINARY_OP(svec4_i8, svec_rem, %);
BINARY_OP(svec4_u8, svec_rem, %);
BINARY_OP(svec4_i16, svec_rem, %);
BINARY_OP(svec4_u16, svec_rem, %);
BINARY_OP(svec4_i32, svec_rem, %);
BINARY_OP(svec4_u32, svec_rem, %);
BINARY_OP(svec4_i64, svec_rem, %);
BINARY_OP(svec4_u64, svec_rem, %);

BINARY_OP_SCALAR(svec4_i8, int8_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_u8, uint8_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_i16, int16_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_u16, uint16_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_i32, int32_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_u32, uint16_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_i64, int64_t, svec_rem, %);
BINARY_OP_SCALAR(svec4_u64, uint64_t, svec_rem, %);


//  4. Ternary

//madd / msub for only int32/u32/float/double

#define TERNERY(VTYPE) \
/**
 * @brief vector multiply and add operation. return a * b + c.
 */ \
FORCEINLINE VTYPE svec_madd(VTYPE a, VTYPE b, VTYPE c) { return a*b+c;} \
/**
 * @brief vector multiply and add operation. return a * b - c.
 */ \
FORCEINLINE VTYPE svec_msub(VTYPE a, VTYPE b, VTYPE c) { return a*b-c;}


TERNERY(svec4_i32);
TERNERY(svec4_u32);
TERNERY(svec4_i64);
TERNERY(svec4_u64);

/**
 * @brief vector multiply and add operation. return a * b + c.
 */
FORCEINLINE svec4_f svec_madd(svec4_f a, svec4_f b, svec4_f c) {
    return vec_madd(a.v, b.v, c.v);
}
/**
 * @brief vector multiply and add operation. return a * b - c.
 */
FORCEINLINE svec4_d svec_madd(svec4_d a, svec4_d b, svec4_d c) {
    return svec4_d(vec_madd(a.v[0], b.v[0], c.v[0]), vec_madd(a.v[1], b.v[1], c.v[1]));
}
/**
 * @brief vector multiply and add operation. return a * b + c.
 */
FORCEINLINE svec4_f svec_msub(svec4_f a, svec4_f b, svec4_f c) {
    return vec_msub(a.v, b.v, c.v);
}
/**
 * @brief vector multiply and add operation. return a * b - c.
 */
FORCEINLINE svec4_d svec_msub(svec4_d a, svec4_d b, svec4_d c) {
    return svec4_d(vec_msub(a.v[0], b.v[0], c.v[0]), vec_msub(a.v[1], b.v[1], c.v[1]));
}

//  5. Max/Min

//add/max/min
template<class T> static FORCEINLINE T add(T a, T b) {
  return a+b;
}
template<class T> static FORCEINLINE T max(T a, T b) {
  return a > b ? a : b;
}
template<class T> static FORCEINLINE T min(T a, T b) {
  return a < b ? a : b;
}
BINARY_OP_OPT_FUNC(svec4_i8, svec4_i8, svec_max, vec_max);
BINARY_OP_OPT_FUNC(svec4_u8, svec4_u8, svec_max, vec_max);
BINARY_OP_OPT_FUNC(svec4_i16, svec4_i16, svec_max, vec_max);
BINARY_OP_OPT_FUNC(svec4_u16, svec4_u16, svec_max, vec_max);
BINARY_OP_OPT_FUNC(svec4_i32, svec4_i32, svec_max, vec_max);
BINARY_OP_OPT_FUNC(svec4_u32, svec4_u32, svec_max, vec_max);
BINARY_OP_FUNC(svec4_i64, svec_max, max<int64_t>);
BINARY_OP_FUNC(svec4_u64, svec_max, max<uint64_t>);
BINARY_OP_OPT_FUNC(svec4_f, svec4_f, svec_max, vec_max);
BINARY_OP_FUNC(svec4_d, svec_max, max<double>);

BINARY_OP_OPT_FUNC(svec4_i8, svec4_i8, svec_min, vec_min);
BINARY_OP_OPT_FUNC(svec4_u8, svec4_u8, svec_min, vec_min);
BINARY_OP_OPT_FUNC(svec4_i16, svec4_i16, svec_min, vec_min);
BINARY_OP_OPT_FUNC(svec4_u16, svec4_u16, svec_min, vec_min);
BINARY_OP_OPT_FUNC(svec4_i32, svec4_i32, svec_min, vec_min);
BINARY_OP_OPT_FUNC(svec4_u32, svec4_u32, svec_min, vec_min);
BINARY_OP_FUNC(svec4_i64, svec_min, min<int64_t>);
BINARY_OP_FUNC(svec4_u64, svec_min, min<uint64_t>);
BINARY_OP_OPT_FUNC(svec4_f, svec4_f, svec_min, vec_min);
BINARY_OP_FUNC(svec4_d, svec_min, min<double>);

// 6. reduce
#define BINARY_OP_REDUCE_FUNC(VTYPE, STYPE, NAME, FUNC) \
static FORCEINLINE STYPE NAME(VTYPE a) {            \
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "reduce");   \
  return FUNC(FUNC(FUNC(a[0], a[1]), a[2]), a[3]); \
}

BINARY_OP_REDUCE_FUNC(svec4_i8, int8_t, svec_reduce_add, add<int8_t>);
BINARY_OP_REDUCE_FUNC(svec4_u8, uint8_t, svec_reduce_add, add<uint8_t>);
BINARY_OP_REDUCE_FUNC(svec4_i16, int16_t, svec_reduce_add, add<int16_t>);
BINARY_OP_REDUCE_FUNC(svec4_u16, uint16_t, svec_reduce_add, add<uint16_t>);
BINARY_OP_REDUCE_FUNC(svec4_i32, int32_t, svec_reduce_add, add<int32_t>);
BINARY_OP_REDUCE_FUNC(svec4_u32, uint32_t, svec_reduce_add, add<uint32_t>);
BINARY_OP_REDUCE_FUNC(svec4_i64, int64_t, svec_reduce_add, add<int64_t>);
BINARY_OP_REDUCE_FUNC(svec4_u64, uint64_t, svec_reduce_add, add<uint64_t>);
BINARY_OP_REDUCE_FUNC(svec4_f, float, svec_reduce_add, add<float>);
BINARY_OP_REDUCE_FUNC(svec4_d, double, svec_reduce_add, add<double>);

BINARY_OP_REDUCE_FUNC(svec4_i8, int8_t, svec_reduce_max, max<int8_t>);
BINARY_OP_REDUCE_FUNC(svec4_u8, uint8_t, svec_reduce_max, max<uint8_t>);
BINARY_OP_REDUCE_FUNC(svec4_i16, int16_t, svec_reduce_max, max<int16_t>);
BINARY_OP_REDUCE_FUNC(svec4_u16, uint16_t, svec_reduce_max, max<uint16_t>);
BINARY_OP_REDUCE_FUNC(svec4_i32, int32_t, svec_reduce_max, max<int32_t>);
BINARY_OP_REDUCE_FUNC(svec4_u32, uint32_t, svec_reduce_max, max<uint32_t>);
BINARY_OP_REDUCE_FUNC(svec4_i64, int64_t, svec_reduce_max, max<int64_t>);
BINARY_OP_REDUCE_FUNC(svec4_u64, uint64_t, svec_reduce_max, max<uint64_t>);
BINARY_OP_REDUCE_FUNC(svec4_f, float, svec_reduce_max, max<float>);
BINARY_OP_REDUCE_FUNC(svec4_d, double, svec_reduce_max, max<double>);

BINARY_OP_REDUCE_FUNC(svec4_i8, int8_t, svec_reduce_min, min<int8_t>);
BINARY_OP_REDUCE_FUNC(svec4_u8, uint8_t, svec_reduce_min, min<uint8_t>);
BINARY_OP_REDUCE_FUNC(svec4_i16, int16_t, svec_reduce_min, min<int16_t>);
BINARY_OP_REDUCE_FUNC(svec4_u16, uint16_t, svec_reduce_min, min<uint16_t>);
BINARY_OP_REDUCE_FUNC(svec4_i32, int32_t, svec_reduce_min, min<int32_t>);
BINARY_OP_REDUCE_FUNC(svec4_u32, uint32_t, svec_reduce_min, min<uint32_t>);
BINARY_OP_REDUCE_FUNC(svec4_i64, int64_t, svec_reduce_min, min<int64_t>);
BINARY_OP_REDUCE_FUNC(svec4_u64, uint64_t, svec_reduce_min, min<uint64_t>);
BINARY_OP_REDUCE_FUNC(svec4_f, float, svec_reduce_min, min<float>);
BINARY_OP_REDUCE_FUNC(svec4_d, double, svec_reduce_min, min<double>);


//  7. Compare
/**
 * Macros for generic compare operations
 */
#define CMP_OP(VTYPE, MTYPE, NAME, OP)                \
  static FORCEINLINE MTYPE svec_##NAME(VTYPE a, VTYPE b) {    \
    INC_STATS_NAME(STATS_COMPARE_SLOW, 1, #NAME);                   \
    uint32_t r0 = (a[0] OP b[0]); \
    uint32_t r1 = (a[1] OP b[1]); \
    uint32_t r2 = (a[2] OP b[2]); \
    uint32_t r3 = (a[3] OP b[3]); \
    return MTYPE(r0,r1,r2,r3);                  \
  }

#define CMP_ALL_OP(VTYPE, MTYPE)    \
  CMP_OP(VTYPE, MTYPE, equal, ==) \
  CMP_OP(VTYPE, MTYPE, not_equal, !=) \
  CMP_OP(VTYPE, MTYPE, less_than, <) \
  CMP_OP(VTYPE, MTYPE, less_equal, <=) \
  CMP_OP(VTYPE, MTYPE, greater_than, >) \
  CMP_OP(VTYPE, MTYPE, greater_equal, >=)

/**
 * Macros for masked operation based on fast operation
 */
#define CMP_MASKED_OP(VTYPE, MTYPE, NAME, OP)   \
  /**
   * @brief Do NAME operation on a and b with mask
   * If mask is true, return the compare result, otherwise return false.
   */\
  FORCEINLINE MTYPE svec_masked_##NAME(VTYPE a, VTYPE b, \
                                        MTYPE mask) { \
    return svec_and(svec_##NAME(a,b) , mask);              \
  }

#define CMP_ALL_MASKED_OP(VTYPE, MTYPE)  \
  CMP_MASKED_OP(VTYPE, MTYPE, equal, ==) \
  CMP_MASKED_OP(VTYPE, MTYPE, not_equal, !=) \
  CMP_MASKED_OP(VTYPE, MTYPE, less_than, <) \
  CMP_MASKED_OP(VTYPE, MTYPE, less_equal, <=) \
  CMP_MASKED_OP(VTYPE, MTYPE, greater_than, >) \
  CMP_MASKED_OP(VTYPE, MTYPE, greater_equal, >=)


/**
 * @brief element by element comparison of two svec_vec4_i1 type object
 * @param a
 * @param b
 * @return a svec_vec4_i1 object
 */
static FORCEINLINE svec4_i1 svec_equal(svec4_i1 a, svec4_i1 b) {
  return (__vector unsigned int)(vec_cmpeq(a.v, b.v));
}

/**
 * @brief element by element comparison of two svec_vec4_i1 type object
 * @param a
 * @param b
 * @return a svec_vec4_i1 object
 */
static FORCEINLINE svec4_i1 svec_not_equal(svec4_i1 a, svec4_i1 b) {
  return ~(__vector unsigned int)(vec_cmpeq(a.v, b.v));
}


static FORCEINLINE svec4_i1 svec_equal(svec4_i8 a, svec4_i8 b) {
    __vector bool char t = vec_cmpeq(a.v,b.v);
    return (__vector unsigned int)vec_unpackh(vec_unpackh(t));
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_i8 a, svec4_i8 b) {
    return ~ svec_equal(a, b);
}

CMP_OP(svec4_i8, svec4_i1, less_than, <);
CMP_OP(svec4_i8, svec4_i1, less_equal, <=);
CMP_OP(svec4_i8, svec4_i1, greater_than, >);
CMP_OP(svec4_i8, svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(svec4_i8, svec4_i1);

static FORCEINLINE svec4_i1 svec_equal(svec4_u8 a, svec4_u8 b) {
    __vector bool char t = vec_cmpeq(a.v,b.v);
    return (__vector unsigned int)vec_unpackh(vec_unpackh(t));
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_u8 a, svec4_u8 b) {
    return ~ svec_equal(a, b);
}

CMP_OP(svec4_u8, svec4_i1, less_than, <);
CMP_OP(svec4_u8, svec4_i1, less_equal, <=);
CMP_OP(svec4_u8, svec4_i1, greater_than, >);
CMP_OP(svec4_u8, svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(svec4_u8, svec4_i1);

/**
 * @brief svec4_i16/svec4_u16 have no fast impl of cmp ops
 */
CMP_ALL_OP(svec4_i16, svec4_i1);
CMP_ALL_MASKED_OP(svec4_i16, svec4_i1);

CMP_ALL_OP(svec4_u16, svec4_i1);
CMP_ALL_MASKED_OP(svec4_u16, svec4_i1);

/**
 * @brief svec4_i32/svec4_u32 have fast impl of cmp ops
 */

static FORCEINLINE svec4_i1 svec_equal(svec4_i32 a, svec4_i32 b) {
  return (__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_i32 a, svec4_i32 b) {
  return ~(__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_than(svec4_i32 a, svec4_i32 b) {
  return (__vector unsigned int)vec_cmplt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_equal(svec4_i32 a, svec4_i32 b) {
  return svec_less_than(a, b) | svec_equal(a, b);
}

static FORCEINLINE svec4_i1 svec_greater_than(svec4_i32 a, svec4_i32 b) {
  return (__vector unsigned int)vec_cmpgt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_greater_equal(svec4_i32 a, svec4_i32 b) {
  return svec_greater_than(a, b) | svec_equal(a, b);
}

CMP_ALL_MASKED_OP(svec4_i32, svec4_i1);

static FORCEINLINE svec4_i1 svec_equal(svec4_u32 a, svec4_u32 b) {
  return (__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_u32 a, svec4_u32 b) {
  return ~(__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_than(svec4_u32 a, svec4_u32 b) {
  return (__vector unsigned int)vec_cmplt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_equal(svec4_u32 a, svec4_u32 b) {
  return svec_less_than(a, b) | svec_equal(a, b);
}

static FORCEINLINE svec4_i1 svec_greater_than(svec4_u32 a, svec4_u32 b) {
  return (__vector unsigned int)vec_cmpgt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_greater_equal(svec4_u32 a, svec4_u32 b) {
  return svec_greater_than(a, b) | svec_equal(a, b);
}

CMP_ALL_MASKED_OP(svec4_u32, svec4_i1);

/**
 * @brief svec_i64/u64 has fast impl for ==/!= on POWER8
 */

static FORCEINLINE svec4_i1 svec_equal(svec4_i64 a, svec4_i64 b) {
#ifdef __POWER8
  __vector signed long long tr1 = vec_cmpeq_p8(a.v[0], b.v[0]);
  __vector signed long long tr2 = vec_cmpeq_p8(a.v[1], b.v[1]);
  svec4_i1 res2 = vec_pack_p8(tr1,tr2);
  return res2;
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "equal_i64");
  unsigned int r0 = a[0] == b[0];
  unsigned int r1 = a[1] == b[1];
  unsigned int r2 = a[2] == b[2];
  unsigned int r3 = a[3] == b[3];
  svec4_i1 res =  svec4_i1(r0,r1,r2,r3);
  return res;
#endif
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_i64 a, svec4_i64 b) {
  return ~ svec_equal(a, b);
}

CMP_OP(svec4_i64, svec4_i1, less_than, <);
CMP_OP(svec4_i64, svec4_i1, less_equal, <=);
CMP_OP(svec4_i64, svec4_i1, greater_than, >);
CMP_OP(svec4_i64, svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(svec4_i64, svec4_i1);

static FORCEINLINE svec4_i1 svec_equal(svec4_u64 a, svec4_u64 b) {
#ifdef __POWER8
  __vector signed long long tr1 = vec_cmpeq_p8(a.v[0], b.v[0]);
  __vector signed long long tr2 = vec_cmpeq_p8(a.v[1], b.v[1]);
  svec4_i1 res2 = vec_pack_p8(tr1,tr2);
  return res2;
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "equal_u64");
  unsigned int r0 = a[0] == b[0];
  unsigned int r1 = a[1] == b[1];
  unsigned int r2 = a[2] == b[2];
  unsigned int r3 = a[3] == b[3];
  svec4_i1 res =  svec4_i1(r0,r1,r2,r3);
  return res;
#endif
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_u64 a, svec4_u64 b) {
  return ~ svec_equal(a, b);
}

CMP_OP(svec4_u64, svec4_i1, less_than, <);
CMP_OP(svec4_u64, svec4_i1, less_equal, <=);
CMP_OP(svec4_u64, svec4_i1, greater_than, >);
CMP_OP(svec4_u64, svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(svec4_u64, svec4_i1);

/**
 * @brief float vec have fast impl of cmp ops
 */

static FORCEINLINE svec4_i1 svec_equal(svec4_f a, svec4_f b) {
  return (__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_not_equal(svec4_f a, svec4_f b) {
  return ~(__vector unsigned int)vec_cmpeq(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_than(svec4_f a, svec4_f b) {
  return (__vector unsigned int)vec_cmplt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_less_equal(svec4_f a, svec4_f b) {
  return (__vector unsigned int)vec_cmple(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_greater_than(svec4_f a, svec4_f b) {
  return (__vector unsigned int)vec_cmpgt(a.v,b.v);
}

static FORCEINLINE svec4_i1 svec_greater_equal(svec4_f a, svec4_f b) {
    return (__vector unsigned int)vec_cmpge(a.v,b.v);
}

CMP_ALL_MASKED_OP(svec4_f, svec4_i1);

/**
 * @brief double vec has fast impl for >, < for POWER8
 * Check why double has not ==, !=
 */
CMP_OP(svec4_d, svec4_i1, equal, ==);
CMP_OP(svec4_d, svec4_i1, not_equal, !=);

static FORCEINLINE svec4_i1 svec_less_than(svec4_d a, svec4_d b) {
#ifdef __POWER8
  __vector signed long long tr1 = (__vector signed long long)vec_cmplt(a.v[0], b.v[0]);
  __vector signed long long tr2 = (__vector signed long long)vec_cmplt(a.v[1], b.v[1]);
  return vec_pack_p8(tr1,tr2);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "less_than_double");
  unsigned int r0 = a[0] < b[0];
  unsigned int r1 = a[1] < b[1];
  unsigned int r2 = a[2] < b[2];
  unsigned int r3 = a[3] < b[3];
  return svec4_i1(r0,r1,r2,r3);
#endif
}

static FORCEINLINE svec4_i1 svec_less_equal(svec4_d a, svec4_d b) {
    return svec_less_than(a, b) | svec_equal(a, b);
}


static FORCEINLINE svec4_i1 svec_greater_than(svec4_d a, svec4_d b) {
#ifdef __POWER8
  __vector signed long long tr1 = (__vector signed long long)vec_cmpgt(a.v[0], b.v[0]);
  __vector signed long long tr2 = (__vector signed long long)vec_cmpgt(a.v[1], b.v[1]);
  return vec_pack_p8(tr1,tr2);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "greater_than_double");
  unsigned int r0 = a[0] > b[0];
  unsigned int r1 = a[1] > b[1];
  unsigned int r2 = a[2] > b[2];
  unsigned int r3 = a[3] > b[3];
  return svec4_i1(r0,r1,r2,r3);
#endif
}

static FORCEINLINE svec4_i1 svec_greater_equal(svec4_d a, svec4_d b) {
    return svec_greater_than(a, b) | svec_equal(a, b);
}

CMP_ALL_MASKED_OP(svec4_d, svec4_i1);

//  8. Cast
#define CAST(FROM, TO, STO)        \
template <class T> static T svec_cast(FROM val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast<TO>(FROM val) {      \
    INC_STATS_NAME(STATS_CAST_SLOW, 1, #FROM"-"#TO);          \
    return TO((STO)val[0],(STO)val[1],(STO)val[2],(STO)val[3]); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_OPT(FROM, TO, STO)        \
template <class T> static T svec_cast(FROM val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast<TO>(FROM val) {      \
    return TO((__vector STO)(val.v)); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_OPT64(FROM, TO, STO)        \
template <class T> static T svec_cast(FROM val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast<TO>(FROM val) {      \
    return TO((__vector STO)(val.v[0]), (__vector STO)(val.v[1])); \
}

/**
 * Here we provide the full cast combinations.
 * Some may have fast impl
 */



//i1 -> all
//CAST(svec4_i1, svec4_i1, uint32_t);
CAST(svec4_i1, svec4_i8, int8_t);  //better way: packing
CAST(svec4_i1, svec4_u8, uint8_t);  //better way: packing
CAST(svec4_i1, svec4_i16, int16_t);  //better way: packing
CAST(svec4_i1, svec4_u16, uint16_t); //better way: packing
CAST_OPT(svec4_i1, svec4_i32, int32_t);
CAST_OPT(svec4_i1, svec4_u32, uint32_t);
CAST(svec4_i1, svec4_i64, int64_t); //better way: unpack, singed ext
CAST(svec4_i1, svec4_u64, uint64_t);//better way: unpack, singed ext
CAST(svec4_i1, svec4_f, float); //si to fp call
CAST(svec4_i1, svec4_d, double);

//i8 -> all
CAST(svec4_i8, svec4_i1, uint32_t);
//CAST(svec4_i8, svec4_i8, int8_t);
CAST_OPT(svec4_i8, svec4_u8, uint8_t);
//CAST(svec4_i8, svec4_i16, int16_t); //better way, use vec_unpackh
template <class T> static T svec_cast(svec4_i8 val);
/**
 * @brief cast val from svec4_i8 type to svec4_i16 type.
 */
template <> FORCEINLINE svec4_i16 svec_cast<svec4_i16>(svec4_i8 val) {
    return vec_unpackh(val.v);
}
//CAST(svec4_i8, svec4_u16, uint16_t); //better way, sext + zero mask and
template <class T> static T svec_cast(svec4_i8 val);
/**
 * @brief cast val from svec4_i8 type to svec4_u16 type.
 */
template <> FORCEINLINE svec4_u16 svec_cast<svec4_u16>(svec4_i8 val) {
    __vector uint16_t v = vec_unpackh(val.v);
    return (v);
}
//CAST(svec4_i8, svec4_i32, int32_t); //better way, use twice vec_unpack
template <class T> static T svec_cast(svec4_i8 val);
/**
 * @brief cast val from svec4_i8 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_i8 val) {
    return vec_unpackh(vec_unpackh(val.v));
}
//CAST(svec4_i8, svec4_u32, uint32_t); //better way, use unpack + zero mask
template <class T> static T svec_cast(svec4_i8 val);
/**
 * @brief cast val from svec4_i8 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_i8 val) {
    __vector uint32_t v = vec_unpackh(vec_unpackh(val.v));
    return (v);
}
CAST(svec4_i8, svec4_i64, int64_t);
CAST(svec4_i8, svec4_u64, uint64_t);
CAST(svec4_i8, svec4_f, float);
CAST(svec4_i8, svec4_d, double);

//u8 -> all
CAST(svec4_u8, svec4_i1, uint32_t);
CAST_OPT(svec4_u8, svec4_i8, int8_t);
//CAST(svec4_u8, svec4_u8, uint8_t);
//CAST(svec4_u8, svec4_i16, int16_t); //better way, use unpack + zero mask
template <class T> static T svec_cast(svec4_u8 val);
/**
 * @brief cast val from svec4_u8 type to svec4_i16 type.
 */
template <> FORCEINLINE svec4_i16 svec_cast<svec4_i16>(svec4_u8 val) {
    __vector int16_t v = vec_unpackh((__vector int8_t)val.v);
    __vector int16_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0};
    return (v & mask);
}
//CAST(svec4_u8, svec4_u16, uint16_t); //better way use unpack + zero mask
template <class T> static T svec_cast(svec4_u8 val);
/**
 * @brief cast val from svec4_u8 type to svec4_u16 type.
 */
template <> FORCEINLINE svec4_u16 svec_cast<svec4_u16>(svec4_u8 val) {
    __vector uint16_t v = vec_unpackh((__vector int8_t)val.v);
    __vector uint16_t mask = {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0};
    return (v & mask);
}
//CAST(svec4_u8, svec4_i32, int32_t);
template <class T> static T svec_cast(svec4_u8 val); //better way use unpack + zero mask
/**
 * @brief cast val from svec4_u8 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_u8 val) {
    __vector int32_t v = vec_unpackh(vec_unpackh((__vector int8_t)val.v));
    __vector int32_t mask = {0xFF, 0xFF, 0xFF, 0xFF};
    return (v & mask);
}
//CAST(svec4_u8, svec4_u32, uint32_t);
template <class T> static T svec_cast(svec4_u8 val); //better way use unpack + zero mask
/**
 * @brief cast val from svec4_u8 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_u8 val) {
    __vector uint32_t v = vec_unpackh(vec_unpackh((__vector int8_t)val.v));
    __vector uint32_t mask = {0xFF, 0xFF, 0xFF, 0xFF};
    return (v & mask);
}
CAST(svec4_u8, svec4_i64, int64_t);
CAST(svec4_u8, svec4_u64, uint64_t);
CAST(svec4_u8, svec4_f, float);
CAST(svec4_u8, svec4_d, double);

//i16 -> all
CAST(svec4_i16, svec4_i1, uint32_t);
CAST(svec4_i16, svec4_i8, int8_t); //could use pack
CAST(svec4_i16, svec4_u8, uint8_t); //could use pack
//CAST(svec4_i16, svec4_i16, int16_t);
CAST_OPT(svec4_i16, svec4_u16, uint16_t);
//CAST(svec4_i16, svec4_i32, int32_t); //use unpack
template <class T> static T svec_cast(svec4_i16 val);
/**
 * @brief cast val from svec4_i16 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_i16 val) {
    return vec_unpackh(val.v);
}
//CAST(svec4_i16, svec4_u32, uint32_t); //use unpack and zeromaskout
template <class T> static T svec_cast(svec4_i16 val);
/**
 * @brief cast val from svec4_i16 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_i16 val) {
    __vector uint32_t v = vec_unpackh(val.v);
    return (v);
}
CAST(svec4_i16, svec4_i64, int64_t);
CAST(svec4_i16, svec4_u64, uint64_t);
CAST(svec4_i16, svec4_f, float);
CAST(svec4_i16, svec4_d, double);

//u16 -> all
CAST(svec4_u16, svec4_i1, uint32_t);
CAST(svec4_u16, svec4_i8, int8_t);
CAST(svec4_u16, svec4_u8, uint8_t);
CAST_OPT(svec4_u16, svec4_i16, int16_t);
//CAST(svec4_u16, svec4_u16, uint16_t);
//CAST(svec4_u16, svec4_i32, int32_t); //use unpack +mask
template <class T> static T svec_cast(svec4_u16 val);
/**
 * @brief cast val from svec4_u16 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_u16 val) {
    __vector int32_t v = vec_unpackh((__vector int16_t)val.v);
    __vector int32_t mask = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    return (v & mask);
}
//CAST(svec4_u16, svec4_u32, uint32_t); //use unpack + mask
template <class T> static T svec_cast(svec4_u16 val);
/**
 * @brief cast val from svec4_u16 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_u16 val) {
    __vector uint32_t v = vec_unpackh((__vector int16_t)val.v);
    __vector uint32_t mask = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    return (v & mask);
}
CAST(svec4_u16, svec4_i64, int64_t);
CAST(svec4_u16, svec4_u64, uint64_t);
CAST(svec4_u16, svec4_f, float);
CAST(svec4_u16, svec4_d, double);

//i32 -> all
CAST(svec4_i32, svec4_i1, uint32_t);
CAST(svec4_i32, svec4_i8, int8_t);
CAST(svec4_i32, svec4_u8, uint8_t);
CAST(svec4_i32, svec4_i16, int16_t);
CAST(svec4_i32, svec4_u16, uint16_t);
//CAST(svec4_i32, svec4_i32, int32_t);
CAST_OPT(svec4_i32, svec4_u32, uint32_t);
//CAST(svec4_i32, svec4_i64, int64_t); //use p8 unpack
template <class T> static T svec_cast(svec4_i32 val);
/**
 * @brief cast val from svec4_i32 type to svec4_i64 type.
 */
template <> FORCEINLINE svec4_i64 svec_cast<svec4_i64>(svec4_i32 val) {
#ifdef __POWER8
  return svec4_i64(vec_unpackh_p8((__vector unsigned int)val.v),vec_unpackl_p8((__vector unsigned int)val.v));
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast i32 to i64");
  return  svec4_i64((int64_t)val[0], (int64_t)val[1], (int64_t)val[2], (int64_t)val[3]);
#endif
}
//CAST(svec4_i32, svec4_u64, uint64_t); //use p8 unpack
template <class T> static T svec_cast(svec4_i32 val);
/**
 * @brief cast val from svec4_i32 type to svec4_u64 type.
 */
template <> FORCEINLINE svec4_u64 svec_cast<svec4_u64>(svec4_i32 val) {
#ifdef __POWER9
  return svec4_u64(vec_unpackh_p8((__vector unsigned int)val.v),vec_unpackl_p8((__vector unsigned int)val.v));
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast i32 to u64");
  return  svec4_u64((uint64_t)val[0], (uint64_t)val[1], (uint64_t)val[2], (uint64_t)val[3]);
#endif
}
//CAST(svec4_i32, svec4_f, float); //use ctf
template <class T> static T svec_cast(svec4_i32 val);
/**
 * @brief cast val from svec4_i32 type to svec4_f type.
 */
template <> FORCEINLINE svec4_f svec_cast<svec4_f> (svec4_i32 val) {
  return vec_ctf(val.v,0);
}
CAST(svec4_i32, svec4_d, double);

//u32 -> all
CAST(svec4_u32, svec4_i1, uint32_t);
CAST(svec4_u32, svec4_i8, int8_t);
CAST(svec4_u32, svec4_u8, uint8_t);
CAST(svec4_u32, svec4_i16, int16_t);
CAST(svec4_u32, svec4_u16, uint16_t);
CAST_OPT(svec4_u32, svec4_i32, int32_t);
//CAST(svec4_u32, svec4_u32, uint32_t);
//CAST(svec4_u32, svec4_i64, int64_t); //use p8 unpack
template <class T> static T svec_cast(svec4_u32 val);
/**
 * @brief cast val from svec4_u32 type to svec4_i64 type.
 */
template <> FORCEINLINE svec4_i64 svec_cast<svec4_i64>(svec4_u32 val) {
#ifdef __POWER9
  return svec4_i64(vec_unpackh_p8((__vector unsigned int)val.v),vec_unpackl_p8((__vector unsigned int)val.v));
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast u32 to i64");
  return  svec4_i64((int64_t)val[0], (int64_t)val[1], (int64_t)val[2], (int64_t)val[3]);
#endif
}
//CAST(svec4_u32, svec4_u64, uint64_t); //use p8 unpack
template <class T> static T svec_cast(svec4_u32 val);
/**
 * @brief cast val from svec4_u32 type to svec4_u64 type.
 */
template <> FORCEINLINE svec4_u64 svec_cast<svec4_u64>(svec4_u32 val) {
#ifdef __POWER9
  return svec4_u64(vec_unpackh_p8((__vector unsigned int)val.v),vec_unpackl_p8((__vector unsigned int)val.v));
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast u32 to u64");
  return  svec4_u64((uint64_t)val[0], (uint64_t)val[1], (uint64_t)val[2], (uint64_t)val[3]);
#endif
}
CAST(svec4_u32, svec4_f, float);
CAST(svec4_u32, svec4_d, double);

//i64-> all
CAST(svec4_i64, svec4_i1, uint32_t);
CAST(svec4_i64, svec4_i8, int8_t);
CAST(svec4_i64, svec4_u8, uint8_t);
CAST(svec4_i64, svec4_i16, int16_t);
CAST(svec4_i64, svec4_u16, uint16_t);
//CAST(svec4_i64, svec4_i32, int32_t); //use p8 trunk
template <class T> static T svec_cast(svec4_i64 val);
/**
 * @brief cast val from svec4_i64 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_i64 val) {
#ifdef __POWER9
  return (__vector signed int)vec_pack_p8(val.v[0],val.v[1]);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast i64 to i32");
  return  svec4_i32((int32_t)val[0], (int32_t)val[1], (int32_t)val[2], (int32_t)val[3]);
#endif
}
//CAST(svec4_i64, svec4_u32, uint32_t); //use p8 trunk
template <class T> static T svec_cast(svec4_i64 val);
/**
 * @brief cast val from svec4_i64 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_i64 val) {
#ifdef __POWER9
  return (__vector unsigned int)vec_pack_p8(val.v[0],val.v[1]);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast i64 to u32");
  return  svec4_u32((uint32_t)val[0], (uint32_t)val[1], (uint32_t)val[2], (uint32_t)val[3]);
#endif
}
//CAST(svec4_i64, svec4_i64, int64_t);
CAST_OPT64(svec4_i64, svec4_u64, uint64_t);
CAST(svec4_i64, svec4_f, float);
CAST(svec4_i64, svec4_d, double);

//u64 -> all
CAST(svec4_u64, svec4_i1, uint32_t);
CAST(svec4_u64, svec4_i8, int8_t);
CAST(svec4_u64, svec4_u8, uint8_t);
CAST(svec4_u64, svec4_i16, int16_t);
CAST(svec4_u64, svec4_u16, uint16_t);
//CAST(svec4_u64, svec4_i32, int32_t); //use p8 pack
template <class T> static T svec_cast(svec4_u64 val);
/**
 * @brief cast val from svec4_u64 type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_u64 val) {
#ifdef __POWER9
  return (__vector signed int)vec_pack_p8(val.v[0],val.v[1]);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast u64 to i32");
  return  svec4_i32((int32_t)val[0], (int32_t)val[1], (int32_t)val[2], (int32_t)val[3]);
#endif
}
//CAST(svec4_u64, svec4_u32, uint32_t); //use p8 pack
template <class T> static T svec_cast(svec4_u64 val);
/**
 * @brief cast val from svec4_u64 type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_u64 val) {
#ifdef __POWER9
  return (__vector unsigned int)vec_pack_p8(val.v[0],val.v[1]);
#else
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "cast u64 to u32");
  return  svec4_u32((uint32_t)val[0], (uint32_t)val[1], (uint32_t)val[2], (uint32_t)val[3]);
#endif
}
CAST_OPT64(svec4_u64, svec4_i64, int64_t);
//CAST(svec4_u64, svec4_u64, uint64_t);
CAST(svec4_u64, svec4_f, float);
CAST(svec4_u64, svec4_d, double);

//float -> all
CAST(svec4_f, svec4_i1, uint32_t);
//CAST(svec4_f, svec4_i8, int8_t); //use cts + pack+pack
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_i8 type.
 */
template <> FORCEINLINE svec4_i8 svec_cast<svec4_i8>(svec4_f val) {
    __vector signed int tsi=vec_splat_s32(0);//{0,0,0,0};
    return vec_pack(vec_pack(vec_cts(val.v, 0), tsi), (__vector signed short)tsi);
}
//CAST(svec4_f, svec4_u8, uint8_t); //use ctu + pack + pack
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_u8 type.
 */
template <> FORCEINLINE svec4_u8 svec_cast<svec4_u8>(svec4_f val) {
    __vector unsigned int tsi=vec_splat_s32(0);//{0,0,0,0};
    return vec_pack(vec_pack(vec_ctu(val.v, 0), tsi), (__vector unsigned short)tsi);

}
//CAST(svec4_f, svec4_i16, int16_t); //use cts + pack
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_i16 type.
 */
template <> FORCEINLINE svec4_i16 svec_cast<svec4_i16>(svec4_f val) {
    __vector signed int tsi=vec_splat_s32(0);//{0,0,0,0};
    return vec_pack(vec_cts(val.v, 0), tsi);
}
//CAST(svec4_f, svec4_u16, uint16_t); //use ctu + pack
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_u16 type.
 */
template <> FORCEINLINE svec4_u16 svec_cast<svec4_u16>(svec4_f val) {
    __vector unsigned int tsi=vec_splat_s32(0);//{0,0,0,0};
    return vec_pack(vec_ctu(val.v, 0), tsi);
}
//CAST(svec4_f, svec4_i32, int32_t);//use cts
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_i32 type.
 */
template <> FORCEINLINE svec4_i32 svec_cast<svec4_i32>(svec4_f val) {
    return vec_cts(val.v, 0);
}
//CAST(svec4_f, svec4_u32, uint32_t); //use ctu
template <class T> static T svec_cast(svec4_f val);
/**
 * @brief cast val from svec4_f type to svec4_u32 type.
 */
template <> FORCEINLINE svec4_u32 svec_cast<svec4_u32>(svec4_f val) {
    return vec_ctu(val.v, 0);
}
CAST(svec4_f, svec4_i64, int64_t);
CAST(svec4_f, svec4_u64, uint64_t);
//CAST(svec4_f, svec4_f, float);
CAST(svec4_f, svec4_d, double);

//double -> all
CAST(svec4_d, svec4_i1, uint32_t);
CAST(svec4_d, svec4_i8, int8_t);
CAST(svec4_d, svec4_u8, uint8_t);
CAST(svec4_d, svec4_i16, int16_t);
CAST(svec4_d, svec4_u16, uint16_t);
CAST(svec4_d, svec4_i32, int32_t);
CAST(svec4_d, svec4_u32, uint32_t);
CAST(svec4_d, svec4_i64, int64_t);
CAST(svec4_d, svec4_u64, uint64_t);
CAST(svec4_d, svec4_f, float);
//CAST(svec4_d, svec4_d, double);

////casts bits, only for 32bit i32/u32 <--> float i64/u64<-->double
//typedef union {
//    int32_t i32;
//    uint32_t u32;
//    float f;
//    int64_t i64;
//    uint64_t u64;
//    double d;
//} BitcastUnion;
//
//#define CAST_BITS(FROM, FROM_F, TO, TO_F)        \
//template <class T> static T svec_cast_bits(FROM val);     \
//template <> FORCEINLINE TO svec_cast_bits<TO>(FROM val) {      \
//    INC_STATS_NAME(STATS_CAST_SLOW, 1, #FROME"-"#TO);          \
//    BitcastUnion u[4]; \
//    u[0].FROM_F = val[0]; \
//    u[1].FROM_F = val[1]; \
//    u[2].FROM_F = val[2]; \
//    u[3].FROM_F = val[3]; \
//    return TO(u[0].TO_F, u[1].TO_F, u[2].TO_F, u[3].TO_F); \
//}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT(FROM, TO, STO)        \
template <class T> static T svec_cast_bits(FROM val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast_bits<TO>(FROM val) {      \
    return TO((__vector STO)(val.v)); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT64(FROM, TO, STO)        \
template <class T> static T svec_cast_bits(FROM val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast_bits<TO>(FROM val) {      \
    return TO((__vector STO)(val.v[0]), (__vector STO)(val.v[1])); \
}


//CAST_BITS(svec4_i32, i32, svec4_f, f);
//CAST_BITS(svec4_u32, u32, svec4_f, f);
//CAST_BITS(svec4_f, f, svec4_i32, i32);
//CAST_BITS(svec4_f, f, svec4_u32, u32);
//
//CAST_BITS(svec4_i64, i64, svec4_d, d);
//CAST_BITS(svec4_u64, u64, svec4_d, d);
//CAST_BITS(svec4_d, d, svec4_i64, i64);
//CAST_BITS(svec4_d, d, svec4_u64, u64);

CAST_BITS_OPT(svec4_i32, svec4_f, float);
CAST_BITS_OPT(svec4_u32, svec4_f, float);
CAST_BITS_OPT(svec4_f, svec4_i32, int32_t);
CAST_BITS_OPT(svec4_f, svec4_u32, uint32_t);

CAST_BITS_OPT64(svec4_i64, svec4_d, double);
CAST_BITS_OPT64(svec4_u64, svec4_d, double);
CAST_BITS_OPT64(svec4_d, svec4_i64, int64_t);
CAST_BITS_OPT64(svec4_d, svec4_u64, uint64_t);



//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////
/**
 * @brief Change a mask type (i1 vector) to a uint64_t integer
 * The method is only used for compatibility of ISPC
 * @param mask the svec_i1 type vector
 * @return a uint64_t integer to represent the mask
 */
static FORCEINLINE uint64_t svec_movmsk(svec4_i1 mask) {
#ifdef __POWER9
  return vec_movmskps_p9(mask.v);
#else
  uint64_t res;
  res = ((mask[0]>>31) & 0x1) |
        ((mask[1]>>30) & 0x2) |
        ((mask[2]>>29) & 0x4) |
        ((mask[3]>>28) & 0x8);
  INC_STATS_NAME(STATS_OTHER_SLOW,1, "svec_movmsk");
  return res;
#endif
}

/**
 * @brief Check any element in the mask vector is true
 * @return true if at least one element in the mask vector is true.
 */
FORCEINLINE bool svec4_i1::any_true() { return svec_any_true(*this); }

/**
 * @brief Check all the elements in the mask vector is true
 * @return true if all the elements in the mask vector are true
 */
FORCEINLINE bool svec4_i1::all_true() { return svec_all_true(*this); }

/**
 * @brief Check all the element sin the mask vector is false
 * @return true if all the elements in the mask vector are false
 */
FORCEINLINE bool svec4_i1::none_true() { return svec_none_true(*this); }

/**
 * @brief reverse the mask's value
 */
FORCEINLINE svec4_i1 svec4_i1::operator~() { return svec_not(*this); }

/**
 * @brief or operator of svec4_i1. E.g. "a | b"
 */
FORCEINLINE svec4_i1 svec4_i1::operator|(svec4_i1 a) { return svec_or(*this, a); }
/**
 * @brief and operator of svec4_i1. E.g. "a & b"
 */
FORCEINLINE svec4_i1 svec4_i1::operator&(svec4_i1 a) { return svec_and(*this, a); }
/**
 * @brief xor operator of svec4_i1. E.g. "a ^ b"
 */
FORCEINLINE svec4_i1 svec4_i1::operator^(svec4_i1 a) { return svec_xor(*this, a); }





/**
 * @brief compare equal, return a bool vector. E.g. "a == b"
 */
FORCEINLINE svec4_i1 svec4_i1::operator ==(svec4_i1 a) {
    return svec_equal(*this, a);
}

/**
 * @brief compare not equal, return a bool vector. E.g. "a != b"
 */
FORCEINLINE svec4_i1 svec4_i1::operator !=(svec4_i1 a) {
    return svec_not_equal(*this, a);
}

/**
 * Below I use macros to declare all vector operators
 *
 */

#define VEC_CMP_IMPL(VTYPE, MTYPE)     \
/**
 * @brief compare equal, return a bool vector. E.g. "a == b"
 */\
  FORCEINLINE MTYPE VTYPE::operator==(VTYPE a) { return svec_equal(*this, a); } \
/**
 * @brief compare not equal, return a bool vector. E.g. "a != b"
 */\
  FORCEINLINE MTYPE VTYPE::operator!=(VTYPE a) { return svec_not_equal(*this, a); } \
/**
 * @brief compare less than, return a bool vector. E.g. "a < b"
 */\
  FORCEINLINE MTYPE VTYPE::operator<(VTYPE a) { return svec_less_than(*this, a); } \
/**
 * @brief compare less equal, return a bool vector. E.g. "a <= b"
 */\
  FORCEINLINE MTYPE VTYPE::operator<=(VTYPE a) { return svec_less_equal(*this, a); } \
/**
 * @brief compare greater than, return a bool vector. E.g. "a > b"
 */\
  FORCEINLINE MTYPE VTYPE::operator>(VTYPE a) { return svec_greater_than(*this, a); } \
/**
 * @brief compare greater equal, return a bool vector. E.g. "a >= b"
 */\
  FORCEINLINE MTYPE VTYPE::operator>=(VTYPE a) { return svec_greater_equal(*this, a); }

VEC_CMP_IMPL(svec4_i8, svec4_i1);
VEC_CMP_IMPL(svec4_u8, svec4_i1);
VEC_CMP_IMPL(svec4_i16, svec4_i1);
VEC_CMP_IMPL(svec4_u16, svec4_i1);
VEC_CMP_IMPL(svec4_i32, svec4_i1);
VEC_CMP_IMPL(svec4_u32, svec4_i1);
VEC_CMP_IMPL(svec4_i64, svec4_i1);
VEC_CMP_IMPL(svec4_u64, svec4_i1);
VEC_CMP_IMPL(svec4_f, svec4_i1);
VEC_CMP_IMPL(svec4_d, svec4_i1);

#define VEC_UNARY_IMPL(VTYPE, STYPE) \
/**
 * @brief get the neg value, return a VTYPE vector. E.g. "-a"
 */\
  FORCEINLINE VTYPE VTYPE::operator-() {return svec_neg(*this); } \
/**
 * @brief Get the sum of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE VTYPE::reduce_add() {return svec_reduce_add(*this); } \
/**
 * @brief Get the max value of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE VTYPE::reduce_max() {return svec_reduce_max(*this); } \
/**
 * @brief Get the min value of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE VTYPE::reduce_min() {return svec_reduce_min(*this); }


#define VEC_BIN_IMPL(VTYPE, STYPE)    \
/**
 * @brief Add two vectors
 */\
  FORCEINLINE VTYPE VTYPE::operator+(VTYPE a) { return svec_add(*this, a); } \
/**
 * @brief Add a vector and a scalar
 */\
  FORCEINLINE VTYPE VTYPE::operator+(STYPE s) { return svec_add_scalar(*this, s); } \
/**
 * @brief Add a scalar and a vector
 */\
  FORCEINLINE VTYPE operator+(STYPE s, VTYPE a) {return svec_scalar_add(s, a);} \
/**
 * @brief Sub two vectors
 */\
  FORCEINLINE VTYPE VTYPE::operator-(VTYPE a) { return svec_sub(*this, a); } \
/**
 * @brief Sub a vector and a scalar
 */\
  FORCEINLINE VTYPE VTYPE::operator-(STYPE s) { return svec_sub_scalar(*this, s); } \
/**
 * @brief Sub a scalar and a vector
 */\
  FORCEINLINE VTYPE operator-(STYPE s, VTYPE a) {return svec_scalar_sub(s, a);} \
/**
 * @brief Multiply two vectors
 */\
  FORCEINLINE VTYPE VTYPE::operator*(VTYPE a) { return svec_mul(*this, a); } \
/**
 * @brief Multiply a vector and a scalar
 */\
  FORCEINLINE VTYPE VTYPE::operator*(STYPE s) { return svec_mul_scalar(*this, s) ;} \
/**
 * @brief Multiply a scalar and a vector
 */\
  FORCEINLINE VTYPE operator*(STYPE s, VTYPE a) {return svec_scalar_mul(s, a);} \
/**
 * @brief Divide a vector by a vector
 */\
  FORCEINLINE VTYPE VTYPE::operator/(VTYPE a) { return svec_div(*this, a); } \
/**
 * @brief Divide a vector by a scalar
 */\
  FORCEINLINE VTYPE VTYPE::operator/(STYPE s) { return svec_div_scalar(*this, s) ;} \
/**
 * @brief Divide a scalar by a vector
 */\
  FORCEINLINE VTYPE operator/(STYPE s, VTYPE a) {return svec_scalar_div(s, a);} \

/**
 * @brief mask class's class method impl
 */
#define MVEC_CLASS_METHOD_IMPL(VTYPE, STYPE) \
/**
 * @brief Return a new vector by loading the value from the pointer p.
 */\
  FORCEINLINE VTYPE VTYPE::load(VTYPE* p){ return svec_load(p); } \
/**
 * @brief Store the vector value to pointer p.
 */\
  FORCEINLINE void VTYPE::store(VTYPE* p){ svec_store(p, *this); }


#define VEC_CLASS_METHOD_IMPL(VTYPE, STYPE) \
  MVEC_CLASS_METHOD_IMPL(VTYPE, STYPE); \
/**
 * @brief Return a new vector by only loading the value from the pointer p if the mask element is true
 */\
  FORCEINLINE VTYPE VTYPE::masked_load(VTYPE* p, svec4_i1 mask){ return svec_masked_load(p, mask); } \
/**
 * @brief Store the vector element's value to pointer p if the mask element is true
 */\
  FORCEINLINE void VTYPE::masked_store(VTYPE* p, svec4_i1 mask){ svec_masked_store(p, *this, mask); } \
  VEC_UNARY_IMPL(VTYPE, STYPE); \
  VEC_BIN_IMPL(VTYPE, STYPE); \
/**
 * @brief Construct a vector by loading a scalar value from pointer p, and splat it to all the elements in the vector
 */\
  FORCEINLINE VTYPE VTYPE::load_const(const STYPE* p) {return svec_load_const(p);} \
/**
 * @brief Construct a vector by loading a scalar value from pointer p, and splat it to all the elements in the vector
 */\
  FORCEINLINE VTYPE VTYPE::load_and_splat(STYPE* p) {return svec_load_and_splat(p); } \
  /**
   * @brief Gather the elements pointed by the vector ptrs if the mask element is true, and return a vector.
   */\
  FORCEINLINE VTYPE VTYPE::gather(svec4_ptr ptrs, svec4_i1 mask) {return svec_gather<VTYPE>(ptrs, mask); } \
  /**
   * @brief Scatter the vector's elements to the locations pointed by the vector ptrs if the mask element is true.
   */\
  FORCEINLINE void VTYPE::scatter(svec4_ptr ptrs, svec4_i1 mask) { svec_scatter(ptrs, *this, mask); } \
  /**
   * @brief Gather the elements pointed by calculating the addresses (b + scale * offsets) if the mask element is true, and return a vector.
   */\
  FORCEINLINE VTYPE VTYPE::gather_base_offsets(STYPE* b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask) { \
    return svec_gather_base_offsets(b, scale, offsets, mask); \
  } \
  /**
   * @brief Gather the elements pointed by calculating the addresses (b + scale * offsets) if the mask element is true, and return a vector.
   */\
  FORCEINLINE VTYPE VTYPE::gather_base_offsets(STYPE* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask) {\
      return svec_gather_base_offsets(b, scale, offsets, mask); \
  } \
  /**
   * @brief Scatter the vector's elements to the addresses (b + scale * offsets) if the mask element is true.
   */\
  FORCEINLINE void VTYPE::scatter_base_offsets(STYPE* b, uint32_t scale, svec4_i32 offsets, svec4_i1 mask) { \
      svec_scatter_base_offsets(b, scale, offsets, *this, mask); \
  } \
  /**
   * @brief Scatter the vector's elements to the addresses (b + scale * offsets) if the mask element is true.
   */\
  FORCEINLINE void VTYPE::scatter_base_offsets(STYPE* b, uint32_t scale, svec4_i64 offsets, svec4_i1 mask) {\
      svec_scatter_base_offsets(b, scale, offsets, *this, mask); \
  } \
  /**
   * @brief Return a new vector by setting all the elements of the new vector with this vector's index element.
   */\
  FORCEINLINE VTYPE VTYPE::broadcast(int32_t index) { return svec_broadcast(*this, index);} \
  /**
   * @brief Return a new vector by rotate this vector's elements. e.g. newVec[i] = thisVec[i+index]
   */\
  FORCEINLINE VTYPE VTYPE::rotate(int32_t index) { return svec_rotate(*this, index); } \
  /**
   * @brief Return a new vector by shuffle this vector's elements with index vector e.g. newVec[i] = thisVec[index[i]]
   */\
  FORCEINLINE VTYPE VTYPE::shuffle(svec4_i32 index) { return svec_shuffle(*this, index); }



MVEC_CLASS_METHOD_IMPL(svec4_i1, uint32_t);
VEC_CLASS_METHOD_IMPL(svec4_i8, int8_t);
VEC_CLASS_METHOD_IMPL(svec4_u8, uint8_t);
VEC_CLASS_METHOD_IMPL(svec4_i16, int16_t);
VEC_CLASS_METHOD_IMPL(svec4_u16, uint16_t);
VEC_CLASS_METHOD_IMPL(svec4_i32, int32_t);
VEC_CLASS_METHOD_IMPL(svec4_u32, uint32_t);
VEC_CLASS_METHOD_IMPL(svec4_i64, int64_t);
VEC_CLASS_METHOD_IMPL(svec4_u64, uint64_t);
VEC_CLASS_METHOD_IMPL(svec4_f, float);
VEC_CLASS_METHOD_IMPL(svec4_d, double);


#define VEC_INT_CLASS_METHOD_IMPL(VTYPE, VTYPE_B, STYPE) \
  /**
   * @brief Or operator. E.g. "a | b".
   */\
  FORCEINLINE VTYPE VTYPE::operator|(VTYPE a) { return svec_or(*this, a); } \
  /**
   * @brief And operator. E.g. "a & b".
   */\
  FORCEINLINE VTYPE VTYPE::operator&(VTYPE a) { return svec_and(*this, a); } \
  /**
   * @brief Xor operator. E.g. "a ^ b".
   */\
  FORCEINLINE VTYPE VTYPE::operator^(VTYPE a) { return svec_xor(*this, a); } \
  /**
   * @brief Left shift operator. E.g. "a << b".The b must be unsigned vector
   */\
  FORCEINLINE VTYPE VTYPE::operator<<(VTYPE_B a) { return svec_shl(*this, a); } \
  /**
   * @brief Left shift operator by a scalar. E.g. "a << 5".
   */\
  FORCEINLINE VTYPE VTYPE::operator<<(STYPE s) { return svec_shl(*this, s); } \
  /**
   * @brief Right shift operator. E.g. "a >> b".The b must be unsigned vector
   */\
  FORCEINLINE VTYPE VTYPE::operator>>(VTYPE_B a) { return svec_shr(*this, a); } \
  /**
   * @brief Right shift operator by a scalar. E.g. "a >> 5".
   */\
  FORCEINLINE VTYPE VTYPE::operator>>(STYPE s) { return svec_shr(*this, s); } \
  /**
   * @brief Remainder operator on a vector. E.g. "a % b".
   */\
  FORCEINLINE VTYPE VTYPE::operator%(VTYPE a) { return svec_rem(*this, a); } \
  /**
   * @brief Remainder operator on a scalar. E.g. "a % 5".
   */\
  FORCEINLINE VTYPE VTYPE::operator%(STYPE s) { return svec_rem(*this, s); }

VEC_INT_CLASS_METHOD_IMPL(svec4_i8, svec4_u8, int8_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_u8, svec4_u8, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_i16, svec4_u16, int16_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_u16, svec4_u16, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_i32, svec4_u32, int32_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_u32, svec4_u32, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_i64, svec4_u64, int64_t);
VEC_INT_CLASS_METHOD_IMPL(svec4_u64, svec4_u64, uint64_t);



#define VEC_FLOAT_CLASS_METHOD_IMPL(VTYPE) \
  /**
   * @brief return the round VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::round() { return svec_round(*this);} \
  /**
   * @brief return the floor VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::floor() { return svec_floor(*this);} \
  /**
   * @brief return the ceil VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::ceil() { return svec_ceil(*this);} \
  /**
   * @brief return the sqrt VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::sqrt() { return svec_sqrt(*this);} \
  /**
   * @brief return the reverse VTYPE vector. (1.0/thisVec)
   */\
  FORCEINLINE VTYPE VTYPE::rcp() { return svec_rcp(*this);} \
  /**
   * @brief return the reverse sqrt VTYPE vector. ( 1.0/sqrt(thisVec) )
   */\
  FORCEINLINE VTYPE VTYPE::rsqrt() { return svec_rsqrt(*this);}\
  /**
   * @brief return the exp VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::exp() {return svec_exp(*this);} \
  /**
   * @brief return the log VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::log() {return svec_log(*this);} \
  /**
   * @brief return the pow VTYPE vector.
   */\
  FORCEINLINE VTYPE VTYPE::pow(VTYPE a) { return svec_pow(*this, a); }


VEC_FLOAT_CLASS_METHOD_IMPL(svec4_f);
VEC_FLOAT_CLASS_METHOD_IMPL(svec4_d);


} //end of namespace vsx4
#endif /* POWER_VSX4_H_ */

