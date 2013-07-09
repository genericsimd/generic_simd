/*
 * intrinsics_utility.h
 *
 *  Created on: Jun 12, 2013
 *      Author: haichuan
 *
 *  Contains the utility functions for intrinsics implementation.
 *  Should be included in all intrinsics header files
 */

#ifndef INTRINSICS_UTILITY_H_
#define INTRINSICS_UTILITY_H_

#include <stdint.h>
#include <iostream>
#define DUMP(v) std::cout << #v << ":" << (v) << std::endl
#if ((ULONG_MAX) == (UINT_MAX))
#define IS32BIT
#else
#define IS64BIT
#endif

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


/**
 * LANES=1 short vector definition for ISPC interfaces
 */
typedef int8_t __vec1_i8;
typedef int16_t __vec1_i16;
typedef int32_t __vec1_i32;
typedef int64_t __vec1_i64;
typedef float __vec1_f;
typedef double __vec1_d;

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




//Used for non-template cases

/**
 * @brief macros to define vector type's [] operators
 */
#define SUBSCRIPT_FUNC(STYPE)    \
FORCEINLINE STYPE& operator[](int index) {   \
  INC_STATS_NAME(STATS_INSERT, 1, "insert "#STYPE);   \
  return ((STYPE *)&v)[index];   \
}  \
const STYPE& operator[](int index) const { \
  INC_STATS_NAME(STATS_EXTRACT, 1, "extract "#STYPE);    \
  return ((STYPE *)&v)[index];   \
}

//Only for I1 output, use 0/1 as output
#define COUT_FUNC_I1(VTYPE, LANES) \
friend std::ostream& operator<< (std::ostream &out, const VTYPE &v) { \
  out << #VTYPE <<"[" << (v[0]?1:0); \
  for(int i = 1; i < LANES ; i++) { out << ", " << (v[i]?1:0);} \
  out << "]"; \
  return out; \
} \

//i8 type need special one for output char as int number
#define COUT_FUNC_I8(VTYPE, LANES) \
friend std::ostream& operator<< (std::ostream &out, const VTYPE &v) { \
  out << #VTYPE <<"[" << short(v[0]); \
  for(int i = 1; i < LANES ; i++) { out << ", " << short(v[i]);} \
  out << "]"; \
  return out; \
} \

#define COUT_FUNC(VTYPE, LANES) \
friend std::ostream& operator<< (std::ostream &out, const VTYPE &v) { \
  out << #VTYPE <<"[" << v[0]; \
  for(int i = 1; i < LANES ; i++) { out << ", " << v[i];} \
  out << "]"; \
  return out; \
} \

/**
 * The below two marcos are only used for template cases implementation
 * of ISPC interfaces. E.g. generic-4 and generic4-8...
 */
#define SUBSCRIPT_FUNC_TLT() \
T& operator[](int index) {return ((T *)&v)[index];}  \
const T& operator[](int index) const {return ((T *)&v)[index];} \

#define COUT_FUNC_TLT(LANES) \
friend std::ostream& operator<< (std::ostream &out, const vec4 &v) { \
  out << "__vec"<< LANES <<"_" << iu_get_type_name<T>() << "[" << v[0]; \
  for(int i = 1; i < LANES ; i++) { out << ", " << v[i];} \
  out << "]"; \
  return out; \
} \

#endif /* INTRINSICS_UTILITY_H_ */
