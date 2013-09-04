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
 * svec-vsx.h
 *
 *  Created on: Jul 7, 2013
 *      Author: Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 */

#ifndef SVEC_VSX_H_
#define SVEC_VSX_H_

#include <stdint.h>
#include <math.h>
#include <altivec.h>
#include <assert.h>
#include <iostream>

#include "gsimd_utility.h"
#include "platform_intrinsics.h"

std::ostream& operator<< (std::ostream &out, uint8_t &v) {
  out << uint16_t(v);
  return out;
}

std::ostream& operator<< (std::ostream &out, int8_t &v) {
  out << int16_t(v);
  return out;
}

namespace vsx {

template<int N, typename REGTYPE, typename STYPE>
class svec_internal {
protected:
  FORCEINLINE int lanes_per_reg() { return sizeof(REGTYPE)/sizeof(STYPE);}
  FORCEINLINE int regs() { return N/lanes_per_reg();}


  FORCEINLINE svec_internal() {}

  FORCEINLINE svec_internal(const REGTYPE vva[]) {
    for(int i=0; i < regs() ; i++) {
      va[i] = vva[i];
    }
  }

  FORCEINLINE svec_internal(const STYPE v) {
    REGTYPE t;
    switch(lanes_per_reg()) {
    case 2: {//uint64_t, int64_t, double for 128bit{
        t = REGTYPE(v,v);
      }
      break;
    case 4: {//
        t = REGTYPE(v,v,v,v);
      }
      break;
    case 8: {
        t = REGTYPE(v,v,v,v,v,v,v,v);
      }
      break;
    case 16: { //
        t = REGTYPE(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v);
      }
      break;
    case 32: {//suppose 256bit SIMD for 8 bit
        t = REGTYPE(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,
                      v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v);
      }
      break;
    } //switch
    for(int i=0; i < N; i+=lanes_per_reg()) {
      va[i/lanes_per_reg()] = t;
    } //for
  }


  FORCEINLINE svec_internal(const STYPE v[]) {
    for(int i=0; i < N; i+=lanes_per_reg()) {
      REGTYPE t;
      switch(lanes_per_reg()) {
      case 2: {//uint64_t, int64_t, double for 128bit{
          t = REGTYPE(v[i], v[i+1]);
        }
        break;
      case 4: {//
          t = REGTYPE(v[i], v[i+1], v[i+2], v[i+3]);
        }
        break;
      case 8: {
          t = REGTYPE(v[i], v[i+1], v[i+2], v[i+3], v[i+4], v[i+5], v[i+6], v[i+7]);
        }
        break;
      case 16: { //
          t = REGTYPE(v[i], v[i+1], v[i+2], v[i+3], v[i+4], v[i+5], v[i+6], v[i+7],
              v[i+8], v[i+9], v[i+10], v[i+11], v[i+12], v[i+13], v[i+14], v[i+15]);
        }
        break;
      case 32: {//suppose 256bit SIMD for 8 bit
          t = REGTYPE(v[i], v[i+1], v[i+2], v[i+3], v[i+4], v[i+5], v[i+6], v[i+7],
               v[i+8], v[i+9], v[i+10], v[i+11], v[i+12], v[i+13], v[i+14], v[i+15],
               v[i+16], v[i+17], v[i+18], v[i+19], v[i+20], v[i+21], v[i+22], v[i+23],
               v[i+24], v[i+25], v[i+26], v[i+27], v[i+28], v[i+29], v[i+30], v[i+31]);
        }
        break;
      } //switch
      va[i/lanes_per_reg()] = t;
    } //for
  }

public:
  /**
   * @brief Internal use for get the storage register
   */
  FORCEINLINE REGTYPE & reg(int index) { return va[index];}
  FORCEINLINE const REGTYPE & reg(int index) const { return va[index];}

  FORCEINLINE STYPE& operator[](int index) {return ((STYPE *)va)[index];}
  FORCEINLINE const STYPE& operator[](int index) const {return ((STYPE *)va)[index]; }

  friend std::ostream& operator<< (std::ostream &out, const svec_internal &v) {
    out << "svec_"<< iu_get_type_name<STYPE>() << "<" << N << ">[";
    stdout_scalar<STYPE>(out, v[0]);
    for(int i = 1; i < N ; i++) {
      out << ", ";
      stdout_scalar<STYPE>(out, v[i]);
    }
    out << "]";
    return out;
  }

