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

/**
 * test_utility.h
 *
 *  Created on: Aug 16, 2013
 *  @author: Haichuan Wang (hwang154@illinois.edu)
 *  @brief: common functions for test different lanes.
 */

#ifndef TEST_UTILITY_H_
#define TEST_UTILITY_H_

#define EXPECT_SVEC_EQ(v1, v2) EXPECT_TRUE(((v1) == (v2)).all_true())
#define EXPECT_SVEC_MASKED_EQ(v1, v2, mask) EXPECT_TRUE((svec_masked_equal((v1), (v2), (mask)) == mask).all_true())

/**
 * @brief macros for check float equal
 */
#define EXPECT_SVEC_FEQ(v1, v2) EXPECT_TRUE( \
    (v1 - v2).abs().reduce_add() < 0.005 * LANES)


#define DUMP(v) std::cout << #v << ":" << (v) << std::endl

template<typename STYPE, typename VTYPE>
VTYPE random_vec(int maxValue) {
  VTYPE vec;
  for (int i=0; i<LANES; i++) {
    STYPE value = (STYPE) rand();
    if (maxValue != -1) value = (STYPE)((int)value % maxValue);
    vec[i] = value;
  }
  return vec;
}

template<typename STYPE, typename VTYPE>
VTYPE random_vec() {
  random_vec<STYPE, VTYPE>(-1);
}

template <class VTYPE, class VTYPE2>
VTYPE ref_shr(VTYPE val, VTYPE2 s) {
  VTYPE ret;
  for(int i = 0; i < LANES; i++) {
    ret[i] = val[i] >> s[i];
  }
  return ret;
}

template <class VTYPE>
VTYPE ref_shr(VTYPE val, int s) {
  VTYPE ret;
  for(int i = 0; i < LANES; i++) {
    ret[i] = val[i] >> s;
  }
  return ret;
}


template <class VTYPE, class VTYPE2>
VTYPE ref_shl(VTYPE val, VTYPE2 s) {
  VTYPE ret;
  for(int i = 0; i < LANES; i++) {
    ret[i] = val[i] << s[i];
  }
  return ret;
}

template <class VTYPE>
VTYPE ref_shl(VTYPE val, int s) {
  VTYPE ret;
  for(int i = 0; i < LANES; i++) {
    ret[i] = val[i] << s;
  }
  return ret;
}



template <class FROM, class TO, class STO>
TO ref_cast(FROM val) {
  TO ret;
  for(int i = 0; i < LANES; i++) {
    ret[i] = (STO)val[i];
  }
  return ret;
}


#endif /* TEST_UTILITY_H_ */
