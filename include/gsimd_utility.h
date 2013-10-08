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
 * gsimd_utility.h
 *
 *  Created on: Jun 12, 2013
 *  @author: Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 *
 *  Contains the utility functions for intrinsics implementation.
 *  Should be included in all intrinsics header files
 */

#ifndef GSIMD_UTILITY_H_
#define GSIMD_UTILITY_H_

#include <stdint.h>
#include <iostream>
#define DUMP(v) std::cout << #v << ":" << (v) << std::endl
#if ((ULONG_MAX) == (UINT_MAX))
#define IS32BIT
#else
#define IS64BIT
#endif

//simple trick to generate a compiler error if invalid template
//arguments are used
template <int Lanes, class T>
struct invalid_template_arguments{
  typedef void type;
};

#if defined(ENABLE_STATS) || defined(ENABLE_STATS_AND_TRACE)
enum {
  STATS_MASKED_LOAD = 0,
  STATS_MASKED_STORE,
  STATS_EXTRACT,
  STATS_INSERT,
  STATS_UNARY_SLOW,
  STATS_BINARY_SLOW,
  STATS_CAST_SLOW,
  STATS_BINARY_FUNC_SLOW,
  STATS_COMPARE_SLOW,
  STATS_GATHER_SLOW,
  STATS_SCATTER_SLOW,
  STATS_SMEAR_SLOW,
  STATS_LOAD_SLOW,
  STATS_STORE_SLOW,
  STATS_OTHER_SLOW,
  LAST_STATS
};

static const char* gStatNames[LAST_STATS] = {
  "masked_load",
  "masked_store",
  "extract",
  "insert",
  "unary_slow",
  "binary_slow",
  "cast_slow",
  "binary_func_slow",
  "compare_slow",
  "gather_slow",
  "scatter_slow",
  "smear_slow",
  "load_slow",
  "store_slow",
  "other_slot"
};
static float gStats[LAST_STATS] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

extern "C" void print_stats()
{
  std::cout << "  DUMP INTRINSICS STATS" << std::endl;
  for (int i=0; i<LAST_STATS; i++) {
    if (gStats[i] > 0) {
      std::cout << "  - " << gStatNames[i] << ": " << gStats[i] << std::endl;
    }
  }
}

#define INC_STATS(stat,inc) gStats[stat] += inc;
#ifdef ENABLE_STATS_AND_TRACE
#define INC_STATS_NAME(stat,inc,opname) \
  std::cout << "slow impl of " << opname << " @ "\
  << __FILE__ << " Line: " << __LINE__ << std::endl; \
  gStats[stat] += inc
#else
#define INC_STATS_NAME(stat,inc,opname) gStats[stat] += inc
#endif // ENABLE_STATS_AND_TRACE
#else
#define INC_STATS_NAME(stat,inc,opname)
#define INC_STATS(stat,inc)
#endif

#define NOT_IMPLEMENTED(msg)                                \
  std::cout << "WARNING: operation " << msg << " is not implemented yet" << std::endl; \
  assert(false);

/*
 * @brief macros to define FORCEINLINE
 * FORCEINLINE is widely used in the interfaces
 */
#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#define PRE_ALIGN(x)  /*__declspec(align(x))*/
#define POST_ALIGN(x)
#define roundf(x) (floorf(x + .5f))
#define round(x) (floor(x + .5))
#else
#define FORCEINLINE inline __attribute__((always_inline))
#define PRE_ALIGN(x)
#define POST_ALIGN(x)  __attribute__ ((aligned(x)))
#endif


/**
 * LANES=1 short vector definition for SIMD Generic Interfaces
 */
typedef uint8_t svec1_u8;
typedef int8_t svec1_i8;
typedef uint16_t svec1_u16;
typedef int16_t svec1_i16;
typedef uint32_t svec1_u32;
typedef int32_t svec1_i32;
typedef uint64_t svec1_u64;
typedef int64_t svec1_i64;
typedef float svec1_f;
typedef double svec1_d;


/*
 * Register scalar type names and use iu_get_type_name to query the short type name
 */
#pragma once
template<typename T> const char *iu_get_type_name();

#define DEFINE_TYPE_NAME(type, name) \
  template<> FORCEINLINE const char *iu_get_type_name<type>(){return name;} \

//Scalar type
DEFINE_TYPE_NAME(int8_t, "i8");
DEFINE_TYPE_NAME(uint8_t, "u8");
DEFINE_TYPE_NAME(int16_t, "i16");
DEFINE_TYPE_NAME(uint16_t, "u16");
DEFINE_TYPE_NAME(int32_t, "i32");
DEFINE_TYPE_NAME(uint32_t, "u32");
DEFINE_TYPE_NAME(int64_t, "i64");
DEFINE_TYPE_NAME(uint64_t, "u64");
DEFINE_TYPE_NAME(float, "f");
DEFINE_TYPE_NAME(double, "d");

#pragma once
template<typename T> FORCEINLINE void stdout_scalar(std::ostream &out, T v) {
  out << v;
}

template<> FORCEINLINE void stdout_scalar<int8_t>(std::ostream &out, int8_t v) {
  out << int16_t(v);
}

template<> FORCEINLINE void stdout_scalar<uint8_t>(std::ostream &out, uint8_t v) {
  out << uint16_t(v);
}

#pragma once
template<int N> const bool check_lanes(int n);

template<> FORCEINLINE const bool check_lanes<2>(int n) { return n == 2; }
template<> FORCEINLINE const bool check_lanes<4>(int n) { return n == 4; }
template<> FORCEINLINE const bool check_lanes<8>(int n) { return n == 8; }
template<> FORCEINLINE const bool check_lanes<16>(int n) { return n == 16; }



///////////////////////////////////////////////////////////
// Below are macros for generic implementations
///////////////////////////////////////////////////////////

/**
 * @brief macros to define a intrinsic based subscript opertor
 */
#define SUBSCRIPT_FUNC_DECL(STYPE)    \
  FORCEINLINE STYPE& operator[](int index); \
  const FORCEINLINE STYPE operator[](int index) const;

#define SUBSCRIPT_FUNC_BOOL_DECL(STYPE)    \
/**
 * @brief A special helper class to support customized subscript[] operations
 */\
  struct Helper { \
    int m_index;   svec<LANES,bool> *m_self; \
    FORCEINLINE Helper(svec<LANES,bool> *p_vec, int index): m_self(p_vec), m_index(index) {} \
    FORCEINLINE void operator=(STYPE value); \
    FORCEINLINE void operator=(Helper helper); \
    FORCEINLINE operator STYPE() const;  \
  }; \
  FORCEINLINE Helper operator[](int index) { return Helper(this, index);} \
  const FORCEINLINE STYPE operator[](int index) const;

//Only for I1 output, use 0/1 as output
#define COUT_FUNC_BOOL_DECL()						\
  friend std::ostream& operator<< (std::ostream &out, const svec<LANES,bool> &v) { \
    out << "svec<" << LANES << ",bool> " << "[" << (v[0]?1:0);			\
    for(int i = 1; i < LANES ; i++) { out << ", " << (v[i]?1:0);}	\
    out << "]";								\
    return out;								\
  }									\

