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
 * @file sse4.h
 * @date  July 20, 2013
 * @author Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 * @brief SIMD LANES=4 interfaces implemented by scalar
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
 *  The current implementation is based on Intel ISPC SIMD intrinsics.
 *  ISPC: http://ispc.github.io/
 */




#ifndef SSE4_H_
#define SSE4_H_

#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include <smmintrin.h>
#include <nmmintrin.h>
#include "gsimd_utility.h"


#ifndef __SSE4_2__
# error "SSE 4.2 must be enabled in the C++ compiler to use this header."
#endif // !__SSE4_2__

namespace sse {

#define LANES 4

//////////////////////////////////////////////////////////////
//
// Constructor Section
//
//////////////////////////////////////////////////////////////
template <int Lanes, class T>
struct svec : public invalid_template_arguments<Lanes,T>::type {
  //here we need to add the static assert
};

template <>
struct svec<4,bool>;
template <>
  struct svec<4,signed char>;
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
template <>
  struct svec<4,void*>;

//required because macros are confused by the , in the template declaration
typedef svec<4,bool> _svec4_i1;
typedef svec<4,signed char> _svec4_i8;
typedef svec<4,unsigned char> _svec4_u8;
typedef svec<4,short> _svec4_i16;
typedef svec<4,unsigned short> _svec4_u16;
typedef svec<4,int> _svec4_i32;
typedef svec<4,unsigned int> _svec4_u32;
typedef svec<4,long long> _svec4_i64;
typedef svec<4,unsigned long long> _svec4_u64;
typedef svec<4,float> _svec4_f;
typedef svec<4,double> _svec4_d;
typedef svec<4,void*> _svec4_ptr;

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

  __m128 v; //only use 4 bits

    /**
     * @brief Default constructor. 
     * @return a vector of 4 undefined values.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param[in] vv a __m128 valye.
     * @return a mask vector whose value is from the vv.
     */
    FORCEINLINE svec(__m128 vv) : v(vv) { }
    /**
     * @brief For internal use only.
     * @param[in] vv a __m128i valye.
     * @return a mask vector whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(_mm_castsi128_ps(vv)) { }
    /** 
     * @brief Constructor.
     * @param[in] a,b,c,d boolean values
     * \note a,b,c,d must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v = _mm_castsi128_ps(_mm_set_epi32(d ? -1 : 0, c ? -1 : 0,
                                         b ? -1 : 0, a ? -1 : 0));
    }
    /**
     * @brief Constructor.
     * @param[in] a a boolean value
     * \note a must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,a,a,a}.
     */
    FORCEINLINE svec(uint32_t a){
      v = (a != 0) ? _mm_castsi128_ps(_mm_set1_epi32(-1)) : _mm_setzero_ps();
    }

    SUBSCRIPT_FUNC_BOOL_DECL(uint32_t);
    COUT_FUNC_BOOL_DECL();
    SVEC_BOOL_CLASS_METHOD_DECL();
};


/**
 * @brief data representation and operations on a vector of 4 signed chars.
 */
