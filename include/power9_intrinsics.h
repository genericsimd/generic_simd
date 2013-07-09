#define VSXW    "=wa"
#define VSXR    "wa"
#define VSXWC   "=&wa"

#define P9_NO_OP 1

#ifdef __POWER9

#define MSKREG  asm ("vs46")

static FORCEINLINE __vector unsigned int setmask(__vector unsigned int mask) {
  ///asm ( "mtvmr8w %x[mask] \n\t": :  [mask] VSXR(mask));
  __vector unsigned int register tgt  MSKREG;
  asm ( "xxlor %x[target],%x[mask],%x[mask]": [target] VSXW (tgt) : [mask] VSXR(mask));
  return tgt;
}

static FORCEINLINE void vec_scatter(__vector signed int src,signed int* ptr,
          __vector signed int off, __vector unsigned int mask) {
  __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "stxv8wx8w %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
                 : 
                 : [xt] VSXR(src), [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
                 : "memory");
}

static FORCEINLINE void vec_scatter(__vector float src, float* ptr,
          __vector signed int off, __vector unsigned int mask) {
  __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "stxv8wx8w %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
                 : 
                 : [xt] VSXR(src), [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
                 : "memory");
}

static FORCEINLINE void vec_scatter(__vector double src, double* ptr,
          __vector signed long long off, __vector signed long long mask) {
  __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "stxv4dx4d %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
                 : 
                 : [xt] VSXR(src), [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
                 : "memory");
}

static FORCEINLINE void vec_scatter(__vector float src, float* ptr,
				    __vector signed long long off1,
				    __vector signed long long off2,
				    __vector unsigned int mask) {
  __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "stxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm] \n\t"
		 "stxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
                 : 
		 : [xt] VSXR(src), [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  
}

static FORCEINLINE void vec_scatter(__vector signed int src, signed int* ptr,
				    __vector signed long long off1,
				    __vector signed long long off2,
				    __vector unsigned int mask) {
  __vector unsigned int register msk  MSKREG  = mask;

  asm volatile ( "stxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm] \n\t"
		 "stxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
                 : 
		 : [xt] VSXR(src), [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  
}

static FORCEINLINE __vector signed int vec_gather(signed int* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector signed int register res;
  __vector unsigned int register msk  MSKREG  = mask;

  asm volatile ( "lxv8wzx8w %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
		 : "memory");
  return res;
}




static FORCEINLINE __vector signed int vec_gather(signed int* ptr,
				      __vector signed long long off1,
				      __vector signed long long off2,
                                      __vector unsigned int mask) {
  __vector signed int res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm] \n\t"
		 "lxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res; 
}


static FORCEINLINE __vector unsigned int vec_gather(unsigned int* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector unsigned int register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv8wzx8w %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
		 : "memory");  
  return res;
}


static FORCEINLINE __vector unsigned int vec_gather(unsigned int* ptr,
     __vector signed long long off1,
     __vector signed long long off2, __vector unsigned int mask) {
  __vector unsigned int res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm] \n\t"
		 "lxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res;

}

static FORCEINLINE __vector float vec_gather(float* ptr,
     __vector unsigned int off, __vector unsigned int mask) {
  __vector float register res;
  __vector unsigned int register msk  MSKREG = mask;
  asm volatile ( "lxv8wzx8w %x[xt],%[ra],%x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off), [xm] VSXR (msk)
		 : "memory");  
  return res;
}

static FORCEINLINE __vector float vec_gather(float* ptr,
     __vector signed long long off1,
     __vector signed long long off2, __vector unsigned int mask) {
  __vector float res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm]\n\t"
		 "lxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res;
}


static FORCEINLINE __vector signed int vec_gather(signed long long* ptr,
     __vector signed long long off1,
     __vector signed long long off2, __vector unsigned int mask) {
  __vector signed int res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], %[ra], %x[xb1],%x[xm] \n\t"
		 "lxvr4w4d %x[xt], %[ra], %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [ra] "b"(ptr), [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res; 
}

//load 64 indexed by 64 
static FORCEINLINE __vector signed long long vec_gather(signed long long* ptr, 
			 __vector signed long long off, __vector unsigned int mask) {
  __vector signed long long register r2;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dx4d %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(r2)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");
  return r2;
}

static FORCEINLINE __vector double vec_gather(double* ptr, 
			 __vector signed long long off, 
                         __vector unsigned int mask) {
  __vector double register r2;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dx4d %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(r2)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");
  return r2;
}

