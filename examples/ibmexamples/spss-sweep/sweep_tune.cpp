#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <timing.h>

#include <perfmeasure.h>


// the simdized version assumes P7 VSX

#ifdef __ALTIVEC__
#include <power_vsx4.h>
using namespace vsx;
#else
#include <generic4.h>
using namespace generic;
#endif


#define MATRIX_SIZE 1000

// an original sweep function for a triangular matrix
void tri_sweep_orig(double *fSym) 
{
  size_t    fNRows  = MATRIX_SIZE;
  size_t    Ipp, Iip, Ipj, Iij, p, i, j;
  double    App, Bpp, absBpp, Aip, Apj;

  for (p = 0; p < fNRows; p++)
    {
      Ipp = (p + 1) * (p + 2) / 2 - 1;
      App   = fSym[Ipp];

      Iij = -1;
      Iip = Ipp - p - 1;

      // The a(p,p) element
      Bpp = -1.0 / App;
      absBpp = fabs( Bpp );
      fSym[ Ipp ] = Bpp;

      for (i = 0; i < fNRows; i++ )
    {
      Iip++;

      if ( i == p )
        {
          // Skip the pivot row
          Iij += (p + 1);
        }
      else
        {
          // The a(i,p) elements, i != p
          if ( i > p )
        {
          Iip += (i - 1);
        }

          Aip = fSym[ Iip ];
          fSym[ Iip ] = absBpp * Aip;

          // The a(i,j) elements, i,j both != p
          if ( Bpp < 0.0 )
        {
          Aip = -Aip;
        }

          Ipj = Ipp - p - 1;

          for (j = 0; j <= i; j++ )
        {
          Iij++;
          Ipj++;

          // Skip the pivot column
          if ( j != p )
            {
              if ( j > p )
            {
              Ipj += (j - 1);
            }

              Apj = fSym[ Ipj ];
              fSym[ Iij ] += Aip * Apj;
            }
        }
        }
    }
    }
}

// a sweep function (without unrolling) for a triangular matrix
#ifdef __ALTIVEC__
void tri_sweep(double *fSym) {
  size_t fNRows = MATRIX_SIZE;
  size_t Ipp, Iip, Ipj, Iij, p, i, j;
  double App, Bpp, absBpp, Aip, Apj;

  for (p = 0; p < fNRows; p++) {
    Ipp = (p + 1) * (p + 2) / 2 - 1;
    App = fSym[Ipp];

    Iij = -1;
    Iip = Ipp - p - 1;

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[Ipp] = Bpp;

    for (i = 0; i < fNRows; i++) {
      Iip++;

      if (i == p) {
        // Skip the pivot row
        Iij += (p + 1);
      } else {
        // The a(i,p) elements, i != p
        if (i > p) {
          Iip += (i - 1);
        }

        Aip = fSym[Iip];
        fSym[Iip] = absBpp * Aip;

        // The a(i,j) elements, i,j both != p
        if (Bpp < 0.0) {
          Aip = -Aip;
        }
        vector double vec_Aip = vec_splats(Aip);
        Ipj = Ipp - p - 1;

        if (i < p) {
          for (j = 0; j < ((i + 1) / 2) * 2; j += 2) {
            Ipj++;
            Iij++;
            vector double vec_Apj = *((vector double*) (fSym + Ipj));
            vector double vec_Aij = *((vector double*) (fSym + Iij));
            vec_Aij = vec_madd(vec_Aip, vec_Apj, vec_Aij);
            vec_vsx_st(vec_Aij, 0, (vector double*) (fSym + Iij));
            Ipj++;
            Iij++;
          }
          for (; j <= i; j++) {
            Iij++;
            Ipj++;
            Apj = fSym[Ipj];
            fSym[Iij] += Aip * Apj;
          }
        } else {
          for (j = 0; j < (p / 2) * 2; j += 2) {
            Ipj++;
            Iij++;
            vector double vec_Apj = *(vector double*) (fSym + Ipj);
            vector double vec_Aij = *(vector double*) (fSym + Iij);
            vec_Aij = vec_madd(vec_Aip, vec_Apj, vec_Aij);
            vec_vsx_st(vec_Aij, 0, (vector double*) (fSym + Iij));
            Ipj++;
            Iij++;
          }
          for (; j < p; j++) {
            Iij++;
            Ipj++;
            Apj = fSym[Ipj];
            fSym[Iij] += Aip * Apj;
          }
          j++;
          Iij++;
          Ipj++;
          if (p % 2) { // p is ODD
            for (; j < ((i + 1) / 2) * 2; j++) {
              Iij++;
              Ipj++;
              Ipj += (j - 1);
              vector double vec_Apj0 = vec_splats(fSym[Ipj]);
              j++;
              Ipj++;
              Ipj += (j - 1);
              vector double vec_Apj1 = vec_splats(fSym[Ipj]);
              vector double vec_Apj = vec_mergeh(vec_Apj0, vec_Apj1);
              vector double vec_Aij = *(vector double*) (fSym + Iij);
              vec_Aij = vec_madd(vec_Aip, vec_Apj, vec_Aij);
              vec_vsx_st(vec_Aij, 0, (vector double*) (fSym + Iij));
              Iij++;
            }
            for (; j <= i; j++) {
              Iij++;
              Ipj++;
              Ipj += (j - 1);
              Apj = fSym[Ipj];
              fSym[Iij] += Aip * Apj;
            }
          } else { //p is EVEN
            for (; j <= i && j == (p + 1); j++) {
              Iij++;
              Ipj++;
              Ipj += (j - 1);
              Apj = fSym[Ipj];
              fSym[Iij] += Aip * Apj;
            }
            for (; j < ((i + 1) / 2) * 2; j++) {
              Iij++;
              Ipj++;
              Ipj += (j - 1);
              vector double vec_Apj0 = vec_splats(fSym[Ipj]);
              j++;
              Ipj++;
              Ipj += (j - 1);
              vector double vec_Apj1 = vec_splats(fSym[Ipj]);
              vector double vec_Apj = vec_mergeh(vec_Apj0, vec_Apj1);
              vector double vec_Aij = *(vector double*) (fSym + Iij);
              vec_Aij = vec_madd(vec_Aip, vec_Apj, vec_Aij);
              vec_vsx_st(vec_Aij, 0, (vector double*) (fSym + Iij));
              Iij++;
            }
            for (; j <= i; j++) {
              Iij++;
              Ipj++;
              Ipj += (j - 1);
              Apj = fSym[Ipj];
              fSym[Iij] += Aip * Apj;
            }
          }
        }
      }
    }
  }
}
#endif


