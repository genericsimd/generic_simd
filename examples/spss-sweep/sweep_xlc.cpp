#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <timing.h>

// select one of the following five options
// the simdized version assumes P7 VSX
//#define ORIG
//#define SPLIT_SCALAR
//#define SPLIT_SIMD
//#define UNROLLED_SCALAR
//#define UNROLLED_SIMD

#if defined(SPLIT_SIMD) || defined(UNROLLED_SIMD)
#define SIMD
#endif

#ifdef SIMD
#include <altivec.h>
#endif

#define	MATRIX_SIZE	1000

// an original sweep function for a triangular matrix
void tri_sweep_orig(double *fSym) 
{
  size_t	fNRows	= MATRIX_SIZE;
  size_t	Ipp, Iip, Ipj, Iij, p, i, j;
  double	App, Bpp, absBpp, Aip, Apj;
	
  for (p = 0; p < fNRows; p++)
    {
      Ipp = (p + 1) * (p + 2) / 2 - 1;
      App	= fSym[Ipp];
		
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

#if defined(SPLIT_SCALAR) || defined(SPLIT_SIMD)
// a sweep function (without unrolling) for a triangular matrix
void tri_sweep(double *fSym) 
{
  size_t	fNRows	= MATRIX_SIZE;
  size_t	Ipp, Iip, Ipj, Iij, p, i, j;
  double	App, Bpp, absBpp, Aip, Apj;
	
  for (p = 0; p < fNRows; p++) 
    {
      Ipp = (p + 1) * (p + 2) / 2 - 1;
      App	= fSym[Ipp];
		
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
#ifdef SIMD
	      vector double vec_Aip = vec_splats(Aip);
#endif	
	      Ipj = Ipp - p - 1;
	
	      if (i < p) 
		{
		  for (j = 0; j < ((i+1)/2)*2; j+=2)
		    {
#ifdef SIMD
		      Ipj++;
		      Iij++;
		      vector double vec_Apj = vec_xld2(0, fSym+Ipj);
		      vector double vec_Aij = vec_xld2(0, fSym+Iij);
		      vec_Aij  = vec_madd(vec_Aip, vec_Apj, vec_Aij);
		      vec_xstd2(vec_Aij, 0, fSym+Iij);
		      Ipj++;
		      Iij++;
#else
		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;

		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;
#endif
		    }
		  for (; j <= i; j++)
		    {
		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;
		    }
		}
	      else
		{
		  for (j = 0; j < (p/2)*2; j+=2)
		    {
#ifdef SIMD
		      Ipj++;
		      Iij++;
		      vector double vec_Apj = vec_xld2(0, fSym+Ipj);
		      vector double vec_Aij = vec_xld2(0, fSym+Iij);
		      vec_Aij  = vec_madd(vec_Aip, vec_Apj, vec_Aij);
		      vec_xstd2(vec_Aij, 0, fSym+Iij);
		      Ipj++;
		      Iij++;
#else
		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;
		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;
#endif
		    }
		  for (; j < p; j++)
		    {
		      Iij++;
		      Ipj++;
		      Apj = fSym[ Ipj ];
		      fSym[ Iij ] += Aip * Apj;
		    }
		  j++; Iij++; Ipj++;
		  if (p%2)
		    { // p is ODD
		      for (; j < ((i+1)/2)*2; j++)
			{
#ifdef SIMD
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  vector double vec_Apj0 = vec_splats(fSym[ Ipj ]);
			  j++;
			  Ipj++;
			  Ipj += (j - 1);
			  vector double vec_Apj1 = vec_splats(fSym[ Ipj ]);
			  vector double vec_Apj  = vec_mergeh(vec_Apj0, vec_Apj1);
			  vector double vec_Aij = vec_xld2(0, fSym+Iij);
			  vec_Aij  = vec_madd(vec_Aip, vec_Apj, vec_Aij);
			  vec_xstd2(vec_Aij, 0, fSym+Iij);
			  Iij++;
#else
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
			  j++;
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
#endif
			}
		      for (; j <= i; j++)
			{
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
			}
		    }
		  else
		    { //p is EVEN
		      for (; j <= i && j == (p+1); j++)
			{
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
			}
		      for (; j < ((i+1)/2)*2; j++)
			{
#ifdef SIMD
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  vector double vec_Apj0 = vec_splats(fSym[ Ipj ]);
			  j++;
			  Ipj++;
			  Ipj += (j - 1);
			  vector double vec_Apj1 = vec_splats(fSym[ Ipj ]);
			  vector double vec_Apj  = vec_mergeh(vec_Apj0, vec_Apj1);
			  vector double vec_Aij = vec_xld2(0, fSym+Iij);
			  vec_Aij  = vec_madd(vec_Aip, vec_Apj, vec_Aij);
			  vec_xstd2(vec_Aij, 0, fSym+Iij);
			  Iij++;
#else
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
			  j++;
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
#endif
			}
		      for (; j <= i; j++)
			{
			  Iij++;
			  Ipj++;
			  Ipj += (j - 1);
			  Apj = fSym[ Ipj ];
			  fSym[ Iij ] += Aip * Apj;
			}
		    }
		}
	    }
	}
    }
}
#endif

