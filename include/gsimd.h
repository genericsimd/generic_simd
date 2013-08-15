/**
 * gsimd.h
 *
 *  Created on: Aug 14, 2013
 *  @author: Haichuan Wang (haichuan@us.ibm.com hwang154@illinois.edu)
 *  @brief: Generic SIMD Library header configuration file
 */

#ifndef GSIMD_H_
#define GSIMD_H_

//a macro to for GSIMD
#define __GSIMD__
#ifdef __ALTIVEC__
#include <power_vsx4.h>
using namespace vsx;
#else
#ifdef __SSE4_2__
#include <sse4.h>
using namespace sse;
#else
#include <generic4.h>
using namespace generic;
#endif //__SSE4_2__
#endif //__ALTIVEC__




#endif /* GSIMD_H_ */