//matrix addr(row,col)
//#define D(i,j, v) printf("[%d,%d]%.20f\n", i, j, v);
//#define DV(i, j, v) \
//    printf("[%d,%d]%.20f\n", i, j, (&v)[0]); \
//    printf("[%d,%d]%.20f\n", i, j+1, (&v)[1]); \
//    printf("[%d,%d]%.20f\n", i, j+2, (&v)[2]); \
//    printf("[%d,%d]%.20f\n", i, j+3, (&v)[3]);
//#define DI(str, p, i, j) printf("%s,p:%d i:%d j:%d\n", #str, p, i, j);

#define D(i,j, v)
#define DV(i, j, v)
#define DI(str, p, i, j)

#define addr(row,col) (fNRows*(row) + (col))

void sweep(double *fSym) {
  int fNRows = MATRIX_SIZE;
  int p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Api0, Ai1p, Ai2p, Ai3p;

  for (p = 0; p < fNRows; p++) {
    DI(p1, p, 0, 0);
    App = fSym[addr(p,p)];

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[addr(p,p)] = Bpp; D(p,p, Bpp);

    for (i = 0; i < p; i++) {
      DI(pi0, p, i, 0);
      double Api0 = fSym[ addr(p,i)];

      fSym[ addr(p,i)] = absBpp * Api0; D(p,i, fSym[ addr(p,i)] );
      if (Bpp < 0.0)
        Api0 = -Api0;

      for (j = 0; j <= i; j++) {
        DI(pij0, p, i, j);
        Apj = fSym[ addr(p,j)];
        fSym[ addr(i+0,j)] += Api0 * Apj; D(i,j, fSym[ addr(i+0,j)]);
      }
    }
    i++;
    for (; i < fNRows; i++) {
      DI(pi1, p, i, 0);
      Aip = fSym[ addr(i,p)];
      fSym[ addr(i,p)] = absBpp * Aip; D(i, p, fSym[addr(i,p)]);

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Aip = -Aip;
      }

      for (j = 0; j < p; j++) {
        DI(pij1, p, i, j);
        Apj = fSym[ addr(p,j)];
        fSym[ addr(i,j)] += Aip * Apj; D(i, j, fSym[ addr(i,j)]);
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        DI(pij2, p, i, j);
        Apj = fSym[ addr(j,p)];
        fSym[ addr(i,j)] += Aip * Apj; D(i,j, fSym[ addr(i,j)] );
      }
    }
  }
}

const svec4_i32 base_off(0,1,2,3);

//#define PTR_LOADSTORE
//#define FMA

#ifdef PTR_LOADSTORE
#define LOAD(VTYPE, ptr) *(VTYPE*)(ptr)
#define STORE(VTYPE, val, ptr) *(VTYPE*)(ptr) = (val)
#else //func load
#define LOAD(VTYPE, ptr) VTYPE::load((VTYPE*)(ptr))
#define STORE(VTYPE, val, ptr) val.store((VTYPE*)(ptr))
#endif //PTR_LOAD

#ifdef FMA
#define MADD(c,a,b) svec_madd((a), (b), (c))
#else //No FMA
#define MADD(c,a,b) ((c) + (a) * (b))
#endif

#ifdef GATHER_SCATTER
#define GATHER svec4_d::gather_base_offsets
#define SCATTER(v, base, scale, off, mask) v.scatter_base_offsets(base, scale, off, mask)
#else //No GATHER
#define GATHER direct_gather
#define SCATTER(v, base, scale, off, mask) direct_scatter(v, base, scale, off, mask)
#endif

//direct gather

FORCEINLINE svec4_d direct_gather(double* base, int scale, svec4_i32 off, svec4_i1 mask) {
  double d0 = *(base + svec_extract(off, 0));
  double d1 = *(base + svec_extract(off, 1));
  double d2 = *(base + svec_extract(off, 2));
  double d3 = *(base + svec_extract(off, 3));
  return svec4_d(d0, d1, d2, d3);
}

FORCEINLINE void direct_scatter(svec4_d v, double* base, int scale, svec4_i32 off, svec4_i1 mask) {
  *(base + svec_extract(off, 0)) = svec_extract(v, 0);
  *(base + svec_extract(off, 1)) = svec_extract(v, 1);
  *(base + svec_extract(off, 2)) = svec_extract(v, 2);
  *(base + svec_extract(off, 3)) = svec_extract(v, 3);
}