#ifdef UNROLLED_SIMD
void sweep_unrolled_simd(double *fSym) 
{
  int fNRows	= MATRIX_SIZE;
  int Ipp, Iip, Ipj, Iij, p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  double Ai0p, Ai1p, Ai2p, Ai3p;
	
  for (p = 0; p < fNRows; p++) 
    {
      Ipp = p * fNRows + p;
      App = fSym[Ipp];
		
      // The a(p,p) element
      Bpp = -1.0 / App;
      absBpp = fabs( Bpp );
      fSym[ Ipp ] = Bpp;
      i = 0;

      for (i = 0; i < p-3; i+=4)
	{
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
	  vector double vec_Ai01p = vec_xld2(0, fSym+Iip);
	  vector double vec_Ai23p = vec_xld2(0, fSym+Iip+2);
	  vector double vec_Ai0p = vec_mergeh(vec_Ai01p, vec_Ai01p);
	  vector double vec_Ai1p = vec_mergel(vec_Ai01p, vec_Ai01p);
	  vector double vec_Ai2p = vec_mergeh(vec_Ai23p, vec_Ai23p);
	  vector double vec_Ai3p = vec_mergel(vec_Ai23p, vec_Ai23p);
	
	  vector double vec_absBpp = vec_splats(absBpp);
	  vec_Ai01p = vec_mul(vec_absBpp, vec_Ai01p);
	  vec_Ai23p = vec_mul(vec_absBpp, vec_Ai23p);
	  vec_xstd2(vec_Ai01p, 0, fSym+Iip);
	  vec_xstd2(vec_Ai23p, 0, fSym+Iip+2);

	  double signBpp = Bpp/absBpp;
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
	  for (j = 0; j < (i/2)*2; j+=2)
	    {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;

	      vector double vec_Apj  = vec_xld2(0, fSym+Ipj);
	      vector double vec_Ai0j = vec_xld2(0, fSym+Ii0j);
	      vector double vec_Ai1j = vec_xld2(0, fSym+Ii1j);
	      vector double vec_Ai2j = vec_xld2(0, fSym+Ii2j);
	      vector double vec_Ai3j = vec_xld2(0, fSym+Ii3j);

	      vec_Ai0j  = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
	      vec_Ai1j  = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
	      vec_Ai2j  = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
	      vec_Ai3j  = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

	      vec_xstd2(vec_Ai0j, 0, fSym+Ii0j);
	      vec_xstd2(vec_Ai1j, 0, fSym+Ii1j);
	      vec_xstd2(vec_Ai2j, 0, fSym+Ii2j);
	      vec_xstd2(vec_Ai3j, 0, fSym+Ii3j);
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

	  for (; j <= i; j++)
	    {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;

	      Apj = fSym[ Ipj ];
	      fSym[ Ii0j ] += Ai0p * Apj;
	      fSym[ Ii1j ] += Ai1p * Apj;
	      fSym[ Ii2j ] += Ai2p * Apj;
	      fSym[ Ii3j ] += Ai3p * Apj;
	    }

	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii1j = fNRows*(i+1)+j;
	  Ii2j = fNRows*(i+2)+j;
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii1j ] += Ai1p * Apj;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii2j = fNRows*(i+2)+j;
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii3j ] += Ai3p * Apj;
	}

      for (; i < p; i++)
	{
	  Iip = p * fNRows + i;

	  double Ai0p = fSym[ Iip ];

	  fSym[ Iip ] = absBpp * Ai0p;
	  if ( Bpp < 0.0 ) Ai0p = -Ai0p;

	  for (j = 0; j <= i; j++)
            {
	      Ipj = p * fNRows + j;
	      Apj = fSym[ Ipj ];
	      Ii0j = fNRows*(i+0)+j;
	      fSym[ Ii0j ] += Ai0p * Apj;
            }
	}
      i++;
      for (; i < fNRows-3; i+=4)
	{
	  //Iip = i * fNRows + p;
	  Ii0p = fNRows*(i+0)+p;
	  Ii1p = fNRows*(i+1)+p;
	  Ii2p = fNRows*(i+2)+p;
	  Ii3p = fNRows*(i+3)+p;
	
	  double Ai0p = fSym[ Ii0p ];
	  double Ai1p = fSym[ Ii1p ];
	  double Ai2p = fSym[ Ii2p ];
	  double Ai3p = fSym[ Ii3p ];

	  fSym[ Ii0p ] = absBpp * Ai0p;
	  fSym[ Ii1p ] = absBpp * Ai1p;
	  fSym[ Ii2p ] = absBpp * Ai2p;
	  fSym[ Ii3p ] = absBpp * Ai3p;
	
	  // The a(i,j) elements, i,j both != p
	  if ( Bpp < 0.0 )
	    {
	      Ai0p = -Ai0p;
	      Ai1p = -Ai1p;
	      Ai2p = -Ai2p;
	      Ai3p = -Ai3p;
	    }
	  vector double vec_Ai0p = vec_splats(Ai0p);
	  vector double vec_Ai1p = vec_splats(Ai1p);
	  vector double vec_Ai2p = vec_splats(Ai2p);
	  vector double vec_Ai3p = vec_splats(Ai3p);
	  if (p%2)
	    { // p is ODD
	      for (j = 0; j < p-1; j+=2)
		{
		  Ipj = p * fNRows + j;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
	
		  vector double vec_Apj  = vec_xld2(0, fSym+Ipj);
		  vector double vec_Ai0j = vec_xld2(0, fSym+Ii0j);
		  vector double vec_Ai1j = vec_xld2(0, fSym+Ii1j);
		  vector double vec_Ai2j = vec_xld2(0, fSym+Ii2j);
		  vector double vec_Ai3j = vec_xld2(0, fSym+Ii3j);

		  vec_Ai0j  = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
		  vec_Ai1j  = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
		  vec_Ai2j  = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
		  vec_Ai3j  = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

		  vec_xstd2(vec_Ai0j, 0, fSym+Ii0j);
		  vec_xstd2(vec_Ai1j, 0, fSym+Ii1j);
		  vec_xstd2(vec_Ai2j, 0, fSym+Ii2j);
		  vec_xstd2(vec_Ai3j, 0, fSym+Ii3j);
#if 0
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
#endif
		}
	      for (; j < p; j++)
		{
		  Ipj = p * fNRows + j;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
	
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
		}
	      // Skip the pivot column
	      j++;
	      for (; j < ((i+1)/2)*2; j+=2)
		{
		  int Ipj0 = (j+0) * fNRows + p;
		  int Ipj1 = (j+1) * fNRows + p;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;

		  //vector double vec_Apj0 = vec_splats(fSym[ Ipj0 ]);
		  //vector double vec_Apj1 = vec_splats(fSym[ Ipj1 ]);
		  vector double vec_Apj0 = vec_xld2(0, fSym + Ipj0 - 1);
		  vector double vec_Apj1 = vec_xld2(0, fSym + Ipj1 - 1);
		  vector double vec_Apj  = vec_mergel(vec_Apj0, vec_Apj1);

		  vector double vec_Ai0j = vec_xld2(0, fSym+Ii0j);
		  vector double vec_Ai1j = vec_xld2(0, fSym+Ii1j);
		  vector double vec_Ai2j = vec_xld2(0, fSym+Ii2j);
		  vector double vec_Ai3j = vec_xld2(0, fSym+Ii3j);

		  vec_Ai0j  = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
		  vec_Ai1j  = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
		  vec_Ai2j  = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
		  vec_Ai3j  = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

		  vec_xstd2(vec_Ai0j, 0, fSym+Ii0j);
		  vec_xstd2(vec_Ai1j, 0, fSym+Ii1j);
		  vec_xstd2(vec_Ai2j, 0, fSym+Ii2j);
		  vec_xstd2(vec_Ai3j, 0, fSym+Ii3j);
#if 0
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
#endif
		}
	      for (; j <= i; j++)
		{
		  Ipj = j * fNRows + p;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
		  
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
		}
	    }
	  else
	    { // p is EVEN
	      for (j = 0; j < p; j+=2)
		{
		  Ipj = p * fNRows + j;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
	
		  vector double vec_Apj  = vec_xld2(0, fSym+Ipj);
		  vector double vec_Ai0j = vec_xld2(0, fSym+Ii0j);
		  vector double vec_Ai1j = vec_xld2(0, fSym+Ii1j);
		  vector double vec_Ai2j = vec_xld2(0, fSym+Ii2j);
		  vector double vec_Ai3j = vec_xld2(0, fSym+Ii3j);

		  vec_Ai0j  = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
		  vec_Ai1j  = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
		  vec_Ai2j  = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
		  vec_Ai3j  = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

		  vec_xstd2(vec_Ai0j, 0, fSym+Ii0j);
		  vec_xstd2(vec_Ai1j, 0, fSym+Ii1j);
		  vec_xstd2(vec_Ai2j, 0, fSym+Ii2j);
		  vec_xstd2(vec_Ai3j, 0, fSym+Ii3j);
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
	      for (; j <= i && j == (p+1); j++)
		{
		  Ipj  = (j+0) * fNRows + p;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
	
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
		}
	      for (; j < ((i+1)/2)*2; j+=2)
		{
		  int Ipj0 = (j+0) * fNRows + p;
		  int Ipj1 = (j+1) * fNRows + p;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;

		  vector double vec_Apj0 = vec_splats(fSym[ Ipj0 ]);
		  vector double vec_Apj1 = vec_splats(fSym[ Ipj1 ]);
		  vector double vec_Apj  = vec_mergeh(vec_Apj0, vec_Apj1);

		  vector double vec_Ai0j = vec_xld2(0, fSym+Ii0j);
		  vector double vec_Ai1j = vec_xld2(0, fSym+Ii1j);
		  vector double vec_Ai2j = vec_xld2(0, fSym+Ii2j);
		  vector double vec_Ai3j = vec_xld2(0, fSym+Ii3j);

		  vec_Ai0j  = vec_madd(vec_Ai0p, vec_Apj, vec_Ai0j);
		  vec_Ai1j  = vec_madd(vec_Ai1p, vec_Apj, vec_Ai1j);
		  vec_Ai2j  = vec_madd(vec_Ai2p, vec_Apj, vec_Ai2j);
		  vec_Ai3j  = vec_madd(vec_Ai3p, vec_Apj, vec_Ai3j);

		  vec_xstd2(vec_Ai0j, 0, fSym+Ii0j);
		  vec_xstd2(vec_Ai1j, 0, fSym+Ii1j);
		  vec_xstd2(vec_Ai2j, 0, fSym+Ii2j);
		  vec_xstd2(vec_Ai3j, 0, fSym+Ii3j);
#if 0
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
#endif
		}
	      for (; j <= i; j++)
		{
		  Ipj = j * fNRows + p;
		  Ii0j = (i+0) * fNRows + j;
		  Ii1j = (i+1) * fNRows + j;
		  Ii2j = (i+2) * fNRows + j;
		  Ii3j = (i+3) * fNRows + j;
		  
		  Apj = fSym[ Ipj ];
		  fSym[ Ii0j ] += Ai0p * Apj;
		  fSym[ Ii1j ] += Ai1p * Apj;
		  fSym[ Ii2j ] += Ai2p * Apj;
		  fSym[ Ii3j ] += Ai3p * Apj;
		}
	    }
	  Ipj = j * fNRows + p;
	  Ii1j = (i+1) * fNRows + j;
	  Ii2j = (i+2) * fNRows + j;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii1j ] += Ai1p * Apj;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = j * fNRows + p;
	  Ii2j = (i+2) * fNRows + j;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = j * fNRows + p;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii3j ] += Ai3p * Apj;
	  
	}
      for (; i < fNRows; i++)
	{
	  Iip = i * fNRows + p;
	
	  Aip = fSym[ Iip ];
	  fSym[ Iip ] = absBpp * Aip;
	
	  // The a(i,j) elements, i,j both != p
	  if ( Bpp < 0.0 )
	    {
	      Aip = -Aip;
	    }
	
	  for (j = 0; j < p; j++)
	    {
	      Iij = i * fNRows + j;
	      Ipj = p * fNRows + j;
	
	      Apj = fSym[ Ipj ];
	      fSym[ Iij ] += Aip * Apj;
	    }
	  // Skip the pivot column
	  j++;
	  for (; j <= i; j++)
	    {
	      Iij = i * fNRows + j;
	      Ipj = j * fNRows + p;

	      Apj = fSym[ Ipj ];
	      fSym[ Iij ] += Aip * Apj;
	    }
	}
    }
}
#endif