//i8 type need special one for output char as int number
#define COUT_FUNC_CHAR_DECL(STYPE)					\
  friend std::ostream& operator<< (std::ostream &out, const svec<LANES,STYPE> &v) { \
    out << "svec<" << LANES << "," << #STYPE <<"> [" << short(v[0]);	\
    for(int i = 1; i < LANES ; i++) { out << ", " << short(v[i]);}	\
    out << "]";								\
    return out;								\
  }									\

#define COUT_FUNC_DECL(STYPE)						\
  friend std::ostream& operator<< (std::ostream &out, const svec<LANES,STYPE> &v) { \
    out << "svec<" << LANES << "," << #STYPE <<"> [" << v[0];		\
    for(int i = 1; i < LANES ; i++) { out << ", " << v[i];}		\
    out << "]";								\
    return out;								\
  }									\


/*!
   @brief macros to define compare methods
   == and != are available for all the types.
 */
#define VEC_CMP_DECL(STYPE)     \
  FORCEINLINE svec<LANES,bool> operator==(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,bool> operator!=(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,bool> operator<(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,bool> operator<=(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,bool> operator>(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,bool> operator>=(svec<LANES,STYPE> a); \

/**
 * @brief macros for unary. note "-" means neg or complement
 */
#define VEC_UNARY_DECL(STYPE) \
  FORCEINLINE svec<LANES,STYPE> operator-(); \
  FORCEINLINE STYPE reduce_add(); \
  FORCEINLINE STYPE reduce_max(); \
  FORCEINLINE STYPE reduce_min();

/**
 * @brief macros for binary operations.
 */
#define VEC_BIN_DECL(STYPE)    \
  FORCEINLINE svec<LANES,STYPE> operator+(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,STYPE> operator+(STYPE s); \
  FORCEINLINE svec<LANES,STYPE> operator-(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,STYPE> operator-(STYPE s); \
  FORCEINLINE svec<LANES,STYPE> operator*(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,STYPE> operator*(STYPE s); \
  FORCEINLINE svec<LANES,STYPE> operator/(svec<LANES,STYPE> a); \
  FORCEINLINE svec<LANES,STYPE> operator/(STYPE s);


/**
 * @brief macros for svec<N,bool> class's class method
 */
#define SVEC_BOOL_CLASS_METHOD_DECL()					\
  FORCEINLINE svec<LANES,bool> operator==(svec<LANES,bool> a);		\
  FORCEINLINE svec<LANES,bool> operator!=(svec<LANES,bool> a);		\
  static FORCEINLINE svec<LANES,bool> load(svec<LANES,bool>* p);	\
  FORCEINLINE void store(svec<LANES,bool>* p);				\
  FORCEINLINE bool any_true();						\
  FORCEINLINE bool all_true();						\
  FORCEINLINE bool none_true();						\
  FORCEINLINE svec<LANES,bool> operator|(svec<LANES,bool>);		\
  FORCEINLINE svec<LANES,bool> operator&(svec<LANES,bool> a);		\
  FORCEINLINE svec<LANES,bool> operator^(svec<LANES,bool> a);		\
  FORCEINLINE svec<LANES,bool> operator~();				\
  FORCEINLINE svec<LANES,bool> operator!();				\
  FORCEINLINE svec<LANES,bool> operator&&(svec<LANES,bool> a);		\
  FORCEINLINE svec<LANES,bool> operator||(svec<LANES,bool> a);


/**
 * @brief macros for non-mask i8 - double types's method
 */
#define VEC_CLASS_METHOD_DECL(STYPE) \
  VEC_CMP_DECL(STYPE);\
  VEC_UNARY_DECL(STYPE);\
  VEC_BIN_DECL(STYPE);\
  static FORCEINLINE svec<LANES,STYPE> load(svec<LANES,STYPE>* p); \
  FORCEINLINE void store(svec<LANES,STYPE>* p); \
  static FORCEINLINE svec<LANES,STYPE> masked_load(svec<LANES,STYPE>* p, svec<LANES,bool> mask); \
  FORCEINLINE void masked_store(svec<LANES,STYPE>* p, svec<LANES,bool> mask); \
  static FORCEINLINE svec<LANES,STYPE> load_const(const STYPE* p); \
  static FORCEINLINE svec<LANES,STYPE> load_and_splat(STYPE* p); \
  static FORCEINLINE svec<LANES,STYPE> gather(svec<LANES,void*> ptrs, svec<LANES,bool> mask);\
  FORCEINLINE void scatter(svec<LANES,void*> ptrs, svec<LANES,bool> mask); \
  static FORCEINLINE svec<LANES,STYPE> gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int32_t> offsets, svec<LANES,bool> mask);\
  static FORCEINLINE svec<LANES,STYPE> gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int64_t> offsets, svec<LANES,bool> mask);\
  FORCEINLINE void scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int32_t> offsets, svec<LANES,bool> mask); \
  FORCEINLINE void scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int64_t> offsets, svec<LANES,bool> mask); \
  static FORCEINLINE svec<LANES,STYPE> gather_stride(STYPE* b, int32_t off, int32_t stride);\
  static FORCEINLINE svec<LANES,STYPE> gather_stride(STYPE* b, int64_t off, int64_t stride);\
  FORCEINLINE void scatter_stride(STYPE* b, int32_t off, int32_t stride); \
  FORCEINLINE void scatter_stride(STYPE* b, int64_t off, int64_t stride); \
  FORCEINLINE svec<LANES,STYPE> broadcast(int32_t index); \
  FORCEINLINE svec<LANES,STYPE> rotate(int32_t index); \
  FORCEINLINE svec<LANES,STYPE> shuffle(svec<LANES, int32_t> index); \
  FORCEINLINE svec<LANES,STYPE> abs();

/**
 * @brief macros method definition for integer vector only
 * Note: shift's operator can only be unsigned vector
 */
#define VEC_INT_CLASS_METHOD_DECL(STYPE, USTYPE) \
  FORCEINLINE svec<LANES, STYPE> operator|(svec<LANES, STYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator&(svec<LANES, STYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator^(svec<LANES, STYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator<<(svec<LANES, USTYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator<<(int32_t s); \
  FORCEINLINE svec<LANES, STYPE> operator>>(svec<LANES, USTYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator>>(int32_t s); \
  FORCEINLINE svec<LANES, STYPE> operator%(svec<LANES, STYPE> a); \
  FORCEINLINE svec<LANES, STYPE> operator%(STYPE s);

/**
 * brief macros for float/double math unary operations
 */
#define VEC_FLOAT_CLASS_METHOD_DECL(STYPE) \
  FORCEINLINE svec<LANES,STYPE> round(); \
  FORCEINLINE svec<LANES,STYPE> floor(); \
  FORCEINLINE svec<LANES,STYPE> ceil(); \
  FORCEINLINE svec<LANES,STYPE> sqrt(); \
  FORCEINLINE svec<LANES,STYPE> rcp(); \
  FORCEINLINE svec<LANES,STYPE> rsqrt(); \
  FORCEINLINE svec<LANES,STYPE> exp(); \
  FORCEINLINE svec<LANES,STYPE> log(); \
  FORCEINLINE svec<LANES,STYPE> pow(svec<LANES,STYPE> a);

// 0. Extract/Insert
/**
 * @brief macros for svec's insert extract method implementation
 * The implementation is based on vector type's subscript operator
 */
#define INSERT_EXTRACT(STYPE)                               \
  static FORCEINLINE STYPE svec_extract(svec<LANES,STYPE> v, int index) {    \
    return v.v[index];                      \
  }                                          \
  static FORCEINLINE void svec_insert(svec<LANES,STYPE> *v, int index, STYPE val) { \
    v->v[index] = val;                      \
  }

// 1. Load / Store

#define LOAD_STORE(STYPE)                       \
/*!
   @brief construct a new vector from values loaded from a pointer
   @param[in] p load address
   \note p does not have to be aligned
   @return a new vector loaded from p
*/                            \
static FORCEINLINE svec<LANES,STYPE> svec_load(const svec<LANES,STYPE> *p) {      \
    STYPE *ptr = (STYPE *)p;                           \
    svec<LANES,STYPE> ret;                                         \
    INC_STATS_NAME(STATS_LOAD_SLOW, 1, "load:svec_"#STYPE);             \
    for (int i = 0; i < LANES; ++i) {ret[i] = ptr[i];}  \
    return ret;                                        \
}                                                      \
/*!
   @brief store a vector to an address
   @param[in] p store address
   \note p does not have to be aligned
   @param[in] v vector to be stored
*/                            \
static FORCEINLINE void svec_store(svec<LANES,STYPE> *p, svec<LANES,STYPE> v) {   \
  STYPE *ptr = (STYPE *)p;                 \
  INC_STATS_NAME(STATS_STORE_SLOW, 1, "store:svec_"#STYPE);              \
  for (int i = 0; i < LANES; ++i) {  ptr[i] = v[i]; } \
}

/**
 * @brief macros for svec's select by mask vector method generic implementation
 */
#define SELECT(STYPE)                               \
static FORCEINLINE svec<LANES,STYPE> svec_select(svec<LANES,bool> mask, svec<LANES,STYPE> a, svec<LANES,STYPE> b) {  \
    svec<LANES,STYPE> ret;                                                \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select:svec_"#STYPE);      \
    for (int i = 0; i < LANES; ++i) {ret[i] = mask[i] ? a[i] : b[i];}  \
    return ret;                                                     \
}

/**
 * @brief macros for svec's select by bool scalar method implementation
 */
#define SELECT_BOOLCOND(STYPE)                       \
/**
 * @brief Select two TYPE vectors by a bool scalar. The same as cond ? a: b
 */\
FORCEINLINE svec<LANES,STYPE> svec_select(bool cond, svec<LANES,STYPE> a, svec<LANES,STYPE> b) {       \
    return cond ? a : b;                                            \
}

// 4. broadcast/rotate/shuffle/smear/setzero
/**
 * @brief macro for broadcast method implementation
 * All broadcast are slow implementation
 */
#define BROADCAST(STYPE)                   \
  static FORCEINLINE svec<LANES,STYPE> svec_broadcast(svec<LANES,STYPE> v, int index) { \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "broadcast");           \
    STYPE bval = v[index];                \
    svec<LANES,STYPE> ret; \
    for (int i = 0; i < LANES; ++i) { ret[i] = bval;}  \
    return ret;                             \
  }

/**
 * @brief macro for broadcast method implementation for lanes4
 * All broadcast are slow implementation
 */
#define BROADCAST_L4(STYPE)                   \
  static FORCEINLINE svec<LANES,STYPE> svec_broadcast(svec<LANES,STYPE> v, int index) { \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "broadcast");           \
    STYPE bval = v[index];                \
    svec<LANES,STYPE> ret(bval,bval,bval,bval);                 \
    return ret;                             \
  }

/**
 * @brief macro for rotate method implementation
 */
#define ROTATE(STYPE)                  \
  static FORCEINLINE svec<LANES,STYPE> svec_rotate(svec<LANES,STYPE> v, int index) {    \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "rotate");          \
    svec<LANES,STYPE> ret; \
    for (int i = 0; i < LANES; ++i) { ret[i] = v[(i+index) & (LANES-1)];} \
    return ret;                             \
  }

/**
 * @brief macro for rotate method implementation
 */
#define ROTATE_L4(STYPE)                  \
  static FORCEINLINE svec<LANES,STYPE> svec_rotate(svec<LANES,STYPE> v, int index) {    \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "rotate");          \
    svec<LANES,STYPE> ret (v[(0+index) & 0x3],            \
               v[(1+index) & 0x3],            \
               v[(2+index) & 0x3],            \
               v[(3+index) & 0x3]);           \
    return ret;                             \
  }


/**
 * @brief macro for shuffle/shuffle2 methods implementation
 */
#define SHUFFLES(STYPE)                 \
  static FORCEINLINE svec<LANES,STYPE> svec_shuffle(svec<LANES,STYPE> v, svec<LANES,int32_t> index) {   \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "shuffle");           \
    svec<LANES,STYPE> ret; \
    for (int i = 0; i < LANES; ++i) { ret[i] = v[index[i] & (LANES-1)]; }\
    return ret;                               \
  }                                   \
  static FORCEINLINE svec<LANES,STYPE> svec_shuffle2(svec<LANES,STYPE> v0, svec<LANES,STYPE> v1, svec<LANES,int32_t> index) { \
    svec<LANES,STYPE> ret;                              \
    NOT_IMPLEMENTED("shuffle 2");                   \
    return ret;                             \
}

/**
 * @brief macro for shuffle/shuffle2 methods implementation
 */
#define SHUFFLES_L4(STYPE)                 \
  static FORCEINLINE svec<LANES,STYPE> svec_shuffle(svec<LANES,STYPE> v, svec<LANES,int32_t> index) {   \
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "shuffle");           \
    svec<LANES,STYPE> ret (v[index[0] & 0x3],    \
               v[index[1] & 0x3],    \
               v[index[2] & 0x3],    \
               v[index[3] & 0x3] );  \
    return ret;                               \
  }                                   \
  static FORCEINLINE svec<LANES,STYPE> svec_shuffle2(svec<LANES,STYPE> v0, svec<LANES,STYPE> v1, svec<LANES,int32_t> index) { \
    svec<LANES,STYPE> ret;                              \
    NOT_IMPLEMENTED("shuffle 2");                   \
    return ret;                             \
}

/**
 * @brief macro for setzero method implementation
 */
#define ZERO(STYPE, NAME)                   \
  static FORCEINLINE svec<LANES,STYPE> svec_zero(svec<LANES,STYPE>) {  \
    svec<LANES,STYPE> ret(0,0,0,0);                        \
    return ret;                                \
  }

//LOAD_CONST
#define LOAD_CONST(STYPE) \
template <class RetVecType> static RetVecType svec_load_const(const STYPE* p); \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_load_const<svec<LANES,STYPE> >(const STYPE* p) { \
  svec<LANES,STYPE> ret; \
    INC_STATS_NAME(STATS_LOAD_SLOW, 1, "load const");           \
    for (int i = 0; i < LANES; ++i) { ret[i] = *p; }\
    return ret; \
} \
template <class RetVecType> static RetVecType svec_load_and_splat(STYPE* p); \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_load_and_splat<svec<LANES,STYPE> >(STYPE* p) { \
  svec<LANES,STYPE> ret; \
  INC_STATS_NAME(STATS_LOAD_SLOW, 1, "load const");           \
  for (int i = 0; i < LANES; ++i) { ret[i] = *p; }\
  return ret; \
}
//Missing gather scatter's generial impl

/**
 * @brief utilities for fast gather general impl
 */
template<typename RetVec, typename RetScalar, typename PTRS, typename MSK>
static FORCEINLINE RetVec
lGatherGeneral(PTRS ptrs, MSK mask) {
  RetScalar r[4];
  if(svec_extract(mask,0)) { r[0] = *((RetScalar*)svec_extract(ptrs, 0));}
  if(svec_extract(mask,1)) { r[1] = *((RetScalar*)svec_extract(ptrs, 1));}
  if(svec_extract(mask,2)) { r[2] = *((RetScalar*)svec_extract(ptrs, 2));}
  if(svec_extract(mask,3)) { r[3] = *((RetScalar*)svec_extract(ptrs, 3));}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return RetVec(r[0],r[1],r[2],r[3]);
}

/**
 * @brief slow implementation of gather general
 * Must use template to specify the return type
 * @param mask
 * @return
 */
#define GATHER_GENERAL(STYPE, PSTYPE) \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_gather<svec<LANES,STYPE> >(svec<LANES,PSTYPE> ptrs, svec<LANES,bool> mask) {   \
  svec<LANES,STYPE> ret;\
  for(int i = 0; i < LANES; ++i) {if(mask[i]){ret[i] = *(STYPE*)(ptrs[i]); } }\
  INC_STATS_NAME(STATS_GATHER_SLOW, 1, "Gather genera"); \
  return ret; \
}

/**
 * @brief slow implementation of gather general
 * Must use template to specify the return type
 * @param mask
 * @return
 */
#define GATHER_GENERAL_L4(STYPE, PSTYPE)         \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_gather<svec<LANES,STYPE> >(svec<LANES,PSTYPE> ptrs, svec<LANES,bool> mask) {   \
  return lGatherGeneral<svec<LANES,STYPE>, STYPE, svec<LANES,PSTYPE>, svec<LANES,bool> >(ptrs, mask);                                \
}

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
  OFF vzero(0);

  offsets = svec_select(mask.v, offsets, vzero);
  r[0] = *(RetScalar *)(p + scale * svec_extract(offsets, 0));
  r[1] = *(RetScalar *)(p + scale * svec_extract(offsets, 1));
  r[2] = *(RetScalar *)(p + scale * svec_extract(offsets, 2));
  r[3] = *(RetScalar *)(p + scale * svec_extract(offsets, 3));
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather offset with select");
  return RetVec(r[0], r[1], r[2], r[3]);
}

#define GATHER_BASE_OFFSETS(STYPE, OSTYPE)         \
FORCEINLINE svec<LANES,STYPE> svec_gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,OSTYPE> offsets, svec<LANES,bool> mask) {   \
  svec<LANES,STYPE> ret;\
  for(int i = 0; i < LANES; ++i) {if(mask[i]){ret[i] = *(STYPE*)((uint8_t*)b + scale * offsets[i]);} }\
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather offset with select"); \
  return ret; \
}

/**
 * @ macros for generic impl of gather base offsets
 */
#define GATHER_BASE_OFFSETS_L4(STYPE, OSTYPE)         \
FORCEINLINE svec<LANES,STYPE> svec_gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,OSTYPE> offsets, svec<LANES,bool> mask) {   \
  return lGatherBaseOffsets<svec<LANES,STYPE>, STYPE, svec<LANES,OSTYPE>, svec<LANES,bool> >((uint8_t*)b, scale, offsets, mask);                                \
}

/**
 * @brief macros for general impl of gather base step
 */
#define GATHER_STRIDE(STYPE, OSTYPE)         \
template <class RetVecType> static RetVecType svec_gather_stride(STYPE* b, OSTYPE o, OSTYPE s); \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_gather_stride<svec<LANES,STYPE> >(STYPE* b, OSTYPE o, OSTYPE s) {   \
  svec<LANES,STYPE> ret; \
  b += o; \
  for(int i = 0; i < LANES; ++i, b+=s) { \
    ret[i] = *b; \
  } \
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather Steps"); \
  return ret; \
}

/**
 * @brief macros for fast impl of gather base step
 */
#define GATHER_STRIDE_L4(STYPE, OSTYPE)         \
template <class RetVecType> static RetVecType svec_gather_stride(STYPE* b, OSTYPE o, OSTYPE s); \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_gather_stride<svec<LANES,STYPE> >(STYPE* b, OSTYPE o, OSTYPE s) {   \
  int64_t off = (int64_t)o; int64_t stride = (int64_t)s;\
  OSTYPE stride2 = stride * 2; \
  STYPE v0 = *(b + off); \
  STYPE v1 = *(b + off + stride); \
  STYPE v2 = *(b + off + stride2); \
  STYPE v3 = *(b + off + stride2 + stride); \
  return svec<LANES,STYPE>(v0, v1, v2, v3); \
}

//#define GATHER_STRIDE_L4(STYPE, OSTYPE)         \
//FORCEINLINE svec<LANES,STYPE> svec_gather_stride(STYPE* b, OSTYPE o, OSTYPE s) {   \
//  int64_t off = (int64_t)o; int64_t stride = (int64_t)s;\
//  b += off; STYPE v0 = *b; \
//  b += stride; STYPE v1 = *b; \
//  b += stride; STYPE v2 = *b; \
//  b += stride; STYPE v3 = *b;  \
//  return svec<LANES,STYPE>(v0, v1, v2, v3); \
//}

#define SCATTER_STRIDE(STYPE, OSTYPE)   \
FORCEINLINE void svec_scatter_stride(STYPE* b, OSTYPE o, OSTYPE s, svec<LANES,STYPE> val) {   \
  b += o; \
  for(int i = 0; i < LANES; ++i, b+=s) { \
    *b = svec_extract(val, i); \
  }\
  INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter stride general svec<LANES,"#STYPE">");          \
}


#define SCATTER_STRIDE_L4(STYPE, OSTYPE)         \
FORCEINLINE void svec_scatter_stride(STYPE* b, OSTYPE o, OSTYPE s, svec<LANES,STYPE> val) {   \
  int64_t off = (int64_t)o; int64_t stride = (int64_t)s;\
  OSTYPE stride2 = stride * 2; \
  *(b + off) = svec_extract(val, 0); \
  *(b + off + stride) = svec_extract(val, 1); \
  *(b + off + stride2) = svec_extract(val, 2); \
  *(b + off + stride2 + stride) = svec_extract(val, 3); \
}

//#define SCATTER_STRIDE_L4(STYPE, OSTYPE)         \
//FORCEINLINE void svec_scatter_stride(STYPE* b, OSTYPE o, OSTYPE s, svec<LANES,STYPE> val) {   \
//  int64_t off = (int64_t)o; int64_t stride = (int64_t)s;\
//  b += off; *b = svec_extract(val, 0);\
//  b += stride; *b = svec_extract(val, 1); \
//  b += stride; *b = svec_extract(val, 2); \
//  b += stride; *b = svec_extract(val, 3); \
//}



#define SCATTER_GENERAL(STYPE, PSTYPE)            \
static FORCEINLINE void svec_scatter(svec<LANES,PSTYPE> ptrs, svec<LANES,STYPE> val, svec<LANES,bool> mask) { \
    for(int i = 0; i < LANES; ++i) { if(mask[i]){ *((STYPE*)ptrs[i]) = val[i];} } \
    INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter general svec<LANES,"#STYPE">");          \
}


/**
 * @brief Utility functions for scatter general fast impl
 */
template<typename STYPE, typename PTRTYPE, typename VTYPE, typename MTYPE>
static FORCEINLINE void lScatterGeneral(PTRTYPE ptrs,
                        VTYPE val, MTYPE mask) {
  if(svec_extract(mask,0)) { *((STYPE*)svec_extract(ptrs, 0)) = val[0]; }
  if(svec_extract(mask,1)) { *((STYPE*)svec_extract(ptrs, 1)) = val[1]; }
  if(svec_extract(mask,2)) { *((STYPE*)svec_extract(ptrs, 2)) = val[2]; }
  if(svec_extract(mask,3)) { *((STYPE*)svec_extract(ptrs, 3)) = val[3]; }
  INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter general");
}

#define SCATTER_GENERAL_L4(STYPE, PSTYPE)            \
static FORCEINLINE void svec_scatter(svec<LANES,PSTYPE> ptrs, svec<LANES,STYPE> val, svec<LANES,bool> mask) { \
    lScatterGeneral<STYPE, svec<LANES,PSTYPE>, svec<LANES,STYPE>, svec<LANES,bool> >(ptrs, val, mask); \
}


/**
 * @ macros for generic impl of scatter base offsets
 */
#define SCATTER_BASE_OFFSETS(STYPE, OSTYPE)         \
FORCEINLINE void svec_scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,OSTYPE> offsets, svec<LANES,STYPE> val, svec<LANES,bool> mask) {   \
  for(int i=0;i<LANES;++i){if(mask[i]){*(STYPE*)((uint8_t*)b + scale * offsets[i]) = val[i];}}\
  INC_STATS_NAME(STATS_SCATTER_SLOW,1,"scatter offset svec<LANES,"#STYPE">");          \
}

/**
 * @brief Utility functions for scatter base offsets fast impl
 */
template<typename STYPE, typename OTYPE, typename VTYPE, typename MTYPE>
static FORCEINLINE void lScatterBaseOffsets(unsigned char *b,
                        uint32_t scale, OTYPE offsets,
                        VTYPE val, MTYPE mask) {
  unsigned char *base = b;
  if(svec_extract(mask,0)) { *(STYPE*)(b + scale * svec_extract(offsets, 0)) = val[0]; }
  if(svec_extract(mask,1)) { *(STYPE*)(b + scale * svec_extract(offsets, 1)) = val[1]; }
  if(svec_extract(mask,2)) { *(STYPE*)(b + scale * svec_extract(offsets, 2)) = val[2]; }
  if(svec_extract(mask,3)) { *(STYPE*)(b + scale * svec_extract(offsets, 3)) = val[3]; }
  INC_STATS_NAME(STATS_SCATTER_SLOW,1, "scatter offset");
}

/**
 * @ macros for generic impl of scatter base offsets
 */
#define SCATTER_BASE_OFFSETS_L4(STYPE, OSTYPE)         \
FORCEINLINE void svec_scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,OSTYPE> offsets, svec<LANES,STYPE> val, svec<LANES,bool> mask) {   \
  lScatterBaseOffsets<STYPE, svec<LANES,OSTYPE>, svec<LANES,STYPE>, svec<LANES,bool> >((uint8_t*)b, scale, offsets, val, mask); \
}




#define MASKED_LOAD_STORE_L4(STYPE)                       \
static FORCEINLINE svec<LANES,STYPE> svec_masked_load(svec<LANES,STYPE> *p, svec<LANES,bool> mask) { \
    return svec_gather_base_offsets((STYPE*)p, sizeof(STYPE), svec<LANES,int32_t>(0,1,2,3), mask);  \
}                                                      \
static FORCEINLINE void svec_masked_store(svec<LANES,STYPE> *p, svec<LANES,STYPE> v, svec<LANES,bool> mask) { \
    svec_scatter_base_offsets((STYPE*)p, sizeof(STYPE), svec<LANES,int32_t>(0,1,2,3), v, mask); \
}

#define MASKED_LOAD_STORE_L8(STYPE)                       \
static FORCEINLINE svec<LANES,STYPE> svec_masked_load(svec<LANES,STYPE> *p, svec<LANES,bool> mask) { \
    return svec_gather_base_offsets((STYPE*)p, sizeof(STYPE), svec<LANES,int32_t>(0,1,2,3,4,5,6,7), mask);  \
}                                                      \
static FORCEINLINE void svec_masked_store(svec<LANES,STYPE> *p, svec<LANES,STYPE> v, svec<LANES,bool> mask) { \
    svec_scatter_base_offsets((STYPE*)p, sizeof(STYPE), svec<LANES,int32_t>(0,1,2,3,4,5,6,7), v, mask); \
}



//////////////////////////////////////////////////////////////
//
// Mask type (i1) interfaces
//
//////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////

template<class T> static FORCEINLINE T abs(T a) {
  return a >= 0 ? a : -a;
}

#define UNARY_OP(STYPE, NAME, OP)            \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> v) {      \
  INC_STATS_NAME(STATS_UNARY_SLOW, 1, #OP);           \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = OP(v[i]); } \
  return ret; \
}

#define UNARY_OP_L4(STYPE, NAME, OP)            \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> v) {      \
  INC_STATS_NAME(STATS_UNARY_SLOW, 1, #OP);           \
  return svec<LANES,STYPE>(OP(svec_extract(v, 0)),\
              OP(svec_extract(v, 1)),\
              OP(svec_extract(v, 2)),\
              OP(svec_extract(v, 3)));\
}

/**
 * @brief macros for generic slow impl of binary operation
 */
#define BINARY_OP(STYPE, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = a[i] OP b[i]; } \
  return ret;                            \
}

#define BINARY_OP2(STYPE, STYPE2, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE2> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = a[i] OP b[i]; } \
  return ret;                            \
}

#define BINARY_OP_FUNC(STYPE, NAME, FUNC)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = FUNC(a[i], b[i]); } \
  return ret;                            \
}

/**
 * @brief macros for generic slow imple of binary operation
 */
#define BINARY_OP_L4(STYPE, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret(svec_extract(a, 0) OP svec_extract(b, 0),\
           svec_extract(a, 1) OP svec_extract(b, 1),\
           svec_extract(a, 2) OP svec_extract(b, 2),\
           svec_extract(a, 3) OP svec_extract(b, 3));\
  return ret;                            \
}