//generic simd 4 based approach
void sweep_simd(double *fSym) {
  int p, i, j;
  int fNRows = MATRIX_SIZE;

  for (p = 0; p < fNRows; p++) {
    DI(p0, p, 0, 0);
    double App = fSym[addr(p,p)];

    // The a(p,p) element
    double Bpp = -1.0 / App;
    double absBpp = fabs(Bpp);
    fSym[addr(p,p)] = Bpp; D(p,p,fSym[addr(p,p)]);

    for (i = 0; i < p - 3; i += 4) { //each time calc 4 doubles
      DI(pi0, p, i, 0);
      svec4_d vec_Api = LOAD(svec4_d, &fSym[addr(p,i)]); //do load
      STORE(svec4_d, (absBpp * vec_Api), &fSym[addr(p,i)]);
      DV(p,i, fSym[addr(p,i)]);

      vec_Api = (Bpp / absBpp) * vec_Api; //if (Bpp < 0.0) Api = -Api;
//      if(Bpp < 0.0) { vec_Api = -vec_Api;}

      svec4_d vec_Api0 = vec_Api.broadcast(0);
      svec4_d vec_Api1 = vec_Api.broadcast(1);
      svec4_d vec_Api2 = vec_Api.broadcast(2);
      svec4_d vec_Api3 = vec_Api.broadcast(3);

      for (j = 0; j < i; j += 4) { //note as i%4==0, j also has the property
        DI(pij0, p, i, j);
        svec4_d vec_Apj = LOAD(svec4_d, &fSym[addr(p,j)]);
        svec4_d vec_Ai0j = LOAD(svec4_d, &fSym[addr(i,j)]);
        svec4_d vec_Ai1j = LOAD(svec4_d, &fSym[addr(i+1,j)]);
        svec4_d vec_Ai2j = LOAD(svec4_d, &fSym[addr(i+2,j)]);
        svec4_d vec_Ai3j = LOAD(svec4_d, &fSym[addr(i+3,j)]);

        vec_Ai0j = MADD(vec_Ai0j, vec_Api0, vec_Apj);
        vec_Ai1j = MADD(vec_Ai1j, vec_Api1, vec_Apj);
        vec_Ai2j = MADD(vec_Ai2j, vec_Api2, vec_Apj);
        vec_Ai3j = MADD(vec_Ai3j, vec_Api3, vec_Apj);

        STORE(svec4_d, vec_Ai0j, &fSym[addr(i,j)]);
        DV(i,j, fSym[addr(i,j)]);
        STORE(svec4_d, vec_Ai1j, &fSym[addr(i+1,j)]);
        DV(i+1,j, fSym[addr(i+1,j)]);
        STORE(svec4_d, vec_Ai2j, &fSym[addr(i+2,j)]);
        DV(i+2,j, fSym[addr(i+2,j)]);
        STORE(svec4_d, vec_Ai3j, &fSym[addr(i+3,j)]);
        DV(i+3,j, fSym[addr(i+3,j)]);
      }

      DI(pi1, p, i, j);
      //then the left part. j = i case.
      svec4_d vec_Apj = LOAD(svec4_d, &fSym[addr(p,j)]);

      fSym[addr(i,j)] += vec_Api[0] * vec_Apj[0]; D(i,j, fSym[addr(i,j)]);
      fSym[addr(i+1,j)] += vec_Api[1] * vec_Apj[0]; D(i+1,j, fSym[addr(i+1,j)]);
      fSym[addr(i+1,j+1)] += vec_Api[1] * vec_Apj[1]; D(i+1,j+1, fSym[addr(i+1,j+1)]);
      fSym[addr(i+2,j)] += vec_Api[2] * vec_Apj[0]; D(i+2,j, fSym[addr(i+2,j)]);
      fSym[addr(i+2,j+1)] += vec_Api[2] * vec_Apj[1]; D(i+2,j+1, fSym[addr(i+2,j+1)]);
      fSym[addr(i+2,j+2)] += vec_Api[2] * vec_Apj[2]; D(i+2,j+2, fSym[addr(i+2,j+2)]);
      //the last one could be in SIMD
      svec4_d vec_Ai3j = LOAD(svec4_d, &fSym[addr(i+3,j)]);
      vec_Ai3j = MADD(vec_Ai3j, vec_Api3, vec_Apj);
      STORE(svec4_d, vec_Ai3j, &fSym[addr(i+3,j)]);
      DV(i+3, j, fSym[addr(i+3,j)]);
    }

    //now i < p loop, rest, for both j and i
    for (; i < p; i++) {
      DI(pi2, p, i, 0);
      double Api = fSym[addr(p,i)];

      fSym[addr(p,i)] = absBpp * Api; D(p, i, fSym[addr(p,i)]);
      if (Bpp < 0.0) {  Api = -Api; }

      for (j = 0; j <= i; j++) {
        DI(pij2, p, i, j);
        double Apj = fSym[addr(p,j)];
        fSym[addr(i,j)] += Api * Apj; D(i,j, fSym[addr(i,j)]);
      }
    }
    i++; //skip the p
    for (; i < fNRows - 3; i += 4) {
      DI(pi3, p, i, 0);
      svec4_i32 off_ip = fNRows * (i + base_off) + p;
      svec4_d vec_Aip = GATHER(fSym, sizeof(double), off_ip, svec4_i1(1));
      SCATTER((absBpp * vec_Aip), fSym, sizeof(double), off_ip, svec4_i1(1));
      D(i,p,fSym[addr(i,p)]); D(i+1,p,fSym[addr(i+1,p)]);
      D(i+2,p,fSym[addr(i+2,p)]); D(i+3,p,fSym[addr(i+3,p)]);

      vec_Aip = (Bpp / absBpp) * vec_Aip; //if (Bpp < 0.0) Api = -Api;
//      if(Bpp < 0.0) { vec_Aip = -vec_Aip;}

      svec4_d vec_Aip0 = vec_Aip.broadcast(0);
      svec4_d vec_Aip1 = vec_Aip.broadcast(1);
      svec4_d vec_Aip2 = vec_Aip.broadcast(2);
      svec4_d vec_Aip3 = vec_Aip.broadcast(3);
      //part 1, j = 0..p, with step 4, always vec_i4
      for(j = 0; j < p - 3; j+=4) {
        DI(pij3, p, i, j);
        svec4_d vec_Apj = LOAD(svec4_d, &fSym[addr(p,j)]);
        //accumulate
        svec4_d vec_Ai0j = LOAD(svec4_d, &fSym[addr(i,j)]);
        svec4_d vec_Ai1j = LOAD(svec4_d, &fSym[addr(i+1,j)]);
        svec4_d vec_Ai2j = LOAD(svec4_d, &fSym[addr(i+2,j)]);
        svec4_d vec_Ai3j = LOAD(svec4_d, &fSym[addr(i+3,j)]);

        vec_Ai0j = MADD(vec_Ai0j, vec_Aip0, vec_Apj);
        vec_Ai1j = MADD(vec_Ai1j, vec_Aip1, vec_Apj);
        vec_Ai2j = MADD(vec_Ai2j, vec_Aip2, vec_Apj);
        vec_Ai3j = MADD(vec_Ai3j, vec_Aip3, vec_Apj);

        STORE(svec4_d, vec_Ai0j, &fSym[addr(i,j)]); DV(i,j, fSym[addr(i,j)]);
        STORE(svec4_d, vec_Ai1j, &fSym[addr(i+1,j)]); DV(i+1,j, fSym[addr(i+1,j)]);
        STORE(svec4_d, vec_Ai2j, &fSym[addr(i+2,j)]); DV(i+2,j, fSym[addr(i+2,j)]);
        STORE(svec4_d, vec_Ai3j, &fSym[addr(i+3,j)]); DV(i+3,j, fSym[addr(i+3,j)]);
      }

      //part 2, j < p rest, always vec_i4
      for(; j < p; j++) {
        DI(pij4, p, i, j);
        double Apj = fSym[ addr(p,j)];
        svec4_i32 off_ij = fNRows * (i + base_off) + j;
        svec4_d vec_Aij = GATHER(fSym, sizeof(double), off_ij, svec4_i1(1));
        vec_Aij = vec_Aij + vec_Aip * Apj;
        SCATTER(vec_Aij, fSym, sizeof(double), off_ij, svec4_i1(1));
        D(i,j,fSym[addr(i,j)]); D(i+1,j,fSym[addr(i+1,j)]);
        D(i+2,j,fSym[addr(i+2,j)]); D(i+3,j,fSym[addr(i+3,p)]);
      }

      j++;

      //part 1, j < i with step 4
      vec_Aip0 = vec_Aip.broadcast(0);
      vec_Aip1 = vec_Aip.broadcast(1);
      vec_Aip2 = vec_Aip.broadcast(2);
      vec_Aip3 = vec_Aip.broadcast(3);
      for(; j < i; j+=4) {
        DI(pij5, p, i, j);
        svec4_i32 off_jp = fNRows * (j + base_off) + p;
        svec4_d vec_Apj = GATHER(fSym, sizeof(double), off_jp, svec4_i1(1));

         //accumulate
        svec4_d vec_Ai0j = LOAD(svec4_d, &fSym[addr(i,j)]);
        svec4_d vec_Ai1j = LOAD(svec4_d, &fSym[addr(i+1,j)]);
        svec4_d vec_Ai2j = LOAD(svec4_d, &fSym[addr(i+2,j)]);
        svec4_d vec_Ai3j = LOAD(svec4_d, &fSym[addr(i+3,j)]);

        vec_Ai0j = MADD(vec_Ai0j, vec_Aip0, vec_Apj);
        vec_Ai1j = MADD(vec_Ai1j, vec_Aip1, vec_Apj);
        vec_Ai2j = MADD(vec_Ai2j, vec_Aip2, vec_Apj);
        vec_Ai3j = MADD(vec_Ai3j, vec_Aip3, vec_Apj);

        STORE(svec4_d, vec_Ai0j, &fSym[addr(i,j)]);DV(i,j, fSym[addr(i,j)])
        STORE(svec4_d, vec_Ai1j, &fSym[addr(i+1,j)]);DV(i+1,j, fSym[addr(i+1,j)])
        STORE(svec4_d, vec_Ai2j, &fSym[addr(i+2,j)]);DV(i+2,j, fSym[addr(i+2,j)])
        STORE(svec4_d, vec_Ai3j, &fSym[addr(i+3,j)]);DV(i+3,j, fSym[addr(i+3,j)]);
      }


      //part 2, j <= i, rest, always vec_i4, i from i to i+3
      for(; j <=i; j++) {
        DI(pij6, p, i, j);
        double Ajp = fSym[ addr(j,p)];
        svec4_i32 off_ij = fNRows * (i + base_off) + j;
        svec4_d vec_Aij = GATHER(fSym, sizeof(double), off_ij, svec4_i1(1));
        vec_Aij = MADD(vec_Aij, vec_Aip, Ajp);
        SCATTER(vec_Aij, fSym, sizeof(double), off_ij, svec4_i1(1));
        D(i,j,fSym[addr(i,j)]); D(i+1,j,fSym[addr(i+1,j)]);
        D(i+2,j,fSym[addr(i+2,j)]); D(i+3,j,fSym[addr(i+3,j)]);
      }
      //now th rest
      DI(pij7, p, i, j);
      double Ajp = fSym[ addr(j,p)];
      fSym[ addr(i+1,j)] += vec_Aip[1] * Ajp; D(i+1, j, fSym[ addr(i+1,j)]);
      fSym[ addr(i+2,j)] += vec_Aip[2] * Ajp; D(i+2, j, fSym[ addr(i+2,j)]);
      fSym[ addr(i+3,j)] += vec_Aip[3] * Ajp; D(i+3, j, fSym[ addr(i+3,j)]);
      j++;
      Ajp = fSym[ addr(j,p)];
      fSym[ addr(i+2,j)] += vec_Aip[2] * Ajp; D(i+2, j, fSym[ addr(i+2,j)]);
      fSym[ addr(i+3,j)] += vec_Aip[3] * Ajp; D(i+3, j, fSym[ addr(i+3,j)]);
      j++;
      Ajp = fSym[ addr(j,p)];
      fSym[ addr(i+3,j)] += vec_Aip[3] * Ajp; D(i+3, j, fSym[ addr(i+3,j)]);
    } // p < i < fNows - 3

    //the left i rows
    for (; i < fNRows; i++) {
      DI(pi4, p, i, 0);
      double Aip = fSym[addr(i,p)];
      fSym[addr(i,p)] = absBpp * Aip; D(i, p, fSym[addr(i,p)]);

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) { Aip = -Aip; }

      for (j = 0; j < p; j++) {
        double Apj = fSym[addr(p,j)];
        fSym[addr(i,j)] += Aip * Apj; D(i, j, fSym[addr(i,j)]);
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        double Apj = fSym[addr(j,p)];
        fSym[addr(i,j)] += Aip * Apj; D(i, j, fSym[addr(i,j)]);
      }
    }
  }
}