template <>
struct svec<4,signed char> {
  __m128i v;

    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i value.
     * @return a signed char vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor
     * @return a vector of 4 signed chars: {a,b,c,d}.
     */
    FORCEINLINE svec(int8_t a, int8_t b, int8_t c, int8_t d) {
      v = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed chars: {a,a,a,a}.
     */
    FORCEINLINE svec( int8_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi8(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int8_t);
    COUT_FUNC_CHAR_DECL(signed char);

    VEC_CLASS_METHOD_DECL(int8_t);
    VEC_INT_CLASS_METHOD_DECL(int8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned chars.
 */
template<>
struct svec<4,unsigned char> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i value.
     * @return a signed char vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor
     * @return a vector of 4 unsigned chars: {a,b,c,d}.
     */
    FORCEINLINE svec(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
      v = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned char value
     * @return a vector of 4 unsigned chars: {a,a,a,a}.
     */
    FORCEINLINE svec(uint8_t a){
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi8(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint8_t);
    COUT_FUNC_CHAR_DECL(unsigned char);

    VEC_CLASS_METHOD_DECL(uint8_t);
    VEC_INT_CLASS_METHOD_DECL(uint8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed short.
 */
template <>
struct svec<4,short> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed short.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed short vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed short: {a,b,c,d}.
     */
    FORCEINLINE svec(int16_t a, int16_t b, int16_t c, int16_t d) {
      v = _mm_set_epi16(0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a signed short
     * @return a vector of 4 signed short: {a,a,a,a}.
     */
    FORCEINLINE svec( int16_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi16(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int16_t);
    COUT_FUNC_DECL(short);

    VEC_CLASS_METHOD_DECL(int16_t);
    VEC_INT_CLASS_METHOD_DECL(int16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned short.
 */
template <>
struct svec<4,unsigned short> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned short.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed short vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned short: {a,b,c,d}.
     */
    FORCEINLINE svec(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
      v = _mm_set_epi16(0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned short
     * @return a vector of 4 unsigned short: {a,a,a,a}.
     */
    FORCEINLINE svec(uint16_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi16(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint16_t);
    COUT_FUNC_DECL(unsigned short);

    VEC_CLASS_METHOD_DECL(uint16_t);
    VEC_INT_CLASS_METHOD_DECL(uint16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 signed int.
 */
template <>
struct svec<4,int> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed int.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed int vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed int: {a,b,c,d}.
     */
    FORCEINLINE svec(int a, int b, int c, int d) {
      v = _mm_set_epi32(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a signed int
     * @return a vector of 4 signed int: {a,a,a,a}.
     */
    FORCEINLINE svec(int32_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi32(a);
      }
    }
    /**
     * @brief transform _svec4_i32 into _m128 float vector
     * @return _m128
     */
    FORCEINLINE operator __m128() const { return _mm_castsi128_ps(v); }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int32_t);
    COUT_FUNC_DECL(int);

    VEC_CLASS_METHOD_DECL(int32_t);
    VEC_INT_CLASS_METHOD_DECL(int32_t, uint32_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned int.
 */
template <>
struct svec<4,unsigned int> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned int.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed int vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned int: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v = _mm_set_epi32(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned int
     * @return a vector of 4 unsigned int: {a,a,a,a}.
     */
    FORCEINLINE svec(uint32_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi32(a);
      }
    }
    /**
     * @brief transform _svec4_i32 into _m128 float vector
     * @return _m128
     */
    FORCEINLINE operator __m128() const { return _mm_castsi128_ps(v); }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint32_t);
    COUT_FUNC_DECL(unsigned int);

    VEC_CLASS_METHOD_DECL(uint32_t);
    VEC_INT_CLASS_METHOD_DECL(uint32_t, uint32_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed long long.
 */
template <>
struct svec<4,long long> { 
  __m128i v[2];
    /**
     * @brief Default constructor,
     * @return a vector of 4 undefined signed long long.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct _svec4_i64 with two _m128i objects
     * @return a signed long long vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i a, __m128i b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed long long: {a,b,c,d}.
     */
    FORCEINLINE svec(int64_t a, int64_t b, int64_t c, int64_t d) {
      v[0] = _mm_set_epi32((b >> 32) & 0xffffffff, b & 0xffffffff,
                           (a >> 32) & 0xffffffff, a & 0xffffffff);
      v[1] = _mm_set_epi32((d >> 32) & 0xffffffff, d & 0xffffffff,
                           (c >> 32) & 0xffffffff, c & 0xffffffff);
    }
    /**
     * @brief Constructor.
     * @param a a signed long long
     * @return a vector of 4 signed long long: {a,a,a,a}.
     */
    FORCEINLINE svec( int64_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v[0] = v[1] = _mm_setzero_si128 ();
      } else {
        int a1 = (a >> 32) & 0xffffffff;
        int a0 = a & 0xffffffff;
        v[0] = v[1] = _mm_set_epi32(a1, a0, a1, a0);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int64_t);
    COUT_FUNC_DECL(long long);

    VEC_CLASS_METHOD_DECL(int64_t);
    VEC_INT_CLASS_METHOD_DECL(int64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned long long.
 */
template <>
struct svec<4,unsigned long long> {
  __m128i v[2];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned long long.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct _svec4_i64 with two _m128i objects
     * @return a signed long long vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i a, __m128i b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned long long: {a,b,c,d}.
     */
    FORCEINLINE svec(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
      v[0] = _mm_set_epi32((b >> 32) & 0xffffffff, b & 0xffffffff,
                           (a >> 32) & 0xffffffff, a & 0xffffffff);
      v[1] = _mm_set_epi32((d >> 32) & 0xffffffff, d & 0xffffffff,
                           (c >> 32) & 0xffffffff, c & 0xffffffff);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned long long.
     * @return a vector of 4 unsigned long long: {a,a,a,a}.
     */
    FORCEINLINE svec( uint64_t a) {
      if(__builtin_constant_p(a) && a == 0) {
         v[0] = v[1] = _mm_setzero_si128 ();
      } else {
        int a1 = (a >> 32) & 0xffffffff;
        int a0 = a & 0xffffffff;
        v[0] = v[1] = _mm_set_epi32(a1, a0, a1, a0);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint64_t);
    COUT_FUNC_DECL(unsigned long long);

    VEC_CLASS_METHOD_DECL(uint64_t);
    VEC_INT_CLASS_METHOD_DECL(uint64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 float.
 */
template<>
struct svec<4,float> {
  __m128 v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined float.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128.
     * @return a float vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128 vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 float: {a,b,c,d}.
     */
    FORCEINLINE svec(float a, float b, float c, float d) {
      v = _mm_set_ps(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a float
     * @return a vector of 4 floats: {a,a,a,a}.
     */
    FORCEINLINE svec( float a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_ps();
      } else {
        v = _mm_set1_ps(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(float);
    COUT_FUNC_DECL(float);

    VEC_CLASS_METHOD_DECL(float);
    VEC_FLOAT_CLASS_METHOD_DECL(float);
};

/**
 * @brief data representation and operations on a vector of 4 double.
 */
template<>
struct svec<4,double> {
  __m128d v[2];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined double.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct _svec4_d with two __vector double values
     * @return a double vector, whose value is from a and b.
     */
    FORCEINLINE svec(__m128d a, __m128d b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 double: {a,b,c,d}.
     */
    FORCEINLINE svec(double a, double b, double c, double d) {
      v[0] = _mm_set_pd(b, a);
      v[1] = _mm_set_pd(d, c);
    }
    /**
     * @brief Constructor.
     * @param a a double
     * @return a vector of 4 doubles: {a,a,a,a}.
     */
    FORCEINLINE svec( double a) {
    if (__builtin_constant_p(a) && a == 0) {
      v[0] = v[1] = _mm_setzero_pd();
    } else {
      v[0] = v[1] = _mm_set1_pd(a);
    }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(double);
    COUT_FUNC_DECL(double);

    VEC_CLASS_METHOD_DECL(double);
    VEC_FLOAT_CLASS_METHOD_DECL(double);
};


//////////////////////////////////////////////////////////////
//
// Data operation interfaces
//
//////////////////////////////////////////////////////////////

//
//// 1. Extract / Insert
//

/**
 * @brief macros for svec's insert extract method implementation
 * The implementation is based on vector type's subscript operator
 */
#define INSERT_EXTRACT_SSE(VTYPE, STYPE)                               \
  static FORCEINLINE STYPE svec_extract(VTYPE v, int index) {    \
    return ((STYPE*)&v)[index];                      \
  }                                          \
  static FORCEINLINE void svec_insert(VTYPE *v, int index, STYPE val) { \
    ((STYPE*)v)[index] = val;                      \
  }

#define INSERT_EXTRACT_SSEOPT(VTYPE, STYPE, FUNC)                               \
  static FORCEINLINE STYPE svec_extract(VTYPE v, int index) {    \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      return (STYPE)_mm_extract_##FUNC(v.v, index);                \
    } else { \
      return ((STYPE*)&v)[index];  \
    } \
  }                                     \
  static FORCEINLINE void svec_insert(VTYPE *v, int index, STYPE val) { \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      v->v = _mm_insert_##FUNC(v->v, val, index);                      \
    } else {\
      ((STYPE*)v)[index] = val;               \
    } \
  }

#define INSERT_EXTRACT_SSEOPT64(VTYPE, STYPE, FUNC)                               \
  static FORCEINLINE STYPE svec_extract(VTYPE v, int index) {    \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      return (STYPE)_mm_extract_##FUNC(v.v[index>>1], index%2);    \
    } else { \
      return ((STYPE*)&v)[index];  \
    } \
  }                                          \
  static FORCEINLINE void svec_insert(VTYPE *v, int index, STYPE val) { \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      v->v[index>>1] = _mm_insert_##FUNC(v->v[index>>1], val, index%2);      \
    } else { \
      ((STYPE*)v)[index] = val;               \
    } \
  }

//i1 use different approach
static FORCEINLINE uint32_t svec_extract(_svec4_i1 v, int index) {
  if(__builtin_constant_p(index) && index >=0 && index < 4) {
    return _mm_extract_epi32(_mm_castps_si128(v.v), index);
  } else
  {
    return ((uint32_t*)&v)[index];
  }

}
static FORCEINLINE void svec_insert(_svec4_i1 *v, int index, uint32_t val) {
  if(__builtin_constant_p(index) && index >=0 && index < 4) {
    v->v = _mm_castsi128_ps(_mm_insert_epi32(_mm_castps_si128(v->v), val ? -1 : 0, index));
  } else {
    ((uint32_t *)v)[index] = val ? -1 : 0;
  }

}
INSERT_EXTRACT_SSEOPT(_svec4_i8, int8_t, epi8);
INSERT_EXTRACT_SSEOPT(_svec4_u8, uint8_t, epi8);
INSERT_EXTRACT_SSEOPT(_svec4_i16, int16_t, epi16);
INSERT_EXTRACT_SSEOPT(_svec4_u16, uint16_t, epi16);
INSERT_EXTRACT_SSEOPT(_svec4_i32, int32_t, epi32);
INSERT_EXTRACT_SSEOPT(_svec4_u32, uint32_t, epi32);
#ifdef __x86_64__
INSERT_EXTRACT_SSEOPT64(_svec4_i64, int64_t, epi64);
INSERT_EXTRACT_SSEOPT64(_svec4_u64, uint64_t, epi64);
#else
INSERT_EXTRACT_SSE(_svec4_i64, int64_t);
INSERT_EXTRACT_SSE(_svec4_u64, uint64_t);
#endif
INSERT_EXTRACT_SSE(_svec4_f, float); //no intrinsics to insert/extract
INSERT_EXTRACT_SSE(_svec4_d, double); //no intrinsics to insert/extract

// 1. Load / Store
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_i1 svec_load(const _svec4_i1 *p) {
  return _svec4_i1(_mm_loadu_ps((float *)(&p->v)));

}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_i1 *p, _svec4_i1 v) {
  _mm_storeu_ps((float *)(&p->v), v.v);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_i8 svec_load(const _svec4_i8 *p) {
  int8_t *ptr = (int8_t *)(&p->v);
  return _svec4_i8(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_i8 *p, _svec4_i8 v) {
  int8_t *ptr = (int8_t *)(&p->v);
  ptr[0] = _mm_extract_epi8(v.v, 0);
  ptr[1] = _mm_extract_epi8(v.v, 1);
  ptr[2] = _mm_extract_epi8(v.v, 2);
  ptr[3] = _mm_extract_epi8(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_u8 svec_load(const _svec4_u8 *p) {
  uint8_t *ptr = (uint8_t *)(&p->v);
  return _svec4_u8(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_u8 *p, _svec4_u8 v) {
  uint8_t *ptr = (uint8_t *)(&p->v);
  ptr[0] = _mm_extract_epi8(v.v, 0);
  ptr[1] = _mm_extract_epi8(v.v, 1);
  ptr[2] = _mm_extract_epi8(v.v, 2);
  ptr[3] = _mm_extract_epi8(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_i16 svec_load(const _svec4_i16 *p) {
  int16_t *ptr = (int16_t *)(&p->v);
  return _svec4_i16(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_i16 *p, _svec4_i16 v) {
  int16_t *ptr = (int16_t *)(&p->v);
  ptr[0] = _mm_extract_epi16(v.v, 0);
  ptr[1] = _mm_extract_epi16(v.v, 1);
  ptr[2] = _mm_extract_epi16(v.v, 2);
  ptr[3] = _mm_extract_epi16(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_u16 svec_load(const _svec4_u16 *p) {
  uint16_t *ptr = (uint16_t *)(&p->v);
  return _svec4_u16(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_u16 *p, _svec4_u16 v) {
  uint16_t *ptr = (uint16_t *)(&p->v);
  ptr[0] = _mm_extract_epi16(v.v, 0);
  ptr[1] = _mm_extract_epi16(v.v, 1);
  ptr[2] = _mm_extract_epi16(v.v, 2);
  ptr[3] = _mm_extract_epi16(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_i32 svec_load(const _svec4_i32 *p) {
  return _svec4_i32(_mm_loadu_si128((__m128i *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_i32 *p, _svec4_i32 v) {
  _mm_storeu_si128((__m128i *)(&p->v), v.v);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_u32 svec_load(const _svec4_u32 *p) {
  return _svec4_u32(_mm_loadu_si128((__m128i *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_u32 *p, _svec4_u32 v) {
  _mm_storeu_si128((__m128i *)(&p->v), v.v);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_i64 svec_load(const _svec4_i64 *p) {
  return _svec4_i64(_mm_loadu_si128((__m128i *)(&p->v[0])),
                   _mm_loadu_si128((__m128i *)(&p->v[1])));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_i64 *p, _svec4_i64 v) {
  _mm_storeu_si128((__m128i *)(&p->v[0]), v.v[0]);
  _mm_storeu_si128((__m128i *)(&p->v[1]), v.v[1]);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_u64 svec_load(const _svec4_u64 *p) {
  return _svec4_u64(_mm_loadu_si128((__m128i *)(&p->v[0])),
                   _mm_loadu_si128((__m128i *)(&p->v[1])));
}
/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_u64 *p, _svec4_u64 v) {
  _mm_storeu_si128((__m128i *)(&p->v[0]), v.v[0]);
  _mm_storeu_si128((__m128i *)(&p->v[1]), v.v[1]);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_f svec_load(const _svec4_f *p) {
  return _svec4_f(_mm_loadu_ps((float *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_f *p, _svec4_f v) {
  _mm_storeu_ps((float *)(&p->v), v.v);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE _svec4_d svec_load(const _svec4_d *p) {
  return _svec4_d(_mm_loadu_pd((double *)(&p->v[0])),
                 _mm_loadu_pd((double *)(&p->v[1])));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(_svec4_d *p, _svec4_d v) {
  _mm_storeu_pd((double *)(&p->v[0]), v.v[0]);
  _mm_storeu_pd((double *)(&p->v[1]), v.v[1]);
}

// 3. Select


/**
 * @brief construct c by selecting elements from two input vectors according to the mask
 * @param mask the mask vector. True: select elements from a; False: select elements from b.
 * @param a the 1st vector. The elements will be selected if the corresponding mask element is true.
 * @param b the 2nd vector. The elements will be selected if the corresponding mask element is true.
 * @return A new vector whose elements is selected from a or b
 */
FORCEINLINE _svec4_i1 svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b) {
  return _mm_blendv_ps(b.v, a.v, mask.v);
}

/**
 * @brief select of _svec4_i8 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_i8 svec_select(_svec4_i1 mask, _svec4_i8 a, _svec4_i8 b) {
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i8");
  return _svec4_i8((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi8(a.v, 0) :
                                                      _mm_extract_epi8(b.v, 0),
                   (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi8(a.v, 1) :
                                                      _mm_extract_epi8(b.v, 1),
                   (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi8(a.v, 2) :
                                                      _mm_extract_epi8(b.v, 2),
                   (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi8(a.v, 3) :
                                                      _mm_extract_epi8(b.v, 3));
}

/**
 * @brief select of _svec4_u8 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_u8 svec_select(_svec4_i1 mask, _svec4_u8 a, _svec4_u8 b) {
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u8");
  return _svec4_u8((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi8(a.v, 0) :
                                                      _mm_extract_epi8(b.v, 0),
                   (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi8(a.v, 1) :
                                                      _mm_extract_epi8(b.v, 1),
                   (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi8(a.v, 2) :
                                                      _mm_extract_epi8(b.v, 2),
                   (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi8(a.v, 3) :
                                                      _mm_extract_epi8(b.v, 3));
}

/**
 * @brief select of _svec4_i16 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_i16 svec_select(_svec4_i1 mask, _svec4_i16 a, _svec4_i16 b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i16");
    return _svec4_i16((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi16(a.v, 0) :
                                                         _mm_extract_epi16(b.v, 0),
                      (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi16(a.v, 1) :
                                                         _mm_extract_epi16(b.v, 1),
                      (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi16(a.v, 2) :
                                                         _mm_extract_epi16(b.v, 2),
                      (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi16(a.v, 3) :
                                                         _mm_extract_epi16(b.v, 3));
}

/**
 * @brief select of _svec4_u16 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_u16 svec_select(_svec4_i1 mask, _svec4_u16 a, _svec4_u16 b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u16");
    return _svec4_u16((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi16(a.v, 0) :
                                                         _mm_extract_epi16(b.v, 0),
                      (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi16(a.v, 1) :
                                                         _mm_extract_epi16(b.v, 1),
                      (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi16(a.v, 2) :
                                                         _mm_extract_epi16(b.v, 2),
                      (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi16(a.v, 3) :
                                                         _mm_extract_epi16(b.v, 3));
}

/**
 * @brief select of _svec4_i32 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_i32 svec_select(_svec4_i1 mask, _svec4_i32 a, _svec4_i32 b) {
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(b.v),
                                        _mm_castsi128_ps(a.v), mask.v));
}

/**
 * @brief select of _svec4_u32 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_u32 svec_select(_svec4_i1 mask, _svec4_u32 a, _svec4_u32 b) {
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(b.v),
                                        _mm_castsi128_ps(a.v), mask.v));
}

/**
 * @brief select of _svec4_i64 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_i64 svec_select(_svec4_i1 mask, _svec4_i64 a, _svec4_i64 b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(_mm_castsi128_pd(b.v[0]), _mm_castsi128_pd(a.v[0]), m0d);
  __m128d r1 = _mm_blendv_pd(_mm_castsi128_pd(b.v[1]), _mm_castsi128_pd(a.v[1]), m1d);
  return _svec4_i64(_mm_castpd_si128(r0), _mm_castpd_si128(r1));
}

/**
 * @brief select of _svec4_u64 vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_u64 svec_select(_svec4_i1 mask, _svec4_u64 a, _svec4_u64 b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(_mm_castsi128_pd(b.v[0]), _mm_castsi128_pd(a.v[0]), m0d);
  __m128d r1 = _mm_blendv_pd(_mm_castsi128_pd(b.v[1]), _mm_castsi128_pd(a.v[1]), m1d);
  return _svec4_u64(_mm_castpd_si128(r0), _mm_castpd_si128(r1));
}

/**
 * @brief select of _svec4_f vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_f svec_select(_svec4_i1 mask, _svec4_f a, _svec4_f b) {
  return _mm_blendv_ps(b.v, a.v, mask.v);
}

/**
 * @brief select of _svec4_d vectors by a mask vector
 * see svec_select(_svec4_i1 mask, _svec4_i1 a, _svec4_i1 b)
 */
FORCEINLINE _svec4_d svec_select(_svec4_i1 mask, _svec4_d a, _svec4_d b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(b.v[0], a.v[0], m0d);
  __m128d r1 = _mm_blendv_pd(b.v[1], a.v[1], m1d);
  return _svec4_d(r0, r1);
}

SELECT_BOOLCOND(bool);
SELECT_BOOLCOND(int8_t);
SELECT_BOOLCOND(uint8_t);
SELECT_BOOLCOND(int16_t);
SELECT_BOOLCOND(uint16_t);
SELECT_BOOLCOND(int32_t);
SELECT_BOOLCOND(uint32_t);
SELECT_BOOLCOND(int64_t);
SELECT_BOOLCOND(uint64_t);
SELECT_BOOLCOND(float);
SELECT_BOOLCOND(double);

// 4. broadcast/rotate/shuffle/smear/setzero
static FORCEINLINE _svec4_i8 svec_broadcast(_svec4_i8 v, int index) {
  return _mm_set1_epi8(v[index]);
}
static FORCEINLINE _svec4_u8 svec_broadcast(_svec4_u8 v, int index) {
  return _mm_set1_epi8(v[index]);
}
static FORCEINLINE _svec4_i16 svec_broadcast(_svec4_i16 v, int index) {
  return _mm_set1_epi16(v[index]);
}
static FORCEINLINE _svec4_u16 svec_broadcast(_svec4_u16 v, int index) {
  return _mm_set1_epi16(v[index]);
}
static FORCEINLINE _svec4_i32 svec_broadcast(_svec4_i32 v, int index) {
  return _mm_set1_epi32(v[index]);
}
static FORCEINLINE _svec4_u32 svec_broadcast(_svec4_u32 v, int index) {
  return _mm_set1_epi32(v[index]);
}

static FORCEINLINE _svec4_i64 svec_broadcast(_svec4_i64 v, int index) {
  int64_t val = v[index];
  return _svec4_i64(val);
}
static FORCEINLINE _svec4_u64 svec_broadcast(_svec4_u64 v, int index) {
  uint64_t val = v[index];
  return _svec4_u64(val);
}

static FORCEINLINE _svec4_f svec_broadcast(_svec4_f v, int index) {
  return _mm_set1_ps(v[index]);
}
static FORCEINLINE _svec4_d svec_broadcast(_svec4_d v, int index) {
  return _svec4_d(_mm_set1_pd(v[index]),
                  _mm_set1_pd(v[index]));
}


ROTATE_L4(int8_t);
ROTATE_L4(uint8_t);
ROTATE_L4(int16_t);
ROTATE_L4(uint16_t);
ROTATE_L4(int32_t);
ROTATE_L4(uint32_t);
ROTATE_L4(int64_t);
ROTATE_L4(uint64_t);
ROTATE_L4(float);
ROTATE_L4(double);

SHUFFLES_L4(int8_t);
SHUFFLES_L4(uint8_t);
SHUFFLES_L4(int16_t);
SHUFFLES_L4(uint16_t);
SHUFFLES_L4(int32_t);
SHUFFLES_L4(uint32_t);
SHUFFLES_L4(int64_t);
SHUFFLES_L4(uint64_t);
SHUFFLES_L4(float);
SHUFFLES_L4(double);


//load const
#define LOAD_CONST_SSE(VTYPE, STYPE) \
template <class RetVecType> static RetVecType svec_load_const(const STYPE* p); \
template<> \
  FORCEINLINE VTYPE svec_load_const<VTYPE>(const STYPE* p) { \
    return VTYPE(*p); \
} \
template <class RetVecType> static RetVecType svec_load_and_splat(STYPE* p); \
template<> \
FORCEINLINE VTYPE svec_load_and_splat<VTYPE>(STYPE* p) { \
  return VTYPE(*p);\
}

LOAD_CONST_SSE(_svec4_i8, int8_t);
LOAD_CONST_SSE(_svec4_u8, uint8_t);
LOAD_CONST_SSE(_svec4_i16, int16_t);
LOAD_CONST_SSE(_svec4_u16, uint16_t);
LOAD_CONST_SSE(_svec4_i32, int32_t);
LOAD_CONST_SSE(_svec4_u32, uint32_t);
LOAD_CONST_SSE(_svec4_i64, int64_t);
LOAD_CONST_SSE(_svec4_u64, uint64_t);
LOAD_CONST_SSE(_svec4_f, float);
LOAD_CONST_SSE(_svec4_d, double);


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
template <>
  struct svec<4,void*> : public _svec4_u64{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p10, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
        _svec4_u64((uint64_t)(p0),(uint64_t)(p1),(uint64_t)(p2),(uint64_t)(p3)){}
};
#else // 32-bit
template <>
struct svec<4,void*>: public _svec4_u32{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p0, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
        _svec4_u32((uint32_t)(p0),(uint32_t)(p1),(uint32_t)(p2),(uint32_t)(p3)){}
};
#endif // __PPC64__

#ifndef DOXYGEN_SHOULD_SKIP_THIS //not want generate svec_gather*/svec_scatter methods

template <class RetVecType> static RetVecType svec_gather(_svec4_u32 ptrs, _svec4_i1 mask);

GATHER_GENERAL_L4(_svec4_i8, int8_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u8, uint8_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL_L4(_svec4_i16, int16_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u16, uint16_t, _svec4_u32, _svec4_i1);
//GATHER_GENERAL_L4(_svec4_i32, int32_t, _svec4_u32, _svec4_i1);
template<>
FORCEINLINE _svec4_i32 svec_gather<_svec4_i32>(_svec4_u32 ptrs, _svec4_i1 mask) {
  _svec4_i32 ret;
  if(svec_extract(mask,0)) { svec_insert(&ret, 0, *((int32_t*)svec_extract(ptrs,0)));}
  if(svec_extract(mask,1)) { svec_insert(&ret, 1, *((int32_t*)svec_extract(ptrs,1)));}
  if(svec_extract(mask,2)) { svec_insert(&ret, 2, *((int32_t*)svec_extract(ptrs,2)));}
  if(svec_extract(mask,3)) { svec_insert(&ret, 3, *((int32_t*)svec_extract(ptrs,3)));}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return ret;
}
//GATHER_GENERAL_L4(_svec4_u32, uint32_t, _svec4_u32, _svec4_i1);
template<>
FORCEINLINE _svec4_u32 svec_gather<_svec4_u32>(_svec4_u32 ptrs, _svec4_i1 mask) {
  return _svec4_u32(svec_gather<_svec4_i32>(ptrs, mask).v);
}

GATHER_GENERAL_L4(_svec4_i64, int64_t, _svec4_u32, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u64, uint64_t, _svec4_u32, _svec4_i1);
//GATHER_GENERAL_L4(_svec4_f, float, _svec4_u32, _svec4_i1);
template<>
FORCEINLINE _svec4_f svec_gather<_svec4_f>(_svec4_u32 ptrs, _svec4_i1 mask) {
  return _svec4_f(_mm_castsi128_ps(svec_gather<_svec4_i32>(ptrs, mask).v));
}
GATHER_GENERAL_L4(_svec4_d, double, _svec4_u32, _svec4_i1);

template <class RetVecType> static RetVecType svec_gather(_svec4_u64 ptrs, _svec4_i1 mask);
GATHER_GENERAL_L4(_svec4_i8, int8_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL_L4(_svec4_i16, int16_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u16, uint16_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u8, uint8_t, _svec4_u64, _svec4_i1);
//GATHER_GENERAL_L4(_svec4_i32, int32_t, _svec4_u64, _svec4_i1);
template<>
FORCEINLINE _svec4_i32 svec_gather<_svec4_i32>(_svec4_u64 ptrs, _svec4_i1 mask) {
  _svec4_i32 ret;
  if(svec_extract(mask,0)) { svec_insert(&ret, 0, *((int32_t*)svec_extract(ptrs,0)));}
  if(svec_extract(mask,1)) { svec_insert(&ret, 1, *((int32_t*)svec_extract(ptrs,1)));}
  if(svec_extract(mask,2)) { svec_insert(&ret, 2, *((int32_t*)svec_extract(ptrs,2)));}
  if(svec_extract(mask,3)) { svec_insert(&ret, 3, *((int32_t*)svec_extract(ptrs,3)));}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return ret;
}
//GATHER_GENERAL_L4(_svec4_u32, uint32_t, _svec4_u64, _svec4_i1);
template<>
FORCEINLINE _svec4_u32 svec_gather<_svec4_u32>(_svec4_u64 ptrs, _svec4_i1 mask) {
  return _svec4_u32(svec_gather<_svec4_i32>(ptrs, mask).v);
}
GATHER_GENERAL_L4(_svec4_i64, int64_t, _svec4_u64, _svec4_i1);
GATHER_GENERAL_L4(_svec4_u64, uint64_t, _svec4_u64, _svec4_i1);
//GATHER_GENERAL_L4(_svec4_f, float, _svec4_u64, _svec4_i1);
template<>
FORCEINLINE _svec4_f svec_gather<_svec4_f>(_svec4_u64 ptrs, _svec4_i1 mask) {
  return _svec4_f(_mm_castsi128_ps(svec_gather<_svec4_i32>(ptrs, mask).v));
}
GATHER_GENERAL_L4(_svec4_d, double, _svec4_u64, _svec4_i1);



GATHER_BASE_OFFSETS_L4(_svec4_i8, int8_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u8, uint8_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i16, int16_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u16, uint16_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i32, int32_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u32, uint32_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i64, int64_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u64, uint64_t, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_f, float, _svec4_i32, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_d, double, _svec4_i32, _svec4_i1);


GATHER_BASE_OFFSETS_L4(_svec4_i8, int8_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u8, uint8_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i16, int16_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u16, uint16_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i32, int32_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u32, uint32_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_i64, int64_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_u64, uint64_t, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_f, float, _svec4_i64, _svec4_i1);
GATHER_BASE_OFFSETS_L4(_svec4_d, double, _svec4_i64, _svec4_i1);

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

SCATTER_GENERAL_L4(_svec4_i8, int8_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u8, uint8_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i16, int16_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u16, uint16_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i32, int32_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u32, uint32_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i64, int64_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u64, uint64_t, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_f, float, _svec4_u32, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_d, double, _svec4_u64, _svec4_i1);


SCATTER_GENERAL_L4(_svec4_i8, int8_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u8, uint8_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i16, int16_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u16, uint16_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i32, int32_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u32, uint32_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_i64, int64_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_u64, uint64_t, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_f, float, _svec4_u64, _svec4_i1);
SCATTER_GENERAL_L4(_svec4_d, double, _svec4_u32, _svec4_i1);



SCATTER_BASE_OFFSETS_L4(_svec4_i8, int8_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u8, uint8_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i16, int16_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u16, uint16_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i32, int32_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u32, uint32_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i64, int64_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u64, uint64_t, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_f, float, _svec4_i32, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_d, double, _svec4_i32, _svec4_i1);


SCATTER_BASE_OFFSETS_L4(_svec4_i8, int8_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u8, uint8_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i16, int16_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u16, uint16_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i32, int32_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u32, uint32_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_i64, int64_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_u64, uint64_t, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_f, float, _svec4_i64, _svec4_i1);
SCATTER_BASE_OFFSETS_L4(_svec4_d, double, _svec4_i64, _svec4_i1);

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
 * @param mask the svec<4,bool> type vector
 * @return true is at least one element in the mask is true
 */
static FORCEINLINE bool svec_any_true(const _svec4_i1& mask) {
  return (_mm_movemask_ps(mask.v)!=0);
}

/**
 * @brief Check all elements of the mask are non-zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are true
 */
static FORCEINLINE bool svec_all_true(const _svec4_i1& mask) {
  return (_mm_movemask_ps(mask.v)==0xF);
}


/**
 * @brief Check none elements of the mask are zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are false
 */
static FORCEINLINE bool svec_none_true(const _svec4_i1& mask) {
  return (_mm_movemask_ps(mask.v)==0);
}

// 2. bit operations

/**
 * @brief return a & b
 */
static FORCEINLINE _svec4_i1 svec_and(_svec4_i1 a, _svec4_i1 b) {
  return _mm_and_ps(a.v, b.v);
}


/**
 * @brief return a | b
 */
static FORCEINLINE _svec4_i1 svec_or(_svec4_i1 a, _svec4_i1 b) {
  return _mm_or_ps(a.v, b.v);
}

/**
 * @brief return a ^ b
 */
static FORCEINLINE _svec4_i1 svec_xor(_svec4_i1 a, _svec4_i1 b) {
  return _mm_xor_ps(a.v, b.v);
}

/**
 * @brief return ~a
 */
static FORCEINLINE _svec4_i1 svec_not(_svec4_i1 a) {
  __m128 allon = _mm_castsi128_ps(_mm_set1_epi32(-1));
  return _mm_xor_ps(a.v, allon);
}

/**
 * @brief Change a mask type (i1 vector) to a uint64_t integer
 * The method is only used for compatibility of ISPC
 * @param mask the svec<4,bool> type vector
 * @return a uint64_t integer to represent the mask
 */
static FORCEINLINE uint64_t svec_movmsk(_svec4_i1 mask) {
  return (uint64_t)_mm_movemask_ps(mask.v);
}


//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////
// 1. Unary

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
static FORCEINLINE _svec4_i8  svec_neg(_svec4_i8 a) {
  return  _mm_sub_epi8(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE _svec4_u8  svec_neg(_svec4_u8 a) {
  return  _mm_sub_epi8(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE _svec4_i16  svec_neg(_svec4_i16 a) {
  return  _mm_sub_epi16(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE _svec4_u16  svec_neg(_svec4_u16 a) {
  return  _mm_sub_epi16(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE _svec4_i32  svec_neg(_svec4_i32 a) {
  return  _mm_sub_epi32(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE _svec4_u32  svec_neg(_svec4_u32 a) {
  return  _mm_sub_epi32(_mm_setzero_si128(), (a.v));
}
//it seems i64/f/d sse overload's "-" operator.
UNARY_OP_OPT64(_svec4_i64, svec_neg, -);
UNARY_OP_OPT64(_svec4_u64, svec_neg, -);
UNARY_OP_OPT(_svec4_f, svec_neg, -);
UNARY_OP_OPT64(_svec4_d, svec_neg, -);

//  2. Math unary
//round
static FORCEINLINE _svec4_f svec_round(_svec4_f a) {
  return _mm_round_ps(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
static FORCEINLINE _svec4_d svec_round(_svec4_d a) {
  return _svec4_d(
      _mm_round_pd(a.v[0], _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC),
      _mm_round_pd(a.v[1], _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
//floor
UNARY_OP_OPT(_svec4_f, svec_floor, _mm_floor_ps);
UNARY_OP_OPT64(_svec4_d, svec_floor, _mm_floor_pd);
//ceil
UNARY_OP_OPT(_svec4_f, svec_ceil, _mm_ceil_ps);
UNARY_OP_OPT64(_svec4_d, svec_ceil, _mm_ceil_pd);
//reverse 1/
static FORCEINLINE _svec4_f svec_rcp(_svec4_f v) {
  __m128 rcp = _mm_rcp_ps(v.v);
  // N-R iteration:
  __m128 m = _mm_mul_ps(v.v, rcp);
  __m128 twominus = _mm_sub_ps(_mm_set1_ps(2.f), m);
  __m128 r = _mm_mul_ps(rcp, twominus);
  return r;
}
UNARY_OP_L4(_svec4_d, svec_rcp, 1.0/);
//reverse sqrt
static FORCEINLINE _svec4_f svec_rsqrt(_svec4_f v) {
  __m128 rsqrt = _mm_rsqrt_ps(v.v);
  // Newton-Raphson iteration to improve precision
  // return 0.5 * rsqrt * (3. - (v * rsqrt) * rsqrt);
  __m128 v_rsqrt = _mm_mul_ps(rsqrt, v.v);
  __m128 v_r_r = _mm_mul_ps(v_rsqrt, rsqrt);
  __m128 three_sub = _mm_sub_ps(_mm_set1_ps(3.f), v_r_r);
  __m128 rs_mul = _mm_mul_ps(rsqrt, three_sub);
  __m128 half_scale = _mm_mul_ps(_mm_set1_ps(0.5), rs_mul);
  return half_scale;
}
UNARY_OP_L4(_svec4_d, svec_rsqrt, 1.0/sqrt);
//sqrt
UNARY_OP_OPT(_svec4_f, svec_sqrt, _mm_sqrt_ps);
UNARY_OP_OPT64(_svec4_d, svec_sqrt, _mm_sqrt_pd);
//exp - _mm_exp_ps/_mm_exp_pd not in gcc but in ICC
UNARY_OP_L4(_svec4_f, svec_exp, expf);
UNARY_OP_L4(_svec4_d, svec_exp, exp);
//log - _mm_log_ps / _mm_log_pd not in gcc but in ICC
UNARY_OP_L4(_svec4_f, svec_log, logf);
UNARY_OP_L4(_svec4_d, svec_log, log);
//abs - for all types
UNARY_OP_L4(_svec4_i8, svec_abs, abs<int8_t>);
static FORCEINLINE _svec4_u8  svec_abs(_svec4_u8 v) { return v;}
UNARY_OP_L4(_svec4_i16, svec_abs, abs<int16_t>);
static FORCEINLINE _svec4_u16  svec_abs(_svec4_u16 v) { return v;}
UNARY_OP_L4(_svec4_i32, svec_abs, abs<int32_t>);
static FORCEINLINE _svec4_u32  svec_abs(_svec4_u32 v) { return v;}
UNARY_OP_L4(_svec4_i64, svec_abs, abs<int64_t>);
static FORCEINLINE _svec4_u64  svec_abs(_svec4_u64 v) { return v;}
//UNARY_OP(_svec4_f, svec_abs, abs);
static FORCEINLINE _svec4_f svec_abs(_svec4_f v) {
  unsigned int x = 0x7fffffff;
  float &f = * (float *)( &x );
  __m128 tmp = _mm_set1_ps(f);
  return _mm_and_ps(v.v, tmp);
}
UNARY_OP_L4(_svec4_d, svec_abs, abs);

//  3. Binary

#define BINARY_OP_OPT_FUNC(TYPE, TYPE_B, NAME, FUNC) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE_B b) { \
  return TYPE(FUNC(a.v, b.v)); \
}

#define BINARY_OP_OPT_FUNC64(TYPE, TYPE_B, NAME, FUNC) \
static FORCEINLINE TYPE NAME(TYPE a, TYPE_B b) { \
  return TYPE(FUNC(a.v[0], b.v[0]), FUNC(a.v[1], b.v[1])); \
}

//add, sub, div, mul.

//add
BINARY_OP_OPT_FUNC(_svec4_i8, _svec4_i8, svec_add, _mm_add_epi8);
BINARY_OP_OPT_FUNC(_svec4_u8, _svec4_u8, svec_add, _mm_add_epi8);
BINARY_OP_OPT_FUNC(_svec4_i16, _svec4_i16, svec_add, _mm_add_epi16);
BINARY_OP_OPT_FUNC(_svec4_u16, _svec4_u16, svec_add, _mm_add_epi16);
BINARY_OP_OPT_FUNC(_svec4_i32, _svec4_i32, svec_add, _mm_add_epi32);
BINARY_OP_OPT_FUNC(_svec4_u32, _svec4_u32, svec_add, _mm_add_epi32);
BINARY_OP_OPT_FUNC64(_svec4_i64, _svec4_i64, svec_add, _mm_add_epi64);
BINARY_OP_OPT_FUNC64(_svec4_u64, _svec4_u64, svec_add, _mm_add_epi64);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_add, _mm_add_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_add, _mm_add_pd);

//sub
BINARY_OP_OPT_FUNC(_svec4_i8, _svec4_i8, svec_sub, _mm_sub_epi8);
BINARY_OP_OPT_FUNC(_svec4_u8, _svec4_u8, svec_sub, _mm_sub_epi8);
BINARY_OP_OPT_FUNC(_svec4_i16, _svec4_i16, svec_sub, _mm_sub_epi16);
BINARY_OP_OPT_FUNC(_svec4_u16, _svec4_u16, svec_sub, _mm_sub_epi16);
BINARY_OP_OPT_FUNC(_svec4_i32, _svec4_i32, svec_sub, _mm_sub_epi32);
BINARY_OP_OPT_FUNC(_svec4_u32, _svec4_u32, svec_sub, _mm_sub_epi32);
BINARY_OP_OPT_FUNC64(_svec4_i64, _svec4_i64, svec_sub, _mm_sub_epi64);
BINARY_OP_OPT_FUNC64(_svec4_u64, _svec4_u64, svec_sub, _mm_sub_epi64);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_sub, _mm_sub_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_sub, _mm_sub_pd);

//mul
BINARY_OP_L4(_svec4_i8, svec_mul, *);
BINARY_OP_L4(_svec4_u8, svec_mul, *);
BINARY_OP_L4(_svec4_i16, svec_mul, *);
BINARY_OP_L4(_svec4_u16, svec_mul, *);
BINARY_OP_OPT_FUNC(_svec4_i32, _svec4_i32, svec_mul, _mm_mullo_epi32);
BINARY_OP_OPT_FUNC(_svec4_u32, _svec4_u32, svec_mul, _mm_mullo_epi32);
BINARY_OP_L4(_svec4_i64, svec_mul, *);
BINARY_OP_L4(_svec4_u64, svec_mul, *);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_mul, _mm_mul_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_mul, _mm_mul_pd);

//div - no _mm_idiv_epi32 and _mm_udiv_epi32
BINARY_OP_L4(_svec4_i8, svec_div, /);
BINARY_OP_L4(_svec4_u8, svec_div, /);
BINARY_OP_L4(_svec4_i16, svec_div, /);
BINARY_OP_L4(_svec4_u16, svec_div, /);
BINARY_OP_L4(_svec4_i32, svec_div, /);
BINARY_OP_L4(_svec4_u32, svec_div, /);
BINARY_OP_L4(_svec4_i64, svec_div, /);
BINARY_OP_L4(_svec4_u64, svec_div, /);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_div, _mm_div_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_div, _mm_div_pd);

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

BIN_VEC_SCAL(_svec4_i8, int8_t);
BIN_VEC_SCAL(_svec4_u8, uint8_t);
BIN_VEC_SCAL(_svec4_i16, int16_t);
BIN_VEC_SCAL(_svec4_u16, uint16_t);
BIN_VEC_SCAL(_svec4_i32, int32_t);
BIN_VEC_SCAL(_svec4_u32, uint32_t);
BIN_VEC_SCAL(_svec4_i64, int64_t);
BIN_VEC_SCAL(_svec4_u64, uint64_t);
BIN_VEC_SCAL(_svec4_f, float);
BIN_VEC_SCAL(_svec4_d, double);




#define INT_BINARY_OP_METHODS(VTYPE, STYPE) \
BINARY_OP_OPT_FUNC(VTYPE, VTYPE, svec_or, _mm_or_si128); \
BINARY_OP_OPT_FUNC(VTYPE, VTYPE, svec_and, _mm_and_si128); \
BINARY_OP_OPT_FUNC(VTYPE, VTYPE, svec_xor, _mm_xor_si128); \
BINARY_OP_L4(VTYPE, svec_rem, %); \
BINARY_OP_SCALAR_L4(VTYPE, STYPE, svec_rem, %);

#define INT_BINARY_OP_METHODS64(VTYPE, STYPE) \
BINARY_OP_OPT_FUNC64(VTYPE, VTYPE, svec_or, _mm_or_si128); \
BINARY_OP_OPT_FUNC64(VTYPE, VTYPE, svec_and, _mm_and_si128); \
BINARY_OP_OPT_FUNC64(VTYPE, VTYPE, svec_xor, _mm_xor_si128); \
BINARY_OP_L4(VTYPE, svec_rem, %); \
BINARY_OP_SCALAR_L4(VTYPE, STYPE, svec_rem, %);




INT_BINARY_OP_METHODS(_svec4_i8, int8_t);
INT_BINARY_OP_METHODS(_svec4_u8, uint8_t);
INT_BINARY_OP_METHODS(_svec4_i16, int16_t);
INT_BINARY_OP_METHODS(_svec4_u16, uint16_t);
INT_BINARY_OP_METHODS(_svec4_i32, int32_t);
INT_BINARY_OP_METHODS(_svec4_u32, uint32_t);
INT_BINARY_OP_METHODS64(_svec4_i64, int64_t);
INT_BINARY_OP_METHODS64(_svec4_u64, uint64_t);


//power only for float - cannot find _mm_pow_ps/pd in gcc
BINARY_OP_FUNC(_svec4_f, svec_pow, powf);
BINARY_OP_FUNC(_svec4_d, svec_pow, pow);

//shift left
BINARY_OP2_L4(_svec4_i8, _svec4_u8, svec_shl, <<);
BINARY_OP2_L4(_svec4_u8, _svec4_u8, svec_shl, <<);
BINARY_OP2_L4(_svec4_i16, _svec4_u16, svec_shl, <<);
BINARY_OP2_L4(_svec4_u16, _svec4_u16, svec_shl, <<);
BINARY_OP2_L4(_svec4_i32, _svec4_u32, svec_shl, <<);
BINARY_OP2_L4(_svec4_u32, _svec4_u32, svec_shl, <<);
BINARY_OP2_L4(_svec4_i64, _svec4_u64, svec_shl, <<);
BINARY_OP2_L4(_svec4_u64, _svec4_u64, svec_shl, <<);

//shift right
BINARY_OP2_L4(_svec4_i8, _svec4_u8, svec_shr, >>);
BINARY_OP2_L4(_svec4_u8, _svec4_u8, svec_shr, >>);
BINARY_OP2_L4(_svec4_i16, _svec4_u16, svec_shr, >>);
BINARY_OP2_L4(_svec4_u16, _svec4_u16, svec_shr, >>);
BINARY_OP2_L4(_svec4_i32, _svec4_u32, svec_shr, >>);
BINARY_OP2_L4(_svec4_u32, _svec4_u32, svec_shr, >>);
BINARY_OP2_L4(_svec4_i64, _svec4_u64, svec_shr, >>);
BINARY_OP2_L4(_svec4_u64, _svec4_u64, svec_shr, >>);

// shift scalar left
BINARY_OP_SCALAR_L4(_svec4_i8, int32_t, svec_shl, <<);
BINARY_OP_SCALAR_L4(_svec4_u8, int32_t, svec_shl, <<);
//BINARY_OP_SCALAR_L4(_svec4_i16, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_i16  svec_shl(_svec4_i16 a, int32_t s) {
  return _svec4_i16(_mm_sll_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}

//BINARY_OP_SCALAR_L4(_svec4_u16, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_u16  svec_shl(_svec4_u16 a, int32_t s) {
  return _svec4_u16(_mm_sll_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_i32, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_i32  svec_shl(_svec4_i32 a, int32_t s) {
  return _svec4_i32(_mm_sll_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_u32, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_u32  svec_shl(_svec4_u32 a, int32_t s) {
  return _svec4_u32(_mm_sll_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_i64, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_i64  svec_shl(_svec4_i64 a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return _svec4_i64(_mm_sll_epi64(a.v[0], amt),
                    _mm_sll_epi64(a.v[1], amt));
}
//BINARY_OP_SCALAR_L4(_svec4_u64, int32_t, svec_shl, <<);
static FORCEINLINE _svec4_u64  svec_shl(_svec4_u64 a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return _svec4_u64(_mm_sll_epi64(a.v[0], amt),
                    _mm_sll_epi64(a.v[1], amt));
}

//shift sclar right
BINARY_OP_SCALAR_L4(_svec4_i8, int32_t, svec_shr, >>);
BINARY_OP_SCALAR_L4(_svec4_u8, int32_t, svec_shr, >>);
//BINARY_OP_SCALAR_L4(_svec4_i16, int32_t, svec_shr, >>);
static FORCEINLINE _svec4_i16  svec_shr(_svec4_i16 a, int32_t s) {
  return _svec4_i16(_mm_sra_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_u16, int32_t, svec_shr, >>);
static FORCEINLINE _svec4_u16  svec_shr(_svec4_u16 a, int32_t s) {
  return _svec4_u16(_mm_srl_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_i32, int32_t, svec_shr, >>);
static FORCEINLINE _svec4_i32  svec_shr(_svec4_i32 a, int32_t s) {
  return _svec4_i32(_mm_sra_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(_svec4_u32, int32_t, svec_shr, >>);
static FORCEINLINE _svec4_u32  svec_shr(_svec4_u32 a, int32_t s) {
  return _svec4_u32(_mm_srl_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
BINARY_OP_SCALAR_L4(_svec4_i64, int32_t, svec_shr, >>);
//BINARY_OP_SCALAR_L4(_svec4_u64, int32_t, svec_shr, >>);
static FORCEINLINE _svec4_u64  svec_shr(_svec4_u64 a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return _svec4_u64(_mm_srl_epi64(a.v[0], amt),
                    _mm_srl_epi64(a.v[1], amt));
}

//  4. Ternary

//madd / msub for only int32/u32/float/double
#define TERNERY_OPT(VTYPE) \
/**
 * @brief vector multiply and add operation. return a * b + c.
 */ \
FORCEINLINE VTYPE svec_madd(VTYPE a, VTYPE b, VTYPE c) { \
  return a * b + c;\
} \
/**
 * @brief vector multiply and add operation. return a * b - c.
 */ \
FORCEINLINE VTYPE svec_msub(VTYPE a, VTYPE b, VTYPE c) { \
  return a * b - c;\
} \
/**
 * @brief vector multiply and add operation. return -(a * b - c).
 */ \
FORCEINLINE VTYPE svec_nmsub(VTYPE a, VTYPE b, VTYPE c) { \
  return c - a * b ;\
}


TERNERY_OPT(_svec4_i32);
TERNERY_OPT(_svec4_u32);
TERNERY_OPT(_svec4_i64);
TERNERY_OPT(_svec4_u64);
TERNERY_OPT(_svec4_f);
TERNERY_OPT(_svec4_d);


//  5. Max/Min
BINARY_OP_FUNC_L4(_svec4_i8, svec_max, max<int8_t>);
BINARY_OP_FUNC_L4(_svec4_u8, svec_max, max<uint8_t>);
BINARY_OP_FUNC_L4(_svec4_i16, svec_max, max<int16_t>);
BINARY_OP_FUNC_L4(_svec4_u16, svec_max, max<uint16_t>);
BINARY_OP_OPT_FUNC(_svec4_i32, _svec4_i32, svec_max, _mm_max_epi32);
BINARY_OP_OPT_FUNC(_svec4_u32, _svec4_u32, svec_max, _mm_max_epu32);
BINARY_OP_FUNC_L4(_svec4_i64, svec_max, max<int64_t>);
BINARY_OP_FUNC_L4(_svec4_u64, svec_max, max<uint64_t>);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_max, _mm_max_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_max, _mm_max_pd);

BINARY_OP_FUNC_L4(_svec4_i8, svec_min, min<int8_t>);
BINARY_OP_FUNC_L4(_svec4_u8, svec_min, min<uint8_t>);
BINARY_OP_FUNC_L4(_svec4_i16, svec_min, min<int16_t>);
BINARY_OP_FUNC_L4(_svec4_u16, svec_min, min<uint16_t>);
BINARY_OP_OPT_FUNC(_svec4_i32, _svec4_i32, svec_min, _mm_min_epi32);
BINARY_OP_OPT_FUNC(_svec4_u32, _svec4_u32, svec_min, _mm_min_epu32);
BINARY_OP_FUNC_L4(_svec4_i64, svec_min, min<int64_t>);
BINARY_OP_FUNC_L4(_svec4_u64, svec_min, min<uint64_t>);
BINARY_OP_OPT_FUNC(_svec4_f, _svec4_f, svec_min, _mm_min_ps);
BINARY_OP_OPT_FUNC64(_svec4_d, _svec4_d, svec_min, _mm_min_pd);



//6. Reduce

#define MAX_MIN_REDUCE_METHODS(VTYPE, STYPE) \
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

//FORCEINLINE _svec4_d svec_preduce_add(_svec4_d v0, _svec4_d v1, _svec4_d v2, _svec4_d v3) {
//  return _svec4_d(
//      svec_reduce_add(v0),
//      svec_reduce_add(v1),
//      svec_reduce_add(v2),
//      svec_reduce_add(v3)
//      );
//}

//FORCEINLINE _svec4_d svec_preduce_add(_svec4_d v0, _svec4_d v1, _svec4_d v2, _svec4_d v3) {
//  __m128d s00 = _mm_hadd_pd(v0.v[0], v0.v[1]);
//  __m128d s01 = _mm_hadd_pd(v1.v[0], v1.v[1]);
//  __m128d s02 = _mm_hadd_pd(v2.v[0], v2.v[1]);
//  __m128d s03 = _mm_hadd_pd(v3.v[0], v3.v[1]);
//
//  __m128d s0 = _mm_hadd_pd(s00, s01);
//  __m128d s1 = _mm_hadd_pd(s02, s03);
//
//  return _svec4_d(s0, s1);
//}

FORCEINLINE _svec4_d svec_preduce_add(_svec4_d v0, _svec4_d v1, _svec4_d v2, _svec4_d v3) {
  __m128d s00 = _mm_add_pd(v0.v[0], v0.v[1]);
  __m128d s01 = _mm_add_pd(v1.v[0], v1.v[1]);
  __m128d s02 = _mm_add_pd(v2.v[0], v2.v[1]);
  __m128d s03 = _mm_add_pd(v3.v[0], v3.v[1]);

  __m128d s0 = _mm_hadd_pd(s00, s01);
  __m128d s1 = _mm_hadd_pd(s02, s03);

  return _svec4_d(s0, s1);
}

//  7. Compare
/**
 * @brief element by element comparison of two svec_vec4_i1 type object
 * @param a
 * @param b
 * @return a svec_vec4_i1 object
 */
static FORCEINLINE _svec4_i1 svec_equal(_svec4_i1 a, _svec4_i1 b) {
  return _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(a.v), _mm_castps_si128(b.v)));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_i1 a, _svec4_i1 b) {
  return svec_not(svec_equal(a, b));
}

static FORCEINLINE _svec4_i1 svec_equal(_svec4_i8 a, _svec4_i8 b) {
  __m128i cmp = _mm_cmpeq_epi8(a.v, b.v);
  return _svec4_i1(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}
static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_i8 a, _svec4_i8 b) {
  return ~(a == b);
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_i8 a, _svec4_i8 b) {
  __m128i cmp = _mm_cmplt_epi8(a.v, b.v);
  return _svec4_i1(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_i8 a, _svec4_i8 b) {
  return (a < b) | (a == b);
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_i8 a, _svec4_i8 b) {
  __m128i cmp = _mm_cmpgt_epi8(a.v, b.v);
  return _svec4_i1(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_i8 a, _svec4_i8 b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(_svec4_i8, _svec4_i1);

static FORCEINLINE _svec4_i1 svec_equal(_svec4_u8 a, _svec4_u8 b) {
  __m128i cmp = _mm_cmpeq_epi8(a.v, b.v);
  return _svec4_i1(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_u8 a, _svec4_u8 b) {
  return ~(a == b);
}

CMP_OP_L4(_svec4_u8, _svec4_i1, less_than, <);
CMP_OP_L4(_svec4_u8, _svec4_i1, less_equal, <=);
CMP_OP_L4(_svec4_u8, _svec4_i1, greater_than, >);
CMP_OP_L4(_svec4_u8, _svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(_svec4_u8, _svec4_i1);


static FORCEINLINE _svec4_i1 svec_equal(_svec4_i16 a, _svec4_i16 b) {
  __m128i cmp = _mm_cmpeq_epi16(a.v, b.v);
  return _svec4_i1(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}
static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_i16 a, _svec4_i16 b) {
  return ~(a == b);
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_i16 a, _svec4_i16 b) {
  __m128i cmp = _mm_cmplt_epi16(a.v, b.v);
  return _svec4_i1(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_i16 a, _svec4_i16 b) {
  return (a < b) | (a == b);
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_i16 a, _svec4_i16 b) {
  __m128i cmp = _mm_cmpgt_epi16(a.v, b.v);
  return _svec4_i1(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_i16 a, _svec4_i16 b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(_svec4_i16, _svec4_i1);

static FORCEINLINE _svec4_i1 svec_equal(_svec4_u16 a, _svec4_u16 b) {
  __m128i cmp = _mm_cmpeq_epi16(a.v, b.v);
  return _svec4_i1(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_u16 a, _svec4_u16 b) {
  return ~(a == b);
}

CMP_OP_L4(_svec4_u16, _svec4_i1, less_than, <);
CMP_OP_L4(_svec4_u16, _svec4_i1, less_equal, <=);
CMP_OP_L4(_svec4_u16, _svec4_i1, greater_than, >);
CMP_OP_L4(_svec4_u16, _svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(_svec4_u16, _svec4_i1);


static FORCEINLINE _svec4_i1 svec_equal(_svec4_i32 a, _svec4_i32 b) {
  __m128i cmp = _mm_cmpeq_epi32(a.v, b.v);
  return _svec4_i1(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}
static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_i32 a, _svec4_i32 b) {
  return ~(a == b);
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_i32 a, _svec4_i32 b) {
  __m128i cmp = _mm_cmplt_epi32(a.v, b.v);
  return _svec4_i1(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_i32 a, _svec4_i32 b) {
  return (a < b) | (a == b);
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_i32 a, _svec4_i32 b) {
  __m128i cmp = _mm_cmpgt_epi32(a.v, b.v);
  return _svec4_i1(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_i32 a, _svec4_i32 b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(_svec4_i32, _svec4_i1);

static FORCEINLINE _svec4_i1 svec_equal(_svec4_u32 a, _svec4_u32 b) {
  __m128i cmp = _mm_cmpeq_epi32(a.v, b.v);
  return _svec4_i1(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_u32 a, _svec4_u32 b) {
  return ~(a == b);
}

CMP_OP_L4(_svec4_u32, _svec4_i1, less_than, <);
CMP_OP_L4(_svec4_u32, _svec4_i1, less_equal, <=);
CMP_OP_L4(_svec4_u32, _svec4_i1, greater_than, >);
CMP_OP_L4(_svec4_u32, _svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(_svec4_u32, _svec4_i1);


static FORCEINLINE _svec4_i1 svec_equal(_svec4_i64 a, _svec4_i64 b) {
  __m128i cmp0 = _mm_cmpeq_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpeq_epi64(a.v[1], b.v[1]);
  return _svec4_i1(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}
static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_i64 a, _svec4_i64 b) {
  return ~(a == b);
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_i64 a, _svec4_i64 b) {
  return ~(a >= b);
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_i64 a, _svec4_i64 b) {
  return (a < b) | (a == b);
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_i64 a, _svec4_i64 b) {
  __m128i cmp0 = _mm_cmpgt_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpgt_epi64(a.v[1], b.v[1]);
  return _svec4_i1(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_i64 a, _svec4_i64 b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(_svec4_i64, _svec4_i1);

static FORCEINLINE _svec4_i1 svec_equal(_svec4_u64 a, _svec4_u64 b) {
  __m128i cmp0 = _mm_cmpeq_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpeq_epi64(a.v[1], b.v[1]);
  return _svec4_i1(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_u64 a, _svec4_u64 b) {
  return ~(a == b);
}

CMP_OP_L4(_svec4_u64, _svec4_i1, less_than, <);
CMP_OP_L4(_svec4_u64, _svec4_i1, less_equal, <=);
CMP_OP_L4(_svec4_u64, _svec4_i1, greater_than, >);
CMP_OP_L4(_svec4_u64, _svec4_i1, greater_equal, >=);
CMP_ALL_MASKED_OP(_svec4_u64, _svec4_i1);


static FORCEINLINE _svec4_i1 svec_equal(_svec4_f a, _svec4_f b) {
  return _mm_cmpeq_ps(a.v, b.v);
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_f a, _svec4_f b) {
  return _mm_cmpneq_ps(a.v, b.v);
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_f a, _svec4_f b) {
  return _mm_cmplt_ps(a.v, b.v);
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_f a, _svec4_f b) {
  return _mm_cmple_ps(a.v, b.v);
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_f a, _svec4_f b) {
  return _mm_cmpgt_ps(a.v, b.v);
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_f a, _svec4_f b) {
  return _mm_cmpge_ps(a.v, b.v);
}

CMP_ALL_MASKED_OP(_svec4_f, _svec4_i1);

static FORCEINLINE _svec4_i1 svec_equal(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmpeq_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpeq_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE _svec4_i1 svec_not_equal(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmpneq_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpneq_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE _svec4_i1 svec_less_than(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmplt_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmplt_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE _svec4_i1 svec_less_equal(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmple_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmple_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE _svec4_i1 svec_greater_than(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmpgt_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpgt_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2 ,0));
}

static FORCEINLINE _svec4_i1 svec_greater_equal(_svec4_d a, _svec4_d b) {
  __m128d cmp0 = _mm_cmpge_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpge_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

CMP_ALL_MASKED_OP(_svec4_d, _svec4_i1);



//  8. Cast

/**
 * Here we provide the full cast combinations.
 * Some may have fast impl
 */


/**
 * @brief cast based on directly change the __mm object type type
 */
#define CAST_OPT(FROM, TO, STO)        \
template <class T> static T svec_cast(FROM val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast<TO>(FROM val) {      \
    return TO((val.v)); \
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
    return TO((val.v[0]),(val.v[1])); \
}


//i1 -> all
//CAST_L4(_svec4_i1, _svec4_i1, uint32_t);
//CAST_L4(_svec4_i1, _svec4_i8, int8_t);  //better way: packing
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_i8 type.
 */
template <> FORCEINLINE _svec4_i8 svec_cast<_svec4_i8>(_svec4_i1 val) {
  return svec_select(val, _svec4_i8(0xff), _svec4_i8(0));
}
//CAST_L4(_svec4_i1, _svec4_u8, uint8_t);  //better way: packing
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_u8 type.
 */
template <> FORCEINLINE _svec4_u8 svec_cast<_svec4_u8>(_svec4_i1 val) {
  return svec_select(val, _svec4_u8(0xff), _svec4_u8(0));
}
//CAST_L4(_svec4_i1, _svec4_i16, int16_t);  //better way: packing
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_i16 type.
 */
template <> FORCEINLINE _svec4_i16 svec_cast<_svec4_i16>(_svec4_i1 val) {
  return svec_select(val, _svec4_i16(0xffff), _svec4_i16(0));
}
//CAST_L4(_svec4_i1, _svec4_u16, uint16_t); //better way: packing
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_u16 type.
 */
template <> FORCEINLINE _svec4_u16 svec_cast<_svec4_u16>(_svec4_i1 val) {
  return svec_select(val, _svec4_u16(0xffff), _svec4_u16(0));
}
//CAST_L4(_svec4_i1, _svec4_i32, int32_t);
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_i32 type.
 */
template <> FORCEINLINE _svec4_i32 svec_cast<_svec4_i32>(_svec4_i1 val) {
  return _mm_castps_si128(val.v);
}
//CAST_L4(_svec4_i1, _svec4_u32, uint32_t);
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_u32 type.
 */
template <> FORCEINLINE _svec4_u32 svec_cast<_svec4_u32>(_svec4_i1 val) {
  return _mm_and_si128(_mm_castps_si128(val.v), _mm_set1_epi32(-1));
}
CAST_L4(_svec4_i1, _svec4_i64, int64_t); //better way: unpack, singed ext
CAST_L4(_svec4_i1, _svec4_u64, uint64_t);//better way: unpack, singed ext
//CAST_L4(_svec4_i1, _svec4_f, float); //si to fp call
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_f type.
 */
template <> FORCEINLINE _svec4_f svec_cast<_svec4_f>(_svec4_i1 val) {
  return svec_select(val, _svec4_f(4294967295.), _svec4_f(0));
}
//CAST_L4(_svec4_i1, _svec4_d, double);
template <class T> static T svec_cast(_svec4_i1 val);
/**
 * @brief cast val from _svec4_i1 type to _svec4_f type.
 */
template <> FORCEINLINE _svec4_d svec_cast<_svec4_d>(_svec4_i1 val) {
  return svec_select(val, _svec4_d(4294967295.), _svec4_d(0));
}

//i8 -> all
CAST_L4(_svec4_i8, _svec4_i1, uint32_t);
//CAST_L4(_svec4_i8, _svec4_i8, int8_t);
CAST_OPT(_svec4_i8, _svec4_u8, uint8_t);
CAST_L4(_svec4_i8, _svec4_i16, int16_t); //better way, use vec_unpackh
CAST_L4(_svec4_i8, _svec4_u16, uint16_t); //better way, sext + zero mask and
CAST_L4(_svec4_i8, _svec4_i32, int32_t); //better way, use twice vec_unpack
CAST_L4(_svec4_i8, _svec4_u32, uint32_t); //better way, use unpack + zero mask
CAST_L4(_svec4_i8, _svec4_i64, int64_t);
CAST_L4(_svec4_i8, _svec4_u64, uint64_t);
CAST_L4(_svec4_i8, _svec4_f, float);
CAST_L4(_svec4_i8, _svec4_d, double);

//u8 -> all
CAST_L4(_svec4_u8, _svec4_i1, uint32_t);
CAST_OPT(_svec4_u8, _svec4_i8, int8_t);
//CAST_L4(_svec4_u8, _svec4_u8, uint8_t);
CAST_L4(_svec4_u8, _svec4_i16, int16_t); //better way, use unpack + zero mask
CAST_L4(_svec4_u8, _svec4_u16, uint16_t); //better way use unpack + zero mask
CAST_L4(_svec4_u8, _svec4_i32, int32_t);
CAST_L4(_svec4_u8, _svec4_u32, uint32_t);
CAST_L4(_svec4_u8, _svec4_i64, int64_t);
CAST_L4(_svec4_u8, _svec4_u64, uint64_t);
CAST_L4(_svec4_u8, _svec4_f, float);
CAST_L4(_svec4_u8, _svec4_d, double);

//i16 -> all
CAST_L4(_svec4_i16, _svec4_i1, uint32_t);
CAST_L4(_svec4_i16, _svec4_i8, int8_t); //could use pack
CAST_L4(_svec4_i16, _svec4_u8, uint8_t); //could use pack
//CAST_L4(_svec4_i16, _svec4_i16, int16_t);
CAST_OPT(_svec4_i16, _svec4_u16, uint16_t);
CAST_L4(_svec4_i16, _svec4_i32, int32_t); //use unpack
CAST_L4(_svec4_i16, _svec4_u32, uint32_t); //use unpack and zeromaskout
CAST_L4(_svec4_i16, _svec4_i64, int64_t);
CAST_L4(_svec4_i16, _svec4_u64, uint64_t);
CAST_L4(_svec4_i16, _svec4_f, float);
CAST_L4(_svec4_i16, _svec4_d, double);

//u16 -> all
CAST_L4(_svec4_u16, _svec4_i1, uint32_t);
CAST_L4(_svec4_u16, _svec4_i8, int8_t);
CAST_L4(_svec4_u16, _svec4_u8, uint8_t);
CAST_OPT(_svec4_u16, _svec4_i16, int16_t);
//CAST_L4(_svec4_u16, _svec4_u16, uint16_t);
CAST_L4(_svec4_u16, _svec4_i32, int32_t); //use unpack +mask
CAST_L4(_svec4_u16, _svec4_u32, uint32_t); //use unpack + mask
CAST_L4(_svec4_u16, _svec4_i64, int64_t);
CAST_L4(_svec4_u16, _svec4_u64, uint64_t);
CAST_L4(_svec4_u16, _svec4_f, float);
CAST_L4(_svec4_u16, _svec4_d, double);

//i32 -> all
CAST_L4(_svec4_i32, _svec4_i1, uint32_t);
CAST_L4(_svec4_i32, _svec4_i8, int8_t);
CAST_L4(_svec4_i32, _svec4_u8, uint8_t);
CAST_L4(_svec4_i32, _svec4_i16, int16_t);
CAST_L4(_svec4_i32, _svec4_u16, uint16_t);
//CAST_L4(_svec4_i32, _svec4_i32, int32_t);
CAST_OPT(_svec4_i32, _svec4_u32, uint32_t);
CAST_L4(_svec4_i32, _svec4_i64, int64_t); //use p8 unpack
CAST_L4(_svec4_i32, _svec4_u64, uint64_t); //use p8 unpack
//CAST_L4(_svec4_i32, _svec4_f, float); //use ctf
template <class T> static T svec_cast(_svec4_i32 val);
/**
 * @brief cast val from _svec4_i32 type to _svec4_f type.
 */
template <> FORCEINLINE _svec4_f svec_cast<_svec4_f> (_svec4_i32 val) {
  return _mm_cvtepi32_ps(val.v);
}
//CAST_L4(_svec4_i32, _svec4_d, double);
template <class T> static T svec_cast(_svec4_i32 val);
/**
 * @brief cast val from _svec4_i32 type to _svec4_d type.
 */
template <> FORCEINLINE _svec4_d svec_cast<_svec4_d> (_svec4_i32 val) {
  __m128d r0 = _mm_cvtepi32_pd(val.v);
  __m128 shuf = _mm_shuffle_ps(_mm_castsi128_ps(val.v),
                               _mm_castsi128_ps(val.v),
                               _MM_SHUFFLE(3, 2, 3, 2));
  __m128d r1 = _mm_cvtepi32_pd(_mm_castps_si128(shuf));
  return _svec4_d(r0, r1);
}

//u32 -> all
CAST_L4(_svec4_u32, _svec4_i1, uint32_t);
CAST_L4(_svec4_u32, _svec4_i8, int8_t);
CAST_L4(_svec4_u32, _svec4_u8, uint8_t);
CAST_L4(_svec4_u32, _svec4_i16, int16_t);
CAST_L4(_svec4_u32, _svec4_u16, uint16_t);
CAST_OPT(_svec4_u32, _svec4_i32, int32_t);
//CAST_L4(_svec4_u32, _svec4_u32, uint32_t);
CAST_L4(_svec4_u32, _svec4_i64, int64_t); //use p8 unpack
CAST_L4(_svec4_u32, _svec4_u64, uint64_t); //use p8 unpack
CAST_L4(_svec4_u32, _svec4_f, float);
CAST_L4(_svec4_u32, _svec4_d, double);

//i64-> all
CAST_L4(_svec4_i64, _svec4_i1, uint32_t);
CAST_L4(_svec4_i64, _svec4_i8, int8_t);
CAST_L4(_svec4_i64, _svec4_u8, uint8_t);
CAST_L4(_svec4_i64, _svec4_i16, int16_t);
CAST_L4(_svec4_i64, _svec4_u16, uint16_t);
CAST_L4(_svec4_i64, _svec4_i32, int32_t); //use p8 trunk
CAST_L4(_svec4_i64, _svec4_u32, uint32_t); //use p8 trunk
//CAST_L4(_svec4_i64, _svec4_i64, int64_t);
CAST_OPT64(_svec4_i64, _svec4_u64, uint64_t);
CAST_L4(_svec4_i64, _svec4_f, float);
CAST_L4(_svec4_i64, _svec4_d, double);

//u64 -> all
CAST_L4(_svec4_u64, _svec4_i1, uint32_t);
CAST_L4(_svec4_u64, _svec4_i8, int8_t);
CAST_L4(_svec4_u64, _svec4_u8, uint8_t);
CAST_L4(_svec4_u64, _svec4_i16, int16_t);
CAST_L4(_svec4_u64, _svec4_u16, uint16_t);
CAST_L4(_svec4_u64, _svec4_i32, int32_t); //use p8 pack
CAST_L4(_svec4_u64, _svec4_u32, uint32_t); //use p8 pack
CAST_OPT64(_svec4_u64, _svec4_i64, int64_t);
//CAST_L4(_svec4_u64, _svec4_u64, uint64_t);
CAST_L4(_svec4_u64, _svec4_f, float);
CAST_L4(_svec4_u64, _svec4_d, double);

//float -> all
CAST_L4(_svec4_f, _svec4_i1, uint32_t);
CAST_L4(_svec4_f, _svec4_i8, int8_t); //use cts + pack+pack
CAST_L4(_svec4_f, _svec4_u8, uint8_t); //use ctu + pack + pack
CAST_L4(_svec4_f, _svec4_i16, int16_t); //use cts + pack
CAST_L4(_svec4_f, _svec4_u16, uint16_t); //use ctu + pack
//CAST_L4(_svec4_f, _svec4_i32, int32_t);//use cts
template <class T> static T svec_cast(_svec4_f val);
/**
 * @brief cast val from _svec4_f type to _svec4_i32 type.
 */
template <> FORCEINLINE _svec4_i32 svec_cast<_svec4_i32>(_svec4_f val) {
  return _mm_cvttps_epi32(val.v);
}
CAST_L4(_svec4_f, _svec4_u32, uint32_t); //use ctu
CAST_L4(_svec4_f, _svec4_i64, int64_t);
CAST_L4(_svec4_f, _svec4_u64, uint64_t);
//CAST_L4(_svec4_f, _svec4_f, float);
//CAST_L4(_svec4_f, _svec4_d, double);
template <class T> static T svec_cast(_svec4_f val);
/**
 * @brief cast val from _svec4_f type to _svec4_d type.
 */
template <> FORCEINLINE _svec4_d svec_cast<_svec4_d>(_svec4_f val) {
  return _svec4_d(_mm_cvtps_pd(val.v),
                  _mm_cvtps_pd(_mm_shuffle_ps(val.v, val.v,
                                              _MM_SHUFFLE(3, 2, 3, 2))));
}

//double -> all
CAST_L4(_svec4_d, _svec4_i1, uint32_t);
CAST_L4(_svec4_d, _svec4_i8, int8_t);
CAST_L4(_svec4_d, _svec4_u8, uint8_t);
CAST_L4(_svec4_d, _svec4_i16, int16_t);
CAST_L4(_svec4_d, _svec4_u16, uint16_t);
//CAST_L4(_svec4_d, _svec4_i32, int32_t);
template <class T> static T svec_cast(_svec4_d val);
/**
 * @brief cast val from _svec4_d type to _svec4_i32 type.
 */
template <> FORCEINLINE _svec4_i32 svec_cast<_svec4_i32>(_svec4_d val) {
  __m128i r0 = _mm_cvtpd_epi32(val.v[0]);
  __m128i r1 = _mm_cvtpd_epi32(val.v[1]);
  return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(r0), _mm_castsi128_ps(r1),
                                         _MM_SHUFFLE(1, 0, 1, 0)));
}

CAST_L4(_svec4_d, _svec4_u32, uint32_t);
CAST_L4(_svec4_d, _svec4_i64, int64_t);
CAST_L4(_svec4_d, _svec4_u64, uint64_t);
//CAST_L4(_svec4_d, _svec4_f, float);
template <class T> static T svec_cast(_svec4_d val);
/**
 * @brief cast val from _svec4_d type to _svec4_f type.
 */
template <> FORCEINLINE _svec4_f svec_cast<_svec4_f>(_svec4_d val) {
  __m128 r0 = _mm_cvtpd_ps(val.v[0]);
  __m128 r1 = _mm_cvtpd_ps(val.v[1]);
  return _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 0, 1, 0));
}
//CAST_L4(_svec4_d, _svec4_d, double);

////casts bits, only for 32bit i32/u32 <--> float i64/u64<-->double


/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT(FROM, TO, func)        \
template <class T> static T svec_cast_bits(FROM val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast_bits<TO>(FROM val) {      \
    return TO(func(val.v)); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT64(FROM, TO, func)        \
template <class T> static T svec_cast_bits(FROM val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE TO svec_cast_bits<TO>(FROM val) {      \
    return TO(func(val.v[0]), func(val.v[1])); \
}


/**
 * @brief cast based on directly change the __vector type
 */
CAST_BITS_OPT(_svec4_i32, _svec4_f, _mm_castsi128_ps);
CAST_BITS_OPT(_svec4_u32, _svec4_f, _mm_castsi128_ps);
CAST_BITS_OPT(_svec4_f, _svec4_i32, _mm_castps_si128);
CAST_BITS_OPT(_svec4_f, _svec4_u32, _mm_castps_si128);

CAST_BITS_OPT64(_svec4_i64, _svec4_d, _mm_castsi128_pd);
CAST_BITS_OPT64(_svec4_u64, _svec4_d, _mm_castsi128_pd);
CAST_BITS_OPT64(_svec4_d, _svec4_i64, _mm_castpd_si128);
CAST_BITS_OPT64(_svec4_d, _svec4_u64, _mm_castpd_si128);


//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////

/**
 * @brief this macro uses sse specific intrinsics to do extract, insert
 */
#define SUBSCRIPT_FUNC_IMPL_SSE(VTYPE, STYPE) \
FORCEINLINE STYPE& VTYPE::operator[](int index) { \
  return ((STYPE *)&v)[index];   \
} \
const FORCEINLINE STYPE  VTYPE::operator[](int index) const { \
  return svec_extract(*this, index); \
}

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
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_i8, int8_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_u8, uint8_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_i16, int16_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_u16, uint16_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_i32, int32_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_u32, uint32_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_i64, int64_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_u64, uint64_t);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_f, float);
SUBSCRIPT_FUNC_IMPL_SSE(_svec4_d, double);

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