#ifdef UNROLLED_SCALAR
#ifdef	UNROLLED_SCALAR_COUNT_ITERS
long long int		num_iters_jip	= 0;
long long int		num_iters_jpi	= 0;
long long int		num_iters_pji	= 0;
#endif
void sweep_unrolled_scalar(double *fSym) 
{
  int fNRows	= MATRIX_SIZE;
  int Ipp, Iip, Ipj, Iij, p, i, j;
  int Ii0j, Ii1j, Ii2j, Ii3j;
  int Ii0p, Ii1p, Ii2p, Ii3p;
  double App, Bpp, absBpp, Aip, Apj;
  
  for (p = 0; p < fNRows; p++)
    {
      Ipp = p * fNRows + p;
      App = fSym[Ipp];
		
      // The a(p,p) element
      Bpp = -1.0 / App;
      absBpp = fabs( Bpp );
      fSym[ Ipp ] = Bpp;
      i = 0;

      for (i = 0; i < p-3; i+=4)
	{
	  Iip = p * fNRows + i;
	
	  double Ai0p = fSym[ Iip+0 ];
	  double Ai1p = fSym[ Iip+1 ];
	  double Ai2p = fSym[ Iip+2 ];
	  double Ai3p = fSym[ Iip+3 ];
	  fSym[ Iip+0 ] = absBpp * Ai0p;
	  fSym[ Iip+1 ] = absBpp * Ai1p;
	  fSym[ Iip+2 ] = absBpp * Ai2p;
	  fSym[ Iip+3 ] = absBpp * Ai3p;
	
	  if ( Bpp < 0.0 )
	    {
	      Ai0p = -Ai0p;
	      Ai1p = -Ai1p;
	      Ai2p = -Ai2p;
	      Ai3p = -Ai3p;
	    }
	  j = 0;
#if		defined(__powerpc64__) && defined(INLINE_FMA) && defined(INLINE_VECTOR)
	  v2df_u	Ai0pv, Ai1pv, Ai2pv, Ai3pv;
	  Ai0pv.df[0] = Ai0pv.df[1] = Ai0p;
	  Ai1pv.df[0] = Ai1pv.df[1] = Ai1p;
	  Ai2pv.df[0] = Ai2pv.df[1] = Ai2p;
	  Ai3pv.df[0] = Ai3pv.df[1] = Ai3p;
	  for (; j <= i - 1; j+=2) {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
	      v2df	Apjv	= *((v2df *) &fSym[Ipj]);
	      *((v2df *) &fSym[Ii0j])	+= Ai0pv.v * Apjv;
   	      *((v2df *) &fSym[Ii1j])	+= Ai1pv.v * Apjv;
	      *((v2df *) &fSym[Ii2j])	+= Ai2pv.v * Apjv;
   	      *((v2df *) &fSym[Ii3j])	+= Ai3pv.v * Apjv;
	  }
#endif	/* defined(__powerpc64__) && defined(INLINE_FMA) && defined(INLINE_VECTOR) */
#if		defined(__s390x__) && defined(INLINE_FMA) && defined(RESCHEDULE_FMA) && defined(DO_PREFETCH)
	  for (; j <= i - 31; j+=32) {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
#define	doIteration(offset)	\
			"ld %12," #offset "(%13);"	\
			"ld %0," #offset "(%4);"	\
			"ld %1," #offset "(%5);"	\
			"ld %2," #offset "(%6);"	\
			"ld %3," #offset "(%7);"	\
		  	"madbr %0,%8,%12;"			\
		  	"madbr %1,%9,%12;"			\
		  	"madbr %2,%10,%12;"			\
		  	"madbr %3,%11,%12;"			\
		  	"std %0," #offset "(%4);"	\
		  	"std %1," #offset "(%5);"	\
		  	"std %2," #offset "(%6);"	\
		  	"std %3," #offset "(%7);"
		  __asm__ __volatile__ (
			"pfd %15,%14(%13);"
			"pfd %15,%14(%4);"
			"pfd %15,%14(%5);"
			"pfd %15,%14(%6);"
			"pfd %15,%14(%7);"
			doIteration(0)
			doIteration(8)
			doIteration(16)
			doIteration(24)
			doIteration(32)
			doIteration(40)
			doIteration(48)
			doIteration(56)
			doIteration(64)
			doIteration(72)
			doIteration(80)
			doIteration(88)
			doIteration(96)
			doIteration(104)
			doIteration(112)
			doIteration(120)
			doIteration(128)
			doIteration(136)
			doIteration(144)
			doIteration(152)
			doIteration(160)
			doIteration(168)
			doIteration(176)
			doIteration(184)
			doIteration(192)
			doIteration(200)
			doIteration(208)
			doIteration(216)
			doIteration(224)
			doIteration(232)
			doIteration(240)
			doIteration(248)
#undef	doIteration
		  	:
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p),
		  	  "f"(4.0), "a" (&fSym[Ipj]), "K"(PREFETCH_DISTANCE), "I"(PREFETCH_CODE)
		  );
	  }
#endif	/* defined(__s390x__) && defined(INLINE_FMA) && defined(RESCHEDULE_FMA) && defined(DO_PREFETCH) */
	  for (; j <= i; j++)
	    {
#ifdef	UNROLLED_SCALAR_COUNT_ITERS
		  num_iters_jip++;
#endif	/* UNROLLED_SCALAR_COUNT_ITERS */
	      //Iij = i * fNRows + j;
	      Ipj = p * fNRows + j;
	      
	      Apj = fSym[ Ipj ];

	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
#if	defined(INLINE_FMA) && defined(__s390x__)
#if	defined(RESCHEDULE_FMA)
		  __asm__ __volatile__ (
		    "ld %0,0(%4);"
		    "ld %1,0(%5);"
		    "ld %2,0(%6);"
		    "ld %3,0(%7);"
		  	"madbr %0,%8,%12;"
		  	"madbr %1,%9,%12;"
		  	"madbr %2,%10,%12;"
		  	"madbr %3,%11,%12;"
		  	"std %0,0(%4);"
		  	"std %1,0(%5);"
		  	"std %2,0(%6);"
		  	"std %3,0(%7);"
		  	: 
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p), "f" (Apj)
		  );
#else	/* defined(RESCHEDULE_FMA) */
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii0j])
		  	: "f" (Ai0p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii1j])
		  	: "f" (Ai1p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii2j])
		  	: "f" (Ai2p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii3j])
		  	: "f" (Ai3p), "f" (Apj)
		  );