void sweep_simd_nogs(double *fSym) {
  int fNRows = MATRIX_SIZE;
  int Ipp, Iip, Ipj, Iij, p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Ai0p, Ai1p, Ai2p, Ai3p;

  for (p = 0; p < fNRows; p++) {
    Ipp = p * fNRows + p;
    App = fSym[Ipp];

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[Ipp] = Bpp;

    for (i = 0; i < p - 3; i += 4) { //each time calc 4 doubles

      svec4_d vec_Api = svec4_d::load((svec4_d*)&fSym[addr(p,i)]); //do load
      (absBpp * vec_Api).store((svec4_d*)&fSym[addr(p,i)]);

      vec_Api = (Bpp / absBpp) * vec_Api; //if (Bpp < 0.0) Api = -Api;

      svec4_d vec_Api0 = vec_Api.broadcast(0);
      svec4_d vec_Api1 = vec_Api.broadcast(1);
      svec4_d vec_Api2 = vec_Api.broadcast(2);
      svec4_d vec_Api3 = vec_Api.broadcast(3);

      for (j = 0; j < i; j += 4) { //note as i%4==0, j also has the property
        svec4_d vec_Apj = svec4_d::load((svec4_d*)&fSym[addr(p,j)]);
        svec4_d vec_Ai0j = svec4_d::load((svec4_d*)&fSym[addr(i,j)]);
        svec4_d vec_Ai1j = svec4_d::load((svec4_d*)&fSym[addr(i+1,j)]);
        svec4_d vec_Ai2j = svec4_d::load((svec4_d*)&fSym[addr(i+2,j)]);
        svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);

        vec_Ai0j = vec_Ai0j + vec_Api0 * vec_Apj;
        vec_Ai1j = vec_Ai1j + vec_Api1 * vec_Apj;
        vec_Ai2j = vec_Ai2j + vec_Api2 * vec_Apj;
        vec_Ai3j = vec_Ai3j + vec_Api3 * vec_Apj;

        vec_Ai0j.store((svec4_d*)&fSym[addr(i,j)]);
        vec_Ai1j.store((svec4_d*)&fSym[addr(i+1,j)]);
        vec_Ai2j.store((svec4_d*)&fSym[addr(i+2,j)]);
        vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
      }

      //then the left part. j = i case.
      svec4_d vec_Apj = svec4_d::load((svec4_d*)&fSym[addr(p,j)]);

      fSym[addr(i,j)] += vec_Api[0] * vec_Apj[0];
      fSym[addr(i+1,j)] += vec_Api[1] * vec_Apj[0];
      fSym[addr(i+1,j+1)] += vec_Api[1] * vec_Apj[1];
      fSym[addr(i+2,j)] += vec_Api[2] * vec_Apj[0];
      fSym[addr(i+2,j+1)] += vec_Api[2] * vec_Apj[1];
      fSym[addr(i+2,j+2)] += vec_Api[2] * vec_Apj[2];
      //the last one could be in SIMD
      svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);
      vec_Ai3j = vec_Ai3j + vec_Api3 * vec_Apj;
      vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
    }

    //now i < p loop, rest, for both j and i
    for (; i < p; i++) {
      double Api = fSym[addr(p,i)];

      fSym[addr(p,i)] = absBpp * Api;
      if (Bpp < 0.0) {  Api = -Api; }

      for (j = 0; j <= i; j++) {
        double Apj = fSym[addr(p,j)];
        fSym[addr(i,j)] += Api * Apj;
      }
    }
    i++;
    for (; i < fNRows - 3; i += 4) {



      Ii0p = fNRows * (i + 0) + p;
      Ii1p = fNRows * (i + 1) + p;
      Ii2p = fNRows * (i + 2) + p;
      Ii3p = fNRows * (i + 3) + p;

      double Ai0p = fSym[Ii0p];
      double Ai1p = fSym[Ii1p];
      double Ai2p = fSym[Ii2p];
      double Ai3p = fSym[Ii3p];

      fSym[Ii0p] = absBpp * Ai0p;
      fSym[Ii1p] = absBpp * Ai1p;
      fSym[Ii2p] = absBpp * Ai2p;
      fSym[Ii3p] = absBpp * Ai3p;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Ai0p = -Ai0p;
        Ai1p = -Ai1p;
        Ai2p = -Ai2p;
        Ai3p = -Ai3p;
      }
      svec4_d vec_Aip0_(Ai0p);
      svec4_d vec_Aip1_(Ai1p);
      svec4_d vec_Aip2_(Ai2p);
      svec4_d vec_Aip3_(Ai3p);

      if (p % 2) { // p is ODD
        j = 0;
        for (; j < p - 1 - 2; j += 4) {
          svec4_d vec_Apj = svec4_d::load((svec4_d*)&fSym[addr(p,j)]);
          //accumulate
          svec4_d vec_Ai0j = svec4_d::load((svec4_d*)&fSym[addr(i,j)]);
          svec4_d vec_Ai1j = svec4_d::load((svec4_d*)&fSym[addr(i+1,j)]);
          svec4_d vec_Ai2j = svec4_d::load((svec4_d*)&fSym[addr(i+2,j)]);
          svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);

          vec_Ai0j = vec_Ai0j + vec_Aip0_ * vec_Apj;
          vec_Ai1j = vec_Ai1j + vec_Aip1_ * vec_Apj;
          vec_Ai2j = vec_Ai2j + vec_Aip2_ * vec_Apj;
          vec_Ai3j = vec_Ai3j + vec_Aip3_ * vec_Apj;

          vec_Ai0j.store((svec4_d*)&fSym[addr(i,j)]);
          vec_Ai1j.store((svec4_d*)&fSym[addr(i+1,j)]);
          vec_Ai2j.store((svec4_d*)&fSym[addr(i+2,j)]);
          vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
        }

        for (; j < p; j++) {
          Ipj = p * fNRows + j;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
        // Skip the pivot column
        j++;
        for (; j < ((i - 1) / 2) * 2; j += 4) {

          svec4_d vec_Apj = svec_gather_base_steps(fSym+addr(j,p), fNRows);
//          int Ipj0 = (j + 0) * fNRows + p;
//          int Ipj1 = (j + 1) * fNRows + p;
//
//#ifndef __SSE4_2__
//          vector double vec_Apj0 = vec_splats(fSym[Ipj0]);
//          vector double vec_Apj1 = vec_splats(fSym[Ipj1]);
//          vector double vec_Apj_0 = vec_mergeh(vec_Apj0, vec_Apj1);
//#else //intel
//          __m128d vec_Apj_0 = _mm_set_pd(fSym[Ipj1], fSym[Ipj0]);
//#endif
//
//          Ipj0 = (j + 2) * fNRows + p;
//          Ipj1 = (j + 3) * fNRows + p;
//#ifndef __SSE4_2__
//           vec_Apj0 = vec_splats(fSym[Ipj0]);
//           vec_Apj1 = vec_splats(fSym[Ipj1]);
//          vector double  vec_Apj_1 = vec_mergeh(vec_Apj0, vec_Apj1);
//#else
//          __m128d vec_Apj_1 = _mm_set_pd(fSym[Ipj1], fSym[Ipj0]);
//#endif
//          svec4_d vec_Apj(vec_Apj_0, vec_Apj_1);

           //accumulate
          svec4_d vec_Ai0j = svec4_d::load((svec4_d*)&fSym[addr(i,j)]);
          svec4_d vec_Ai1j = svec4_d::load((svec4_d*)&fSym[addr(i+1,j)]);
          svec4_d vec_Ai2j = svec4_d::load((svec4_d*)&fSym[addr(i+2,j)]);
          svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);

          vec_Ai0j = vec_Ai0j + vec_Aip0_ * vec_Apj;
          vec_Ai1j = vec_Ai1j + vec_Aip1_ * vec_Apj;
          vec_Ai2j = vec_Ai2j + vec_Aip2_ * vec_Apj;
          vec_Ai3j = vec_Ai3j + vec_Aip3_ * vec_Apj;

          vec_Ai0j.store((svec4_d*)&fSym[addr(i,j)]);
          vec_Ai1j.store((svec4_d*)&fSym[addr(i+1,j)]);
          vec_Ai2j.store((svec4_d*)&fSym[addr(i+2,j)]);
          vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
        }

        for (; j <= i; j++) {
          Ipj = j * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
      } else { // p is EVEN
        j = 0;
        for (; j < p - 2; j += 4) {

          svec4_d vec_Apj = svec4_d::load((svec4_d*)&fSym[addr(p,j)]);
           //accumulate
          svec4_d vec_Ai0j = svec4_d::load((svec4_d*)&fSym[addr(i,j)]);
          svec4_d vec_Ai1j = svec4_d::load((svec4_d*)&fSym[addr(i+1,j)]);
          svec4_d vec_Ai2j = svec4_d::load((svec4_d*)&fSym[addr(i+2,j)]);
          svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);

          vec_Ai0j = vec_Ai0j + vec_Aip0_ * vec_Apj;
          vec_Ai1j = vec_Ai1j + vec_Aip1_ * vec_Apj;
          vec_Ai2j = vec_Ai2j + vec_Aip2_ * vec_Apj;
          vec_Ai3j = vec_Ai3j + vec_Aip3_ * vec_Apj;

          vec_Ai0j.store((svec4_d*)&fSym[addr(i,j)]);
          vec_Ai1j.store((svec4_d*)&fSym[addr(i+1,j)]);
          vec_Ai2j.store((svec4_d*)&fSym[addr(i+2,j)]);
          vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
        }
        //use scalar to replace the missing part
        for(; j < p; j++) {
          Ipj = p * fNRows + j;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
        // Skip the pivot column
        j++;
        for (; j <= i && j == (p + 1); j++) {
          Ipj = (j + 0) * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
        for (; j < ((i - 1) / 2) * 2; j += 4) {
          svec4_d vec_Apj = svec_gather_base_steps(fSym+addr(j,p), fNRows);
//          int Ipj0 = (j + 0) * fNRows + p;
//          int Ipj1 = (j + 1) * fNRows + p;
//#ifndef __SSE4_2__
//          vector double vec_Apj0 = vec_splats(fSym[Ipj0]);
//          vector double vec_Apj1 = vec_splats(fSym[Ipj1]);
//          vector double vec_Apj_0 = vec_mergeh(vec_Apj0, vec_Apj1);
//#else //intel
//          __m128d vec_Apj_0 = _mm_set_pd(fSym[Ipj1], fSym[Ipj0]);
//#endif
//          Ipj0 = (j + 2) * fNRows + p;
//          Ipj1 = (j + 3) * fNRows + p;
//#ifndef __SSE4_2__
//           vec_Apj0 = vec_splats(fSym[Ipj0]);
//           vec_Apj1 = vec_splats(fSym[Ipj1]);
//          vector double  vec_Apj_1 = vec_mergeh(vec_Apj0, vec_Apj1);
//#else //intel
//          __m128d vec_Apj_1 = _mm_set_pd(fSym[Ipj1], fSym[Ipj0]);
//#endif
//          svec4_d vec_Apj(vec_Apj_0, vec_Apj_1);


           //accumulate
          svec4_d vec_Ai0j = svec4_d::load((svec4_d*)&fSym[addr(i,j)]);
          svec4_d vec_Ai1j = svec4_d::load((svec4_d*)&fSym[addr(i+1,j)]);
          svec4_d vec_Ai2j = svec4_d::load((svec4_d*)&fSym[addr(i+2,j)]);
          svec4_d vec_Ai3j = svec4_d::load((svec4_d*)&fSym[addr(i+3,j)]);

          vec_Ai0j = vec_Ai0j + vec_Aip0_ * vec_Apj;
          vec_Ai1j = vec_Ai1j + vec_Aip1_ * vec_Apj;
          vec_Ai2j = vec_Ai2j + vec_Aip2_ * vec_Apj;
          vec_Ai3j = vec_Ai3j + vec_Aip3_ * vec_Apj;


          vec_Ai0j.store((svec4_d*)&fSym[addr(i,j)]);
          vec_Ai1j.store((svec4_d*)&fSym[addr(i+1,j)]);
          vec_Ai2j.store((svec4_d*)&fSym[addr(i+2,j)]);
          vec_Ai3j.store((svec4_d*)&fSym[addr(i+3,j)]);
        }

        for (; j <= i; j++) {
          Ipj = j * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
      }
      Ipj = j * fNRows + p;
      Ii1j = (i + 1) * fNRows + j;
      Ii2j = (i + 2) * fNRows + j;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii1j] += Ai1p * Apj;
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = j * fNRows + p;
      Ii2j = (i + 2) * fNRows + j;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = j * fNRows + p;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii3j] += Ai3p * Apj;

    }
    for (; i < fNRows; i++) {
      Iip = i * fNRows + p;

      Aip = fSym[Iip];
      fSym[Iip] = absBpp * Aip;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Aip = -Aip;
      }

      for (j = 0; j < p; j++) {
        Iij = i * fNRows + j;
        Ipj = p * fNRows + j;

        Apj = fSym[Ipj];
        fSym[Iij] += Aip * Apj;
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        Iij = i * fNRows + j;
        Ipj = j * fNRows + p;

        Apj = fSym[Ipj];
        fSym[Iij] += Aip * Apj;
      }
    }
  }
}

