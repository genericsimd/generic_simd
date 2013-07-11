#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <timing.h>
#include <power_vsx4.h>

// the simdized version assumes P7 VSX

using namespace vsx;


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

//matrix addr(row,col)
#define addr(row,col) (fNRows*(row) + (col))

void sweep(double *fSym) {
  int fNRows = MATRIX_SIZE;
  int p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Api0, Ai1p, Ai2p, Ai3p;

  for (p = 0; p < fNRows; p++) {
    App = fSym[addr(p,p)];

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[addr(p,p)] = Bpp;

    for (i = 0; i < p; i++) {
      double Api0 = fSym[ addr(p,i)];

      fSym[ addr(p,i)] = absBpp * Api0;
      if (Bpp < 0.0)
        Api0 = -Api0;

      for (j = 0; j <= i; j++) {
        Apj = fSym[ addr(p,j)];
        fSym[ addr(i+0,j)] += Api0 * Apj;
      }
    }
    i++;
    for (; i < fNRows; i++) {
      Aip = fSym[ addr(i,p)];
      fSym[ addr(i,p)] = absBpp * Aip;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Aip = -Aip;
      }

      for (j = 0; j < p; j++) {
        Apj = fSym[ addr(p,j)];
        fSym[ addr(i,j)] += Aip * Apj;
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        Apj = fSym[ addr(j,p)];
        fSym[ addr(i,j)] += Aip * Apj;
      }
    }
  }
}