/**
 * @brief macros for generic slow imple of binary operation, style 2
 */
#define BINARY_OP2_L4(STYPE, STYPE2, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE2> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret(svec_extract(a, 0) OP svec_extract(b, 0),\
           svec_extract(a, 1) OP svec_extract(b, 1),\
           svec_extract(a, 2) OP svec_extract(b, 2),\
           svec_extract(a, 3) OP svec_extract(b, 3));\
  return ret;                            \
}


#define BINARY_OP_FUNC_L4(STYPE, NAME, FUNC)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret(FUNC(svec_extract(a, 0), svec_extract(b, 0)),\
           FUNC(svec_extract(a, 1), svec_extract(b, 1)),\
           FUNC(svec_extract(a, 2), svec_extract(b, 2)),\
           FUNC(svec_extract(a, 3), svec_extract(b, 3))); \
  return ret;                            \
}

/**
 * @brief macros for binary: vector op scalar
 */
#define BINARY_OP_SCALAR_L4(STYPE, STYPE2, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, STYPE2 s) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret(svec_extract(a, 0) OP s,\
            svec_extract(a, 1) OP s,\
            svec_extract(a, 2) OP s,\
            svec_extract(a, 3) OP s);\
  return ret;                            \
}

/**
 * @brief macros for binary: vector op scalar
 */