#endif	/* defined(RESCHEDULE_FMA) */
#else	/* defined(INLINE_FMA) && defined(__s390x__) */
	      fSym[ Ii0j ] += Ai0p * Apj;
	      fSym[ Ii1j ] += Ai1p * Apj;
	      fSym[ Ii2j ] += Ai2p * Apj;
	      fSym[ Ii3j ] += Ai3p * Apj;
#endif	/* defined(INLINE_FMA) && defined(__s390x__) */
	    }
	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii1j = fNRows*(i+1)+j;
	  Ii2j = fNRows*(i+2)+j;
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii1j ] += Ai1p * Apj;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii2j = fNRows*(i+2)+j;
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = p * fNRows + j;
	  Apj = fSym[ Ipj ];
	  Ii3j = fNRows*(i+3)+j;
	  fSym[ Ii3j ] += Ai3p * Apj;
	}

      for (; i < p; i++)
	{
	  Iip = p * fNRows + i;

	  double Ai0p = fSym[ Iip ];

	  fSym[ Iip ] = absBpp * Ai0p;
	  if ( Bpp < 0.0 ) Ai0p = -Ai0p;

	  for (j = 0; j <= i; j++)
            {
	      Ipj = p * fNRows + j;
	      Apj = fSym[ Ipj ];
	      Ii0j = fNRows*(i+0)+j;
	      fSym[ Ii0j ] += Ai0p * Apj;
            }
	}
      i++;
      for (; i < fNRows-3; i+=4)
	{
	  //Iip = i * fNRows + p;
	  Ii0p = fNRows*(i+0)+p;
	  Ii1p = fNRows*(i+1)+p;
	  Ii2p = fNRows*(i+2)+p;
	  Ii3p = fNRows*(i+3)+p;
	
	  double Ai0p = fSym[ Ii0p ];
	  double Ai1p = fSym[ Ii1p ];
	  double Ai2p = fSym[ Ii2p ];
	  double Ai3p = fSym[ Ii3p ];

	  fSym[ Ii0p ] = absBpp * Ai0p;
	  fSym[ Ii1p ] = absBpp * Ai1p;
	  fSym[ Ii2p ] = absBpp * Ai2p;
	  fSym[ Ii3p ] = absBpp * Ai3p;
	
	  // The a(i,j) elements, i,j both != p
	  if ( Bpp < 0.0 )
	    {
	      Ai0p = -Ai0p;
	      Ai1p = -Ai1p;
	      Ai2p = -Ai2p;
	      Ai3p = -Ai3p;
	    }
	  j = 0;
#if		defined(__powerpc64__) && defined(INLINE_FMA) && defined(INLINE_VECTOR)
	  v2df_u	Ai0pv, Ai1pv, Ai2pv, Ai3pv;
	  Ai0pv.df[0] = Ai0pv.df[1] = Ai0p;
	  Ai1pv.df[0] = Ai1pv.df[1] = Ai1p;
	  Ai2pv.df[0] = Ai2pv.df[1] = Ai2p;
	  Ai3pv.df[0] = Ai3pv.df[1] = Ai3p;
	  for (; j < p - 1; j+=2) {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
	      v2df	Apjv	= *((v2df *) &fSym[Ipj]);
	      *((v2df *) &fSym[Ii0j])	+= Ai0pv.v * Apjv;
   	      *((v2df *) &fSym[Ii1j])	+= Ai1pv.v * Apjv;
	      *((v2df *) &fSym[Ii2j])	+= Ai2pv.v * Apjv;
   	      *((v2df *) &fSym[Ii3j])	+= Ai3pv.v * Apjv;
	  }
#endif
#if		defined(__s390x__) && defined(INLINE_FMA) && defined(RESCHEDULE_FMA) && defined(DO_PREFETCH)
	  for (; j < p - 31; j+=32) {
	      Ipj = p * fNRows + j;
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
#define	doIteration(offset)	\
			"ld %12," #offset "(%13);"	\
			"ld %0," #offset "(%4);"	\
			"ld %1," #offset "(%5);"	\
			"ld %2," #offset "(%6);"	\
			"ld %3," #offset "(%7);"	\
		  	"madbr %0,%8,%12;"			\
		  	"madbr %1,%9,%12;"			\
		  	"madbr %2,%10,%12;"			\
		  	"madbr %3,%11,%12;"			\
		  	"std %0," #offset "(%4);"	\
		  	"std %1," #offset "(%5);"	\
		  	"std %2," #offset "(%6);"	\
		  	"std %3," #offset "(%7);"
		  __asm__ __volatile__ (
			"pfd %15,%14(%13);"
			"pfd %15,%14(%4);"
			"pfd %15,%14(%5);"
			"pfd %15,%14(%6);"
			"pfd %15,%14(%7);"
			doIteration(0)
			doIteration(8)
			doIteration(16)
			doIteration(24)
			doIteration(32)
			doIteration(40)
			doIteration(48)
			doIteration(56)
			doIteration(64)
			doIteration(72)
			doIteration(80)
			doIteration(88)
			doIteration(96)
			doIteration(104)
			doIteration(112)
			doIteration(120)
			doIteration(128)
			doIteration(136)
			doIteration(144)
			doIteration(152)
			doIteration(160)
			doIteration(168)
			doIteration(176)
			doIteration(184)
			doIteration(192)
			doIteration(200)
			doIteration(208)
			doIteration(216)
			doIteration(224)
			doIteration(232)
			doIteration(240)
			doIteration(248)
#undef	doIteration
		  	:
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p),
		  	  "f"(4.0), "a" (&fSym[Ipj]), "K"(PREFETCH_DISTANCE), "I"(PREFETCH_CODE)
		  );
	  }