#ifdef __ALTIVEC__
//original intrinsics based approach
void sweep_unrolled_simd(double *fSym) {
  int fNRows = MATRIX_SIZE;
  int Ipp, Iip, Ipj, Iij, p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Ai0p, Ai1p, Ai2p, Ai3p;

  for (p = 0; p < fNRows; p++) {
    Ipp = p * fNRows + p;
    App = fSym[Ipp];

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[Ipp] = Bpp;
    i = 0;

    for (i = 0; i < p - 3; i += 4) {
      Iip = p * fNRows + i;

#if 0
      double Ai0p = fSym[ Iip+0 ];
      double Ai1p = fSym[ Iip+1 ];
      double Ai2p = fSym[ Iip+2 ];
      double Ai3p = fSym[ Iip+3 ];

      fSym[ Iip+0 ] = absBpp * Ai0p;
      fSym[ Iip+1 ] = absBpp * Ai1p;
      fSym[ Iip+2 ] = absBpp * Ai2p;
      fSym[ Iip+3 ] = absBpp * Ai3p;
#endif
      vector double vec_Ai01p = *(vector double*) (fSym + Iip);
      vector double vec_Ai23p = *(vector double*) (fSym + Iip + 2);
      vector double vec_Ai0p = vec_mergeh(vec_Ai01p, vec_Ai01p);
      vector double vec_Ai1p = vec_mergel(vec_Ai01p, vec_Ai01p);
      vector double vec_Ai2p = vec_mergeh(vec_Ai23p, vec_Ai23p);
      vector double vec_Ai3p = vec_mergel(vec_Ai23p, vec_Ai23p);

      vector double vec_absBpp = vec_splats(absBpp);
      vec_Ai01p = vec_mul(vec_absBpp, vec_Ai01p);
      vec_Ai23p = vec_mul(vec_absBpp, vec_Ai23p);
      vec_vsx_st(vec_Ai01p, 0, (vector double*) (fSym + Iip));
      vec_vsx_st(vec_Ai23p, 0, (vector double*) (fSym + Iip + 2));

      double signBpp = Bpp / absBpp;
      vector double vec_signBpp = vec_splats(signBpp);
      vec_Ai0p = vec_mul(vec_signBpp, vec_Ai0p);
      vec_Ai1p = vec_mul(vec_signBpp, vec_Ai1p);
      vec_Ai2p = vec_mul(vec_signBpp, vec_Ai2p);
      vec_Ai3p = vec_mul(vec_signBpp, vec_Ai3p);
#if 0
      if ( Bpp < 0.0 )
      {
        Ai0p = -Ai0p;
        Ai1p = -Ai1p;
        Ai2p = -Ai2p;
        Ai3p = -Ai3p;
      }
#endif
      for (j = 0; j < (i / 2) * 2; j += 2) {
        Ipj = p * fNRows + j;
        Ii0j = fNRows * (i + 0) + j;
        Ii1j = fNRows * (i + 1) + j;
        Ii2j = fNRows * (i + 2) + j;
        Ii3j = fNRows * (i + 3) + j;

        vector double vec_Apj = *(vector double*) (fSym + Ipj);
        vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
        vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
        vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
        vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);

        vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
        vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
        vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
        vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

        vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
        vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
        vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
        vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
#if 0
        Apj = fSym[ Ipj ];
        fSym[ Ii0j ] += Ai0p * Apj;
        fSym[ Ii1j ] += Ai1p * Apj;
        fSym[ Ii2j ] += Ai2p * Apj;
        fSym[ Ii3j ] += Ai3p * Apj;
#endif
      }

      Ai0p = vec_extract(vec_Ai0p, 0);
      Ai1p = vec_extract(vec_Ai1p, 0);
      Ai2p = vec_extract(vec_Ai2p, 0);
      Ai3p = vec_extract(vec_Ai3p, 0);

      for (; j <= i; j++) {
        Ipj = p * fNRows + j;
        Ii0j = fNRows * (i + 0) + j;
        Ii1j = fNRows * (i + 1) + j;
        Ii2j = fNRows * (i + 2) + j;
        Ii3j = fNRows * (i + 3) + j;

        Apj = fSym[Ipj];
        fSym[Ii0j] += Ai0p * Apj;
        fSym[Ii1j] += Ai1p * Apj;
        fSym[Ii2j] += Ai2p * Apj;
        fSym[Ii3j] += Ai3p * Apj;
      }

      Ipj = p * fNRows + j;
      Apj = fSym[Ipj];
      Ii1j = fNRows * (i + 1) + j;
      Ii2j = fNRows * (i + 2) + j;
      Ii3j = fNRows * (i + 3) + j;
      fSym[Ii1j] += Ai1p * Apj;
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = p * fNRows + j;
      Apj = fSym[Ipj];
      Ii2j = fNRows * (i + 2) + j;
      Ii3j = fNRows * (i + 3) + j;
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = p * fNRows + j;
      Apj = fSym[Ipj];
      Ii3j = fNRows * (i + 3) + j;
      fSym[Ii3j] += Ai3p * Apj;
    }

    for (; i < p; i++) {
      Iip = p * fNRows + i;

      double Ai0p = fSym[Iip];

      fSym[Iip] = absBpp * Ai0p;
      if (Bpp < 0.0)
        Ai0p = -Ai0p;

      for (j = 0; j <= i; j++) {
        Ipj = p * fNRows + j;
        Apj = fSym[Ipj];
        Ii0j = fNRows * (i + 0) + j;
        fSym[Ii0j] += Ai0p * Apj;
      }
    }
    i++;
    for (; i < fNRows - 3; i += 4) {
      //Iip = i * fNRows + p;
      Ii0p = fNRows * (i + 0) + p;
      Ii1p = fNRows * (i + 1) + p;
      Ii2p = fNRows * (i + 2) + p;
      Ii3p = fNRows * (i + 3) + p;

      double Ai0p = fSym[Ii0p];
      double Ai1p = fSym[Ii1p];
      double Ai2p = fSym[Ii2p];
      double Ai3p = fSym[Ii3p];

      fSym[Ii0p] = absBpp * Ai0p;
      fSym[Ii1p] = absBpp * Ai1p;
      fSym[Ii2p] = absBpp * Ai2p;
      fSym[Ii3p] = absBpp * Ai3p;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Ai0p = -Ai0p;
        Ai1p = -Ai1p;
        Ai2p = -Ai2p;
        Ai3p = -Ai3p;
      }
      vector double vec_Ai0p = vec_splats(Ai0p);
      vector double vec_Ai1p = vec_splats(Ai1p);
      vector double vec_Ai2p = vec_splats(Ai2p);
      vector double vec_Ai3p = vec_splats(Ai3p);
      if (p % 2) { // p is ODD
        for (j = 0; j < p - 1; j += 2) {
          Ipj = p * fNRows + j;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          vector double vec_Apj = *(vector double*) (fSym + Ipj);
          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);

          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
#if 0
          Apj = fSym[ Ipj ];
          fSym[ Ii0j ] += Ai0p * Apj;
          fSym[ Ii1j ] += Ai1p * Apj;
          fSym[ Ii2j ] += Ai2p * Apj;
          fSym[ Ii3j ] += Ai3p * Apj;
#endif
        }
        for (; j < p; j++) {
          Ipj = p * fNRows + j;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
        // Skip the pivot column
        j++;
        for (; j < ((i + 1) / 2) * 2; j += 2) {
          int Ipj0 = (j + 0) * fNRows + p;
          int Ipj1 = (j + 1) * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          //vector double vec_Apj0 = vec_splats(fSym[ Ipj0 ]);
          //vector double vec_Apj1 = vec_splats(fSym[ Ipj1 ]);
          vector double vec_Apj0 = *(vector double*) (fSym + Ipj0 - 1);
          vector double vec_Apj1 = *(vector double*) (fSym + Ipj1 - 1);
          vector double vec_Apj = vec_mergel(vec_Apj0, vec_Apj1);

          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);

          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
#if 0
          Apj = fSym[ Ipj ];
          fSym[ Ii0j ] += Ai0p * Apj;
          fSym[ Ii1j ] += Ai1p * Apj;
          fSym[ Ii2j ] += Ai2p * Apj;
          fSym[ Ii3j ] += Ai3p * Apj;
#endif
        }
        for (; j <= i; j++) {
          Ipj = j * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
      } else { // p is EVEN
        for (j = 0; j < p; j += 2) {
          Ipj = p * fNRows + j;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          vector double vec_Apj = *(vector double*) (fSym + Ipj);
          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);

          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
#if 0
          Apj = fSym[ Ipj ];
          fSym[ Ii0j ] += Ai0p * Apj;
          fSym[ Ii1j ] += Ai1p * Apj;
          fSym[ Ii2j ] += Ai2p * Apj;
          fSym[ Ii3j ] += Ai3p * Apj;
#endif
        }
        // Skip the pivot column
        j++;
        for (; j <= i && j == (p + 1); j++) {
          Ipj = (j + 0) * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
        for (; j < ((i + 1) / 2) * 2; j += 2) {
          int Ipj0 = (j + 0) * fNRows + p;
          int Ipj1 = (j + 1) * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          vector double vec_Apj0 = vec_splats(fSym[Ipj0]);
          vector double vec_Apj1 = vec_splats(fSym[Ipj1]);
          vector double vec_Apj = vec_mergeh(vec_Apj0, vec_Apj1);

          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);

          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
#if 0
          Apj = fSym[ Ipj ];
          fSym[ Ii0j ] += Ai0p * Apj;
          fSym[ Ii1j ] += Ai1p * Apj;
          fSym[ Ii2j ] += Ai2p * Apj;
          fSym[ Ii3j ] += Ai3p * Apj;
#endif
        }
        for (; j <= i; j++) {
          Ipj = j * fNRows + p;
          Ii0j = (i + 0) * fNRows + j;
          Ii1j = (i + 1) * fNRows + j;
          Ii2j = (i + 2) * fNRows + j;
          Ii3j = (i + 3) * fNRows + j;

          Apj = fSym[Ipj];
          fSym[Ii0j] += Ai0p * Apj;
          fSym[Ii1j] += Ai1p * Apj;
          fSym[Ii2j] += Ai2p * Apj;
          fSym[Ii3j] += Ai3p * Apj;
        }
      }
      Ipj = j * fNRows + p;
      Ii1j = (i + 1) * fNRows + j;
      Ii2j = (i + 2) * fNRows + j;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii1j] += Ai1p * Apj;
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = j * fNRows + p;
      Ii2j = (i + 2) * fNRows + j;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii2j] += Ai2p * Apj;
      fSym[Ii3j] += Ai3p * Apj;
      j++;
      Ipj = j * fNRows + p;
      Ii3j = (i + 3) * fNRows + j;
      Apj = fSym[Ipj];
      fSym[Ii3j] += Ai3p * Apj;

    }
    for (; i < fNRows; i++) {
      Iip = i * fNRows + p;

      Aip = fSym[Iip];
      fSym[Iip] = absBpp * Aip;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Aip = -Aip;
      }

      for (j = 0; j < p; j++) {
        Iij = i * fNRows + j;
        Ipj = p * fNRows + j;

        Apj = fSym[Ipj];
        fSym[Iij] += Aip * Apj;
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        Iij = i * fNRows + j;
        Ipj = j * fNRows + p;

        Apj = fSym[Ipj];
        fSym[Iij] += Aip * Apj;
      }
    }
  }
}
#endif