#define BINARY_OP_SCALAR(STYPE, NAME, OP)            \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, STYPE s) {           \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);              \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = a[i] OP s; } \
  return ret;                            \
}
/**
 * @brief macros for binary shift: vector op intvalue
 */
#define BINARY_SHT_SCALAR(STYPE, SHTTYPE, NAME, OP)            \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, SHTTYPE s) {           \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);              \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = a[i] OP s; } \
  return ret;                            \
}
/**
 * @brief macros for binary: scalar op vector
 */
#define BINARY_SCALAR_OP(STYPE, NAME, OP)                \
static FORCEINLINE svec<LANES,STYPE> NAME(STYPE s, svec<LANES,STYPE> a) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,STYPE> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = s OP a[i]; }\
  return ret;                            \
}

#define TERNERY(STYPE) \
/**
 * @brief vector multiply and add operation. return a * b + c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_madd(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> res; \
  for(int i = 0; i < LANES; ++i) { res[i] = a[i]*b[i]+c[i]; } \
  return res; \
} \
/**
 * @brief vector multiply and add operation. return a * b - c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_msub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> res; \
  for(int i = 0; i < LANES; ++i) { res[i] = a[i]*b[i]-c[i]; } \
  return res; \
} \
/**
 * @brief vector multiply and add operation. return -( a * b - c ).
 */ \