#endif
	  for (; j < p; j++)
	    {
#ifdef	UNROLLED_SCALAR_COUNT_ITERS
		  num_iters_jpi++;
#endif		
	      Ipj = p * fNRows + j;
	      Ii0j = (i+0) * fNRows + j;
	      Ii1j = (i+1) * fNRows + j;
	      Ii2j = (i+2) * fNRows + j;
	      Ii3j = (i+3) * fNRows + j;
		  Apj = fSym[ Ipj ];
#if	defined(INLINE_FMA) && defined(__s390x__)
#if	defined(RESCHEDULE_FMA)
		  __asm__ __volatile__ (
		    "ld %0,0(%4);"
		    "ld %1,0(%5);"
		    "ld %2,0(%6);"
		    "ld %3,0(%7);"
		  	"madbr %0,%8,%12;"
		  	"madbr %1,%9,%12;"
		  	"madbr %2,%10,%12;"
		  	"madbr %3,%11,%12;"
		  	"std %0,0(%4);"
		  	"std %1,0(%5);"
		  	"std %2,0(%6);"
		  	"std %3,0(%7);"
		  	: 
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p), "f" (Apj)
		  );
#else
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii0j])
		  	: "f" (Ai0p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii1j])
		  	: "f" (Ai1p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii2j])
		  	: "f" (Ai2p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii3j])
		  	: "f" (Ai3p), "f" (Apj)
		  );