static FORCEINLINE __vector double vec_gather_l(double* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector double register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dzx4wl %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");  
  return res;
}

static FORCEINLINE __vector double vec_gather_r(double* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector double register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dzx4wr %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");  
  return res;
}


static FORCEINLINE __vector signed long long vec_gather_l(signed long long* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector signed long long register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dzx4wl %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");  
  return res;
}

static FORCEINLINE __vector signed long long vec_gather_r(signed long long* ptr,
          __vector unsigned int off, __vector unsigned int mask) {
  __vector signed long long register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dzx4wr %x[xt], %[ra], %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [ra] "b"(ptr), [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");  
  return res;
}

// GENERIC GAther with pointers as arg; No scalling is achieved by passing register 0
static FORCEINLINE  __vector signed int vec_gather(__vector signed long long off1,
				      __vector signed long long off2, 
                                      __vector unsigned int mask) {
  __vector signed int res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], 0, %x[xb1],%x[xm] \n\t"
		 "lxvr4w4d %x[xt], 0, %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res; 
}

static FORCEINLINE  __vector float vec_gather_float(__vector signed long long off1,
						    __vector signed long long off2, 
						    __vector unsigned int mask) {
  __vector float res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxvl4w4d %x[xt], 0, %x[xb1],%x[xm] \n\t"
		 "lxvr4w4d %x[xt], 0, %x[xb2],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXWC(res)
		 : [xb1] VSXR(off1), [xb2] VSXR(off2),  [xm] VSXR (msk)
		 : "memory");  

  return res; 
}

static FORCEINLINE __vector signed long long vec_gather(__vector signed long long off,
                                                        __vector unsigned int mask) {
  __vector signed long long register res;
   __vector unsigned int register msk  MSKREG  = mask;
  asm volatile ( "lxv4dx4d %x[xt], 0, %x[xb],%x[xm]"
#ifdef P9_NO_OP
		 "\n\t ori 0,0,0"
                 "\n\t ori 0,0,0"
#endif
		 : [xt] VSXW(res)
		 : [xb] VSXR(off),  [xm] VSXR (msk)
		 : "memory");
   return res;
 }


