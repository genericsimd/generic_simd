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

The original source code covered by the above license above has been
modified significantly by IBM Corp.
Copyright 2013 the Generic SIMD Intrinsic Library project authors. All rights reserved.

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

/**
 * @file generic4.h
 * @date  July 14, 2013
 * @author Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 * @brief SIMD LANES=4 interfaces implemented by scalar
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




#ifndef GENERIC4_H_
#define GENERIC4_H_

#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include "gsimd_utility.h"

namespace generic {

#define LANES 4

//////////////////////////////////////////////////////////////
//
// Constructor Section
//
//////////////////////////////////////////////////////////////

struct _svec4_ptr;

template <int Lanes, class T>
struct svec : public invalid_template_arguments<Lanes,T>::type {
  //here we need to add the static assert
};

template <>
struct svec<4,bool>;
template <>
  struct svec<4,char>;
template <>
  struct svec<4,unsigned char>;
template <>
  struct svec<4,short>;
template <>
  struct svec<4,unsigned short>;
template <>
  struct svec<4,int>;
template <>
  struct svec<4,unsigned int>;
template <>
  struct svec<4,long long>;
template <>
  struct svec<4,unsigned long long>;
template <>
  struct svec<4,float>;
template <>
  struct svec<4,double>;

//required because macros are confused by the , in the template declaration
//and to reduce typing
typedef svec<4,bool> _svec4_i1;
typedef svec<4,char> _svec4_i8;
typedef svec<4,unsigned char> _svec4_u8;
typedef svec<4,short> _svec4_i16;
typedef svec<4,unsigned short> _svec4_u16;
typedef svec<4,int> _svec4_i32;
typedef svec<4,unsigned int> _svec4_u32;
typedef svec<4,long long> _svec4_i64;
typedef svec<4,unsigned long long> _svec4_u64;
typedef svec<4,float> _svec4_f;
typedef svec<4,double> _svec4_d;

/**
 * @brief Data representation and operations on a vector of 4 boolean values.
 * This is used in predicated vector operations. Specifically the ith value of 
 * _svec4_i1 indicates whether the ith lane of a predicated vector operation is 
 * enabled or not.
 * 
 * See also gather, scatter, load, store, and compare operations.
 */

template<>
struct svec<4,bool> {

    uint32_t v; //only use 4 bits

    /**
     * @brief Default constructor. 
     * @return a vector of 4 undefined values.
     */
    FORCEINLINE svec() { v = 0;}
    /** 
     * @brief Constructor.
     * @param[in] a,b,c,d boolean values
     * \note a,b,c,d must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v = ((a ? 1 : 0) |(b ? 2 : 0)|(c ? 4 : 0)|(d ? 8 : 0));
    }
    /**
     * @brief Constructor.
     * @param[in] a a boolean value
     * \note a must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,a,a,a}.
     */
    FORCEINLINE svec( uint32_t a){
      v = a ? 15 : 0;
    }

    SUBSCRIPT_FUNC_OPT_DECL(_svec4_i1, uint32_t);
    COUT_FUNC_I1(_svec4_i1, LANES);
    MVEC_CLASS_METHOD_DECL(_svec4_i1, uint32_t);
};


/**
 * @brief data representation and operations on a vector of 4 signed chars.
 */