#endif
#else
	      fSym[ Ii0j ] += Ai0p * Apj;
	      fSym[ Ii1j ] += Ai1p * Apj;
	      fSym[ Ii2j ] += Ai2p * Apj;
	      fSym[ Ii3j ] += Ai3p * Apj;
#endif
	    }
	  // Skip the pivot column
	  j++;
#if		defined(__powerpc64__) && defined(INLINE_FMA) && defined(INLINE_VECTOR)
	  Ai0pv.df[0] = Ai0pv.df[1] = Ai0p;
	  Ai1pv.df[0] = Ai1pv.df[1] = Ai1p;
	  Ai2pv.df[0] = Ai2pv.df[1] = Ai2p;
	  Ai3pv.df[0] = Ai3pv.df[1] = Ai3p;
	  for (; j <= i - 1; j+=2) {
	      Ii0j = fNRows*(i+0)+j;
	      Ii1j = fNRows*(i+1)+j;
	      Ii2j = fNRows*(i+2)+j;
	      Ii3j = fNRows*(i+3)+j;
	      v2df	Apjv;
	      __asm__ __volatile__ (
	      	"xxmrghd %0,%1,%2;"
	      	: "=wd"(Apjv)
	      	: "wd"(fSym[(j+0)*fNRows+p]), "wd"(fSym[(j+1)*fNRows+p]) 
	      );
	      *((v2df *) &fSym[Ii0j])	+= Ai0pv.v * Apjv;
   	      *((v2df *) &fSym[Ii1j])	+= Ai1pv.v * Apjv;
	      *((v2df *) &fSym[Ii2j])	+= Ai2pv.v * Apjv;
   	      *((v2df *) &fSym[Ii3j])	+= Ai3pv.v * Apjv;
	  }