FORCEINLINE svec<LANES,STYPE> svec_nmsub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> res; \
  for(int i = 0; i < LANES; ++i) { res[i] = -(a[i]*b[i]-c[i]); } \
  return res; \
}

#define TERNERY_L4(STYPE) \
/**
 * @brief vector multiply and add operation. return a * b + c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_madd(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> ret(svec_extract(a, 0) * svec_extract(b, 0) + svec_extract(c, 0),\
            svec_extract(a, 1) * svec_extract(b, 1) + svec_extract(c, 1),\
            svec_extract(a, 2) * svec_extract(b, 2) + svec_extract(c, 2),\
            svec_extract(a, 3) * svec_extract(b, 3) + svec_extract(c, 3));\
  return ret;                            \
} \
/**
 * @brief vector multiply and add operation. return a * b - c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_msub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> ret(svec_extract(a, 0) * svec_extract(b, 0) - svec_extract(c, 0),\
            svec_extract(a, 1) * svec_extract(b, 1) - svec_extract(c, 1),\
            svec_extract(a, 2) * svec_extract(b, 2) - svec_extract(c, 2),\
            svec_extract(a, 3) * svec_extract(b, 3) - svec_extract(c, 3));\
  return ret;                            \
} \
/**
 * @brief vector multiply and add operation. return - ( a * b - c).
 */ \
