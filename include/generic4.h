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

#include "gsimd_utility.h"

namespace generic {

#define LANES 4
//////////////////////////////////////////////////////////////
//
// Constructor Section
//
//////////////////////////////////////////////////////////////

template <>
struct svec<LANES,bool>;
template <>
  struct svec<LANES,int8_t>;
template <>
  struct svec<LANES,uint8_t>;
template <>
  struct svec<LANES,int16_t>;
template <>
  struct svec<LANES,uint16_t>;
template <>
  struct svec<LANES,int32_t>;
template <>
  struct svec<LANES,uint32_t>;
template <>
  struct svec<LANES,int64_t>;
template <>
  struct svec<LANES,uint64_t>;
template <>
  struct svec<LANES,float>;
template <>
  struct svec<LANES,double>;
template <>
  struct svec<LANES,void*>;

/**
 * @brief Data representation and operations on a vector of 4 boolean values.
 * This is used in predicated vector operations. Specifically the ith value of 
 * svec<4,bool> indicates whether the ith lane of a predicated vector operation is
 * enabled or not.
 * 
 * See also gather, scatter, load, store, and compare operations.
 */
template<>
struct svec<LANES,bool> {

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

    SUBSCRIPT_FUNC_BOOL_DECL(uint32_t);
    COUT_FUNC_BOOL_DECL();
    SVEC_BOOL_CLASS_METHOD_DECL();
};


/**
 * @brief data representation and operations on a vector of 4 signed chars.
 */
template <>
struct svec<LANES,int8_t> {
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
    SUBSCRIPT_FUNC_DECL(int8_t);
    COUT_FUNC_CHAR_DECL(int8_t);