#endif
#if		defined(__s390x__) && defined(INLINE_FMA) && defined(RESCHEDULE_FMA) && defined(DO_PREFETCH)
	  for (; j <= i - 31; j+=32) {
	      Ipj = j * fNRows + p;
	      Ii0j = (i+0) * fNRows + j;
	      Ii1j = (i+1) * fNRows + j;
	      Ii2j = (i+2) * fNRows + j;
	      Ii3j = (i+3) * fNRows + j;
#define	doIteration(offset)	\
			"ldy %12," #offset "000(%13);"	\
			"ld %0," #offset "(%4);"	\
			"ld %1," #offset "(%5);"	\
			"ld %2," #offset "(%6);"	\
			"ld %3," #offset "(%7);"	\
		  	"madbr %0,%8,%12;"			\
		  	"madbr %1,%9,%12;"			\
		  	"madbr %2,%10,%12;"			\
		  	"madbr %3,%11,%12;"			\
		  	"std %0," #offset "(%4);"	\
		  	"std %1," #offset "(%5);"	\
		  	"std %2," #offset "(%6);"	\
		  	"std %3," #offset "(%7);"
		  __asm__ __volatile__ (
			"pfd %15,%14(%4);"
			"pfd %15,%14(%5);"
			"pfd %15,%14(%6);"
			"pfd %15,%14(%7);"
			doIteration(0)
			doIteration(8)
			doIteration(16)
			doIteration(24)
			doIteration(32)
			doIteration(40)
			doIteration(48)
			doIteration(56)
			doIteration(64)
			doIteration(72)
			doIteration(80)
			doIteration(88)
			doIteration(96)
			doIteration(104)
			doIteration(112)
			doIteration(120)
			doIteration(128)
			doIteration(136)
			doIteration(144)
			doIteration(152)
			doIteration(160)
			doIteration(168)
			doIteration(176)
			doIteration(184)
			doIteration(192)
			doIteration(200)
			doIteration(208)
			doIteration(216)
			doIteration(224)
			doIteration(232)
			doIteration(240)
			doIteration(248)
#undef	doIteration
		  	:
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p),
		  	  "f"(4.0), "a" (&fSym[Ipj]), "K"(PREFETCH_DISTANCE), "I"(PREFETCH_CODE)
		  );
	  }
#endif
	  for (; j <= i; j++)
	    {
#ifdef	UNROLLED_SCALAR_COUNT_ITERS
		  num_iters_pji++;
#endif		
	      Ipj = j * fNRows + p;
	      Ii0j = (i+0) * fNRows + j;
	      Ii1j = (i+1) * fNRows + j;
	      Ii2j = (i+2) * fNRows + j;
	      Ii3j = (i+3) * fNRows + j;

	      Apj = fSym[ Ipj ];
#if	defined(INLINE_FMA) && defined(__s390x__)
#if	defined(RESCHEDULE_FMA)
		  __asm__ __volatile__ (
		    "ld %0,0(%4);"
		    "ld %1,0(%5);"
		    "ld %2,0(%6);"
		    "ld %3,0(%7);"
		  	"madbr %0,%8,%12;"
		  	"madbr %1,%9,%12;"
		  	"madbr %2,%10,%12;"
		  	"madbr %3,%11,%12;"
		  	"std %0,0(%4);"
		  	"std %1,0(%5);"
		  	"std %2,0(%6);"
		  	"std %3,0(%7);"
		  	: 
		  	: "f"(0.0), "f"(1.0), "f"(2.0), "f"(3.0),
		  	  "a" (&fSym[Ii0j]), "a" (&fSym[Ii1j]), "a" (&fSym[Ii2j]), "a" (&fSym[Ii3j]),
		  	  "f" (Ai0p), "f" (Ai1p), "f" (Ai2p), "f" (Ai3p), "f" (Apj)
		  );
#else
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii0j])
		  	: "f" (Ai0p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii1j])
		  	: "f" (Ai1p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii2j])
		  	: "f" (Ai2p), "f" (Apj)
		  );
		  __asm__ __volatile__ (
		  	"madbr %0,%1,%2;"
		  	: "+f" (fSym[Ii3j])
		  	: "f" (Ai3p), "f" (Apj)
		  );
#endif
#else
	      fSym[ Ii0j ] += Ai0p * Apj;
	      fSym[ Ii1j ] += Ai1p * Apj;
	      fSym[ Ii2j ] += Ai2p * Apj;
	      fSym[ Ii3j ] += Ai3p * Apj;