FORCEINLINE svec<LANES,STYPE> svec_nmsub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  svec<LANES,STYPE> ret(- (svec_extract(a, 0) * svec_extract(b, 0) - svec_extract(c, 0)),\
            - (svec_extract(a, 1) * svec_extract(b, 1) - svec_extract(c, 1)),\
            - (svec_extract(a, 2) * svec_extract(b, 2) - svec_extract(c, 2)),\
            - (svec_extract(a, 3) * svec_extract(b, 3) - svec_extract(c, 3)));\
  return ret;                            \
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

#define BINARY_OP_REDUCE_FUNC(STYPE, NAME, FUNC) \
static FORCEINLINE STYPE NAME(svec<LANES,STYPE> a) {            \
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "reduce");   \
  STYPE r = a[0]; \
  for(int i = 1; i < LANES; ++i) { r = FUNC(r, a[i]); } \
  return r; \
}

#define BINARY_OP_REDUCE_FUNC_L4(STYPE, NAME, FUNC) \
static FORCEINLINE STYPE NAME(svec<LANES,STYPE> a) {            \
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "reduce");   \
  return FUNC(FUNC(FUNC(a[0], a[1]), a[2]), a[3]); \
}

//  7. Compare
/**
 * @brief macros for binary: vector op scalar
 */
#define CMP_OP(STYPE, NAME, OP)                \
static FORCEINLINE svec<LANES,bool> svec_##NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {                   \
  INC_STATS_NAME(STATS_BINARY_SLOW, 1, #NAME);               \
  svec<LANES,bool> ret; \
  for (int i = 0; i < LANES; ++i) { ret[i] = a[i] OP b[i]; } \
  return ret;                            \
}

#define CMP_OP_L4(STYPE, NAME, OP)                \
  static FORCEINLINE svec<LANES,bool> svec_##NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) {    \
    INC_STATS_NAME(STATS_COMPARE_SLOW, 1, #NAME);                   \
    uint32_t r0 = (a[0] OP b[0]); \
    uint32_t r1 = (a[1] OP b[1]); \
    uint32_t r2 = (a[2] OP b[2]); \
    uint32_t r3 = (a[3] OP b[3]); \
    return svec<LANES,bool>(r0,r1,r2,r3);                  \
  }

/**
 * Macros for masked operation based on fast operation
 */
#define CMP_MASKED_OP(STYPE, NAME, OP) \
/**
 * @brief Do NAME operation on a and b with mask
 * If mask is true, return the compare result, otherwise return false.
 */\