template <>
struct svec<4,char> {
    int8_t v[LANES];

    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief Constructor
     * @return a vector of 4 signed chars: {a,b,c,d}.
     */
    FORCEINLINE svec(int8_t a, int8_t b, int8_t c, int8_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed chars: {a,a,a,a}.
     */
    FORCEINLINE svec( int8_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_i8, int8_t);
    COUT_FUNC_I8(_svec4_i8, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_i8, int8_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_i8, _svec4_u8, int8_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned chars.
 */

template<>
struct svec<4,unsigned char> {
    uint8_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief Constructor
     * @return a vector of 4 unsigned chars: {a,b,c,d}.
     */
    FORCEINLINE svec(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a an unsigned char value
     * @return a vector of 4 unsigned chars: {a,a,a,a}.
     */
    FORCEINLINE svec(uint8_t a){
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_u8, uint8_t);
    COUT_FUNC_I8(_svec4_u8, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_u8, uint8_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_u8, _svec4_u8, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed short.
 */
template <>
struct svec<4,short> {
    int16_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed short.
     */
    FORCEINLINE svec() { }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed short: {a,b,c,d}.
     */
    FORCEINLINE svec(int16_t a, int16_t b, int16_t c, int16_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a signed short
     * @return a vector of 4 signed short: {a,a,a,a}.
     */
    FORCEINLINE svec( int16_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_i16, int16_t);
    COUT_FUNC(_svec4_i16, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_i16, int16_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_i16, _svec4_u16, int16_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned short.
 */
template <>
struct svec<4,unsigned short> {
    uint16_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned short.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned short: {a,b,c,d}.
     */
    FORCEINLINE svec(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a an unsigned short
     * @return a vector of 4 unsigned short: {a,a,a,a}.
     */
    FORCEINLINE svec( uint16_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_u16, uint16_t);
    COUT_FUNC(_svec4_u16, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_u16, uint16_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_u16, _svec4_u16, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 signed int.
 */
template <>
struct svec<4,int> {
    int32_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed int.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed int: {a,b,c,d}.
     */
    FORCEINLINE svec(int a, int b, int c, int d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a signed int
     * @return a vector of 4 signed int: {a,a,a,a}.
     */
    FORCEINLINE svec(int32_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_i32, int32_t);
    COUT_FUNC(_svec4_i32, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_i32, int32_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_i32, _svec4_u32, int32_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned int.
 */
template <>
struct svec<4,unsigned int> {
   uint32_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned int.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned int: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a an unsigned int
     * @return a vector of 4 unsigned int: {a,a,a,a}.
     */
    FORCEINLINE svec( uint32_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_u32, uint32_t);
    COUT_FUNC(_svec4_u32, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_u32, uint32_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_u32, _svec4_u32, uint32_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed long long.
 */
template <>
struct svec<4,long long> { 
    int64_t v[LANES];
    /**
     * @brief Default constructor,
     * @return a vector of 4 undefined signed long long.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed long long: {a,b,c,d}.
     */
    FORCEINLINE svec(int64_t a, int64_t b, int64_t c, int64_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a signed long long
     * @return a vector of 4 signed long long: {a,a,a,a}.
     */
    FORCEINLINE svec( int64_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_i64, int64_t);
    COUT_FUNC(_svec4_i64, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_i64, int64_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_i64, _svec4_u64, int64_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned long long.
 */
template <>
struct svec<4,unsigned long long> {
    uint64_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned long long.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned long long: {a,b,c,d}.
     */
    FORCEINLINE svec(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a an unsigned long long.
     * @return a vector of 4 unsigned long long: {a,a,a,a}.
     */
    FORCEINLINE svec( uint64_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_u64, uint64_t);
    COUT_FUNC(_svec4_u64, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_u64, uint64_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_INT_CLASS_METHOD_DECL(_svec4_u64, _svec4_u64, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 float.
 */
template<>
struct svec<4,float> {
    float v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined float.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 float: {a,b,c,d}.
     */
    FORCEINLINE svec(float a, float b, float c, float d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a float
     * @return a vector of 4 floats: {a,a,a,a}.
     */
    FORCEINLINE svec( float a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_f, float);
    COUT_FUNC(_svec4_f, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_f, float, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_FLOAT_CLASS_METHOD_DECL(_svec4_f);
};

/**
 * @brief data representation and operations on a vector of 4 double.
 */
template<>
struct svec<4,double> {
    double v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined double.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 double: {a,b,c,d}.
     */
    FORCEINLINE svec(double a, double b, double c, double d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a double
     * @return a vector of 4 doubles: {a,a,a,a}.
     */
    FORCEINLINE svec( double a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(_svec4_d, double);
    COUT_FUNC(_svec4_d, LANES);

    VEC_CLASS_METHOD_DECL(_svec4_d, double, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
    VEC_FLOAT_CLASS_METHOD_DECL(_svec4_d);
};


//////////////////////////////////////////////////////////////
//
// Data operation interfaces
//
//////////////////////////////////////////////////////////////

//
//// 1. Extract / Insert
//
//i1 use different approach
static FORCEINLINE uint32_t svec_extract(_svec4_i1 v, int index) {
  return (v.v & (1 << index)) ? -1 : 0;
}
static FORCEINLINE void svec_insert(_svec4_i1 *v, int index, uint32_t val) {
  if(!val) {
    v->v &= ~(1 << index);
  } else {
    v->v |= (1 << index);
  }
}
INSERT_EXTRACT(_svec4_i8, int8_t);
INSERT_EXTRACT(_svec4_u8, uint8_t);
INSERT_EXTRACT(_svec4_i16, int16_t);
INSERT_EXTRACT(_svec4_u16, uint16_t);
INSERT_EXTRACT(_svec4_i32, int32_t);
INSERT_EXTRACT(_svec4_u32, uint32_t);
INSERT_EXTRACT(_svec4_i64, int64_t);
INSERT_EXTRACT(_svec4_u64, uint64_t);
INSERT_EXTRACT(_svec4_f, float);
INSERT_EXTRACT(_svec4_d, double);

// 1. Load / Store
LOAD_STORE(_svec4_i1, uint32_t);
LOAD_STORE(_svec4_i8, int8_t);
LOAD_STORE(_svec4_u8, uint8_t);
LOAD_STORE(_svec4_i16, int16_t);
LOAD_STORE(_svec4_u16, uint16_t);
LOAD_STORE(_svec4_i32, int32_t);
LOAD_STORE(_svec4_u32, uint32_t);
LOAD_STORE(_svec4_i64, int64_t);
LOAD_STORE(_svec4_u64, uint64_t);
LOAD_STORE(_svec4_f, float);
LOAD_STORE(_svec4_d, double);

// 3. Select
static FORCEINLINE _svec4_i1 svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b) {
  _svec4_i1 ret;
  ret.v = (a.v & mask.v) | (b.v & ~mask.v);
  return ret;
}
SELECT(_svec4_i8, _svec4_i1);
SELECT(_svec4_u8, _svec4_i1);
SELECT(_svec4_i16, _svec4_i1);
SELECT(_svec4_u16, _svec4_i1);
SELECT(_svec4_i32, _svec4_i1);
SELECT(_svec4_u32, _svec4_i1);
SELECT(_svec4_i64, _svec4_i1);
SELECT(_svec4_u64, _svec4_i1);
SELECT(_svec4_f, _svec4_i1);
SELECT(_svec4_d, _svec4_i1);

SELECT_BOOLCOND(_svec4_i1);
SELECT_BOOLCOND(_svec4_i8);
SELECT_BOOLCOND(_svec4_u8);
SELECT_BOOLCOND(_svec4_i16);
SELECT_BOOLCOND(_svec4_u16);
SELECT_BOOLCOND(_svec4_i32);
SELECT_BOOLCOND(_svec4_u32);
SELECT_BOOLCOND(_svec4_i64);
SELECT_BOOLCOND(_svec4_u64);
SELECT_BOOLCOND(_svec4_f);
SELECT_BOOLCOND(_svec4_d);

// 4. broadcast/rotate/shuffle/smear/setzero
BROADCAST(_svec4_i8, int8_t);
BROADCAST(_svec4_u8, uint8_t);
BROADCAST(_svec4_i16, int16_t);
BROADCAST(_svec4_u16, uint16_t);
BROADCAST(_svec4_i32, int32_t);
BROADCAST(_svec4_u32, uint32_t);
BROADCAST(_svec4_i64, int64_t);
BROADCAST(_svec4_u64, uint64_t);
BROADCAST(_svec4_f, float);
BROADCAST(_svec4_d, double);

ROTATE(_svec4_i8, int8_t);
ROTATE(_svec4_u8, uint8_t);
ROTATE(_svec4_i16, int16_t);
ROTATE(_svec4_u16, uint16_t);
ROTATE(_svec4_i32, int32_t);
ROTATE(_svec4_u32, uint32_t);
ROTATE(_svec4_i64, int64_t);
ROTATE(_svec4_u64, uint64_t);
ROTATE(_svec4_f, float);
ROTATE(_svec4_d, double);

SHUFFLES(_svec4_i8, int8_t, _svec4_i32);
SHUFFLES(_svec4_u8, uint8_t, _svec4_i32);
SHUFFLES(_svec4_i16, int16_t, _svec4_i32);
SHUFFLES(_svec4_u16, uint16_t, _svec4_i32);
SHUFFLES(_svec4_i32, int32_t, _svec4_i32);
SHUFFLES(_svec4_u32, uint32_t, _svec4_i32);
SHUFFLES(_svec4_i64, int64_t, _svec4_i32);
SHUFFLES(_svec4_u64, uint64_t, _svec4_i32);
SHUFFLES(_svec4_f, float, _svec4_i32);
SHUFFLES(_svec4_d, double, _svec4_i32);

//load const
LOAD_CONST(_svec4_i8, int8_t);
LOAD_CONST(_svec4_u8, uint8_t);
LOAD_CONST(_svec4_i16, int16_t);
LOAD_CONST(_svec4_u16, uint16_t);
LOAD_CONST(_svec4_i32, int32_t);
LOAD_CONST(_svec4_u32, uint32_t);
LOAD_CONST(_svec4_i64, int64_t);
LOAD_CONST(_svec4_u64, uint64_t);
LOAD_CONST(_svec4_f, float);
LOAD_CONST(_svec4_d, double);


// 5. Gather / Scatter
/**
 * including gather general/gather base offsets, scatter general, scatter base offsets
 *
 * Here, we will define a special compile time dependent type for ptrs vector
 *
 */
/**
 * @brief data representation and operations on a vector of 4 pointers.
 * This is only used in gather and scatter.
 * @note In 32bit platform, _svec4_ptr extends _svec4_u32, while in 64bit platform, _svec4_ptr extends _svec4_u64.
 * @see gather and scatter
 */
#if defined(__x86_64__) || defined(__PPC64__)
struct _svec4_ptr : public _svec4_u64{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p10, p1, p2, p3}.
     */
    FORCEINLINE _svec4_ptr(void* p0, void* p1, void* p2, void* p3):
        _svec4_u64((uint64_t)(p0),(uint64_t)(p1),(uint64_t)(p2),(uint64_t)(p3)){}
};
#else // 32-bit
struct _svec4_ptr: public _svec4_u32{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p0, p1, p2, p3}.
     */
    FORCEINLINE _svec4_ptr(void* p0, void* p1, void* p2, void* p3):
        _svec4_u32((uint32_t)(p0),(uint32_t)(p1),(uint32_t)(p2),(uint32_t)(p3)){}
};
#endif // __PPC64__

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


template <class RetVecType> static RetVecType svec_gather(_svec4_u32 ptrs, _svec4_i1 mask);
template <class RetVecType> static RetVecType svec_gather(_svec4_u64 ptrs, _svec4_i1 mask);

GATHER_GENERAL(_svec4_i8, int8_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_i8, int8_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_u8, uint8_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_u8, uint8_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_i16, int16_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_i16, int16_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_u16, uint16_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_u16, uint16_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_i32, int32_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_i32, int32_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_u32, uint32_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_u32, uint32_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_i64, int64_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_i64, int64_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_u64, uint64_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_u64, uint64_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_f, float, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_f, float, _svec4_u64, _svec4_i1);
GATHER_GENERAL(_svec4_d, double, _svec4_u32, _svec4_i1);
GATHER_GENERAL(_svec4_d, double, _svec4_u64, _svec4_i1);

GATHER_BASE_OFFSETS(_svec4_i8, int8_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i8, int8_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u8, uint8_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u8, uint8_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i16, int16_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i16, int16_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u16, uint16_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u16, uint16_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i32, int32_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i32, int32_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u32, uint32_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u32, uint32_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i64, int64_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_i64, int64_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u64, uint64_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_u64, uint64_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_f, float, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_f, float, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_d, double, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS(_svec4_d, double, _svec4_i64, _svec4_i1);

GATHER_STRIDE_L4(_svec4_i8, int8_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i8, int8_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u8, uint8_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u8, uint8_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i16, int16_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i16, int16_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u16, uint16_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u16, uint16_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i32, int32_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i32, int32_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u32, uint32_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u32, uint32_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i64, int64_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_i64, int64_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u64, uint64_t, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_u64, uint64_t, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_f, float, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_f, float, int64_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_d, double, int32_t, _svec4_i1);
GATHER_STRIDE_L4(_svec4_d, double, int64_t, _svec4_i1);



SCATTER_GENERAL(_svec4_i8, int8_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_i8, int8_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_u8, uint8_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_u8, uint8_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_i16, int16_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_i16, int16_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_u16, uint16_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_u16, uint16_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_i32, int32_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_i32, int32_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_u32, uint32_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_u32, uint32_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_i64, int64_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_i64, int64_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_u64, uint64_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_u64, uint64_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_f, float, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_f, float, _svec4_u64, _svec4_i1);
SCATTER_GENERAL(_svec4_d, double, _svec4_u32, _svec4_i1);
SCATTER_GENERAL(_svec4_d, double, _svec4_u64, _svec4_i1);

SCATTER_BASE_OFFSETS(_svec4_i8, int8_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i8, int8_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u8, uint8_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u8, uint8_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i16, int16_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i16, int16_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u16, uint16_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u16, uint16_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i32, int32_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i32, int32_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u32, uint32_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u32, uint32_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i64, int64_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_i64, int64_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u64, uint64_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_u64, uint64_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_f, float, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_f, float, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_d, double, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS(_svec4_d, double, _svec4_i64, _svec4_i1);

SCATTER_STRIDE_L4(_svec4_i8, int8_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i8, int8_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u8, uint8_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u8, uint8_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i16, int16_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i16, int16_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u16, uint16_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u16, uint16_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i32, int32_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i32, int32_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u32, uint32_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u32, uint32_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i64, int64_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_i64, int64_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u64, uint64_t, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_u64, uint64_t, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_f, float, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_f, float, int64_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_d, double, int32_t, _svec4_i1);
SCATTER_STRIDE_L4(_svec4_d, double, int64_t, _svec4_i1);

#endif //DOXYGEN_SHOULD_SKIP_THIS


//  5. masked load/masked store

//Masked load/store is implemented based on gather_base_offsets/scatter_base_offsets
//Here we only use offsets with 32bit

MASKED_LOAD_STORE(_svec4_i8, int8_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_u8, uint8_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_i16, int16_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_u16, uint16_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_i32, int32_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_u32, uint32_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_i64, int64_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_u64, uint64_t, _svec4_i1);
MASKED_LOAD_STORE(_svec4_f, float, _svec4_i1);
MASKED_LOAD_STORE(_svec4_d, double, _svec4_i1);

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
static FORCEINLINE bool svec_any_true(const _svec4_i1& mask) {
  return (mask.v != 0);
}

/**
 * @brief Check all elements of the mask are non-zero
 * @param mask the svec_i1 type vector
 * @return true is all elements in the mask are true
 */
static FORCEINLINE bool svec_all_true(const _svec4_i1& mask) {
  return (mask.v & 0xF) == 0xF;
}


/**
 * @brief Check none elements of the mask are zero
 * @param mask the svec_i1 type vector
 * @return true is all elements in the mask are false
 */
static FORCEINLINE bool svec_none_true(const _svec4_i1& mask) {
  return (mask.v == 0);
}

// 2. bit operations

/**
 * @brief return a & b
 */
static FORCEINLINE _svec4_i1 svec_and(_svec4_i1 a, _svec4_i1 b) {
  _svec4_i1 ret;
  ret.v = a.v & b.v;
  return ret;
}


/**
 * @brief return a | b
 */
static FORCEINLINE _svec4_i1 svec_or(_svec4_i1 a, _svec4_i1 b) {
  _svec4_i1 ret;
  ret.v = a.v | b.v;
  return ret;
}

/**
 * @brief return a ^ b
 */
static FORCEINLINE _svec4_i1 svec_xor(_svec4_i1 a, _svec4_i1 b) {
  _svec4_i1 ret;
  ret.v = a.v ^ b.v;
  return ret;
}

/**
 * @brief return ~a
 */
static FORCEINLINE _svec4_i1 svec_not(_svec4_i1 a) {
  _svec4_i1 ret;
  ret.v = ~a.v;
  return ret;
}

/**
 * @brief Change a mask type (i1 vector) to a uint64_t integer
 * The method is only used for compatibility of ISPC
 * @param mask the svec_i1 type vector
 * @return a uint64_t integer to represent the mask
 */
static FORCEINLINE uint64_t svec_movmsk(_svec4_i1 mask) {
  return (uint64_t)(mask.v);
}


//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////
// 1. Unary

// neg operation
UNARY_OP(_svec4_i8, svec_neg, -);
UNARY_OP(_svec4_u8, svec_neg, -);
UNARY_OP(_svec4_i16, svec_neg, -);
UNARY_OP(_svec4_u16, svec_neg, -);
UNARY_OP(_svec4_i32, svec_neg, -);
UNARY_OP(_svec4_u32, svec_neg, -);
UNARY_OP(_svec4_i64, svec_neg, -);
UNARY_OP(_svec4_u64, svec_neg, -);
UNARY_OP(_svec4_f, svec_neg, -);
UNARY_OP(_svec4_d, svec_neg, -);

//  2. Math unary
//round
UNARY_OP(_svec4_f, svec_round, roundf);
UNARY_OP(_svec4_d, svec_round, round);
//floor
UNARY_OP(_svec4_f, svec_floor, floorf);
UNARY_OP(_svec4_d, svec_floor, floor);
//ceil
UNARY_OP(_svec4_f, svec_ceil, ceilf);
UNARY_OP(_svec4_d, svec_ceil, ceil);
//reverse 1/
UNARY_OP(_svec4_f, svec_rcp, 1.0/);
UNARY_OP(_svec4_d, svec_rcp, 1.0/);
//reverse sqrt
UNARY_OP(_svec4_f, svec_rsqrt, 1.0/sqrtf);
UNARY_OP(_svec4_d, svec_rsqrt, 1.0/sqrt);
//sqrt
UNARY_OP(_svec4_f, svec_sqrt, sqrtf);
UNARY_OP(_svec4_d, svec_sqrt, sqrt);
//exp
UNARY_OP(_svec4_f, svec_exp, expf);
UNARY_OP(_svec4_d, svec_exp, exp);
//log
UNARY_OP(_svec4_f, svec_log, logf);
UNARY_OP(_svec4_d, svec_log, log);
//abs - for all types
UNARY_OP(_svec4_i8, svec_abs, abs<int8_t>);
static FORCEINLINE _svec4_u8  svec_abs(_svec4_u8 v) { return v;}
UNARY_OP(_svec4_i16, svec_abs, abs<int16_t>);
static FORCEINLINE _svec4_u16  svec_abs(_svec4_u16 v) { return v;}
UNARY_OP(_svec4_i32, svec_abs, abs<int32_t>);
static FORCEINLINE _svec4_u32  svec_abs(_svec4_u32 v) { return v;}
UNARY_OP(_svec4_i64, svec_abs, abs<int64_t>);
static FORCEINLINE _svec4_u64  svec_abs(_svec4_u64 v) { return v;}
UNARY_OP(_svec4_f, svec_abs, abs);
UNARY_OP(_svec4_d, svec_abs, abs);

//  3. Binary

//add, sub, div, mul.
#define BINARY_OP_METHODS(VTYPE, STYPE) \
BINARY_OP(VTYPE, svec_add, +); \
BINARY_OP(VTYPE, svec_sub, -); \
BINARY_OP(VTYPE, svec_mul, *); \
BINARY_OP(VTYPE, svec_div, /); \
BINARY_OP_SCALAR(VTYPE, STYPE, svec_add_scalar, +); \
BINARY_SCALAR_OP(VTYPE, STYPE, svec_scalar_add, +); \
BINARY_OP_SCALAR(VTYPE, STYPE, svec_sub_scalar, -); \
BINARY_SCALAR_OP(VTYPE, STYPE, svec_scalar_sub, -); \
BINARY_OP_SCALAR(VTYPE, STYPE, svec_mul_scalar, *); \
BINARY_SCALAR_OP(VTYPE, STYPE, svec_scalar_mul, *); \
BINARY_OP_SCALAR(VTYPE, STYPE, svec_div_scalar, /); \
BINARY_SCALAR_OP(VTYPE, STYPE, svec_scalar_div, /); \

#define INT_BINARY_OP_METHODS(VTYPE, STYPE) \
BINARY_OP(VTYPE, svec_or, |); \
BINARY_OP(VTYPE, svec_and, &); \
BINARY_OP(VTYPE, svec_xor, ^); \
BINARY_OP_SCALAR(VTYPE, int32_t, svec_shl, <<); \
BINARY_OP_SCALAR(VTYPE, int32_t, svec_shr, >>); \
BINARY_OP(VTYPE, svec_rem, %); \
BINARY_OP_SCALAR(VTYPE, STYPE, svec_rem, %);

BINARY_OP_METHODS(_svec4_i8, int8_t);
BINARY_OP_METHODS(_svec4_u8, uint8_t);
BINARY_OP_METHODS(_svec4_i16, int16_t);
BINARY_OP_METHODS(_svec4_u16, uint16_t);
BINARY_OP_METHODS(_svec4_i32, int32_t);
BINARY_OP_METHODS(_svec4_u32, uint32_t);
BINARY_OP_METHODS(_svec4_i64, int64_t);
BINARY_OP_METHODS(_svec4_u64, uint64_t);
BINARY_OP_METHODS(_svec4_f, float);
BINARY_OP_METHODS(_svec4_d, double);

INT_BINARY_OP_METHODS(_svec4_i8, int8_t);
INT_BINARY_OP_METHODS(_svec4_u8, uint8_t);
INT_BINARY_OP_METHODS(_svec4_i16, int16_t);
INT_BINARY_OP_METHODS(_svec4_u16, uint16_t);
INT_BINARY_OP_METHODS(_svec4_i32, int32_t);
INT_BINARY_OP_METHODS(_svec4_u32, uint32_t);
INT_BINARY_OP_METHODS(_svec4_i64, int64_t);
INT_BINARY_OP_METHODS(_svec4_u64, uint64_t);


//power only for float
BINARY_OP_FUNC(_svec4_f, svec_pow, powf);
BINARY_OP_FUNC(_svec4_d, svec_pow, pow);

//shift left
BINARY_OP2(_svec4_i8, _svec4_u8, svec_shl, <<);
BINARY_OP2(_svec4_u8, _svec4_u8, svec_shl, <<);
BINARY_OP2(_svec4_i16, _svec4_u16, svec_shl, <<);
BINARY_OP2(_svec4_u16, _svec4_u16, svec_shl, <<);
BINARY_OP2(_svec4_i32, _svec4_u32, svec_shl, <<);
BINARY_OP2(_svec4_u32, _svec4_u32, svec_shl, <<);
BINARY_OP2(_svec4_i64, _svec4_u64, svec_shl, <<);
BINARY_OP2(_svec4_u64, _svec4_u64, svec_shl, <<);

//shift right
BINARY_OP2(_svec4_i8, _svec4_u8, svec_shr, >>);
BINARY_OP2(_svec4_u8, _svec4_u8, svec_shr, >>);
BINARY_OP2(_svec4_i16, _svec4_u16, svec_shr, >>);
BINARY_OP2(_svec4_u16, _svec4_u16, svec_shr, >>);
BINARY_OP2(_svec4_i32, _svec4_u32, svec_shr, >>);
BINARY_OP2(_svec4_u32, _svec4_u32, svec_shr, >>);
BINARY_OP2(_svec4_i64, _svec4_u64, svec_shr, >>);
BINARY_OP2(_svec4_u64, _svec4_u64, svec_shr, >>);

//  4. Ternary

//madd / msub for only int32/u32/float/double
TERNERY(_svec4_i32);
TERNERY(_svec4_u32);
TERNERY(_svec4_i64);
TERNERY(_svec4_u64);
TERNERY(_svec4_f);
TERNERY(_svec4_d);


//  5. Max/Min & 6. Reduce
#define MAX_MIN_REDUCE_METHODS(VTYPE, STYPE) \
BINARY_OP_FUNC(VTYPE, svec_max, max<STYPE>); \
BINARY_OP_FUNC(VTYPE, svec_min, min<STYPE>); \
BINARY_OP_REDUCE_FUNC(VTYPE, STYPE, svec_reduce_add, add<STYPE>); \
BINARY_OP_REDUCE_FUNC(VTYPE, STYPE, svec_reduce_max, max<STYPE>); \
BINARY_OP_REDUCE_FUNC(VTYPE, STYPE, svec_reduce_min, min<STYPE>); \

MAX_MIN_REDUCE_METHODS(_svec4_i8, int8_t);
MAX_MIN_REDUCE_METHODS(_svec4_u8, uint8_t);
MAX_MIN_REDUCE_METHODS(_svec4_i16, int16_t);
MAX_MIN_REDUCE_METHODS(_svec4_u16, uint16_t);
MAX_MIN_REDUCE_METHODS(_svec4_i32, int32_t);
MAX_MIN_REDUCE_METHODS(_svec4_u32, uint32_t);
MAX_MIN_REDUCE_METHODS(_svec4_i64, int64_t);
MAX_MIN_REDUCE_METHODS(_svec4_u64, uint64_t);
MAX_MIN_REDUCE_METHODS(_svec4_f, float);
MAX_MIN_REDUCE_METHODS(_svec4_d, double);

FORCEINLINE _svec4_d svec_preduce_add(_svec4_d v0, _svec4_d v1, _svec4_d v2, _svec4_d v3) {
  return _svec4_d(
      svec_reduce_add(v0),
      svec_reduce_add(v1),
      svec_reduce_add(v2),
      svec_reduce_add(v3)
      );
}


//  7. Compare
CMP_ALL_OP(_svec4_i8, _svec4_i1);
CMP_ALL_OP(_svec4_u8, _svec4_i1);
CMP_ALL_OP(_svec4_i16, _svec4_i1);
CMP_ALL_OP(_svec4_u16, _svec4_i1);
CMP_ALL_OP(_svec4_i32, _svec4_i1);
CMP_ALL_OP(_svec4_u32, _svec4_i1);
CMP_ALL_OP(_svec4_i64, _svec4_i1);
CMP_ALL_OP(_svec4_u64, _svec4_i1);
CMP_ALL_OP(_svec4_f, _svec4_i1);
CMP_ALL_OP(_svec4_d, _svec4_i1);

/**
 * @brief element by element comparison of two svec_vec4_i1 type object
 * @param a
 * @param b
 * @return a svec_vec4_i1 object
 */
CMP_OP(_svec4_i1, _svec4_i1, equal, ==);
CMP_OP(_svec4_i1, _svec4_i1, not_equal, !=);

//  8. Cast

/**
 * Here we provide the full cast combinations.
 * Some may have fast impl
 */

//i1 -> all
//CAST(_svec4_i1, _svec4_i1, uint32_t);
CAST(_svec4_i1, _svec4_i8, int8_t);  //better way: packing
CAST(_svec4_i1, _svec4_u8, uint8_t);  //better way: packing
CAST(_svec4_i1, _svec4_i16, int16_t);  //better way: packing
CAST(_svec4_i1, _svec4_u16, uint16_t); //better way: packing
CAST(_svec4_i1, _svec4_i32, int32_t);
CAST(_svec4_i1, _svec4_u32, uint32_t);
CAST(_svec4_i1, _svec4_i64, int64_t); //better way: unpack, singed ext
CAST(_svec4_i1, _svec4_u64, uint64_t);//better way: unpack, singed ext
CAST(_svec4_i1, _svec4_f, float); //si to fp call
CAST(_svec4_i1, _svec4_d, double);

//i8 -> all
CAST(_svec4_i8, _svec4_i1, uint32_t);
//CAST(_svec4_i8, _svec4_i8, int8_t);
CAST(_svec4_i8, _svec4_u8, uint8_t);
CAST(_svec4_i8, _svec4_i16, int16_t); //better way, use vec_unpackh
CAST(_svec4_i8, _svec4_u16, uint16_t); //better way, sext + zero mask and
CAST(_svec4_i8, _svec4_i32, int32_t); //better way, use twice vec_unpack
CAST(_svec4_i8, _svec4_u32, uint32_t); //better way, use unpack + zero mask
CAST(_svec4_i8, _svec4_i64, int64_t);
CAST(_svec4_i8, _svec4_u64, uint64_t);
CAST(_svec4_i8, _svec4_f, float);
CAST(_svec4_i8, _svec4_d, double);

//u8 -> all
CAST(_svec4_u8, _svec4_i1, uint32_t);
CAST(_svec4_u8, _svec4_i8, int8_t);
//CAST(_svec4_u8, _svec4_u8, uint8_t);
CAST(_svec4_u8, _svec4_i16, int16_t); //better way, use unpack + zero mask
CAST(_svec4_u8, _svec4_u16, uint16_t); //better way use unpack + zero mask
CAST(_svec4_u8, _svec4_i32, int32_t);
CAST(_svec4_u8, _svec4_u32, uint32_t);
CAST(_svec4_u8, _svec4_i64, int64_t);
CAST(_svec4_u8, _svec4_u64, uint64_t);
CAST(_svec4_u8, _svec4_f, float);
CAST(_svec4_u8, _svec4_d, double);

//i16 -> all
CAST(_svec4_i16, _svec4_i1, uint32_t);
CAST(_svec4_i16, _svec4_i8, int8_t); //could use pack
CAST(_svec4_i16, _svec4_u8, uint8_t); //could use pack
//CAST(_svec4_i16, _svec4_i16, int16_t);
CAST(_svec4_i16, _svec4_u16, uint16_t);
CAST(_svec4_i16, _svec4_i32, int32_t); //use unpack
CAST(_svec4_i16, _svec4_u32, uint32_t); //use unpack and zeromaskout
CAST(_svec4_i16, _svec4_i64, int64_t);
CAST(_svec4_i16, _svec4_u64, uint64_t);
CAST(_svec4_i16, _svec4_f, float);
CAST(_svec4_i16, _svec4_d, double);

//u16 -> all
CAST(_svec4_u16, _svec4_i1, uint32_t);
CAST(_svec4_u16, _svec4_i8, int8_t);
CAST(_svec4_u16, _svec4_u8, uint8_t);
CAST(_svec4_u16, _svec4_i16, int16_t);
//CAST(_svec4_u16, _svec4_u16, uint16_t);
CAST(_svec4_u16, _svec4_i32, int32_t); //use unpack +mask
CAST(_svec4_u16, _svec4_u32, uint32_t); //use unpack + mask
CAST(_svec4_u16, _svec4_i64, int64_t);
CAST(_svec4_u16, _svec4_u64, uint64_t);
CAST(_svec4_u16, _svec4_f, float);
CAST(_svec4_u16, _svec4_d, double);

//i32 -> all
CAST(_svec4_i32, _svec4_i1, uint32_t);
CAST(_svec4_i32, _svec4_i8, int8_t);
CAST(_svec4_i32, _svec4_u8, uint8_t);
CAST(_svec4_i32, _svec4_i16, int16_t);
CAST(_svec4_i32, _svec4_u16, uint16_t);
//CAST(_svec4_i32, _svec4_i32, int32_t);
CAST(_svec4_i32, _svec4_u32, uint32_t);
CAST(_svec4_i32, _svec4_i64, int64_t); //use p8 unpack
CAST(_svec4_i32, _svec4_u64, uint64_t); //use p8 unpack
CAST(_svec4_i32, _svec4_f, float); //use ctf
CAST(_svec4_i32, _svec4_d, double);

//u32 -> all
CAST(_svec4_u32, _svec4_i1, uint32_t);
CAST(_svec4_u32, _svec4_i8, int8_t);
CAST(_svec4_u32, _svec4_u8, uint8_t);
CAST(_svec4_u32, _svec4_i16, int16_t);
CAST(_svec4_u32, _svec4_u16, uint16_t);
CAST(_svec4_u32, _svec4_i32, int32_t);
//CAST(_svec4_u32, _svec4_u32, uint32_t);
CAST(_svec4_u32, _svec4_i64, int64_t); //use p8 unpack
CAST(_svec4_u32, _svec4_u64, uint64_t); //use p8 unpack
CAST(_svec4_u32, _svec4_f, float);
CAST(_svec4_u32, _svec4_d, double);

//i64-> all
CAST(_svec4_i64, _svec4_i1, uint32_t);
CAST(_svec4_i64, _svec4_i8, int8_t);
CAST(_svec4_i64, _svec4_u8, uint8_t);
CAST(_svec4_i64, _svec4_i16, int16_t);
CAST(_svec4_i64, _svec4_u16, uint16_t);
CAST(_svec4_i64, _svec4_i32, int32_t); //use p8 trunk
CAST(_svec4_i64, _svec4_u32, uint32_t); //use p8 trunk
//CAST(_svec4_i64, _svec4_i64, int64_t);
CAST(_svec4_i64, _svec4_u64, uint64_t);
CAST(_svec4_i64, _svec4_f, float);
CAST(_svec4_i64, _svec4_d, double);

//u64 -> all
CAST(_svec4_u64, _svec4_i1, uint32_t);
CAST(_svec4_u64, _svec4_i8, int8_t);
CAST(_svec4_u64, _svec4_u8, uint8_t);
CAST(_svec4_u64, _svec4_i16, int16_t);
CAST(_svec4_u64, _svec4_u16, uint16_t);
CAST(_svec4_u64, _svec4_i32, int32_t); //use p8 pack
CAST(_svec4_u64, _svec4_u32, uint32_t); //use p8 pack
CAST(_svec4_u64, _svec4_i64, int64_t);
//CAST(_svec4_u64, _svec4_u64, uint64_t);
CAST(_svec4_u64, _svec4_f, float);
CAST(_svec4_u64, _svec4_d, double);

//float -> all
CAST(_svec4_f, _svec4_i1, uint32_t);
CAST(_svec4_f, _svec4_i8, int8_t); //use cts + pack+pack
CAST(_svec4_f, _svec4_u8, uint8_t); //use ctu + pack + pack
CAST(_svec4_f, _svec4_i16, int16_t); //use cts + pack
CAST(_svec4_f, _svec4_u16, uint16_t); //use ctu + pack
CAST(_svec4_f, _svec4_i32, int32_t);//use cts
CAST(_svec4_f, _svec4_u32, uint32_t); //use ctu
CAST(_svec4_f, _svec4_i64, int64_t);
CAST(_svec4_f, _svec4_u64, uint64_t);
//CAST(_svec4_f, _svec4_f, float);
CAST(_svec4_f, _svec4_d, double);

//double -> all
CAST(_svec4_d, _svec4_i1, uint32_t);
CAST(_svec4_d, _svec4_i8, int8_t);
CAST(_svec4_d, _svec4_u8, uint8_t);
CAST(_svec4_d, _svec4_i16, int16_t);
CAST(_svec4_d, _svec4_u16, uint16_t);
CAST(_svec4_d, _svec4_i32, int32_t);
CAST(_svec4_d, _svec4_u32, uint32_t);
CAST(_svec4_d, _svec4_i64, int64_t);
CAST(_svec4_d, _svec4_u64, uint64_t);
CAST(_svec4_d, _svec4_f, float);
//CAST(_svec4_d, _svec4_d, double);

////casts bits, only for 32bit i32/u32 <--> float i64/u64<-->double


/**
 * @brief cast based on directly change the __vector type
 */
CAST_BITS(_svec4_i32, i32, _svec4_f, f);
CAST_BITS(_svec4_u32, u32, _svec4_f, f);
CAST_BITS(_svec4_f, f, _svec4_i32, i32);
CAST_BITS(_svec4_f, f, _svec4_u32, u32);

CAST_BITS(_svec4_i64, i64, _svec4_d, d);
CAST_BITS(_svec4_u64, u64, _svec4_d, d);
CAST_BITS(_svec4_d, d, _svec4_i64, i64);
CAST_BITS(_svec4_d, d, _svec4_u64, u64);


//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////

//add the impl of i1's
FORCEINLINE void _svec4_i1::Helper::operator=(uint32_t value) {
  svec_insert(m_self, m_index, value);
}
FORCEINLINE void _svec4_i1::Helper::operator=(_svec4_i1::Helper helper) {
  svec_insert(m_self, m_index, helper.operator uint32_t());
}
FORCEINLINE _svec4_i1::Helper::operator uint32_t() const {
  return svec_extract(*m_self, m_index);
}
const FORCEINLINE uint32_t _svec4_i1::operator[](int index) const {
  return svec_extract(*this, index);
}
SUBSCRIPT_FUNC_IMPL(_svec4_i8, int8_t);
SUBSCRIPT_FUNC_IMPL(_svec4_u8, uint8_t);
SUBSCRIPT_FUNC_IMPL(_svec4_i16, int16_t);
SUBSCRIPT_FUNC_IMPL(_svec4_u16, uint16_t);
SUBSCRIPT_FUNC_IMPL(_svec4_i32, int32_t);
SUBSCRIPT_FUNC_IMPL(_svec4_u32, uint32_t);
SUBSCRIPT_FUNC_IMPL(_svec4_i64, int64_t);
SUBSCRIPT_FUNC_IMPL(_svec4_u64, uint64_t);
SUBSCRIPT_FUNC_IMPL(_svec4_f, float);
SUBSCRIPT_FUNC_IMPL(_svec4_d, double);

/**
 * @brief Check if any element in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if at least one element in the mask vector is true, otherwise false
 */
FORCEINLINE bool _svec4_i1::any_true() { return svec_any_true(*this); }

/**
 * @brief Check if all the elements in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are true, otherwise false.
 */
FORCEINLINE bool _svec4_i1::all_true() { return svec_all_true(*this); }

/**
 * @brief Check all the elements in the mask vector is false. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are false, otherwise false.
 */
FORCEINLINE bool _svec4_i1::none_true() { return svec_none_true(*this); }

/**
 * @brief Element-wise bit-wise compliment operator. E.g., "~a"
 * @return the result of bit-wise compliment as a boolean vector. 
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator~() { return svec_not(*this); }

/**
 * @brief Element-wise bit-wise OR operator. E.g., "a | b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise OR as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator|(_svec4_i1 a) { return svec_or(*this, a); }
/**
 * @brief Element-wise bit-wise AND operator. E.g., "a & b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise AND as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator&(_svec4_i1 a) { return svec_and(*this, a); }
/**
 * @brief Element-wise bit-wise XOR operator. E.g., "a ^ b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise XOR as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator^(_svec4_i1 a) { return svec_xor(*this, a); }
/**
 * @brief Element-wise bit-wise not operator. E.g., "!a"
 * @return the result of bit-wise compliment as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator!() { return svec_not(*this); }

/**
 * @brief Element-wise boolean AND operator. E.g., "a && b"
 * @param[in] a a boolean vector
 * @return the result of boolean AND as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator&&(_svec4_i1 a) { return svec_and(*this, a); }
/**
 * @brief Element-wise boolean OR operator. E.g., "a || b"
 * @param[in] a a boolean vector
 * @return the result of boolean OR as a boolean vector.
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator||(_svec4_i1 a) { return svec_or(*this, a); }
/**
 * @brief Element-wise compare equal. E.g., "a == b"
 * @param[in] a a boolean vector
 * @return the result of compare-equal as a boolean vector
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator ==(_svec4_i1 a) {
    return svec_equal(*this, a);
}

/**
 * @brief Element-wise compare not equal, return a bool vector. E.g. "a != b"
 * @param[in] a a boolean vector
 * @return the result of compare-not-equal as a boolean vector
 */
FORCEINLINE _svec4_i1 _svec4_i1::operator !=(_svec4_i1 a) {
    return svec_not_equal(*this, a);
}

VEC_CMP_IMPL(_svec4_i8, _svec4_i1);
VEC_CMP_IMPL(_svec4_u8, _svec4_i1);
VEC_CMP_IMPL(_svec4_i16, _svec4_i1);
VEC_CMP_IMPL(_svec4_u16, _svec4_i1);
VEC_CMP_IMPL(_svec4_i32, _svec4_i1);
VEC_CMP_IMPL(_svec4_u32, _svec4_i1);
VEC_CMP_IMPL(_svec4_i64, _svec4_i1);
VEC_CMP_IMPL(_svec4_u64, _svec4_i1);
VEC_CMP_IMPL(_svec4_f, _svec4_i1);
VEC_CMP_IMPL(_svec4_d, _svec4_i1);

MVEC_CLASS_METHOD_IMPL(_svec4_i1, uint32_t);
VEC_CLASS_METHOD_IMPL(_svec4_i8, int8_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_u8, uint8_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_i16, int16_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_u16, uint16_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_i32, int32_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_u32, uint32_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_i64, int64_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_u64, uint64_t, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_f, float, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);
VEC_CLASS_METHOD_IMPL(_svec4_d, double, _svec4_i1, _svec4_ptr, _svec4_i32, _svec4_i64);

VEC_INT_CLASS_METHOD_IMPL(_svec4_i8, _svec4_u8, int8_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_u8, _svec4_u8, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_i16, _svec4_u16, int16_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_u16, _svec4_u16, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_i32, _svec4_u32, int32_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_u32, _svec4_u32, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_i64, _svec4_u64, int64_t);
VEC_INT_CLASS_METHOD_IMPL(_svec4_u64, _svec4_u64, uint64_t);

VEC_FLOAT_CLASS_METHOD_IMPL(_svec4_f);
VEC_FLOAT_CLASS_METHOD_IMPL(_svec4_d);

#undef LANES
} //end of namespace vsx4
#endif /* POWER_VSX4_H_ */