static FORCEINLINE __vector unsigned int        vec_smear_i32_p9(unsigned int *ptr) {
  __vector unsigned int r;
  asm ("lxvwsx %x[xt], 0, %[xb]" : [xt] VSXW(r) : [xb] "b"(ptr): "memory");
  //asm ("lxvwsx %x[xt],%y1"      : [xt] VSXW (r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector float               vec_smear_float_p9(float *ptr) {
  __vector float r;
  //asm ("lxvwsx %x[xt], 0, %[xb]" : [xt] VSXW(r) : [xb] "b"(ptr): "memory");
  asm ("lxvwsx %x[xt],%y1"      : [xt] VSXW (r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector float               vec_smear_const_float_p9(const float *ptr) {
  __vector float r;
  //asm ("lxvwsx %x[xt],%y1"      : [xt] VSXW (r) : "Z"(*ptr) );
  asm ("lxvwsx %x[xt], 0, %[xb]" : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

static FORCEINLINE __vector unsigned int        vec_smear_i32_p9(unsigned int a){
  __vector unsigned int register r;
  asm ("insertwz %x[xt], %[xa], 7": [xt] VSXW(r) : [xa] "r"(a) );
  return r;
}

static FORCEINLINE __vector unsigned long long  vec_smear_i64_p9(unsigned long long a){
  __vector unsigned long long register r;
  asm ("insertd  %x[xt], %[xa], 7": [xt] VSXW(r) : [xa] "r"(a) );
  return r;
}

static FORCEINLINE uint64_t                     vec_movmskps_p9(__vector unsigned int v) {
  uint64_t r;
  asm ( "extractwz  %x1, %0, 0x7": "=b"(r)       : VSXR(v) );
  return r;
}




// extract element

static FORCEINLINE float __extract_element0(__vector float v) { float res; asm ( "extractwz %x1, %0, 0x0": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE float __extract_element1(__vector float v) { float res; asm ( "extractwz %x1, %0, 0x1": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE float __extract_element2(__vector float v) { float res; asm ( "extractwz %x1, %0, 0x2": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE float __extract_element3(__vector float v) { float res; asm ( "extractwz %x1, %0, 0x3": "=b"(res) : VSXR(v) ); return res; }

static FORCEINLINE signed int __extract_element0(__vector signed int v) { signed int res; asm ( "extractwz %x1, %0, 0x0": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE signed int __extract_element1(__vector signed int v) { signed int res; asm ( "extractwz %x1, %0, 0x1": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE signed int __extract_element2(__vector signed int v) { signed int res; asm ( "extractwz %x1, %0, 0x2": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE signed int __extract_element3(__vector signed int v) { signed int res; asm ( "extractwz %x1, %0, 0x3": "=b"(res) : VSXR(v) ); return res; }

static FORCEINLINE unsigned int __extract_element0(__vector unsigned int v) { unsigned int res; asm ( "extractwz %x1, %0, 0x0": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE unsigned int __extract_element1(__vector unsigned int v) { unsigned int res; asm ( "extractwz %x1, %0, 0x1": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE unsigned int __extract_element2(__vector unsigned int v) { unsigned int res; asm ( "extractwz %x1, %0, 0x2": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE unsigned int __extract_element3(__vector unsigned int v) { unsigned int res; asm ( "extractwz %x1, %0, 0x3": "=b"(res) : VSXR(v) ); return res; }

static FORCEINLINE long long __extract_element0(__vector signed long long v) { long long res; asm ( "extractd %x1, %0, 0x0": "=b"(res) : VSXR(v) ); return res; }
static FORCEINLINE long long __extract_element1(__vector signed long long v) { long long res; asm ( "extractd %x1, %0, 0x1": "=b"(res) : VSXR(v) ); return res; }



//insert element
static FORCEINLINE __vector float __insert_element0(__vector float v, float val) {
  asm ( "insertwz    %x[xt], %[ra], 0x0": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector float __insert_element1(__vector float v, float val) {
  asm ( "insertwz    %x[xt], %[ra], 0x1": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector float __insert_element2(__vector float v, float val) {
  asm ( "insertwz    %x[xt], %[ra], 0x2": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector float __insert_element3(__vector float v, float val) {
  asm ( "insertwz    %x[xt], %[ra], 0x3": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}

/////
static FORCEINLINE __vector signed int __insert_element0(__vector signed int v, signed int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x0": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector signed int __insert_element1(__vector signed int v, signed int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x1": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector signed int __insert_element2(__vector signed int v, signed int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x2": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector signed int __insert_element3(__vector signed int v, signed int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x3": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}

/////
static FORCEINLINE __vector unsigned int __insert_element0(__vector unsigned int v,unsigned int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x0": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector unsigned int __insert_element1(__vector unsigned int v,unsigned int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x1": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector unsigned int __insert_element2(__vector unsigned int v,unsigned int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x2": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
static FORCEINLINE __vector unsigned int __insert_element3(__vector unsigned int v,unsigned int val) {
  asm ( "insertwz    %x[xt], %[ra], 0x3": [xt] VSXW(v) : [ra] "r"(val) ); return v; 
}
#endif

//
//// POWER8 intrinsics
//

#ifdef __POWER8

////////////////////////////////////////////////
//   int 64 math/logic operations
static FORCEINLINE __vector signed long long vec_add_p8(__vector signed long long a,
					                  __vector signed long long b){
  __vector signed long long register r;
  asm ("vaddudm %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector signed long long vec_sub_p8(__vector signed long long a,
                                                          __vector signed long long b){
  __vector signed long long register r;
  asm ("vsubudm %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}


static FORCEINLINE __vector signed long long vec_sld_p8(__vector signed long long a,
                                                          __vector signed long long b){
  __vector signed long long register r;
  asm ("vsld %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}


static FORCEINLINE __vector unsigned int       vec_pack_p8(__vector signed long long a,
					                   __vector signed long long b){
  __vector unsigned int  register r;
  asm ("vpkuhum %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_unpackh_p8(__vector unsigned int a){
  __vector unsigned long long register r;
  asm ("vupkhsw %[xt], %[xa]"        : [xt] "=v"(r)  : [xa] "v"(a)    );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_unpackl_p8(__vector unsigned int a){
  __vector unsigned long long register r;
  asm ("vupklsw %[xt], %[xa]"        : [xt] "=v"(r)  : [xa] "v"(a)    );
  return r;
}

static FORCEINLINE __vector signed long long vec_cmpeq_p8(__vector signed long long a,
						            __vector signed long long b){
  __vector signed long long register r;
  asm ("vcmpequd %[xt], %[xa], %[xb]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_sel_p8(__vector unsigned long long a,
					                  __vector unsigned long long b,
					                  __vector unsigned long long m /*mask*/){
  __vector unsigned long long register r;
  asm ("vsel %[xt], %[xa], %[xb], %[xc]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b), [xc] "v"(m));
  return r;
}

static FORCEINLINE __vector double vec_sel_p8(__vector double a,__vector double b, __vector double m){
  __vector double register r;
  asm ("vsel %[xt], %[xa], %[xb], %[xc]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b), [xc] "v"(m));
  return r;
}

static FORCEINLINE __vector signed int   vec_smear_p8(signed int a){
  __vector signed int register r, t;
  asm ("mtvsrwz %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned int vec_smear_p8(unsigned int a){
  __vector unsigned int register r, t;
  asm ("mtvsrwz %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector float        vec_smear_p8(float a){
  __vector float register r, t;
  asm ("xscvdpspn %x[xt], %[xa]"    : [xt] VSXW(t) : [xa] "f"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned long long  vec_smear_i64_p8(long long a){
  __vector unsigned long long register r, t;
  asm ("mtvsrd  %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltd %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector float               vec_smear_float_p8(float *ptr){
  __vector float register r, t;
  //asm ("lxsiwzx %x[xt], 0, %[xb]"   : [xt] VSXW(t) : [xb] "b"(ptr): "memory");
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(t) : "Z"(*ptr));
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned int        vec_smear_i32_p8(unsigned int *ptr){
  __vector unsigned int register r, t;
  //asm ("lxsiwzx %x[xt], 0, %[xb]"   : [xt] VSXW(t) : [xb] "b"(ptr): "memory");
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(t) : "Z"(*ptr));
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}


static FORCEINLINE __vector unsigned int vec_mul_p8(__vector unsigned int a,__vector unsigned int b) {
  __vector unsigned int register r;
  asm ("vmuluwm %[xt], %[xa], %[xb]": [xt] "=v"(r)  : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

//P8 specific extract 
static FORCEINLINE uint64_t vec_extract_l(__vector int a) {
  uint64_t register r;
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(a) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_r(__vector int a) {
  uint64_t register r;
  __vector int register tmp;
  asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(a), [xb] "v"(a),[im] "i"(8) );
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(tmp) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_l(__vector float a) {
  uint64_t register r;
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(a) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_r(__vector float a) {
  uint64_t register r;
  __vector int register tmp;
  asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(a), [xb] "v"(a),[im] "i"(8) );
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(tmp) );
  return r;
}

#define GATHER_WORD_OFF32_P8(TYPE) \
static FORCEINLINE __vector TYPE vec_gather_p8(TYPE *ptr0, \
					       TYPE *ptr1, \
					       TYPE *ptr2, \
					       TYPE *ptr3){ \
  __vector TYPE register r0,r1,r2,r3;			    \
  __vector TYPE register t0,t1;				    \
  __vector TYPE register r;						\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r0) : "Z"(*ptr0));	\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r1) : "Z"(*ptr1));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(t0) : [xa] VSXR(r0), [xb] VSXR(r1) ); \
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r2) : "Z"(*ptr2));	\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r3) : "Z"(*ptr3));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(t1) : [xa] VSXR(r2), [xb] VSXR(r3) ); \
  asm ("vpkudum %[xt], %[xa], %[xb]"         : [xt] "=v"(r) : [xa] "v"(t0), [xb] "v"(t1) ); \
  return r;								\
}									

GATHER_WORD_OFF32_P8(float)
GATHER_WORD_OFF32_P8(signed int)
GATHER_WORD_OFF32_P8(unsigned int)



#define GATHER_D_WORD_OFF32_P8(TYPE) \
static FORCEINLINE __vector TYPE vec_gather_p8(TYPE *ptr0, \
					       TYPE *ptr1){	    \
  __vector TYPE register r0,r1;			    \
  __vector TYPE register r;						\
  asm ("lxvdsx %x[xt],%y1"         : [xt] VSXW(r0) : "Z"(*ptr0));	\
  asm ("lxvdsx %x[xt],%y1"         : [xt] VSXW(r1) : "Z"(*ptr1));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(r) : [xa] VSXR(r0), [xb] VSXR(r1) ); \
  return r;								\
}									

GATHER_D_WORD_OFF32_P8(double)
GATHER_D_WORD_OFF32_P8(signed long)

//POWER 8 Scatter Intrinsics

#define SCATTER_WORD_OFF32_P8(TYPE,IMM)					\
void vec_scatter_step_##IMM(TYPE* ptr0, __vector TYPE val){ \
 __vector TYPE register tmp;							\
 asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(val), [xb] "v"(val),[im] "i"(IMM) ); \
 asm ("stxsiwx %x[xt],%y1"         : : [xt] VSXR(tmp), "Z"(*ptr0));	\
}

#define SCATTER_WORD_OFF32_Z_P8(TYPE)					\
void vec_scatter_step_0(TYPE* ptr0, __vector TYPE val){ \
 asm ("stxsiwx %x[xt],%y1"         : : [xt] VSXR(val), "Z"(*ptr0));	\
}

SCATTER_WORD_OFF32_Z_P8(float)
SCATTER_WORD_OFF32_P8(float,4)
SCATTER_WORD_OFF32_P8(float,8)
SCATTER_WORD_OFF32_P8(float,12)

SCATTER_WORD_OFF32_Z_P8(signed int)
SCATTER_WORD_OFF32_P8(signed int,4)
SCATTER_WORD_OFF32_P8(signed int,8)
SCATTER_WORD_OFF32_P8(signed int,12)

SCATTER_WORD_OFF32_Z_P8(unsigned int)
SCATTER_WORD_OFF32_P8(unsigned int,4)
SCATTER_WORD_OFF32_P8(unsigned int,8)
SCATTER_WORD_OFF32_P8(unsigned int,12)

#define SCATTER_D_WORD_OFF32_P8(TYPE) \
void vec_scatter_step_8(TYPE* ptr0, __vector TYPE val){ \
 __vector TYPE tmp;							\
 asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(val), [xb] "v"(val),[im] "i"(8) ); \
 asm ("stxsdx %x[xt],%y1"         : : [xt] VSXR(tmp), "Z"(*ptr0));	\
}

#define SCATTER_D_WORD_OFF32_Z_P8(TYPE) \
void vec_scatter_step_0(TYPE* ptr0, __vector TYPE val){ \
 asm ("stxsdx %x[xt],%y1"         : : [xt] VSXR(val), "Z"(*ptr0));	\
}

SCATTER_D_WORD_OFF32_P8(double)
SCATTER_D_WORD_OFF32_P8(signed long)
SCATTER_D_WORD_OFF32_Z_P8(double)
SCATTER_D_WORD_OFF32_Z_P8(signed long)
#endif

/// already implemented in POWER7
//

static FORCEINLINE __vector float               vec_splat_p7(__vector float a, const int v){
  __vector float register r;
  asm ("xxspltw %x[xt], %x[xa],%[im] "  : [xt] VSXW(r) : [xa] VSXR(a), [im] "i"(v)    );
  return r;
}

static FORCEINLINE __vector signed int          vec_splat_p7(__vector signed int a, const int v){
  __vector signed int register r;
  asm ("xxspltw %x[xt], %x[xa],%[im] "  : [xt] VSXW(r) : [xa] VSXR(a), [im] "i"(v)    );
  return r;
}

static FORCEINLINE __vector double              vec_smear_p7(double a){
  __vector double register r;
  asm ("xxspltd %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] "f"(a)   );
  return r;
}

static FORCEINLINE __vector float vec_zero_p7(){
  __vector float register r;
  asm   ("vspltisw %[xt], 0": [xt] "=v"(r) );
  return r;
}

static FORCEINLINE __vector unsigned long long   vec_smear_i64_p7(unsigned long long *ptr) {
  __vector unsigned long long r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector long long   vec_smear_i64_p7(long long *ptr) {
  __vector long long r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector double               vec_smear_double_p7(double *ptr) {
  __vector double r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector double               vec_smear_const_double_p7(const double *ptr) {
  __vector double r;
  //asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  asm ("lxvdsx %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

static FORCEINLINE __vector unsigned long long   vec_smear_const_i64_p7(const long long *ptr) {
  __vector unsigned long long r;
  //asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  asm ("lxvdsx %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

static FORCEINLINE __vector float               vec_smear_const_float_p7(const __vector float *ptr) {
  __vector float r;
  asm ("lxvw4x %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

static FORCEINLINE __vector float vec_neg_p7(__vector float a) {
  __vector float register r;
  asm ("xvnegsp %x[xt], %x[xa]"    : [xt] VSXW(r) : [xa] VSXR(a));
  return r;
}