FORCEINLINE svec<LANES,bool> svec_masked_##NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b, \
                               svec<LANES,bool> mask) { \
  return svec_and(svec_##NAME(a,b) , mask);              \
}




#define CMP_ALL_NOMASK_OP(STYPE)    \
  CMP_OP(STYPE, equal, ==) \
  CMP_OP(STYPE, not_equal, !=) \
  CMP_OP(STYPE, less_than, <) \
  CMP_OP(STYPE, less_equal, <=) \
  CMP_OP(STYPE, greater_than, >) \
  CMP_OP(STYPE, greater_equal, >=)

#define CMP_ALL_NOMASK_OP_L4(STYPE)    \
  CMP_OP_L4(STYPE, equal, ==) \
  CMP_OP_L4(STYPE, not_equal, !=) \
  CMP_OP_L4(STYPE, less_than, <) \
  CMP_OP_L4(STYPE, less_equal, <=) \
  CMP_OP_L4(STYPE, greater_than, >) \
  CMP_OP_L4(STYPE, greater_equal, >=)

#define CMP_ALL_MASKED_OP(STYPE)  \
  CMP_MASKED_OP(STYPE, equal, ==) \
  CMP_MASKED_OP(STYPE, not_equal, !=) \
  CMP_MASKED_OP(STYPE, less_than, <) \
  CMP_MASKED_OP(STYPE, less_equal, <=) \
  CMP_MASKED_OP(STYPE, greater_than, >) \
  CMP_MASKED_OP(STYPE, greater_equal, >=)

#define CMP_ALL_OP(STYPE)    \
  CMP_ALL_NOMASK_OP(STYPE) \
  CMP_ALL_MASKED_OP(STYPE)

//  8. Cast
#define CAST(SFROM, STO)        \
template <class T> static T svec_cast(svec<LANES,SFROM> val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    INC_STATS_NAME(STATS_CAST_SLOW, 1, "svec<LANES,"#SFROM">-svec<LANES,"#STO">");          \
    svec<LANES,STO> ret; \
    for (int i = 0; i < LANES; ++i) { ret[i] = (STO)val[i]; } \
    return ret; \
}

#define CAST_L4(SFROM, STO)        \
template <class T> static T svec_cast(svec<LANES,SFROM> val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    INC_STATS_NAME(STATS_CAST_SLOW, 1, "svec<LANES,"#SFROM">-svec<LANES,"#STO">");          \
    return svec<LANES,STO>((STO)val[0],(STO)val[1],(STO)val[2],(STO)val[3]); \
}

typedef union {
    int32_t i32;
    uint32_t u32;
    float f;
    int64_t i64;
    uint64_t u64;
    double d;
} BitcastUnion;

#define CAST_BITS(SFROM, FROM_F, STO, TO_F)        \
template <class T> static T svec_cast_bits(svec<LANES,SFROM> val);     \
template <> FORCEINLINE svec<LANES,STO> svec_cast_bits<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    INC_STATS_NAME(STATS_CAST_SLOW, 1, "svec<LANES,"#SFROM">-svec<LANES,"#STO">");          \
    BitcastUnion u[LANES]; \
    svec<LANES,STO> ret; \
    for(int i = 0; i < LANES; ++i) {u[i].FROM_F = val[i]; ret[i] = u[i].TO_F;} \
    return ret; \
}


//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////

#define SUBSCRIPT_FUNC_IMPL(STYPE) \
FORCEINLINE STYPE& svec<LANES,STYPE>::operator[](int index) { \
  INC_STATS_NAME(STATS_INSERT, 1, "insert "#STYPE);   \
  return v[index];   \
} \
const FORCEINLINE STYPE  svec<LANES,STYPE>::operator[](int index) const { \
  return v[index]; \
}


/**
 * Below I use macros to declare all vector operators
 *
 */

#define VEC_CMP_IMPL(STYPE)     \
/**
 * @brief Element-wise compare equal, return a bool vector, e.g., "a == b"
 * @param[in] a a vector
 * @return the result of compare equal as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator==(svec<LANES,STYPE> a) { return svec_equal(*this, a); } \
/**
 * @brief Element-wise compare not equal, return a bool vector. E.g. "a != b"
 * @param[in] a a vector
 * @return the result of compare not equal as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator!=(svec<LANES,STYPE> a) { return svec_not_equal(*this, a); } \
/**
 * @brief Element-wise compare less than, return a bool vector. E.g. "a < b"
 * @param[in] a a vector
 * @return the result of compare less than as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator<(svec<LANES,STYPE> a) { return svec_less_than(*this, a); } \
/**
 * @brief Element-wise compare less equal, return a bool vector. E.g. "a <= b"
 * @param[in] a a vector
 * @return the result of compare less equal as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator<=(svec<LANES,STYPE> a) { return svec_less_equal(*this, a); } \
/**
 * @brief Element-wise compare greater than, return a bool vector. E.g. "a > b"
 * @param[in] a a vector
 * @return the result of compare greater than as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator>(svec<LANES,STYPE> a) { return svec_greater_than(*this, a); } \
/**
 * @brief Element-wise compare greater equal, return a bool vector. E.g. "a >= b"
 * @param[in] a a vector
 * @return the result of compare greater equal as a boolean vector.
 */\
  FORCEINLINE svec<LANES,bool> svec<LANES,STYPE>::operator>=(svec<LANES,STYPE> a) { return svec_greater_equal(*this, a); }

#define VEC_UNARY_IMPL(STYPE) \
/**
 * @brief get the neg value, return a VTYPE vector. E.g. "-a"
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator-() {return svec_neg(*this); } \
/**
 * @brief Get the sum of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE svec<LANES,STYPE>::reduce_add() {return svec_reduce_add(*this); } \
/**
 * @brief Get the max value of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE svec<LANES,STYPE>::reduce_max() {return svec_reduce_max(*this); } \
/**
 * @brief Get the min value of all the element values in the vector. return a STYPE scalar
 */\
  FORCEINLINE STYPE svec<LANES,STYPE>::reduce_min() {return svec_reduce_min(*this); }


#define VEC_BIN_IMPL(STYPE)    \
/**
 * @brief Add two vectors
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator+(svec<LANES,STYPE> a) { return svec_add(*this, a); } \
/**
 * @brief Add a vector and a scalar
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator+(STYPE s) { return svec_add_scalar(*this, s); } \
/**
 * @brief Add a scalar and a vector
 */\
  FORCEINLINE svec<LANES,STYPE> operator+(STYPE s, svec<LANES,STYPE> a) {return svec_scalar_add(s, a);} \
/**
 * @brief Sub two vectors
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator-(svec<LANES,STYPE> a) { return svec_sub(*this, a); } \
/**
 * @brief Sub a vector and a scalar
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator-(STYPE s) { return svec_sub_scalar(*this, s); } \
/**
 * @brief Sub a scalar and a vector
 */\
  FORCEINLINE svec<LANES,STYPE> operator-(STYPE s, svec<LANES,STYPE> a) {return svec_scalar_sub(s, a);} \
/**
 * @brief Multiply two vectors
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator*(svec<LANES,STYPE> a) { return svec_mul(*this, a); } \
/**
 * @brief Multiply a vector and a scalar
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator*(STYPE s) { return svec_mul_scalar(*this, s) ;} \
/**
 * @brief Multiply a scalar and a vector
 */\
  FORCEINLINE svec<LANES,STYPE> operator*(STYPE s, svec<LANES,STYPE> a) {return svec_scalar_mul(s, a);} \
/**
 * @brief Divide a vector by a vector
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator/(svec<LANES,STYPE> a) { return svec_div(*this, a); } \
/**
 * @brief Divide a vector by a scalar
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator/(STYPE s) { return svec_div_scalar(*this, s) ;} \
/**
 * @brief Divide a scalar by a vector
 */\
  FORCEINLINE svec<LANES,STYPE> operator/(STYPE s, svec<LANES,STYPE> a) {return svec_scalar_div(s, a);} \

  /**
   * @brief mask class's class method impl
   */
  #define MVEC_CLASS_METHOD_IMPL(STYPE) \
  /*!
     @brief load the vector from the pointer p
     @param[in] p load address
     \note p does not have to be aligned
     @return no return value. This vector is updated with value loaded from p.
   */\
    FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::load(svec<LANES,STYPE>* p){ return svec_load(p); } \
  /*!
     @brief Store the vector to address p.
     @param[in] p store address
     \note p does not have to be aligned
   */\
    FORCEINLINE void svec<LANES,STYPE>::store(svec<LANES,STYPE>* p){ svec_store(p, *this); }


#define VEC_CLASS_METHOD_IMPL(STYPE) \
  MVEC_CLASS_METHOD_IMPL(STYPE); \
/*!
   @brief Return a new vector by only loading the value from the pointer p if the mask element is true
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::masked_load(svec<LANES,STYPE>* p, svec<LANES,bool> mask){ return svec_masked_load(p, mask); } \
/*!
   @brief Store the vector element's value to pointer p if the mask element is true
 */\
  FORCEINLINE void svec<LANES,STYPE>::masked_store(svec<LANES,STYPE>* p, svec<LANES,bool> mask){ svec_masked_store(p, *this, mask); } \
  VEC_UNARY_IMPL(STYPE); \
  VEC_BIN_IMPL(STYPE); \
/*!
   @brief Construct a vector by loading a scalar value from pointer p, and splat it to all the elements in the vector
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::load_const(const STYPE* p) {return svec_load_const<svec<LANES,STYPE> >(p);} \
/*!
   @brief Construct a vector by loading a scalar value from pointer p, and splat it to all the elements in the vector
 */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::load_and_splat(STYPE* p) {return svec_load_and_splat<svec<LANES,STYPE> >(p); } \
  /*!
     @brief Gather the elements pointed by the vector ptrs if the mask element is true, and return a vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::gather(svec<LANES,void*> ptrs, svec<LANES,bool> mask) {return svec_gather<svec<LANES,STYPE> >(ptrs, mask); } \
  /*!
     @brief Scatter the vector's elements to the locations pointed by the vector ptrs if the mask element is true.
   */\
  FORCEINLINE void svec<LANES,STYPE>::scatter(svec<LANES,void*> ptrs, svec<LANES,bool> mask) { svec_scatter(ptrs, *this, mask); } \
  /*!
     @brief Gather the elements pointed by calculating the addresses ((char*)b + scale * offsets) if the mask element is true, and return a vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int32_t> offsets, svec<LANES,bool> mask) { \
    return svec_gather_base_offsets(b, scale, offsets, mask); \
  } \
  /*!
     @brief Gather the elements pointed by calculating the addresses ((char*)b + scale * offsets) if the mask element is true, and return a vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::gather_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int64_t> offsets, svec<LANES,bool> mask) {\
      return svec_gather_base_offsets(b, scale, offsets, mask); \
  } \
  /*!
     @brief Scatter the vector's elements to the addresses ((char*)b + scale * offsets) if the mask element is true.
   */\
  FORCEINLINE void svec<LANES,STYPE>::scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int32_t> offsets, svec<LANES,bool> mask) { \
      svec_scatter_base_offsets(b, scale, offsets, *this, mask); \
  } \
  /*!
     @brief Scatter the vector's elements to the addresses ((char*)b + scale * offsets) if the mask element is true.
   */\
  FORCEINLINE void svec<LANES,STYPE>::scatter_base_offsets(STYPE* b, uint32_t scale, svec<LANES,int64_t> offsets, svec<LANES,bool> mask) {\
      svec_scatter_base_offsets(b, scale, offsets, *this, mask); \
  } \
  /*!
     @brief Gather the elements pointed by (b+off, b++off+stride, b+off+2*stride, b+off+3*step).
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::gather_stride(STYPE* b, int32_t off, int32_t stride) { \
    return svec_gather_stride<svec<LANES,STYPE> >(b, off, stride); \
  } \
  /*!
     @brief Gather the elements pointed by (b+off, b++off+stride, b+off+2*stride, b+off+3*step).
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::gather_stride(STYPE* b, int64_t off, int64_t stride) {\
      return svec_gather_stride<svec<LANES,STYPE> >(b, off, stride); \
  } \
  /*!
     @brief Scatter the vector's elements to the addresses (b+off, b++off+stride, b+off+2*stride, b+off+3*step).
   */\
  FORCEINLINE void svec<LANES,STYPE>::scatter_stride(STYPE* b, int32_t off, int32_t stride) { \
      svec_scatter_stride(b, off, stride, *this); \
  } \
  /*!
     @brief Scatter the vector's elements to the addresses (b+off, b++off+stride, b+off+2*stride, b+off+3*step).
   */\
  FORCEINLINE void svec<LANES,STYPE>::scatter_stride(STYPE* b, int64_t off, int64_t stride) {\
    svec_scatter_stride(b, off, stride, *this); \
  } \
  /*!
     @brief Return a new vector by setting all the elements of the new vector with this vector's index element.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::broadcast(int32_t index) { return svec_broadcast(*this, index);} \
  /*!
     @brief Return a new vector by rotate this vector's elements. e.g. newVec[i] = thisVec[i+index]
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::rotate(int32_t index) { return svec_rotate(*this, index); } \
  /*!
     @brief Return a new vector by shuffle this vector's elements with index vector e.g. newVec[i] = thisVec[index[i]]
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::shuffle(svec<LANES,int32_t> index) { return svec_shuffle(*this, index); } \
  /*!
     @brief Return a new vector of the vector's abs value.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::abs() { return svec_abs(*this); }

#define VEC_INT_CLASS_METHOD_IMPL(STYPE, STYPE2) \
  /**
   * @brief Or operator. E.g. "a | b".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator|(svec<LANES,STYPE> a) { return svec_or(*this, a); } \
  /**
   * @brief And operator. E.g. "a & b".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator&(svec<LANES,STYPE> a) { return svec_and(*this, a); } \
  /**
   * @brief Xor operator. E.g. "a ^ b".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator^(svec<LANES,STYPE> a) { return svec_xor(*this, a); } \
  /**
   * @brief Left shift operator. E.g. "a << b".The b must be unsigned vector
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator<<(svec<LANES,STYPE2> a) { return svec_shl(*this, a); } \
  /**
   * @brief Left shift operator by a scalar. E.g. "a << 5".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator<<(int32_t s) { return svec_shl(*this, s); } \
  /**
   * @brief Right shift operator. E.g. "a >> b".The b must be unsigned vector
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator>>(svec<LANES,STYPE2> a) { return svec_shr(*this, a); } \
  /**
   * @brief Right shift operator by a scalar. E.g. "a >> 5".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator>>(int32_t s) { return svec_shr(*this, s); } \
  /**
   * @brief Remainder operator on a vector. E.g. "a % b".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator%(svec<LANES,STYPE> a) { return svec_rem(*this, a); } \
  /**
   * @brief Remainder operator on a scalar. E.g. "a % 5".
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::operator%(STYPE s) { return svec_rem(*this, s); }


#define VEC_FLOAT_CLASS_METHOD_IMPL(STYPE) \
  /**
   * @brief return the round VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::round() { return svec_round(*this);} \
  /**
   * @brief return the floor VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::floor() { return svec_floor(*this);} \
  /**
   * @brief return the ceil VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::ceil() { return svec_ceil(*this);} \
  /**
   * @brief return the sqrt VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::sqrt() { return svec_sqrt(*this);} \
  /**
   * @brief return the reverse VTYPE vector. (1.0/thisVec)
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::rcp() { return svec_rcp(*this);} \
  /**
   * @brief return the reverse sqrt VTYPE vector. ( 1.0/sqrt(thisVec) )
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::rsqrt() { return svec_rsqrt(*this);}\
  /**
   * @brief return the exp VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::exp() {return svec_exp(*this);} \
  /**
   * @brief return the log VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::log() {return svec_log(*this);} \
  /**
   * @brief return the pow VTYPE vector.
   */\
  FORCEINLINE svec<LANES,STYPE> svec<LANES,STYPE>::pow(svec<LANES,STYPE> a) { return svec_pow(*this, a); }


#endif /* GSIMD_UTILITY_H_ */