    VEC_CLASS_METHOD_DECL(int8_t);
    VEC_INT_CLASS_METHOD_DECL(int8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned chars.
 */

template<>
struct svec<LANES,uint8_t> {
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
    SUBSCRIPT_FUNC_DECL(uint8_t);
    COUT_FUNC_CHAR_DECL(uint8_t);

    VEC_CLASS_METHOD_DECL(uint8_t);
    VEC_INT_CLASS_METHOD_DECL(uint8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed short.
 */
template <>
struct svec<LANES,int16_t> {
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
    SUBSCRIPT_FUNC_DECL(int16_t);
    COUT_FUNC_DECL(int16_t);

    VEC_CLASS_METHOD_DECL(int16_t);
    VEC_INT_CLASS_METHOD_DECL(int16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned short.
 */
template <>
struct svec<LANES,uint16_t> {
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
    SUBSCRIPT_FUNC_DECL(uint16_t);
    COUT_FUNC_DECL(uint16_t);

    VEC_CLASS_METHOD_DECL(uint16_t);
    VEC_INT_CLASS_METHOD_DECL(uint16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 signed int.
 */
template <>
struct svec<LANES,int32_t> {
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
    SUBSCRIPT_FUNC_DECL(int32_t);
    COUT_FUNC_DECL(int32_t);

    VEC_CLASS_METHOD_DECL(int32_t);
    VEC_INT_CLASS_METHOD_DECL(int32_t, uint32_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned int.
 */
template <>
struct svec<LANES,uint32_t> {
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
    SUBSCRIPT_FUNC_DECL(uint32_t);
    COUT_FUNC_DECL(uint32_t);

    VEC_CLASS_METHOD_DECL(uint32_t);
    VEC_INT_CLASS_METHOD_DECL(uint32_t, uint32_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed 64-bit int.
 */
template <>
struct svec<LANES,int64_t> {
    int64_t v[LANES];
    /**
     * @brief Default constructor,
     * @return a vector of 4 undefined signed 64-bit int.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed 64-bit int: {a,b,c,d}.
     */
    FORCEINLINE svec(int64_t a, int64_t b, int64_t c, int64_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a a signed 64-bit int
     * @return a vector of 4 signed 64-bit int: {a,a,a,a}.
     */
    FORCEINLINE svec( int64_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int64_t);
    COUT_FUNC_DECL(int64_t);

    VEC_CLASS_METHOD_DECL(int64_t);
    VEC_INT_CLASS_METHOD_DECL(int64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned 64-bit int.
 */
template <>
struct svec<LANES,uint64_t> {
    uint64_t v[LANES];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned 64-bit int.
     */
    FORCEINLINE svec() { }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned 64-bit int: {a,b,c,d}.
     */
    FORCEINLINE svec(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
      v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    /**
     * @brief Constructor.
     * @param a an unsigned 64-bit int.
     * @return a vector of 4 unsigned 64-bit int: {a,a,a,a}.
     */
    FORCEINLINE svec( uint64_t a) {
      v[0] = v[1] = v[2] = v[3] = a;
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint64_t);
    COUT_FUNC_DECL(uint64_t);

    VEC_CLASS_METHOD_DECL(uint64_t);
    VEC_INT_CLASS_METHOD_DECL(uint64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 float.
 */
template<>
struct svec<LANES,float> {
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
    SUBSCRIPT_FUNC_DECL(float);
    COUT_FUNC_DECL(float);

    VEC_CLASS_METHOD_DECL(float);
    VEC_FLOAT_CLASS_METHOD_DECL(float);
};

/**
 * @brief data representation and operations on a vector of 4 double.
 */
template<>
struct svec<LANES,double> {
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
//i1 use different approach
static FORCEINLINE uint32_t svec_extract(svec<LANES,bool> v, int index) {
  return (v.v & (1 << index)) ? -1 : 0;
}
static FORCEINLINE void svec_insert(svec<LANES,bool> *v, int index, uint32_t val) {
  if(!val) {
    v->v &= ~(1 << index);
  } else {
    v->v |= (1 << index);
  }
}
INSERT_EXTRACT(int8_t);
INSERT_EXTRACT(uint8_t);
INSERT_EXTRACT(int16_t);
INSERT_EXTRACT(uint16_t);
INSERT_EXTRACT(int32_t);
INSERT_EXTRACT(uint32_t);
INSERT_EXTRACT(int64_t);
INSERT_EXTRACT(uint64_t);
INSERT_EXTRACT(float);
INSERT_EXTRACT(double);

// 1. Load / Store
LOAD_STORE(bool);
LOAD_STORE(int8_t);
LOAD_STORE(uint8_t);
LOAD_STORE(int16_t);
LOAD_STORE(uint16_t);
LOAD_STORE(int32_t);
LOAD_STORE(uint32_t);
LOAD_STORE(int64_t);
LOAD_STORE(uint64_t);
LOAD_STORE(float);
LOAD_STORE(double);

// 3. Select
static FORCEINLINE svec<LANES,bool> svec_select(svec<LANES,bool> mask, svec<LANES,bool> a, svec<LANES,bool> b) {
  svec<LANES,bool> ret;
  ret.v = (a.v & mask.v) | (b.v & ~mask.v);
  return ret;
}
SELECT(int8_t);
SELECT(uint8_t);
SELECT(int16_t);
SELECT(uint16_t);
SELECT(int32_t);
SELECT(uint32_t);
SELECT(int64_t);
SELECT(uint64_t);
SELECT(float);
SELECT(double);

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
BROADCAST(int8_t);
BROADCAST(uint8_t);
BROADCAST(int16_t);
BROADCAST(uint16_t);
BROADCAST(int32_t);
BROADCAST(uint32_t);
BROADCAST(int64_t);
BROADCAST(uint64_t);
BROADCAST(float);
BROADCAST(double);

ROTATE(int8_t);
ROTATE(uint8_t);
ROTATE(int16_t);
ROTATE(uint16_t);
ROTATE(int32_t);
ROTATE(uint32_t);
ROTATE(int64_t);
ROTATE(uint64_t);
ROTATE(float);
ROTATE(double);

SHUFFLES(int8_t);
SHUFFLES(uint8_t);
SHUFFLES(int16_t);
SHUFFLES(uint16_t);
SHUFFLES(int32_t);
SHUFFLES(uint32_t);
SHUFFLES(int64_t);
SHUFFLES(uint64_t);
SHUFFLES(float);
SHUFFLES(double);

//load const
LOAD_CONST(int8_t);
LOAD_CONST(uint8_t);
LOAD_CONST(int16_t);
LOAD_CONST(uint16_t);
LOAD_CONST(int32_t);
LOAD_CONST(uint32_t);
LOAD_CONST(int64_t);
LOAD_CONST(uint64_t);
LOAD_CONST(float);
LOAD_CONST(double);


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
 * @note In 32bit platform, svec<4,void*>_ptr extends svec<4,uint32_t>, while in 64bit platform, svec<4,void*> extends svec<4,uint64_t>.
 * @see gather and scatter
 */
#if defined(__x86_64__) || defined(__PPC64__)
template<>
struct svec<LANES,void*> : public svec<LANES,uint64_t>{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p10, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
    svec<LANES,uint64_t>((uint64_t)(p0),(uint64_t)(p1),(uint64_t)(p2),(uint64_t)(p3)){}
};
#else // 32-bit
template<>
  struct svec<LANES,void*>: public svec<LANES,uint32_t>{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p0, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
    svec<LANES,uint32_t>((uint32_t)(p0),(uint32_t)(p1),(uint32_t)(p2),(uint32_t)(p3)){}
};
#endif // __PPC64__

#ifndef DOXYGEN_SHOULD_SKIP_THIS //not want generate svec_gather*/svec_scatter methods

template <class RetVecType> static RetVecType svec_gather(svec<LANES, uint32_t> ptrs, svec<LANES,bool> mask);
template <class RetVecType> static RetVecType svec_gather(svec<LANES, uint64_t> ptrs, svec<LANES,bool> mask);

GATHER_GENERAL(int8_t, uint32_t);
GATHER_GENERAL(int8_t, uint64_t);
GATHER_GENERAL(uint8_t, uint32_t);
GATHER_GENERAL(uint8_t, uint64_t);
GATHER_GENERAL(int16_t, uint32_t);
GATHER_GENERAL(int16_t, uint64_t);
GATHER_GENERAL(uint16_t, uint32_t);
GATHER_GENERAL(uint16_t, uint64_t);
GATHER_GENERAL(int32_t, uint32_t);
GATHER_GENERAL(int32_t, uint64_t);
GATHER_GENERAL(uint32_t, uint32_t);
GATHER_GENERAL(uint32_t, uint64_t);
GATHER_GENERAL(int64_t, uint32_t);
GATHER_GENERAL(int64_t, uint64_t);
GATHER_GENERAL(uint64_t, uint32_t);
GATHER_GENERAL(uint64_t, uint64_t);
GATHER_GENERAL(float, uint32_t);
GATHER_GENERAL(float, uint64_t);
GATHER_GENERAL(double, uint32_t);
GATHER_GENERAL(double, uint64_t);

GATHER_BASE_OFFSETS(int8_t, int32_t);
GATHER_BASE_OFFSETS(int8_t, int64_t);
GATHER_BASE_OFFSETS(uint8_t, int32_t);
GATHER_BASE_OFFSETS(uint8_t, int64_t);
GATHER_BASE_OFFSETS(int16_t, int32_t);
GATHER_BASE_OFFSETS(int16_t, int64_t);
GATHER_BASE_OFFSETS(uint16_t, int32_t);
GATHER_BASE_OFFSETS(uint16_t, int64_t);
GATHER_BASE_OFFSETS(int32_t, int32_t);
GATHER_BASE_OFFSETS(int32_t, int64_t);
GATHER_BASE_OFFSETS(uint32_t, int32_t);
GATHER_BASE_OFFSETS(uint32_t, int64_t);
GATHER_BASE_OFFSETS(int64_t, int32_t);
GATHER_BASE_OFFSETS(int64_t, int64_t);
GATHER_BASE_OFFSETS(uint64_t, int32_t);
GATHER_BASE_OFFSETS(uint64_t, int64_t);
GATHER_BASE_OFFSETS(float, int32_t);
GATHER_BASE_OFFSETS(float, int64_t);
GATHER_BASE_OFFSETS(double, int32_t);
GATHER_BASE_OFFSETS(double, int64_t);

GATHER_STRIDE(int8_t, int32_t);
GATHER_STRIDE(int8_t, int64_t);
GATHER_STRIDE(uint8_t, int32_t);
GATHER_STRIDE(uint8_t, int64_t);
GATHER_STRIDE(int16_t, int32_t);
GATHER_STRIDE(int16_t, int64_t);
GATHER_STRIDE(uint16_t, int32_t);
GATHER_STRIDE(uint16_t, int64_t);
GATHER_STRIDE(int32_t, int32_t);
GATHER_STRIDE(int32_t, int64_t);
GATHER_STRIDE(uint32_t, int32_t);
GATHER_STRIDE(uint32_t, int64_t);
GATHER_STRIDE(int64_t, int32_t);
GATHER_STRIDE(int64_t, int64_t);
GATHER_STRIDE(uint64_t, int32_t);
GATHER_STRIDE(uint64_t, int64_t);
GATHER_STRIDE(float, int32_t);
GATHER_STRIDE(float, int64_t);
GATHER_STRIDE(double, int32_t);
GATHER_STRIDE(double, int64_t);


SCATTER_GENERAL(int8_t, uint32_t);
SCATTER_GENERAL(int8_t, uint64_t);
SCATTER_GENERAL(uint8_t, uint32_t);
SCATTER_GENERAL(uint8_t, uint64_t);
SCATTER_GENERAL(int16_t, uint32_t);
SCATTER_GENERAL(int16_t, uint64_t);
SCATTER_GENERAL(uint16_t, uint32_t);
SCATTER_GENERAL(uint16_t, uint64_t);
SCATTER_GENERAL(int32_t, uint32_t);
SCATTER_GENERAL(int32_t, uint64_t);
SCATTER_GENERAL(uint32_t, uint32_t);
SCATTER_GENERAL(uint32_t, uint64_t);
SCATTER_GENERAL(int64_t, uint32_t);
SCATTER_GENERAL(int64_t, uint64_t);
SCATTER_GENERAL(uint64_t, uint32_t);
SCATTER_GENERAL(uint64_t, uint64_t);
SCATTER_GENERAL(float, uint32_t);
SCATTER_GENERAL(float, uint64_t);
SCATTER_GENERAL(double, uint32_t);
SCATTER_GENERAL(double, uint64_t);

SCATTER_BASE_OFFSETS(int8_t, int32_t);
SCATTER_BASE_OFFSETS(int8_t, int64_t);
SCATTER_BASE_OFFSETS(uint8_t, int32_t);
SCATTER_BASE_OFFSETS(uint8_t, int64_t);
SCATTER_BASE_OFFSETS(int16_t, int32_t);
SCATTER_BASE_OFFSETS(int16_t, int64_t);
SCATTER_BASE_OFFSETS(uint16_t, int32_t);
SCATTER_BASE_OFFSETS(uint16_t, int64_t);
SCATTER_BASE_OFFSETS(int32_t, int32_t);
SCATTER_BASE_OFFSETS(int32_t, int64_t);
SCATTER_BASE_OFFSETS(uint32_t, int32_t);
SCATTER_BASE_OFFSETS(uint32_t, int64_t);
SCATTER_BASE_OFFSETS(int64_t, int32_t);
SCATTER_BASE_OFFSETS(int64_t, int64_t);
SCATTER_BASE_OFFSETS(uint64_t, int32_t);
SCATTER_BASE_OFFSETS(uint64_t, int64_t);
SCATTER_BASE_OFFSETS(float, int32_t);
SCATTER_BASE_OFFSETS(float, int64_t);
SCATTER_BASE_OFFSETS(double, int32_t);
SCATTER_BASE_OFFSETS(double, int64_t);

SCATTER_STRIDE(int8_t, int32_t);
SCATTER_STRIDE(int8_t, int64_t);
SCATTER_STRIDE(uint8_t, int32_t);
SCATTER_STRIDE(uint8_t, int64_t);
SCATTER_STRIDE(int16_t, int32_t);
SCATTER_STRIDE(int16_t, int64_t);
SCATTER_STRIDE(uint16_t, int32_t);
SCATTER_STRIDE(uint16_t, int64_t);
SCATTER_STRIDE(int32_t, int32_t);
SCATTER_STRIDE(int32_t, int64_t);
SCATTER_STRIDE(uint32_t, int32_t);
SCATTER_STRIDE(uint32_t, int64_t);
SCATTER_STRIDE(int64_t, int32_t);
SCATTER_STRIDE(int64_t, int64_t);
SCATTER_STRIDE(uint64_t, int32_t);
SCATTER_STRIDE(uint64_t, int64_t);
SCATTER_STRIDE(float, int32_t);
SCATTER_STRIDE(float, int64_t);
SCATTER_STRIDE(double, int32_t);
SCATTER_STRIDE(double, int64_t);

#endif //DOXYGEN_SHOULD_SKIP_THIS


//  5. masked load/masked store

//Masked load/store is implemented based on gather_base_offsets/scatter_base_offsets
//Here we only use offsets with 32bit

MASKED_LOAD_STORE_L4(int8_t);
MASKED_LOAD_STORE_L4(uint8_t);
MASKED_LOAD_STORE_L4(int16_t);
MASKED_LOAD_STORE_L4(uint16_t);
MASKED_LOAD_STORE_L4(int32_t);
MASKED_LOAD_STORE_L4(uint32_t);
MASKED_LOAD_STORE_L4(int64_t);
MASKED_LOAD_STORE_L4(uint64_t);
MASKED_LOAD_STORE_L4(float);
MASKED_LOAD_STORE_L4(double);

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
static FORCEINLINE bool svec_any_true(const svec<LANES,bool>& mask) {
  return (mask.v != 0);
}

/**
 * @brief Check all elements of the mask are non-zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are true
 */
static FORCEINLINE bool svec_all_true(const svec<LANES,bool>& mask) {
  return (mask.v & 0xF) == 0xF;
}


/**
 * @brief Check none elements of the mask are zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are false
 */
static FORCEINLINE bool svec_none_true(const svec<LANES,bool>& mask) {
  return (mask.v == 0);
}

// 2. bit operations

/**
 * @brief return a & b
 */
static FORCEINLINE svec<LANES,bool> svec_and(svec<LANES,bool> a, svec<LANES,bool> b) {
  svec<LANES,bool> ret;
  ret.v = a.v & b.v;
  return ret;
}


/**
 * @brief return a | b
 */
static FORCEINLINE svec<LANES,bool> svec_or(svec<LANES,bool> a, svec<LANES,bool> b) {
  svec<LANES,bool> ret;
  ret.v = a.v | b.v;
  return ret;
}

/**
 * @brief return a ^ b
 */
static FORCEINLINE svec<LANES,bool> svec_xor(svec<LANES,bool> a, svec<LANES,bool> b) {
  svec<LANES,bool> ret;
  ret.v = a.v ^ b.v;
  return ret;
}

/**
 * @brief return ~a
 */
static FORCEINLINE svec<LANES,bool> svec_not(svec<LANES,bool> a) {
  svec<LANES,bool> ret;
  ret.v = ~a.v;
  return ret;
}

/**
 * @brief Change a mask type (i1 vector) to a uint64_t integer
 * The method is only used for compatibility of ISPC
 * @param mask the svec<4,bool> type vector
 * @return a uint64_t integer to represent the mask
 */
static FORCEINLINE uint64_t svec_movmsk(svec<LANES,bool> mask) {
  return (uint64_t)(mask.v);
}


//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////
// 1. Unary

// neg operation
UNARY_OP(int8_t, svec_neg, -);
UNARY_OP(uint8_t, svec_neg, -);
UNARY_OP(int16_t, svec_neg, -);
UNARY_OP(uint16_t, svec_neg, -);
UNARY_OP(int32_t, svec_neg, -);
UNARY_OP(uint32_t, svec_neg, -);
UNARY_OP(int64_t, svec_neg, -);
UNARY_OP(uint64_t, svec_neg, -);
UNARY_OP(float, svec_neg, -);
UNARY_OP(double, svec_neg, -);

//  2. Math unary
//round
UNARY_OP(float, svec_round, roundf);
UNARY_OP(double, svec_round, round);
//floor
UNARY_OP(float, svec_floor, floorf);
UNARY_OP(double, svec_floor, floor);
//ceil
UNARY_OP(float, svec_ceil, ceilf);
UNARY_OP(double, svec_ceil, ceil);
//reverse 1/
UNARY_OP(float, svec_rcp, 1.0/);
UNARY_OP(double, svec_rcp, 1.0/);
//reverse sqrt
UNARY_OP(float, svec_rsqrt, 1.0/sqrtf);
UNARY_OP(double, svec_rsqrt, 1.0/sqrt);
//sqrt
UNARY_OP(float, svec_sqrt, sqrtf);
UNARY_OP(double, svec_sqrt, sqrt);
//exp
UNARY_OP(float, svec_exp, expf);
UNARY_OP(double, svec_exp, exp);
//log
UNARY_OP(float, svec_log, logf);
UNARY_OP(double, svec_log, log);
//abs - for all types
UNARY_OP(int8_t, svec_abs, abs<int8_t>);
static FORCEINLINE svec<LANES,uint8_t>  svec_abs(svec<LANES,uint8_t> v) { return v;}
UNARY_OP(int16_t, svec_abs, abs<int16_t>);
static FORCEINLINE svec<LANES,uint16_t>  svec_abs(svec<LANES,uint16_t> v) { return v;}
UNARY_OP(int32_t, svec_abs, abs<int32_t>);
static FORCEINLINE svec<LANES,uint32_t>  svec_abs(svec<LANES,uint32_t> v) { return v;}
UNARY_OP(int64_t, svec_abs, abs<int64_t>);
static FORCEINLINE svec<LANES,uint64_t>  svec_abs(svec<LANES,uint64_t> v) { return v;}
UNARY_OP(float, svec_abs, abs);
UNARY_OP(double, svec_abs, abs);

//  3. Binary

//add, sub, div, mul.
#define BINARY_OP_METHODS(STYPE) \
BINARY_OP(STYPE, svec_add, +); \
BINARY_OP(STYPE, svec_sub, -); \
BINARY_OP(STYPE, svec_mul, *); \
BINARY_OP(STYPE, svec_div, /); \
BINARY_OP_SCALAR(STYPE, svec_add_scalar, +); \
BINARY_SCALAR_OP(STYPE, svec_scalar_add, +); \
BINARY_OP_SCALAR(STYPE, svec_sub_scalar, -); \
BINARY_SCALAR_OP(STYPE, svec_scalar_sub, -); \
BINARY_OP_SCALAR(STYPE, svec_mul_scalar, *); \
BINARY_SCALAR_OP(STYPE, svec_scalar_mul, *); \
BINARY_OP_SCALAR(STYPE, svec_div_scalar, /); \
BINARY_SCALAR_OP(STYPE, svec_scalar_div, /); \

#define INT_BINARY_OP_METHODS(STYPE) \
BINARY_OP(STYPE, svec_or, |); \
BINARY_OP(STYPE, svec_and, &); \
BINARY_OP(STYPE, svec_xor, ^); \
BINARY_SHT_SCALAR(STYPE, int32_t, svec_shl, <<); \
BINARY_SHT_SCALAR(STYPE, int32_t, svec_shr, >>); \
BINARY_OP(STYPE, svec_rem, %); \
BINARY_OP_SCALAR(STYPE, svec_rem, %);

BINARY_OP_METHODS(int8_t);
BINARY_OP_METHODS(uint8_t);
BINARY_OP_METHODS(int16_t);
BINARY_OP_METHODS(uint16_t);
BINARY_OP_METHODS(int32_t);
BINARY_OP_METHODS(uint32_t);
BINARY_OP_METHODS(int64_t);
BINARY_OP_METHODS(uint64_t);
BINARY_OP_METHODS(float);
BINARY_OP_METHODS(double);

INT_BINARY_OP_METHODS(int8_t);
INT_BINARY_OP_METHODS(uint8_t);
INT_BINARY_OP_METHODS(int16_t);
INT_BINARY_OP_METHODS(uint16_t);
INT_BINARY_OP_METHODS(int32_t);
INT_BINARY_OP_METHODS(uint32_t);
INT_BINARY_OP_METHODS(int64_t);
INT_BINARY_OP_METHODS(uint64_t);


//power only for float
BINARY_OP_FUNC(float, svec_pow, powf);
BINARY_OP_FUNC(double, svec_pow, pow);

//shift left
BINARY_OP2(int8_t, uint8_t, svec_shl, <<);
BINARY_OP2(uint8_t, uint8_t, svec_shl, <<);
BINARY_OP2(int16_t, uint16_t, svec_shl, <<);
BINARY_OP2(uint16_t, uint16_t, svec_shl, <<);
BINARY_OP2(int32_t, uint32_t, svec_shl, <<);
BINARY_OP2(uint32_t, uint32_t, svec_shl, <<);
BINARY_OP2(int64_t, uint64_t, svec_shl, <<);
BINARY_OP2(uint64_t, uint64_t, svec_shl, <<);

//shift right
BINARY_OP2(int8_t, uint8_t, svec_shr, >>);
BINARY_OP2(uint8_t, uint8_t, svec_shr, >>);
BINARY_OP2(int16_t, uint16_t, svec_shr, >>);
BINARY_OP2(uint16_t, uint16_t, svec_shr, >>);
BINARY_OP2(int32_t, uint32_t, svec_shr, >>);
BINARY_OP2(uint32_t, uint32_t, svec_shr, >>);
BINARY_OP2(int64_t, uint64_t, svec_shr, >>);
BINARY_OP2(uint64_t, uint64_t, svec_shr, >>);

//  4. Ternary

//madd / msub for only int32/u32/float/double
TERNERY(int32_t);
TERNERY(uint32_t);
TERNERY(int64_t);
TERNERY(uint64_t);
TERNERY(float);
TERNERY(double);


//  5. Max/Min & 6. Reduce
#define MAX_MIN_REDUCE_METHODS(STYPE) \
BINARY_OP_FUNC(STYPE, svec_max, max<STYPE>); \
BINARY_OP_FUNC(STYPE, svec_min, min<STYPE>); \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_add, add<STYPE>); \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_max, max<STYPE>); \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_min, min<STYPE>); \

MAX_MIN_REDUCE_METHODS(int8_t);
MAX_MIN_REDUCE_METHODS(uint8_t);
MAX_MIN_REDUCE_METHODS(int16_t);
MAX_MIN_REDUCE_METHODS(uint16_t);
MAX_MIN_REDUCE_METHODS(int32_t);
MAX_MIN_REDUCE_METHODS(uint32_t);
MAX_MIN_REDUCE_METHODS(int64_t);
MAX_MIN_REDUCE_METHODS(uint64_t);
MAX_MIN_REDUCE_METHODS(float);
MAX_MIN_REDUCE_METHODS(double);

FORCEINLINE svec<LANES,float> svec_preduce_add(svec<LANES,float> v0, svec<LANES,float> v1, svec<LANES,float> v2, svec<LANES,float> v3) {
  return svec<LANES,float>(
      svec_reduce_add(v0),
      svec_reduce_add(v1),
      svec_reduce_add(v2),
      svec_reduce_add(v3)
      );
}

FORCEINLINE svec<LANES,double> svec_preduce_add(svec<LANES,double> v0, svec<LANES,double> v1, svec<LANES,double> v2, svec<LANES,double> v3) {
  return svec<LANES,double>(
      svec_reduce_add(v0),
      svec_reduce_add(v1),
      svec_reduce_add(v2),
      svec_reduce_add(v3)
      );
}


//  7. Compare
CMP_ALL_OP(int8_t);
CMP_ALL_OP(uint8_t);
CMP_ALL_OP(int16_t);
CMP_ALL_OP(uint16_t);
CMP_ALL_OP(int32_t);
CMP_ALL_OP(uint32_t);
CMP_ALL_OP(int64_t);
CMP_ALL_OP(uint64_t);
CMP_ALL_OP(float);
CMP_ALL_OP(double);

/**
 * @brief element by element comparison of two svec<4,bool> type object
 * @param a
 * @param b
 * @return a svec<4,bool> object
 */
CMP_OP(bool, equal, ==);
CMP_OP(bool, not_equal, !=);

//  8. Cast

/**
 * Here we provide the full cast combinations.
 * Some may have fast impl
 */

//i1 -> all
//CAST(bool, uint32_t);
CAST(bool, int8_t);  //better way: packing
CAST(bool, uint8_t);  //better way: packing
CAST(bool, int16_t);  //better way: packing
CAST(bool, uint16_t); //better way: packing
CAST(bool, int32_t);
CAST(bool, uint32_t);
CAST(bool, int64_t); //better way: unpack, singed ext
CAST(bool, uint64_t);//better way: unpack, singed ext
CAST(bool, float); //si to fp call
CAST(bool, double);

//i8 -> all
CAST(int8_t, bool);
//CAST(int8_t, int8_t);
CAST(int8_t, uint8_t);
CAST(int8_t, int16_t); //better way, use vec_unpackh
CAST(int8_t, uint16_t); //better way, sext + zero mask and
CAST(int8_t, int32_t); //better way, use twice vec_unpack
CAST(int8_t, uint32_t); //better way, use unpack + zero mask
CAST(int8_t, int64_t);
CAST(int8_t, uint64_t);
CAST(int8_t, float);
CAST(int8_t, double);

//u8 -> all
CAST(uint8_t, bool);
CAST(uint8_t, int8_t);
//CAST(uint8_t, uint8_t);
CAST(uint8_t, int16_t); //better way, use unpack + zero mask
CAST(uint8_t, uint16_t); //better way use unpack + zero mask
CAST(uint8_t, int32_t);
CAST(uint8_t, uint32_t);
CAST(uint8_t, int64_t);
CAST(uint8_t, uint64_t);
CAST(uint8_t, float);
CAST(uint8_t, double);

//i16 -> all
CAST(int16_t, bool);
CAST(int16_t, int8_t); //could use pack
CAST(int16_t, uint8_t); //could use pack
//CAST(int16_t, int16_t);
CAST(int16_t, uint16_t);
CAST(int16_t, int32_t); //use unpack
CAST(int16_t, uint32_t); //use unpack and zeromaskout
CAST(int16_t, int64_t);
CAST(int16_t, uint64_t);
CAST(int16_t, float);
CAST(int16_t, double);

//u16 -> all
CAST(uint16_t, bool);
CAST(uint16_t, int8_t);
CAST(uint16_t, uint8_t);
CAST(uint16_t, int16_t);
//CAST(uint16_t, uint16_t);
CAST(uint16_t, int32_t); //use unpack +mask
CAST(uint16_t, uint32_t); //use unpack + mask
CAST(uint16_t, int64_t);
CAST(uint16_t, uint64_t);
CAST(uint16_t, float);
CAST(uint16_t, double);

//i32 -> all
CAST(int32_t, bool);
CAST(int32_t, int8_t);
CAST(int32_t, uint8_t);
CAST(int32_t, int16_t);
CAST(int32_t, uint16_t);
//CAST(int32_t, int32_t);
CAST(int32_t, uint32_t);
CAST(int32_t, int64_t); //use p8 unpack
CAST(int32_t, uint64_t); //use p8 unpack
CAST(int32_t, float); //use ctf
CAST(int32_t, double);

//u32 -> all
CAST(uint32_t, bool);
CAST(uint32_t, int8_t);
CAST(uint32_t, uint8_t);
CAST(uint32_t, int16_t);
CAST(uint32_t, uint16_t);
CAST(uint32_t, int32_t);
//CAST(uint32_t, uint32_t);
CAST(uint32_t, int64_t); //use p8 unpack
CAST(uint32_t, uint64_t); //use p8 unpack
CAST(uint32_t, float);
CAST(uint32_t, double);

//i64-> all
CAST(int64_t, bool);
CAST(int64_t, int8_t);
CAST(int64_t, uint8_t);
CAST(int64_t, int16_t);
CAST(int64_t, uint16_t);
CAST(int64_t, int32_t); //use p8 trunk
CAST(int64_t, uint32_t); //use p8 trunk
//CAST(int64_t, int64_t);
CAST(int64_t, uint64_t);
CAST(int64_t, float);
CAST(int64_t, double);

//u64 -> all
CAST(uint64_t, bool);
CAST(uint64_t, int8_t);
CAST(uint64_t, uint8_t);
CAST(uint64_t, int16_t);
CAST(uint64_t, uint16_t);
CAST(uint64_t, int32_t); //use p8 pack
CAST(uint64_t, uint32_t); //use p8 pack
CAST(uint64_t, int64_t);
//CAST(uint64_t, uint64_t);
CAST(uint64_t, float);
CAST(uint64_t, double);

//float -> all
CAST(float, bool);
CAST(float, int8_t); //use cts + pack+pack
CAST(float, uint8_t); //use ctu + pack + pack
CAST(float, int16_t); //use cts + pack
CAST(float, uint16_t); //use ctu + pack
CAST(float, int32_t);//use cts
CAST(float, uint32_t); //use ctu
CAST(float, int64_t);
CAST(float, uint64_t);
//CAST(float, float);
CAST(float, double);

//double -> all
CAST(double, bool);
CAST(double, int8_t);
CAST(double, uint8_t);
CAST(double, int16_t);
CAST(double, uint16_t);
CAST(double, int32_t);
CAST(double, uint32_t);
CAST(double, int64_t);
CAST(double, uint64_t);
CAST(double, float);
//CAST(double, double);

////casts bits, only for 32bit i32/u32 <--> float i64/u64<-->double


/**
 * @brief cast based on directly change the __vector type
 */
CAST_BITS(int32_t, i32, float, f);
CAST_BITS(uint32_t, u32, float, f);
CAST_BITS(float, f, int32_t, i32);
CAST_BITS(float, f, uint32_t, u32);

CAST_BITS(int64_t, i64, double, d);
CAST_BITS(uint64_t, u64, double, d);
CAST_BITS(double, d, int64_t, i64);
CAST_BITS(double, d, uint64_t, u64);


//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////

//add the impl of i1's
FORCEINLINE void svec<LANES,bool>::Helper::operator=(uint32_t value) {
  svec_insert(m_self, m_index, value);
}
FORCEINLINE void svec<LANES,bool>::Helper::operator=(svec<LANES,bool>::Helper helper) {
  svec_insert(m_self, m_index, helper.operator uint32_t());
}
FORCEINLINE svec<LANES,bool>::Helper::operator uint32_t() const {
  return svec_extract(*m_self, m_index);
}
const FORCEINLINE uint32_t svec<LANES,bool>::operator[](int index) const {
  return svec_extract(*this, index);
}
SUBSCRIPT_FUNC_IMPL(int8_t);
SUBSCRIPT_FUNC_IMPL(uint8_t);
SUBSCRIPT_FUNC_IMPL(int16_t);
SUBSCRIPT_FUNC_IMPL(uint16_t);
SUBSCRIPT_FUNC_IMPL(int32_t);
SUBSCRIPT_FUNC_IMPL(uint32_t);
SUBSCRIPT_FUNC_IMPL(int64_t);
SUBSCRIPT_FUNC_IMPL(uint64_t);
SUBSCRIPT_FUNC_IMPL(float);
SUBSCRIPT_FUNC_IMPL(double);

/**
 * @brief Check if any element in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if at least one element in the mask vector is true, otherwise false
 */
FORCEINLINE bool svec<LANES,bool>::any_true() { return svec_any_true(*this); }

/**
 * @brief Check if all the elements in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are true, otherwise false.
 */
FORCEINLINE bool svec<LANES,bool>::all_true() { return svec_all_true(*this); }

/**
 * @brief Check all the elements in the mask vector is false. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are false, otherwise false.
 */
FORCEINLINE bool svec<LANES,bool>::none_true() { return svec_none_true(*this); }

/**
 * @brief Element-wise bit-wise compliment operator. E.g., "~a"
 * @return the result of bit-wise compliment as a boolean vector. 
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator~() { return svec_not(*this); }

/**
 * @brief Element-wise bit-wise OR operator. E.g., "a | b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise OR as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator|(svec<LANES,bool> a) { return svec_or(*this, a); }
/**
 * @brief Element-wise bit-wise AND operator. E.g., "a & b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise AND as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator&(svec<LANES,bool> a) { return svec_and(*this, a); }
/**
 * @brief Element-wise bit-wise XOR operator. E.g., "a ^ b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise XOR as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator^(svec<LANES,bool> a) { return svec_xor(*this, a); }
/**
 * @brief Element-wise bit-wise not operator. E.g., "!a"
 * @return the result of bit-wise compliment as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator!() { return svec_not(*this); }

/**
 * @brief Element-wise boolean AND operator. E.g., "a && b"
 * @param[in] a a boolean vector
 * @return the result of boolean AND as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator&&(svec<LANES,bool> a) { return svec_and(*this, a); }
/**
 * @brief Element-wise boolean OR operator. E.g., "a || b"
 * @param[in] a a boolean vector
 * @return the result of boolean OR as a boolean vector.
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator||(svec<LANES,bool> a) { return svec_or(*this, a); }
/**
 * @brief Element-wise compare equal. E.g., "a == b"
 * @param[in] a a boolean vector
 * @return the result of compare-equal as a boolean vector
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator ==(svec<LANES,bool> a) {
    return svec_equal(*this, a);
}

/**
 * @brief Element-wise compare not equal, return a bool vector. E.g. "a != b"
 * @param[in] a a boolean vector
 * @return the result of compare-not-equal as a boolean vector
 */
FORCEINLINE svec<LANES,bool> svec<LANES,bool>::operator !=(svec<LANES,bool> a) {
    return svec_not_equal(*this, a);
}

VEC_CMP_IMPL(int8_t);
VEC_CMP_IMPL(uint8_t);
VEC_CMP_IMPL(int16_t);
VEC_CMP_IMPL(uint16_t);
VEC_CMP_IMPL(int32_t);
VEC_CMP_IMPL(uint32_t);
VEC_CMP_IMPL(int64_t);
VEC_CMP_IMPL(uint64_t);
VEC_CMP_IMPL(float);
VEC_CMP_IMPL(double);

MVEC_CLASS_METHOD_IMPL(bool);
VEC_CLASS_METHOD_IMPL(int8_t);
VEC_CLASS_METHOD_IMPL(uint8_t);
VEC_CLASS_METHOD_IMPL(int16_t);
VEC_CLASS_METHOD_IMPL(uint16_t);
VEC_CLASS_METHOD_IMPL(int32_t);
VEC_CLASS_METHOD_IMPL(uint32_t);
VEC_CLASS_METHOD_IMPL(int64_t);
VEC_CLASS_METHOD_IMPL(uint64_t);
VEC_CLASS_METHOD_IMPL(float);
VEC_CLASS_METHOD_IMPL(double);

VEC_INT_CLASS_METHOD_IMPL(int8_t, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(uint8_t, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(int16_t, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(uint16_t, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(int32_t, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(uint32_t, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(int64_t, uint64_t);
VEC_INT_CLASS_METHOD_IMPL(uint64_t, uint64_t);

VEC_FLOAT_CLASS_METHOD_IMPL(float);
VEC_FLOAT_CLASS_METHOD_IMPL(double);

#undef LANES
} //end of namespace generic
#endif /* POWER_VSX4_H_ */