  REGTYPE va[N/(sizeof(REGTYPE)/sizeof(STYPE))];
};


template <int N>
class svec_bool: public svec_internal<N, __vector uint32_t, uint32_t> {

public:
  FORCEINLINE svec_bool() { }
  FORCEINLINE svec_bool(const __vector uint32_t vva[]) : svec_internal<N, __vector uint32_t, uint32_t>(vva) { }

  /**
   * @brief bool type's initial function need set each element full bits, either 0 or 0xFFFFFFFF.
   * @param v an array of bool values.
   * @return a svec_bool type object
   */
  FORCEINLINE svec_bool(const bool v[]) {
    for(int i=0; i < N; i+=4) {
      __vector uint32_t t = { v[i] ? -1 : 0, v[i+1] ? -1 : 0,
                              v[i+2] ? -1 : 0, v[i+3] ? -1 : 0 };
      this->va[i>>2] = t;
    }
  }

  FORCEINLINE svec_bool(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    __vector uint32_t t = {v0 ? -1 : 0, v1 ? -1 : 0, v2 ? -1 : 0, v3 ? -1 : 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_bool(const bool &v0, const bool &v1, const bool &v2, const bool &v3,
                        const bool &v4, const bool &v5, const bool &v6, const bool &v7) {
    __vector uint32_t t0 = {v0 ? -1 : 0, v1 ? -1 : 0, v2 ? -1 : 0, v3 ? -1 : 0};
    this->va[0] = t0;
    __vector uint32_t t1 = {v4 ? -1 : 0, v5 ? -1 : 0, v6 ? -1 : 0, v7 ? -1 : 0};
    this->va[1] = t1;
  }
  FORCEINLINE svec_bool(const bool& v) {
    if(__builtin_constant_p(v)){
      __vector uint32_t t = (v) ? vec_splat_s32(-1) : vec_splat_s32(0);
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector uint32_t, uint32_t>(v);
    }
  }
};


template <int N>
class svec_i8: public svec_internal<N, __vector int8_t, int8_t> {

public:
  FORCEINLINE svec_i8() { }
  FORCEINLINE svec_i8(const __vector int8_t vva[]) : svec_internal<N, __vector int8_t, int8_t>(vva) {}
  FORCEINLINE svec_i8(const int8_t v[]) : svec_internal<N, __vector int8_t, int8_t>(v) {}
  FORCEINLINE svec_i8(const int8_t& v0, const int8_t& v1, const int8_t& v2, const int8_t& v3) {
    __vector int8_t t = {v0, v1, v2, v3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_i8(const int8_t& v0, const int8_t& v1, const int8_t& v2, const int8_t& v3,
                      const int8_t& v4, const int8_t& v5, const int8_t& v6, const int8_t& v7) {
    __vector int8_t t = {v0, v1, v2, v3, v4, v5, v6, v7, 0, 0, 0, 0, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_i8(const int8_t& v) {
    if(__builtin_constant_p(v) && (v <= 15) && (v >= -16)){
      __vector int8_t t = vec_splat_s8(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector int8_t, int8_t>(v);
    }
  }

};

template <int N>
class svec_u8: public svec_internal<N, __vector uint8_t, uint8_t> {

public:
  FORCEINLINE svec_u8() { }
  FORCEINLINE svec_u8(const __vector uint8_t vva[]) : svec_internal<N, __vector uint8_t, uint8_t>(vva) {}
  FORCEINLINE svec_u8(const uint8_t v[]) : svec_internal<N, __vector uint8_t, uint8_t>(v) {}
  FORCEINLINE svec_u8(const uint8_t& v0, const uint8_t& v1, const uint8_t& v2, const uint8_t& v3) {
    __vector uint8_t t = {v0, v1, v2, v3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_u8(const uint8_t& v0, const uint8_t& v1, const uint8_t& v2, const uint8_t& v3,
                      const uint8_t& v4, const uint8_t& v5, const uint8_t& v6, const uint8_t& v7) {
    __vector uint8_t t = {v0, v1, v2, v3, v4, v5, v6, v7, 0, 0, 0, 0, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_u8(const uint8_t& v) {
    if(__builtin_constant_p(v) && (v <= 15)){
      __vector uint8_t t = vec_splat_u8(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector uint8_t, uint8_t>(v);
    }
  }
};

template <int N>
class svec_i16: public svec_internal<N, __vector int16_t, int16_t> {

public:
  FORCEINLINE svec_i16() { }
  FORCEINLINE svec_i16(const __vector int16_t vva[]) : svec_internal<N, __vector int16_t, int16_t>(vva) {}
  FORCEINLINE svec_i16(const int16_t v[]) : svec_internal<N, __vector int16_t, int16_t>(v) {}
  FORCEINLINE svec_i16(const int16_t& v0, const int16_t& v1, const int16_t& v2, const int16_t& v3) {
    __vector int16_t t = {v0, v1, v2, v3, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_i16(const int16_t& v0, const int16_t& v1, const int16_t& v2, const int16_t& v3,
                      const int16_t& v4, const int16_t& v5, const int16_t& v6, const int16_t& v7) {
    __vector int16_t t = {v0, v1, v2, v3, v4, v5, v6, v7};
    this->va[0] = t;
  }
  FORCEINLINE svec_i16(const int16_t& v) {
    if(__builtin_constant_p(v) && (v <= 15) && (v >= -16)){
      __vector int16_t t = vec_splat_s16(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector int16_t, int16_t>(v);
    }
  }
};

template <int N>
class svec_u16: public svec_internal<N, __vector uint16_t, uint16_t> {

public:
  FORCEINLINE svec_u16() { }
  FORCEINLINE svec_u16(const __vector uint16_t vva[]) : svec_internal<N, __vector uint16_t, uint16_t>(vva) {}
  FORCEINLINE svec_u16(const uint16_t v[]) : svec_internal<N, __vector uint16_t, uint16_t>(v) {}
  FORCEINLINE svec_u16(const uint16_t& v0, const uint16_t& v1, const uint16_t& v2, const uint16_t& v3) {
    __vector uint16_t t = {v0, v1, v2, v3, 0, 0, 0, 0};
    this->va[0] = t;
  }
  FORCEINLINE svec_u16(const uint16_t& v0, const uint16_t& v1, const uint16_t& v2, const uint16_t& v3,
                      const uint16_t& v4, const uint16_t& v5, const uint16_t& v6, const uint16_t& v7) {
    __vector uint16_t t = {v0, v1, v2, v3, v4, v5, v6, v7};
    this->va[0] = t;
  }
  FORCEINLINE svec_u16(const uint16_t& v) {
    if(__builtin_constant_p(v) && (v <= 15)){
      __vector uint16_t t = vec_splat_u16(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector uint16_t, uint16_t>(v);
    }
  }
};

template <int N>
class svec_i32: public svec_internal<N, __vector int32_t, int32_t> {

public:
  FORCEINLINE svec_i32() { }
  FORCEINLINE svec_i32(const __vector int32_t vva[]) : svec_internal<N, __vector int32_t, int32_t>(vva) {}
  FORCEINLINE svec_i32(const int32_t v[]) : svec_internal<N, __vector int32_t, int32_t>(v) {}
  FORCEINLINE svec_i32(const int32_t& v0, const int32_t& v1, const int32_t& v2, const int32_t& v3) {
    __vector int32_t t = {v0, v1, v2, v3};
    this->va[0] = t;
  }
  FORCEINLINE svec_i32(const int32_t& v0, const int32_t& v1, const int32_t& v2, const int32_t& v3,
                      const int32_t& v4, const int32_t& v5, const int32_t& v6, const int32_t& v7) {
    __vector int32_t t0 = {v0, v1, v2, v3};
    this->va[0] = t0;
    __vector int32_t t1 = {v4, v5, v6, v7};
    this->va[1] = t1;
  }
  FORCEINLINE svec_i32(const int32_t& v) {
    if(__builtin_constant_p(v) && (v <= 15) && (v >= -16)){
      __vector int32_t t = vec_splat_s32(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //fo
    } else {
      svec_internal<N, __vector int32_t, int32_t>(v);
    }
  }
};

template <int N>
class svec_u32: public svec_internal<N, __vector uint32_t, uint32_t> {

public:
  FORCEINLINE svec_u32() { }
  FORCEINLINE svec_u32(const __vector uint32_t vva[]) : svec_internal<N, __vector uint32_t, uint32_t>(vva) {}
  FORCEINLINE svec_u32(const uint32_t v[]) : svec_internal<N, __vector uint32_t, uint32_t>(v) {}
  FORCEINLINE svec_u32(const uint32_t& v0, const uint32_t& v1, const uint32_t& v2, const uint32_t& v3) {
    __vector uint32_t t = {v0, v1, v2, v3};
    this->va[0] = t;
  }
  FORCEINLINE svec_u32(const uint32_t& v0, const uint32_t& v1, const uint32_t& v2, const uint32_t& v3,
                      const uint32_t& v4, const uint32_t& v5, const uint32_t& v6, const uint32_t& v7) {
    __vector uint32_t t0 = {v0, v1, v2, v3};
    this->va[0] = t0;
    __vector uint32_t t1 = {v4, v5, v6, v7};
    this->va[1] = t1;
  }
  FORCEINLINE svec_u32(const uint32_t& v) {
    if(__builtin_constant_p(v) && (v <= 15)){
      __vector uint32_t t = vec_splat_u8(v); //will gen one instr.vspltisb
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //for
    } else {
      svec_internal<N, __vector uint32_t, uint32_t>(v);
    }
  }
};


template <int N>
class svec_i64: public svec_internal<N, __vector int64_t, int64_t> {

public:
  FORCEINLINE svec_i64() { }
  FORCEINLINE svec_i64(const __vector int64_t vva[]) : svec_internal<N, __vector int64_t, int64_t>(vva) {}
  FORCEINLINE svec_i64(const int64_t v[]) : svec_internal<N, __vector int64_t, int64_t>(v) {}
  FORCEINLINE svec_i64(const int64_t& v0, const int64_t& v1, const int64_t& v2, const int64_t& v3) {
    __vector int64_t t0 = {v0, v1};
    this->va[0] = t0;
    __vector int64_t t1 = {v2, v3};
    this->va[1] = t1;
  }
  FORCEINLINE svec_i64(const int64_t& v0, const int64_t& v1, const int64_t& v2, const int64_t& v3,
                      const int64_t& v4, const int64_t& v5, const int64_t& v6, const int64_t& v7) {
    __vector int64_t t0 = {v0, v1};
    this->va[0] = t0;
    __vector int64_t t1 = {v2, v3};
    this->va[1] = t1;
    __vector int64_t t2 = {v4, v5};
    this->va[2] = t2;
    __vector int64_t t3 = {v6, v7};
    this->va[3] = t3;
  }
  FORCEINLINE svec_i64(const int64_t& v) {
    if(__builtin_constant_p(v)){
      __vector int64_t t;
#ifdef __POWER8
      if ((v >= -16l) && (v <= 15l)) {
        const int iv = (int)v;
        __vector signed int x = {iv,iv,iv,iv};
        t = vec_unpackh_p8(x);
      } else
#endif
      t = (__vector int64_t)(v,v);

      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //for
    } else {
      svec_internal<N, __vector int64_t, int64_t>(v);
    }
  }
};

template <int N>
class svec_u64: public svec_internal<N, __vector uint64_t, uint64_t> {

public:
  FORCEINLINE svec_u64() { }
  FORCEINLINE svec_u64(const __vector uint64_t vva[]) : svec_internal<N, __vector uint64_t, uint64_t>(vva) {}
  FORCEINLINE svec_u64(const uint64_t v[]) : svec_internal<N, __vector uint64_t, uint64_t>(v) {}
  FORCEINLINE svec_u64(const uint64_t& v0, const uint64_t& v1, const uint64_t& v2, const uint64_t& v3) {
    __vector uint64_t t0 = {v0, v1};
    this->va[0] = t0;
    __vector uint64_t t1 = {v2, v3};
    this->va[1] = t1;
  }
  FORCEINLINE svec_u64(const uint64_t& v0, const uint64_t& v1, const uint64_t& v2, const uint64_t& v3,
                      const uint64_t& v4, const uint64_t& v5, const uint64_t& v6, const uint64_t& v7) {
    __vector uint64_t t0 = {v0, v1};
    this->va[0] = t0;
    __vector uint64_t t1 = {v2, v3};
    this->va[1] = t1;
    __vector uint64_t t2 = {v4, v5};
    this->va[2] = t2;
    __vector uint64_t t3 = {v6, v7};
    this->va[3] = t3;
  }
  FORCEINLINE svec_u64(const uint64_t& v) {
    if(__builtin_constant_p(v)){
      __vector uint64_t t;
#ifdef __POWER8
      if ((v >= 0ul) && (v <= 31ul)) {
        const int iv = (int)v;
        __vector signed int x = {iv,iv,iv,iv};
        t = vec_unpackh_p8(x);
      } else
#endif
      t = (__vector uint64_t)(v,v);

      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //for
    } else {
      svec_internal<N, __vector uint64_t, uint64_t>(v);
    }
  }
};


template <int N>
class svec_f: public svec_internal<N, __vector float, float> {

public:
  FORCEINLINE svec_f() { }
  FORCEINLINE svec_f(const __vector float vva[]) : svec_internal<N, __vector float, float>(vva) {}
  FORCEINLINE svec_f(const float v[]) : svec_internal<N, __vector float, float>(v) {}
  FORCEINLINE svec_f(const float& v0, const float& v1, const float& v2, const float& v3) {
    __vector float t = {v0, v1, v2, v3};
    this->va[0] = t;
  }
  FORCEINLINE svec_f(const float& v0, const float& v1, const float& v2, const float& v3,
                      const float& v4, const float& v5, const float& v6, const float& v7) {
    __vector float t0 = {v0, v1, v2, v3};
    this->va[0] = t0;
    __vector float t1 = {v4, v5, v6, v7};
    this->va[1] = t1;
  }
  FORCEINLINE svec_f(const float& v) {
    if(__builtin_constant_p(v)){
      __vector float t;
      float p; int iv;
      p = 1.0; iv = (int)(p*v);
      if (( (((float)iv)/p) == v ) && (iv >= -32) && (iv <= 15)) {
        t = vec_ctf(vec_splat_s32(iv),0);
      } else {
        p = 2.0; iv = (int)(p*v);
        if (( (((float)iv)/p) == v ) && (iv >= -16) && (iv <= 15)) {
          t = vec_ctf(vec_splat_s32(iv),1);
        } else {
          p = 4.0; iv = (int)(p*v);
          if (( (((float)iv)/p) == v ) && (iv >= -16) && (iv <= 15)) {
            t = vec_ctf(vec_splat_s32(iv),2);
          } else {
            t = (__vector float)(v, v, v, v);
          }
        }
      }
      for(int i=0; i < N; i+=this->lanes_per_reg()) {
        this->va[i/this->lanes_per_reg()] = t;
      } //for
    } else { //use built-in constructor
      svec_internal<N, __vector float, float>(v);
    }
  }

};

template <int N>
class svec_d: public svec_internal<N, __vector double, double> {

public:
  FORCEINLINE svec_d() { }
  FORCEINLINE svec_d(const __vector double vva[]) : svec_internal<N, __vector double, double>(vva) {}
  FORCEINLINE svec_d(const double v[]) : svec_internal<N, __vector double, double>(v) {}
  FORCEINLINE svec_d(const double& v0, const double& v1, const double& v2, const double& v3) {
    __vector double t0 = {v0, v1};
    this->va[0] = t0;
    __vector double t1 = {v2, v3};
    this->va[1] = t1;
  }
  FORCEINLINE svec_d(const double& v0, const double& v1, const double& v2, const double& v3,
                      const double& v4, const double& v5, const double& v6, const double& v7) {
    __vector double t0 = {v0, v1};
    this->va[0] = t0;
    __vector double t1 = {v2, v3};
    this->va[1] = t1;
    __vector double t2 = {v4, v5};
    this->va[2] = t2;
    __vector double t3 = {v6, v7};
    this->va[3] = t3;
  }
  FORCEINLINE svec_d(const double& v) {
    __vector double t = vec_smear_p7(v);
    for(int i=0; i < N; i+=this->lanes_per_reg()) {
      this->va[i/this->lanes_per_reg()] = t;
    } //for
  }
};


////////////Section of class member functions




} //namespace vsx


#endif /* SVEC_VSX_H_ */