void initTriMatrix(double *matrix, int COUNT) 
{
  int count = 1;
  for (; count <= COUNT; count++) 
    {
      *matrix++ = 1.0 / (double) count;
    }
}

void initMatrix(double *matrix, int size) {
  int i, j, count = 1;
  for (i = 0; i < size; i++)
    {
      for (j = 0; j <= i; j++)
    {
      matrix[i*size+j] = 1.0 / (double) count++;
    }
      for (; j < size; j++)
    {
      matrix[i*size+j] = 0.0;
    }
    }
}

int compareTriMatrix(double *m0, double *m1, int COUNT) 
{
  int i;
  for (i = 0; i < COUNT;i++) 
    {
      if (m0[i] != m1[i]) 
    {
      printf("i=%d m0=%f m1=%f\n", i, m0[i], m1[i]);
      return 0;
    }
    }
  return 1;
}

/**
 * Compare a normal matrix with a tri matrix
 * @param mat a square matrix
 * @param tri a matrix stored as tri matrix
 * @param size matrix size, one dimension
 * @return 1 equal, 0 not equal
 */
int compareMatrix(double *mat, double *tri, int size)
{

  double err = 0;
  int i, j, count = 0, num_errors = 0;
  for (i = 0; i < size; i++)
    {
      for (j = 0; j <= i; j++, count++)
    {
      int Iij = i * size + j;
      err += fabs(mat[Iij] - tri[count]);
      if (mat[Iij] != tri[count])
        {
          printf("i=%d j=%d Iij=%d mat=%.20f tri=%.20f\n",
             i, j, Iij, mat[Iij], tri[count]);
          num_errors++;
          //return 0;
        }
    }
      for (; j < size; j++)
    {
      int Iij = i * size + j;
         err += fabs(mat[Iij]);
      if (mat[Iij] != 0.0)
        {
          printf("i=%d j=%d Iij=%d mat=%f\n",
             i, j, Iij, mat[Iij]);
          num_errors++;
        }
    }
    }
//  if (num_errors) return 0;

  if (err == 0.0) { return 1;}
  else {
    printf("accumulate error =%f\n", err);
  }
  return 0;
}