void sweep_simd(double *fSym) {
  int fNRows = MATRIX_SIZE;
  int Ipp, Iip, Ipj, Iij, p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Ai0p, Ai1p, Ai2p, Ai3p;

  for (p = 0; p < fNRows; p++) {
//    printf("==>Inside p loop, p=%d\n", p);
    App = fSym[addr(p,p)];

    // The a(p,p) element
    Bpp = -1.0 / App;
    absBpp = fabs(Bpp);
    fSym[addr(p,p)] = Bpp;

    for (i = 0; i < p - 3; i += 4) { //each time calc 4 doubles
//      printf("==>  Inside i step4 loop, i=[%d,%d)\n", i, i+4);
      svec4_d vec_Api = *(svec4_d*)(&fSym[addr(p,i)]); //do load
      *(svec4_d*)(&fSym[addr(p,i)]) = absBpp * vec_Api; //add and store


      vec_Api = (Bpp / absBpp) * vec_Api; //if (Bpp < 0.0) Api = -Api;

      for (j = 0; j < i; j += 4) { //note as i%4==0, j also has the property
//        printf("==>    Inside j step4 loop, j=[%d,%d)\n", j, j+4);
        svec4_d vec_Apj = svec4_d::load((svec4_d*)(&fSym[addr(p,j)]));
        //accumulate
        svec4_d vec_Ai0j = svec4_d::load((svec4_d*)(&fSym[addr(i,j)]));
        svec4_d vec_Ai1j = svec4_d::load((svec4_d*)(&fSym[addr(i+1,j)]));
        svec4_d vec_Ai2j = svec4_d::load((svec4_d*)(&fSym[addr(i+2,j)]));
        svec4_d vec_Ai3j = svec4_d::load((svec4_d*)(&fSym[addr(i+3,j)]));

        vec_Ai0j = vec_Ai0j + vec_Api[0] * vec_Apj;
        vec_Ai1j = vec_Ai1j + vec_Api[1] * vec_Apj;
        vec_Ai2j = vec_Ai2j + vec_Api[2] * vec_Apj;
        vec_Ai3j = vec_Ai3j + vec_Api[3] * vec_Apj;

        vec_Ai0j.store((svec4_d*)(&fSym[addr(i,j)]));
        vec_Ai1j.store((svec4_d*)(&fSym[addr(i+1,j)]));
        vec_Ai2j.store((svec4_d*)(&fSym[addr(i+2,j)]));
        vec_Ai3j.store((svec4_d*)(&fSym[addr(i+3,j)]));
      }
      //then the left part. j = i case.

//      printf("==>    after j step4 loop, j=%d\n", j);
      svec4_d vec_Apj = *(svec4_d*)(&fSym[addr(p,j)]); //do load
      fSym[addr(i,j)] += vec_Api[0] * vec_Apj[0];
      fSym[addr(i+1,j)] += vec_Api[1] * vec_Apj[0];
      fSym[addr(i+1,j+1)] += vec_Api[1] * vec_Apj[1];
      fSym[addr(i+2,j)] += vec_Api[2] * vec_Apj[0];
      fSym[addr(i+2,j+1)] += vec_Api[2] * vec_Apj[1];
      fSym[addr(i+2,j+2)] += vec_Api[2] * vec_Apj[2];
      fSym[addr(i+3,j)] += vec_Api[3] * vec_Apj[0];
      fSym[addr(i+3,j+1)] += vec_Api[3] * vec_Apj[1];
      fSym[addr(i+3,j+2)] += vec_Api[3] * vec_Apj[2];
      fSym[addr(i+3,j+3)] += vec_Api[3] * vec_Apj[3];
    }

    //now i < p loop, rest, for both j and i
    for (; i < p; i++) {
      double Api = fSym[addr(p,i)];

      fSym[addr(p,i)] = absBpp * Api;
      if (Bpp < 0.0)
        Api = -Api;

      for (j = 0; j <= i; j++) {
        Apj = fSym[addr(p,j)];
        fSym[addr(i,j)] += Api * Apj;
      }
    }
    i++; //skip the p
    for (; i < fNRows - 3; i += 4) {
//      printf("==>  Inside i step4 loop, i=[%d,%d)\n", i, i+4);
      //Iip = i * fNRows + p;
      //gather and scatter
      svec4_i32 base_off(0,1,2,3);
      svec4_i32 off = fNRows * (i + base_off) + p;
      svec4_d vec_Aip = svec4_d::gather_base_offsets(fSym, sizeof(double), off, svec4_i1(1));

      (absBpp * vec_Aip).scatter_base_offsets(fSym, sizeof(double), off, svec4_i1(1));

      vec_Aip = (Bpp / absBpp) * vec_Aip; //if (Bpp < 0.0) Api = -Api;

      //part 1, j = 0..p, with step 4, always vec_i4
      for(j = 0; j < p - 3; j+=4) {
//        printf("==>    Inside j < p-3 step 4, j=%d\n", j);
        svec4_d vec_Apj = svec4_d::load((svec4_d*)(&fSym[addr(p,j)]));
        //accumulate
        svec4_d vec_Ai0j = svec4_d::load((svec4_d*)(&fSym[addr(i,j)]));
        svec4_d vec_Ai1j = svec4_d::load((svec4_d*)(&fSym[addr(i+1,j)]));
        svec4_d vec_Ai2j = svec4_d::load((svec4_d*)(&fSym[addr(i+2,j)]));
        svec4_d vec_Ai3j = svec4_d::load((svec4_d*)(&fSym[addr(i+3,j)]));

        vec_Ai0j = vec_Ai0j + vec_Aip[0] * vec_Apj;
        vec_Ai1j = vec_Ai1j + vec_Aip[1] * vec_Apj;
        vec_Ai2j = vec_Ai2j + vec_Aip[2] * vec_Apj;
        vec_Ai3j = vec_Ai3j + vec_Aip[3] * vec_Apj;

        vec_Ai0j.store((svec4_d*)(&fSym[addr(i,j)]));
        vec_Ai1j.store((svec4_d*)(&fSym[addr(i+1,j)]));
        vec_Ai2j.store((svec4_d*)(&fSym[addr(i+2,j)]));
        vec_Ai3j.store((svec4_d*)(&fSym[addr(i+3,j)]));
      }

      //part 2, j < p rest, always vec_i4
      for(; j < p; j++) {
//        printf("==>    Rest j < p, j=%d\n", j);
        Apj = fSym[ addr(p,j)];
        svec4_i32 base_off(0,1,2,3);
        svec4_i32 off_ij = fNRows * (i + base_off) + j;
        svec4_d vec_Aij = svec4_d::gather_base_offsets(fSym, sizeof(double), off_ij, svec4_i1(1));
        vec_Aij = vec_Aij + vec_Aip * Apj;
        vec_Aij.scatter_base_offsets(fSym, sizeof(double), off_ij, svec4_i1(1));
      }
      j++;

      //part 1, p< j <= i, with step 4, always vec_i4
      //here i is regular,but j may not so regular, so the upper bound should be careful
      for(; j < i; j+=4) {
//        printf("==>    Inside j <i, step 4 j=%d\n", j);
        //gather and scatter
        svec4_i32 base_off(0,1,2,3);
        svec4_i32 off = fNRows * (j + base_off) + p;
        svec4_d vec_Apj = svec4_d::gather_base_offsets(fSym, sizeof(double), off, svec4_i1(1));

        //svec4_d vec_Apj = svec4_d::load((svec4_d*)(&fSym[addr(p,j)]));
        //accumulate
        svec4_d vec_Ai0j = svec4_d::load((svec4_d*)(&fSym[addr(i,j)]));
        svec4_d vec_Ai1j = svec4_d::load((svec4_d*)(&fSym[addr(i+1,j)]));
        svec4_d vec_Ai2j = svec4_d::load((svec4_d*)(&fSym[addr(i+2,j)]));
        svec4_d vec_Ai3j = svec4_d::load((svec4_d*)(&fSym[addr(i+3,j)]));

        vec_Ai0j = vec_Ai0j + vec_Aip[0] * vec_Apj;
        vec_Ai1j = vec_Ai1j + vec_Aip[1] * vec_Apj;
        vec_Ai2j = vec_Ai2j + vec_Aip[2] * vec_Apj;
        vec_Ai3j = vec_Ai3j + vec_Aip[3] * vec_Apj;

        vec_Ai0j.store((svec4_d*)(&fSym[addr(i,j)]));
        vec_Ai1j.store((svec4_d*)(&fSym[addr(i+1,j)]));
        vec_Ai2j.store((svec4_d*)(&fSym[addr(i+2,j)]));
        vec_Ai3j.store((svec4_d*)(&fSym[addr(i+3,j)]));
      }


      //part 2, j <= i, rest, always vec_i4, i from i to i+3
      //Gather Ajp

      for(; j <=i; j++) {
//        printf("==>    Rest j <=i, j=%d\n", j);
        double Apj = fSym[ addr(j,p)];

        fSym[ addr(i,j)] += vec_Aip[0] * Apj;
        fSym[ addr(i+1,j)] += vec_Aip[1] * Apj;
        fSym[ addr(i+2,j)] += vec_Aip[2] * Apj;
        fSym[ addr(i+3,j)] += vec_Aip[3] * Apj;
      }
      //now th rest
//      printf("==>    Rest j <=i, j=%d\n", j);
      Apj = fSym[ addr(j,p)];
      fSym[ addr(i+1,j)] += vec_Aip[1] * Apj;
      fSym[ addr(i+2,j)] += vec_Aip[2] * Apj;
      fSym[ addr(i+3,j)] += vec_Aip[3] * Apj;

      j++;
//      printf("==>    Rest j <=i, j=%d\n", j);
      Apj = fSym[ addr(j,p)];
      fSym[ addr(i+2,j)] += vec_Aip[2] * Apj;
      fSym[ addr(i+3,j)] += vec_Aip[3] * Apj;

      j++;
//      printf("==>    Rest j <=i, j=%d\n", j);
      Apj = fSym[ addr(j,p)];
      fSym[ addr(i+3,j)] += vec_Aip[3] * Apj;


//      double Ai0p = vec_Aip[0];
//      double Ai1p = vec_Aip[1];
//      double Ai2p = vec_Aip[2];
//      double Ai3p = vec_Aip[3];
//
//      vector double vec_Ai0p = vec_splats(Ai0p);
//      vector double vec_Ai1p = vec_splats(Ai1p);
//      vector double vec_Ai2p = vec_splats(Ai2p);
//      vector double vec_Ai3p = vec_splats(Ai3p);
//      if (p % 2) { // p is ODD
//        for (j = 0; j < p - 1; j += 2) {
//          Ipj = p * fNRows + j;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          vector double vec_Apj = *(vector double*) (fSym + Ipj);
//          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
//          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
//          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
//          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);
//
//          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
//          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
//          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
//          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);
//
//          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
//          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
//          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
//          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
//#if 0
//          Apj = fSym[ Ipj ];
//          fSym[ Ii0j ] += Ai0p * Apj;
//          fSym[ Ii1j ] += Ai1p * Apj;
//          fSym[ Ii2j ] += Ai2p * Apj;
//          fSym[ Ii3j ] += Ai3p * Apj;
//#endif
//        }
//        for (; j < p; j++) {
//          Ipj = p * fNRows + j;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          Apj = fSym[Ipj];
//          fSym[Ii0j] += Ai0p * Apj;
//          fSym[Ii1j] += Ai1p * Apj;
//          fSym[Ii2j] += Ai2p * Apj;
//          fSym[Ii3j] += Ai3p * Apj;
//        }
//        // Skip the pivot column
//        j++;
//        for (; j < ((i + 1) / 2) * 2; j += 2) {
//          int Ipj0 = (j + 0) * fNRows + p;
//          int Ipj1 = (j + 1) * fNRows + p;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          //vector double vec_Apj0 = vec_splats(fSym[ Ipj0 ]);
//          //vector double vec_Apj1 = vec_splats(fSym[ Ipj1 ]);
//          vector double vec_Apj0 = *(vector double*) (fSym + Ipj0 - 1);
//          vector double vec_Apj1 = *(vector double*) (fSym + Ipj1 - 1);
//          vector double vec_Apj = vec_mergel(vec_Apj0, vec_Apj1);
//
//          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
//          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
//          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
//          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);
//
//          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
//          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
//          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
//          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);
//
//          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
//          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
//          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
//          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
//#if 0
//          Apj = fSym[ Ipj ];
//          fSym[ Ii0j ] += Ai0p * Apj;
//          fSym[ Ii1j ] += Ai1p * Apj;
//          fSym[ Ii2j ] += Ai2p * Apj;
//          fSym[ Ii3j ] += Ai3p * Apj;
//#endif
//        }
//        for (; j <= i; j++) {
//          Ipj = j * fNRows + p;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          Apj = fSym[Ipj];
//          fSym[Ii0j] += Ai0p * Apj;
//          fSym[Ii1j] += Ai1p * Apj;
//          fSym[Ii2j] += Ai2p * Apj;
//          fSym[Ii3j] += Ai3p * Apj;
//        }
//      } else { // p is EVEN
//        for (j = 0; j < p; j += 2) {
//          Ipj = p * fNRows + j;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          vector double vec_Apj = *(vector double*) (fSym + Ipj);
//          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
//          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
//          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
//          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);
//
//          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
//          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
//          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
//          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);
//
//          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
//          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
//          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
//          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
//#if 0
//          Apj = fSym[ Ipj ];
//          fSym[ Ii0j ] += Ai0p * Apj;
//          fSym[ Ii1j ] += Ai1p * Apj;
//          fSym[ Ii2j ] += Ai2p * Apj;
//          fSym[ Ii3j ] += Ai3p * Apj;
//#endif
//        }
//        // Skip the pivot column
//        j++;
//        for (; j <= i && j == (p + 1); j++) {
//          Ipj = (j + 0) * fNRows + p;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          Apj = fSym[Ipj];
//          fSym[Ii0j] += Ai0p * Apj;
//          fSym[Ii1j] += Ai1p * Apj;
//          fSym[Ii2j] += Ai2p * Apj;
//          fSym[Ii3j] += Ai3p * Apj;
//        }
//        for (; j < ((i + 1) / 2) * 2; j += 2) {
//          int Ipj0 = (j + 0) * fNRows + p;
//          int Ipj1 = (j + 1) * fNRows + p;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          vector double vec_Apj0 = vec_splats(fSym[Ipj0]);
//          vector double vec_Apj1 = vec_splats(fSym[Ipj1]);
//          vector double vec_Apj = vec_mergeh(vec_Apj0, vec_Apj1);
//
//          vector double vec_Ai0j = *(vector double*) (fSym + Ii0j);
//          vector double vec_Ai1j = *(vector double*) (fSym + Ii1j);
//          vector double vec_Ai2j = *(vector double*) (fSym + Ii2j);
//          vector double vec_Ai3j = *(vector double*) (fSym + Ii3j);
//
//          vec_Ai0j = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
//          vec_Ai1j = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
//          vec_Ai2j = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
//          vec_Ai3j = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);
//
//          vec_vsx_st(vec_Ai0j, 0, (vector double*) (fSym + Ii0j));
//          vec_vsx_st(vec_Ai1j, 0, (vector double*) (fSym + Ii1j));
//          vec_vsx_st(vec_Ai2j, 0, (vector double*) (fSym + Ii2j));
//          vec_vsx_st(vec_Ai3j, 0, (vector double*) (fSym + Ii3j));
//#if 0
//          Apj = fSym[ Ipj ];
//          fSym[ Ii0j ] += Ai0p * Apj;
//          fSym[ Ii1j ] += Ai1p * Apj;
//          fSym[ Ii2j ] += Ai2p * Apj;
//          fSym[ Ii3j ] += Ai3p * Apj;
//#endif
//        }
//        for (; j <= i; j++) {
//          Ipj = j * fNRows + p;
//          Ii0j = (i + 0) * fNRows + j;
//          Ii1j = (i + 1) * fNRows + j;
//          Ii2j = (i + 2) * fNRows + j;
//          Ii3j = (i + 3) * fNRows + j;
//
//          Apj = fSym[Ipj];
//          fSym[Ii0j] += Ai0p * Apj;
//          fSym[Ii1j] += Ai1p * Apj;
//          fSym[Ii2j] += Ai2p * Apj;
//          fSym[Ii3j] += Ai3p * Apj;
//        }
//      }
//      Ipj = j * fNRows + p;
//      Ii1j = (i + 1) * fNRows + j;
//      Ii2j = (i + 2) * fNRows + j;
//      Ii3j = (i + 3) * fNRows + j;
//      Apj = fSym[Ipj];
//      fSym[Ii1j] += Ai1p * Apj;
//      fSym[Ii2j] += Ai2p * Apj;
//      fSym[Ii3j] += Ai3p * Apj;
//      j++;
//      Ipj = j * fNRows + p;
//      Ii2j = (i + 2) * fNRows + j;
//      Ii3j = (i + 3) * fNRows + j;
//      Apj = fSym[Ipj];
//      fSym[Ii2j] += Ai2p * Apj;
//      fSym[Ii3j] += Ai3p * Apj;
//      j++;
//      Ipj = j * fNRows + p;
//      Ii3j = (i + 3) * fNRows + j;
//      Apj = fSym[Ipj];
//      fSym[Ii3j] += Ai3p * Apj;
//
    } // p < i < fNows - 3
    //the left i rows
    for (; i < fNRows; i++) {

      Aip = fSym[addr(i,p)];
      fSym[addr(i,p)] = absBpp * Aip;

      // The a(i,j) elements, i,j both != p
      if (Bpp < 0.0) {
        Aip = -Aip;
      }

      for (j = 0; j < p; j++) {
        Apj = fSym[addr(p,j)];
        fSym[addr(i,j)] += Aip * Apj;
      }
      // Skip the pivot column
      j++;
      for (; j <= i; j++) {
        Apj = fSym[addr(j,p)];
        fSym[addr(i,j)] += Aip * Apj;
      }
    }
  }
}


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
//      if (mat[Iij] != tri[count])
//        {
//          printf("i=%d j=%d Iij=%d mat=%f tri=%f\n",
//             i, j, Iij, mat[Iij], tri[count]);
//          num_errors++;
//          //return 0;
//        }
    }
      for (; j < size; j++)
    {
      int Iij = i * size + j;
         err += fabs(mat[Iij]);
//      if (mat[Iij] != 0.0)
//        {
//          printf("i=%d j=%d Iij=%d mat=%f\n",
//             i, j, Iij, mat[Iij]);
//          num_errors++;
//        }
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


  //run base.
  initTriMatrix(matrix1, TRI_COUNT);
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    tri_sweep_orig(matrix1);
  }
  double org_t = get_elapsed_seconds();
  printf("Org Version Time = %f seconds\n", org_t);
  

  //run tri simd
  initTriMatrix(matrix2, TRI_COUNT);
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    tri_sweep(matrix2);
  }
  double tri_simd_t = get_elapsed_seconds();
  printf("Split SIMD Version Time = %f seconds\n", tri_simd_t);
  if (compareTriMatrix(matrix2, matrix1, TRI_COUNT)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }

  //run org-normal version
  initMatrix(matrix2, MATRIX_SIZE);
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep(matrix2);
  }
  double normal_t = get_elapsed_seconds();
  printf("Normal Version Time = %f seconds\n", normal_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }
  

  //run org-normal version
  initMatrix(matrix2, MATRIX_SIZE);
  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
    sweep_simd(matrix2);
  }
  double simd_t = get_elapsed_seconds();
  printf("SIMD Version Time = %f seconds\n", simd_t);
  if (compareMatrix(matrix2, matrix1, MATRIX_SIZE)) {
    printf("PASSED!\n");
  } else {
    printf("FAILED!\n");
  }


  return 0;
}