#endif
	    }
	  Ipj = j * fNRows + p;
	  Ii1j = (i+1) * fNRows + j;
	  Ii2j = (i+2) * fNRows + j;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii1j ] += Ai1p * Apj;
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = j * fNRows + p;
	  Ii2j = (i+2) * fNRows + j;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii2j ] += Ai2p * Apj;
	  fSym[ Ii3j ] += Ai3p * Apj;
	  j++;
	  Ipj = j * fNRows + p;
	  Ii3j = (i+3) * fNRows + j;
	  Apj = fSym[ Ipj ];
	  fSym[ Ii3j ] += Ai3p * Apj;
	}
      for (; i < fNRows; i++)
	{
	  Iip = i * fNRows + p;
	
	  Aip = fSym[ Iip ];
	  fSym[ Iip ] = absBpp * Aip;
	
	  // The a(i,j) elements, i,j both != p
	  if ( Bpp < 0.0 )
	    {
	      Aip = -Aip;
	    }
	
	  for (j = 0; j < p; j++)
	    {
	      Iij = i * fNRows + j;
	      Ipj = p * fNRows + j;
	
	      Apj = fSym[ Ipj ];
	      fSym[ Iij ] += Aip * Apj;
	    }
	  // Skip the pivot column
	  j++;
	  for (; j <= i; j++)
	    {
	      Iij = i * fNRows + j;
	      Ipj = j * fNRows + p;

	      Apj = fSym[ Ipj ];
	      fSym[ Iij ] += Aip * Apj;
	    }
	}
    }
}
#endif

void initTriMatrix(double *matrix, int COUNT) 
{
  int count	= 1;
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

int compareMatrix(double *mat, double *tri, int size) 
{
  int i, j, count = 0, num_errors = 0;
  for (i = 0; i < size; i++) 
    {
      for (j = 0; j <= i; j++, count++)
	{
	  int Iij = i * size + j;
	  if (mat[Iij] != tri[count])
	    {
	      printf("i=%d j=%d Iij=%d mat=%f tri=%f\n", 
		     i, j, Iij, mat[Iij], tri[count]);
	      num_errors++;
	      //return 0;
	    }
	}
      for (; j < size; j++)
	{
	  int Iij = i * size + j;
	  if (mat[Iij] != 0.0)
	    {
	      printf("i=%d j=%d Iij=%d mat=%f\n", 
		     i, j, Iij, mat[Iij]);
	      num_errors++;
	    }
	}
    }
  if (num_errors) return 0;
  return 1;
}

#define NUM_ITERATIONS 1

int main(int argc, char *argv[])
{
  int count;
  printf("MATRIX_SIZE=%d,NUM_ITERATIONS=%d,%s\n", MATRIX_SIZE, NUM_ITERATIONS,
#ifdef ORIG
	 "ORIG"
#elif defined(SPLIT_SCALAR)
	 "SPLIT_SCALAR"
#elif defined(SPLIT_SIMD)
	 "SPLIT_SIMD"
#elif defined(UNROLLED_SCALAR)
	 "UNROLLED_SCALAR"
#elif defined(UNROLLED_SIMD)
	 "UNROLLED_SIMD"
#else
	 "UNKNOWN???"
#endif
	 );
  int TRI_COUNT = MATRIX_SIZE * (MATRIX_SIZE + 1) / 2;
  int COUNT     = MATRIX_SIZE * MATRIX_SIZE;

  double *matrix1 = (double *)malloc((COUNT+1) * sizeof(double));
  if (((unsigned long)matrix1)%16) matrix1++;

  double *matrix2 = (double *)malloc((COUNT+1) * sizeof(double));
  if (((unsigned long)matrix2)%16) matrix2++;

#if defined(ORIG) || defined(SPLIT_SCALAR) || defined(SPLIT_SIMD)
  // we initialize a triangular matrix
  initTriMatrix(matrix1, TRI_COUNT);
#else
  // we initialize a square matrix (but use only the lower triangular part)
  initMatrix(matrix1, MATRIX_SIZE);
#endif

  reset_and_start_stimer();
  for (count = 0; count < NUM_ITERATIONS; count++) {
#ifdef ORIG
    tri_sweep_orig(matrix1);
#elif defined(SPLIT_SCALAR) || defined(SPLIT_SIMD)
    tri_sweep(matrix1);
#elif defined(UNROLLED_SIMD)
    sweep_unrolled_simd(matrix1);
#elif defined(UNROLLED_SCALAR)
    sweep_unrolled_scalar(matrix1);
#else
    printf("unknown option\n");
#endif
  }
  double dt = get_elapsed_seconds();
  printf("Time = %f seconds\n", dt);

#ifdef	UNROLLED_SCALAR_COUNT_ITERS
  printf("num_iters_jip = %ld, num_iters_jpi = %ld, num_iters_pji = %ld\n",
	 num_iters_jip, num_iters_jpi, num_iters_pji);
#endif

  initTriMatrix(matrix2, TRI_COUNT);
  
  for (count = 0; count < NUM_ITERATIONS; count++)
    tri_sweep_orig(matrix2);
  
#if defined(ORIG) || defined(SPLIT_SCALAR) || defined(SPLIT_SIMD)
      if (compareTriMatrix(matrix1, matrix2, TRI_COUNT))
#else
	if (compareMatrix(matrix1, matrix2, MATRIX_SIZE))
#endif
	  {
	    printf("PASSED!\n");
	  } 
	else
	  {
	    printf("FAILED!\n");
	    return -1;
	  }

  return 0;
}