#define NUM_ITERATIONS 1

int main(int argc, char *argv[])
{
  int count;
  printf("MATRIX_SIZE=%d,NUM_ITERATIONS=%d\n", MATRIX_SIZE, NUM_ITERATIONS);

  int TRI_COUNT = MATRIX_SIZE * (MATRIX_SIZE + 1) / 2;
  int COUNT     = MATRIX_SIZE * MATRIX_SIZE;

  //matrix1 stores org base result
  double *matrix1 = (double *)malloc((COUNT+1) * sizeof(double));
  if (((unsigned long)matrix1)%16) matrix1++; //make 16 align


  //matrix2 stores different versions of optimization
  double *matrix2 = (double *)malloc((COUNT+1) * sizeof(double));
  if (((unsigned long)matrix2)%16) matrix2++; //make 16 align
  HPM_PERF_CREATE;

  //run base.
  initTriMatrix(matrix1, TRI_COUNT);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    tri_sweep_orig(matrix1);
  }
  double org_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Tri-Org Version Time = %f seconds\n", org_t);
  

  //run tri simd
#ifdef __ALTIVEC__
  initTriMatrix(matrix2, TRI_COUNT);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    tri_sweep(matrix2);
  }
  double tri_simd_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Tri-Split SIMD Version Time = %f seconds\n", tri_simd_t);
  if (compareTriMatrix(matrix2, matrix1, TRI_COUNT)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }
#endif

  //run org-normal version
  initMatrix(matrix2, MATRIX_SIZE);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep(matrix2);
  }
  double normal_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Normal Scalar Version Time = %f seconds\n", normal_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }
  

  //run generic simd4 version
  initMatrix(matrix2, MATRIX_SIZE);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep_simd(matrix2);
  }
  double simd_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Generic SIMD svec4 Version Time = %f seconds\n", simd_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }

  //run generic simd4 version
  initMatrix(matrix2, MATRIX_SIZE);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep_simd_nogs(matrix2);
  }
  double simd_nogs_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Generic SIMD svec4 no gather-scatter version Time = %f seconds\n", simd_nogs_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }

  //run unrolled simd-normal version
#ifdef __ALTIVEC__
  initMatrix(matrix2, MATRIX_SIZE);
  HPM_PERF_START;
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep_unrolled_simd(matrix2);
  }
  double unrolled_simd_t = get_elapsed_seconds();
  HPM_PERF_STOP;
  printf("Unrolled SIMD svec4 Version Time = %f seconds\n", unrolled_simd_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }
#endif
  HPM_PERF_CLOSE;


  return 0;
}
