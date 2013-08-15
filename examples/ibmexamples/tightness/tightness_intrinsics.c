#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <timing.h>

#define VECTSIZE 2048 * 1024
int vectsize = VECTSIZE;
// select one of the following five options
//#define TIGHTNESS_1
//#define TIGHTNESS_2
//#define TIGHTNESS_3
//#define TIGHTNESS_4

// define(SIMD)   -> use logd2() and other intrinsics for p7 vsx
// define(VECTOR) -> use vlog() function
// define(SIMD) && define(VECTOR) -> use vlog() function and other intrinsics for p7 vsx
// define(FAST_LOG) -> use flog() function
// none:   base scalar code
//#define SIMD 1
//#define VECTOR 1

#define UNROLL 8
//#define FAST_LOG

#ifdef SIMD
#include <mass_simdp7.h>
vector double logd2(vector double);
vector double log1pd2(vector double);
const vector double vzero = (vector double)(0.0);
#endif

double *A;
double *B;
double *T;
double *T1;
double simple_tightness_1(int N, double *A, double *B);
double simple_tightness_2(int N, double *A, double *B);
double simple_tightness_3(int N, double *A, double *B);
double simple_tightness_4(int N, double *A, double *B);

void init_vect(int N, double *A, double *B)
{
  int i;
  for (i = 0; i < N; i++)
    {
      A[i] = (double)(i+1)*1000.0;
      //if ((i%4) == 0) { A[i] *= -1.0; }
      B[i] = (double)(i+2);
    }
}

#ifdef	PARALLEL
#if (UNROLL!=1) && (UNROLL!=2) && (UNROLL!=4) && (UNROLL!=8) && (UNROLL!=16)
#error -1 unsupported UNROLL count
#endif
#include	"mt.h"
extern micro_thread_t	_mt[SMT_LEVEL];

typedef struct {
	int					i_begin;
	int					i_end;
	double				x;
	double const		*A;
	int 				n;
	int 				vlen;
}	kernel_args_t;

static kernel_args_t	_kernel_args[SMT_LEVEL];

void initKernelArgs(kernel_args_t *args, double const *A, const int n, const int vlen) {
	args->A		= A;
	args->n		= n;
	args->vlen	= vlen;
}

void doKernelFor(void *args) {
	kernel_args_t	*kernel_args	= (kernel_args_t *) args;
	int				i_begin			= kernel_args->i_begin;
	int				i_end			= kernel_args->i_end;
#if !defined(SMT1) && !defined(SMT2) && !defined(SMT4) && !defined(SMT12) && !defined(SMT14)
	double const	*A				= kernel_args->A;
	int const		n			= kernel_args->n;
	int 			vlen			= kernel_args->vlen;
#else
	int const		n			= kernel_args->n;
	int 			vlen			= UNROLL;
#endif
	double			E;
	int				i;
	double			x				= 0.0;
	double			T[UNROLL];
	double			B[UNROLL];
	for (i = i_begin; i < i_end - (UNROLL - 1); i += UNROLL) {
		E		= 0.0;
		T[0]	= A[i+0]>0 ? ((double) A[i+0]/(double) n) : 1.0;
#if (UNROLL >= 2)
		T[1]	= A[i+1]>0 ? ((double) A[i+1]/(double) n) : 1.0;
#endif
#if (UNROLL >= 4)
		T[2]	= A[i+2]>0 ? ((double) A[i+2]/(double) n) : 1.0;
		T[3]	= A[i+3]>0 ? ((double) A[i+3]/(double) n) : 1.0;
#endif
#if (UNROLL >= 8)
		T[4]	= A[i+4]>0 ? ((double) A[i+4]/(double) n) : 1.0;
		T[5]	= A[i+5]>0 ? ((double) A[i+5]/(double) n) : 1.0;
		T[6]	= A[i+6]>0 ? ((double) A[i+6]/(double) n) : 1.0;
		T[7]	= A[i+7]>0 ? ((double) A[i+7]/(double) n) : 1.0;
#endif
#if (UNROLL == 16)
		T[8]	= A[i+8]>0 ? ((double) A[i+8]/(double) n) : 1.0;
		T[9]	= A[i+9]>0 ? ((double) A[i+9]/(double) n) : 1.0;
		T[10]	= A[i+10]>0 ? ((double) A[i+10]/(double) n) : 1.0;
		T[11]	= A[i+11]>0 ? ((double) A[i+11]/(double) n) : 1.0;
		T[12]	= A[i+12]>0 ? ((double) A[i+12]/(double) n) : 1.0;
		T[13]	= A[i+13]>0 ? ((double) A[i+13]/(double) n) : 1.0;
		T[14]	= A[i+14]>0 ? ((double) A[i+14]/(double) n) : 1.0;
		T[15]	= A[i+15]>0 ? ((double) A[i+15]/(double) n) : 1.0;
#endif

#ifdef ENABLE_VLOG
		vlog(B, T, &vlen);
#else
		B[0] = log(T[0]);
#if (UNROLL >= 2)
		B[1] = log(T[1]);
#endif
#if (UNROLL >= 4)
		B[2] = log(T[2]);
		B[3] = log(T[3]);
#endif
#if (UNROLL >= 8)
		B[4] = log(T[4]);
		B[5] = log(T[5]);
		B[6] = log(T[6]);
		B[7] = log(T[7]);
#endif
#if (UNROLL == 16)
		B[8] = log(T[8]);
		B[9] = log(T[9]);
		B[10] = log(T[10]);
		B[11] = log(T[11]);
		B[12] = log(T[12]);
		B[13] = log(T[13]);
		B[14] = log(T[14]);
		B[15] = log(T[15]);
#endif
#endif
#if (UNROLL == 1)
		E		-= T[0]*B[0];
#elif (UNROLL == 2)
		E		-= T[0]*B[0]+T[1]*B[1];
#elif (UNROLL == 4)
		E		-= T[0]*B[0]+T[1]*B[1]+T[2]*B[2]+T[3]*B[3];
#elif (UNROLL == 8)
		E		-= T[0]*B[0]+T[1]*B[1]+T[2]*B[2]+T[3]*B[3]+T[4]*B[4]+T[5]*B[5]+T[6]*B[6]+T[7]*B[7];
#elif (UNROLL == 16)
		E		-= T[0]*B[0]+T[1]*B[1]+T[2]*B[2]+T[3]*B[3]+T[4]*B[4]+T[5]*B[5]+T[6]*B[6]+T[7]*B[7]
		  +T[8]*B[8]+T[9]*B[9]+T[10]*B[10]+T[11]*B[11]+T[12]*B[12]+T[13]*B[13]+T[14]*B[14]+T[15]*B[15];
#endif
		x		+= E;
	}
	vlen	= 1;
	for (; i < i_end; i++) {
		E		= 0.0;
		T[0]	= A[i+0]>0 ? ((double) A[i+0]/(double) n) : 1.0;
#ifdef ENABLE_VLOG
		vlog(B, T, &vlen);
#else
		B[0] = log(T[0]);
#endif
		E		-= T[0]*B[0];
		x		+= E;
	}
	kernel_args->x	= x;
}

void spawnKernelFor(int thread_id, int i_begin, int i_end) {
	kernel_args_t	*kernel_args	= &_kernel_args[thread_id];
	kernel_args->i_begin			= i_begin;
	kernel_args->i_end				= i_end;
#if	0
	printf("Thread %d i = %d-%d\n", thread_id, i_begin, i_end);
	fflush(stdout);
#endif
	micro_thread_run(&_mt[thread_id], doKernelFor, kernel_args);
}
#endif	/* PARALLEL */

#ifdef	TIGHTNESS_1
void verify_vect_1(int N, double result, double *A, double *B)
{
  int i;
  double x = 0; double E = 0; int n = N; double q;
  for (i = 0; i < N; i++)
    {
      E = 0;
      q = (double)A[i]/(double)n;
      if (q > 0)
	E -= q * log(q);
      x += E;
    }
  if ((x != 0.0 && fabs((x -result)/x) > 0.00001) ||
      (x == 0.0 && result != 0.0))
    {
      printf("result %f != %f\n", result, x);
    }
  else
    {
      printf("result %f == %f\n", result, x);
    }
}
#endif

#ifdef	TIGHTNESS_2
void verify_vect_2(int N, double result, double *A, double *B)
{
  int i;
  double x = 0;
  for (i = 0; i < N; i++)
    {
      x += log(1.0 + A[i]/B[i]);
    }
  if ((x != 0.0 && fabs((x -result)/x) > 0.00001) ||
      (x == 0.0 && result != 0.0))
    {
      printf("result %f != %f\n", result, x);
    }
  else
    {
      printf("result %f == %f\n", result, x);
    }
}
#endif

#ifdef	TIGHTNESS_3
void verify_vect_3(int N, double result, double *A, double *B)
{
  int i, count = 0;
  double x, diff, diff_total = 0.0;
  for (i = 0; i < N; i++)
    {
      x = log(A[i]);
      diff = fabs(x - B[i])/x;
      if (diff > 0.00001) {
	count++; diff_total += diff;
#if 0
      } else {
	printf ("%d %f==%f\n", i, x, B[i]);
#endif
      }
    }
  printf("count=%d diff_total=%f\n", count, diff_total);
}
#endif

#ifdef	TIGHTNESS_4
void verify_vect_4(int N, double result, double *A, double *B)
{
  int i;
  double x = 0;
  for (i = 0; i < N; i++)
    {
      x += log(A[i]);
    }
  if ((x != 0.0 && fabs((x -result)/x) > 0.00001) ||
      (x == 0.0 && result != 0.0))
    {
      printf("result %f != %f\n", result, x);
    }
  else
    {
      printf("result %f == %f\n", result, x);
    }
}
#endif

void print_config()
{
  printf("SIMD=%s,VECTOR=%s,UNROLL=%d,FAST_LOG=%s,%s\n",
#ifdef SIMD
	 "YES",
#else
	 "NO",
#endif
#ifdef VECTOR
	 "YES",
#else
	 "NO",
#endif
	 UNROLL,
#ifdef	FAST_LOG
	 "YES",
#else
	 "NO",
#endif
#ifdef TIGHTNESS_1
	 "TIGHTNESS_1"
#elif defined(TIGHTNESS_2)
	 "TIGHTNESS_2"
#elif defined(TIGHTNESS_3)
	 "TIGHTNESS_3"
#elif defined(TIGHTNESS_4)
	 "TIGHTNESS_4"
#else
	 "UNKNOWN???"
#endif
	 );
}

volatile double result;

//#include	"funcAddr.h"

unsigned int num_threads = 1;

void usage(char *cmd)
{
  printf("usage: %s [-vectsize <vectsize>] [-repeats <repeat_count>] [-num_threads <num_threads>] [-verify]\n", cmd);
  exit(-1);
}

 
#ifdef TIGHTNESS_2
void test(int N, double *A, double *B)
   {
   int i;
  double x = 0;
#if (UNROLL == 8)
  for (i = 0; i < N; i+=8)
    {
#ifdef	FAST_LOG
         x += flog(1.0 + A[i+0]/B[i+0]);
         x += flog(1.0 + A[i+1]/B[i+1]);
         x += flog(1.0 + A[i+2]/B[i+2]);
         x += flog(1.0 + A[i+3]/B[i+3]);
         x += flog(1.0 + A[i+4]/B[i+4]);
         x += flog(1.0 + A[i+5]/B[i+5]);
         x += flog(1.0 + A[i+6]/B[i+6]);
         x += flog(1.0 + A[i+7]/B[i+7]);
#else
         x += log(1.0 + A[i+0]/B[i+0]);
         x += log(1.0 + A[i+1]/B[i+1]);
         x += log(1.0 + A[i+2]/B[i+2]);
         x += log(1.0 + A[i+3]/B[i+3]);
         x += log(1.0 + A[i+4]/B[i+4]);
         x += log(1.0 + A[i+5]/B[i+5]);
         x += log(1.0 + A[i+6]/B[i+6]);
         x += log(1.0 + A[i+7]/B[i+7]);
#endif
       }
#elif (UNROLL == 4)
     for (i = 0; i < N; i+=4)
       {
#ifdef	FAST_LOG
         x += flog(1.0 + A[i+0]/B[i+0]);
         x += flog(1.0 + A[i+1]/B[i+1]);
         x += flog(1.0 + A[i+2]/B[i+2]);
         x += flog(1.0 + A[i+3]/B[i+3]);
#else
         x += log(1.0 + A[i+0]/B[i+0]);
         x += log(1.0 + A[i+1]/B[i+1]);
         x += log(1.0 + A[i+2]/B[i+2]);
         x += log(1.0 + A[i+3]/B[i+3]);
#endif
       }
#elif (UNROLL == 2)
     for (i = 0; i < N; i+=2)
       {
#ifdef	FAST_LOG
         x += flog(1.0 + A[i+0]/B[i+0]);
         x += flog(1.0 + A[i+1]/B[i+1]);
#else
         x += log(1.0 + A[i+0]/B[i+0]);
         x += log(1.0 + A[i+1]/B[i+1]);
#endif
       }
#else
     for (i = 0; i < N; i++)
       {
#ifdef	FAST_LOG
         x += flog(1.0 + A[i]/B[i]);
#else
         x += log(1.0 + A[i]/B[i]);
#endif
    }
#endif


   }
#endif

#ifdef TIGHTNESS_3
void test(int N, double *A, double *B)
   {
   int i;
#if (UNROLL == 8)
  for (i = 0; i < N; i+=8)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
      B[i+2] = flog(A[i+2]);
      B[i+3] = flog(A[i+3]);
      B[i+4] = flog(A[i+4]);
      B[i+5] = flog(A[i+5]);
      B[i+6] = flog(A[i+6]);
      B[i+7] = flog(A[i+7]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
      B[i+2] = log(A[i+2]);
      B[i+3] = log(A[i+3]);
      B[i+4] = log(A[i+4]);
      B[i+5] = log(A[i+5]);
      B[i+6] = log(A[i+6]);
      B[i+7] = log(A[i+7]);
#endif
    }
#elif (UNROLL == 4)
  for (i = 0; i < N; i+=4)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
      B[i+2] = flog(A[i+2]);
      B[i+3] = flog(A[i+3]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
      B[i+2] = log(A[i+2]);
      B[i+3] = log(A[i+3]);
#endif
    }
#elif (UNROLL == 2)
  for (i = 0; i < N; i+=2)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
#endif
    }
#else
  for (i = 0; i < N; i++)
    {
#ifdef	FAST_LOG
      B[i] = flog(A[i]);
#else
      B[i] = log(A[i]);
#endif
    }
#endif
  }
#endif
int main(int argc, char *argv[])
{
  int verify = 0;
  long int MAX_COUNT = 1; long int count;
  double a = 1.0;
  struct tms cpustTime;
  struct tms cpuedTime;

#ifdef	PARALLEL
  int	thread_id;
#endif
  /*printf("tightness entry = %p\n",
#ifdef TIGHTNESS_1
  funcAddr(simple_tightness_1)
#elif defined(TIGHTNESS_2)
  funcAddr(simple_tightness_2)
#elif defined(TIGHTNESS_3)
  funcAddr(simple_tightness_3)
#elif defined(TIGHTNESS_4)
  funcAddr(simple_tightness_4)
#else
  (void *) 0
#endif
  );*/
  if (argc > 1) {
    //MAX_COUNT = atol(argv[1]);
    int c = 1;
    while (c < argc) {
      if (strcmp(argv[c], "-verify") == 0) {
	verify = 1;
	c++;
      } else if (strcmp(argv[c], "-num_threads") == 0) {
	c++;
	num_threads = atol(argv[c]);
	c++;
	/*if (num_threads > SMT_LEVEL) {
	  printf("num_threads %d should be no greater than %d\n", num_threads, SMT_LEVEL);
	  exit(-1);
	}*/
      } else if (strcmp(argv[c], "-vectsize") == 0) {
	c++;
	vectsize = atol(argv[c]);
	c++;
	if (vectsize > VECTSIZE) {
	  printf("vectsize %d should be no greater than %d\n", vectsize, VECTSIZE);
	  exit(-1);
	}
      } else if (strcmp(argv[c], "-repeats") == 0) {
	c++;
	MAX_COUNT = atol(argv[c]);
	c++;
      } else {
	usage(argv[0]);
      }
    }
  } else {
    usage(argv[0]);
  }
#ifdef SMT4
  num_threads = 4;
#elif defined(SMT2)
  num_threads = 2;
#elif defined(SMT1) || defined(SMT12) || defined(SMT14)
  num_threads = 1;
#endif
  printf("VECTSIZE=%d vectsize=%d MAX_COUNT=%ld verify=%d num_threads=%d %s %s\n", VECTSIZE, vectsize, MAX_COUNT, verify, num_threads,
#ifdef ENABLE_VLOG
	 "ENABLE_VLOG"
#else
	 "DISABLE_VLOG"
#endif
	 ,
#ifdef SMT4
	 "SMT4"
#elif defined(SMT2)
	 "SMT2"
#elif defined(SMT1)
	 "SMT1"
#elif defined(SMT12)
	 "SMT12"
#elif defined(SMT14)
	 "SMT14"
#else
	 "SMT"
#endif
	 );
  A = (double *)malloc((VECTSIZE+1)*sizeof(double));
  if (((unsigned long)A)&(sizeof(double)*2-1)) A++;
  B = (double *)malloc((VECTSIZE+1)*sizeof(double));
  if (((unsigned long)B)&(sizeof(double)*2-1)) B++;
  T = (double *)malloc((VECTSIZE+1)*sizeof(double));
  if (((unsigned long)T)&(sizeof(double)*2-1)) T++;
  T1= (double *)malloc((VECTSIZE+1)*sizeof(double));
  if (((unsigned long)T1)&(sizeof(double)*2-1)) T1++;

  print_config();

  init_vect(VECTSIZE, A, B);

#ifdef	PARALLEL
  mt_symbols();
  mt_init(); // start collecting a trace
  for (thread_id = 1; thread_id < num_threads; thread_id++)
	  micro_thread_create(&_mt[thread_id]);
#endif

  reset_and_start_stimer();
  for (count = 0; count < MAX_COUNT; count++) 
    {
        //clock_t c = clock();
	//times(&cpustTime);
#ifdef TIGHTNESS_1
      result = simple_tightness_1(vectsize, A, B);
#elif defined(TIGHTNESS_2)
      result = simple_tightness_2(VECTSIZE, A, B);
#elif defined(TIGHTNESS_3)
      result = simple_tightness_3(VECTSIZE, A, B);
#elif defined(TIGHTNESS_4)
      result = simple_tightness_4(VECTSIZE, A, B);
#else
      printf("unknown kernel\n");
#endif
      //c = clock() - c;      
      //printf ("%f\n", ((float)c) * 1000/CLOCKS_PER_SEC);
 
    }
  double dt = get_elapsed_seconds();
  printf("Time = %f seconds\n", dt);
	//times(&cpuedTime);
	//printf("%f\n", ((double)cpuedTime.tms_utime - (double) cpustTime.tms_utime) * (double)1000 / CLK_TCK);
     //printf("SIMD takes %d clock ticks (%f miliseconds) to finish\n",c , ((float)c)/CLOCKS_PER_SEC * 1000 );
      //printf ("avg run time: %f", ((float)c) * 1000/CLOCKS_PER_SEC / count);

#ifdef	PARALLEL
  for (thread_id = 1; thread_id < num_threads; thread_id++)
	  micro_thread_stop(&_mt[thread_id]);
  mt_exit(); // stop collecting a trace
#endif


  if (verify) 
    {
#ifdef TIGHTNESS_1
      verify_vect_1(vectsize, result, A, B);
#elif defined(TIGHTNESS_2)
      verify_vect_2(VECTSIZE, result, A, B);
#elif defined(TIGHTNESS_3)
      verify_vect_3(VECTSIZE, result, A, B);
#elif defined(TIGHTNESS_4)
      verify_vect_4(VECTSIZE, result, A, B);
#else
      printf("unknown kernel\n");
#endif
    }
}

#ifdef	TIGHTNESS_1
#ifdef	PARALLEL
#ifdef  SMT4
double simple_tightness_1(int N, double *A, double *B)
{
  double x;
  int chunk_size	= N / 4;

  _kernel_args[1].n             = N;
  _kernel_args[1].i_begin	= 0;
  _kernel_args[1].i_end		= chunk_size;
  _kernel_args[2].n             = N;
  _kernel_args[2].i_begin	= chunk_size;
  _kernel_args[2].i_end		= chunk_size*2;
  _kernel_args[3].n             = N;
  _kernel_args[3].i_begin	= chunk_size*2;
  _kernel_args[3].i_end		= chunk_size*3;

  //micro_thread_run(_mt+1, doKernelFor, _kernel_args+1);
   micro_thread_t *mt = _mt+3;
   mt->fn    = (void (*)(void *))doKernelFor;
   mt->data  = (void *)(_kernel_args+3);
   _unpause_store_mt3(THREAD_RUNNING, mt);
   mt--;
   mt->fn    = (void (*)(void *))doKernelFor;
   mt->data  = (void *)(_kernel_args+2);
   _unpause_store_mt2(THREAD_RUNNING, mt);
   mt--;
   mt->fn    = (void (*)(void *))doKernelFor;
   mt->data  = (void *)(_kernel_args+1);
   _unpause_store_mt1(THREAD_RUNNING, mt);

  _kernel_args[0].n             = N;
  _kernel_args[0].i_begin	= chunk_size*3;
  _kernel_args[0].i_end		= N;

  doKernelFor(_kernel_args);

  //micro_thread_wait(_mt+1);
  mt = _mt+3;
  cap_mt3(THREAD_RUNNING, mt);
  mt--;
  cap_mt2(THREAD_RUNNING, mt);
  mt--;
  cap_mt1(THREAD_RUNNING, mt);

  x  = _kernel_args[1].x;
  x += _kernel_args[2].x;
  x += _kernel_args[3].x;
  x += _kernel_args[0].x;

  return x;
}
#elif defined(SMT14)
double simple_tightness_1(int N, double *A, double *B)
{
  double x;
  int chunk_size	= N / 4;

  _kernel_args[3].n             = N;
  _kernel_args[3].i_begin	= chunk_size*2;
  _kernel_args[3].i_end		= chunk_size*3;
  _kernel_args[2].n             = N;
  _kernel_args[2].i_begin	= chunk_size;
  _kernel_args[2].i_end		= chunk_size*2;
  _kernel_args[1].n             = N;
  _kernel_args[1].i_begin	= 0;
  _kernel_args[1].i_end		= chunk_size;
  _kernel_args[0].n             = N;
  _kernel_args[0].i_begin	= chunk_size*3;
  _kernel_args[0].i_end		= N;

  doKernelFor(_kernel_args+3);
  doKernelFor(_kernel_args+2);
  doKernelFor(_kernel_args+1);
  doKernelFor(_kernel_args);

  x  = _kernel_args[1].x;
  x += _kernel_args[2].x;
  x += _kernel_args[3].x;
  x += _kernel_args[0].x;

  return x;
}
#elif defined(SMT2)
double simple_tightness_1(int N, double *A, double *B)
{
  double x;
  //int vlen = UNROLL;
  int chunk_size	= N / 2;
  _kernel_args[0].n    = N;
  _kernel_args[1].n    = N;

  _kernel_args[1].i_begin	= 0;
  _kernel_args[1].i_end		= chunk_size;

  //micro_thread_run(_mt+1, doKernelFor, _kernel_args+1);
   micro_thread_t *mt = _mt+1;
   mt->fn    = (void (*)(void *))doKernelFor;
   mt->data  = (void *)(_kernel_args+1);
   _unpause_store_mt1(THREAD_RUNNING, mt);

  _kernel_args[0].i_begin	= chunk_size;
  _kernel_args[0].i_end		= N;

  doKernelFor(_kernel_args);

  //micro_thread_wait(_mt+1);
  cap_mt1(THREAD_RUNNING, mt);

  x  = _kernel_args[1].x;
  x += _kernel_args[0].x;

  return x;
}
#elif defined(SMT12)
double simple_tightness_1(int N, double *A, double *B)
{
  double x;
  //int vlen = UNROLL;
  int chunk_size	= N / 2;
  _kernel_args[0].n    = N;
  _kernel_args[1].n    = N;

  _kernel_args[1].i_begin	= 0;
  _kernel_args[1].i_end		= chunk_size;

  doKernelFor(_kernel_args+1);

  _kernel_args[0].i_begin	= chunk_size;
  _kernel_args[0].i_end		= N;

  doKernelFor(_kernel_args);

  x  = _kernel_args[1].x;
  x += _kernel_args[0].x;

  return x;
}
#elif defined(SMT1)
double simple_tightness_1(int N, double *A, double *B)
{
  double x;

  _kernel_args[0].n             = N;
  _kernel_args[0].i_begin	= 0;
  _kernel_args[0].i_end		= N;

  doKernelFor(_kernel_args);

  x  = _kernel_args[0].x;

  return x;
}
#else
double simple_tightness_1(int N, double *A, double *B)
{
  int i;
  double x = 0; double E = 0; int n = N; double q;
  int	chunk_size;
  int	thread_id;

  int vlen = UNROLL;
  chunk_size	= N / num_threads;
  i				= 0;
  initKernelArgs(&_kernel_args[0], A, n, vlen);
  if (chunk_size > 0) {
	  for (thread_id = 1; thread_id < num_threads; thread_id++)
		  initKernelArgs(&_kernel_args[thread_id], A, n, vlen);
	  for (thread_id = 1; thread_id < num_threads; i += chunk_size, thread_id++)
		  spawnKernelFor(thread_id, i, i + chunk_size);
  }
  _kernel_args[0].i_begin	= i;
  _kernel_args[0].i_end		= N;
#if	0
	printf("Thread %d i = %d-%d\n", 0, i, N);
	fflush(stdout);
#endif
  doKernelFor(&_kernel_args[0]);
  if (chunk_size > 0) {
	  for (thread_id = 1; thread_id < num_threads; thread_id++) {
		  micro_thread_wait(&_mt[thread_id]);
		  x += _kernel_args[thread_id].x;
	  }
  }
  x 			+= _kernel_args[0].x;
  return x;
}
#endif
#else /* PARALLEL */
double simple_tightness_1(int N, double *A, double *B)
{
  int i;
  double x = 0; double E = 0; int n = N; double q;
#if defined(SIMD) && !defined(VECTOR)
  // simd version
  vector double *vec_A = (vector double *)A;
  vector double *vec_B = (vector double *)B;
  vector double vec_X0 = vzero, vec_X1 = vzero, vec_X2 = vzero, vec_X3 = vzero;
  vector double vec_X4 = vzero, vec_X5 = vzero, vec_X6 = vzero, vec_X7 = vzero;
  vector double vec_one = vec_splats(1.0);
  vector double vec_zero = vec_splats(0.0);
  vector double vec_n  = vec_splats((double)n);
  vector double vec_q0, vec_q1, vec_q2, vec_q3, vec_q4, vec_q5, vec_q6, vec_q7;
  vector bool long long vec_c0, vec_c1, vec_c2, vec_c3, vec_c4, vec_c5, vec_c6, vec_c7;
  vector double vec_l0, vec_l1, vec_l2, vec_l3, vec_l4, vec_l5, vec_l6, vec_l7;
  vector double vec_S0;
#if (UNROLL == 8)
  for (i = 0; i < N/2; i+=8)
    {
      vec_q0 = vec_div(vec_A[i+0], vec_n);
      vec_q1 = vec_div(vec_A[i+1], vec_n);
      vec_q2 = vec_div(vec_A[i+2], vec_n);
      vec_q3 = vec_div(vec_A[i+3], vec_n);
      vec_q4 = vec_div(vec_A[i+4], vec_n);
      vec_q5 = vec_div(vec_A[i+5], vec_n);
      vec_q6 = vec_div(vec_A[i+6], vec_n);
      vec_q7 = vec_div(vec_A[i+7], vec_n);

      vec_c0 = vec_cmpgt(vec_q0, vec_zero);
      vec_c1 = vec_cmpgt(vec_q1, vec_zero);
      vec_c2 = vec_cmpgt(vec_q2, vec_zero);
      vec_c3 = vec_cmpgt(vec_q3, vec_zero);
      vec_c4 = vec_cmpgt(vec_q4, vec_zero);
      vec_c5 = vec_cmpgt(vec_q5, vec_zero);
      vec_c6 = vec_cmpgt(vec_q6, vec_zero);
      vec_c7 = vec_cmpgt(vec_q7, vec_zero);

      vec_q0 = vec_sel(vec_one, vec_q0, vec_c0);
      vec_q1 = vec_sel(vec_one, vec_q1, vec_c1);
      vec_q2 = vec_sel(vec_one, vec_q2, vec_c2);
      vec_q3 = vec_sel(vec_one, vec_q3, vec_c3);
      vec_q4 = vec_sel(vec_one, vec_q4, vec_c4);
      vec_q5 = vec_sel(vec_one, vec_q5, vec_c5);
      vec_q6 = vec_sel(vec_one, vec_q6, vec_c6);
      vec_q7 = vec_sel(vec_one, vec_q7, vec_c7);

      vec_l0 = logd2(vec_q0);
      vec_l1 = logd2(vec_q1);
      vec_l2 = logd2(vec_q2);
      vec_l3 = logd2(vec_q3);
      vec_l4 = logd2(vec_q4);
      vec_l5 = logd2(vec_q5);
      vec_l6 = logd2(vec_q6);
      vec_l7 = logd2(vec_q7);

      vec_X0 = vec_nmsub(vec_q0, vec_l0, vec_X0);
      vec_X1 = vec_nmsub(vec_q1, vec_l1, vec_X1);
      vec_X2 = vec_nmsub(vec_q2, vec_l2, vec_X2);
      vec_X3 = vec_nmsub(vec_q3, vec_l3, vec_X3);
      vec_X4 = vec_nmsub(vec_q4, vec_l4, vec_X4);
      vec_X5 = vec_nmsub(vec_q5, vec_l5, vec_X5);
      vec_X6 = vec_nmsub(vec_q6, vec_l6, vec_X6);
      vec_X7 = vec_nmsub(vec_q7, vec_l7, vec_X7);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X4 = vec_add(vec_X4, vec_X5);
  vec_X6 = vec_add(vec_X6, vec_X7);

  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_X4 = vec_add(vec_X4, vec_X6);
  vec_X0 = vec_add(vec_X0, vec_X4);

  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 4)
  for (i = 0; i < N/2; i+=4)
    {
      vec_q0 = vec_div(vec_A[i+0], vec_n);
      vec_q1 = vec_div(vec_A[i+1], vec_n);
      vec_q2 = vec_div(vec_A[i+2], vec_n);
      vec_q3 = vec_div(vec_A[i+3], vec_n);
      vec_c0 = vec_cmpgt(vec_q0, vec_zero);
      vec_c1 = vec_cmpgt(vec_q1, vec_zero);
      vec_c2 = vec_cmpgt(vec_q2, vec_zero);
      vec_c3 = vec_cmpgt(vec_q3, vec_zero);
      vec_q0 = vec_sel(vec_one, vec_q0, vec_c0);
      vec_q1 = vec_sel(vec_one, vec_q1, vec_c1);
      vec_q2 = vec_sel(vec_one, vec_q2, vec_c2);
      vec_q3 = vec_sel(vec_one, vec_q3, vec_c3);
      vec_l0 = logd2(vec_q0);
      vec_l1 = logd2(vec_q1);
      vec_l2 = logd2(vec_q2);
      vec_l3 = logd2(vec_q3);
      vec_X0 = vec_nmsub(vec_q0, vec_l0, vec_X0);
      vec_X1 = vec_nmsub(vec_q1, vec_l1, vec_X1);
      vec_X2 = vec_nmsub(vec_q2, vec_l2, vec_X2);
      vec_X3 = vec_nmsub(vec_q3, vec_l3, vec_X3);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 2)
  for (i = 0; i < N/2; i+=2)
    {
      vec_q0 = vec_div(vec_A[i+0], vec_n);
      vec_q1 = vec_div(vec_A[i+1], vec_n);
      vec_c0 = vec_cmpgt(vec_q0, vec_zero);
      vec_c1 = vec_cmpgt(vec_q1, vec_zero);
      vec_q0 = vec_sel(vec_one, vec_q0, vec_c0);
      vec_q1 = vec_sel(vec_one, vec_q1, vec_c1);
      vec_l0 = logd2(vec_q0);
      vec_l1 = logd2(vec_q1);
      vec_X0 = vec_nmsub(vec_q0, vec_l0, vec_X0);
      vec_X1 = vec_nmsub(vec_q1, vec_l1, vec_X1);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#else
  for (i = 0; i < N/2; i++)
    {
      vec_q0 = vec_div(vec_A[i+0], vec_n);
      vec_c0 = vec_cmpgt(vec_q0, vec_zero);
      vec_q0 = vec_sel(vec_one, vec_q0, vec_c0);
      vec_l0 = logd2(vec_q0);
      vec_X0 = vec_nmsub(vec_q0, vec_l0, vec_X0);
    }
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif
#elif defined(VECTOR)
  // vector version
#ifdef SIMD
  // simd version
  vector double *vec_A = (vector double *)A;
  vector double *vec_B = (vector double *)B;
  vector double vec_X0 = vzero, vec_X1 = vzero, vec_X2 = vzero, vec_X3 = vzero;
  vector double vec_one = vec_splats(1.0);
  vector double vec_zero = vec_splats(0.0);
  vector double vec_n  = vec_splats((double)n);
  vector double vec_q0, vec_q1, vec_q2, vec_q3;
  vector bool long long vec_c0, vec_c1, vec_c2, vec_c3;
  vector double vec_t0, vec_t1, vec_t2, vec_t3;
  vector double vec_S0;
  vector double *vec_T = (vector double *)T;
  vector double *vec_T1= (vector double *)T1;
#endif
#if	(UNROLL == -1)
  int	vlen	= N;
  E = 0;
  for (i = 0; i < N; i++) T[i] = A[i] > 0 ? ((double) A[i]/ (double) n) : 1.0;
  vlog(B, T, &vlen);
  for (i = 0; i < N; i++) E -= T[i]*B[i];
  x += E;
#elif (UNROLL == 8)
  int vlen = 8;
  for (i = 0; i < N; i+=8)
    {
#ifdef SIMD
      vec_q0  = vec_div(vec_A[i/2+0], vec_n);
      vec_q1  = vec_div(vec_A[i/2+1], vec_n);
      vec_q2  = vec_div(vec_A[i/2+2], vec_n);
      vec_q3  = vec_div(vec_A[i/2+3], vec_n);
      vec_c0  = vec_cmpgt(vec_q0, vec_zero);
      vec_c1  = vec_cmpgt(vec_q1, vec_zero);
      vec_c2  = vec_cmpgt(vec_q2, vec_zero);
      vec_c3  = vec_cmpgt(vec_q3, vec_zero);
      vec_T[0]= vec_sel(vec_one, vec_q0, vec_c0);
      vec_T[1]= vec_sel(vec_one, vec_q1, vec_c1);
      vec_T[2]= vec_sel(vec_one, vec_q2, vec_c2);
      vec_T[3]= vec_sel(vec_one, vec_q3, vec_c3);
      vlog(T1, T, &vlen);
      vec_t0  = vec_mul(vec_T[0], vec_T1[0]);
      vec_t1  = vec_mul(vec_T[1], vec_T1[1]);
      vec_t2  = vec_mul(vec_T[2], vec_T1[2]);
      vec_t3  = vec_mul(vec_T[3], vec_T1[3]);
      vec_X0  = vec_add(vec_X0, vec_t0);
      vec_X1  = vec_add(vec_X1, vec_t1);
      vec_X2  = vec_add(vec_X2, vec_t2);
      vec_X3  = vec_add(vec_X3, vec_t3);
#else
      E = 0;
      T[0] = (A[i+0]>0)?((double)A[i+0]/(double)n):1.0;
      T[1] = (A[i+1]>0)?((double)A[i+1]/(double)n):1.0;
      T[2] = (A[i+2]>0)?((double)A[i+2]/(double)n):1.0;
      T[3] = (A[i+3]>0)?((double)A[i+3]/(double)n):1.0;
      T[4] = (A[i+4]>0)?((double)A[i+4]/(double)n):1.0;
      T[5] = (A[i+5]>0)?((double)A[i+5]/(double)n):1.0;
      T[6] = (A[i+6]>0)?((double)A[i+6]/(double)n):1.0;
      T[7] = (A[i+7]>0)?((double)A[i+7]/(double)n):1.0;
      vlog(B, T, &vlen);
      E -= (T[0]*B[0]+T[1]*B[1]+T[2]*B[2]+T[3]*B[3]
	    +T[4]*B[4]+T[5]*B[5]+T[6]*B[6]+T[7]*B[7]);
      x += E;
#endif
    }
#ifdef SIMD
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = -1.0 * vec_extract(vec_X0, 0);
#endif
#elif (UNROLL == 4)
  int vlen = 4;
#ifdef	PARALLEL
  chunk_size	= N / num_threads;
  i				= 0;
  initKernelArgs(&_kernel_args[0], A, n, vlen);
  if (chunk_size > 0) {
	  for (thread_id = 1; thread_id < num_threads; thread_id++)
		  initKernelArgs(&_kernel_args[thread_id], A, n, vlen);
	  for (thread_id = 1; thread_id < num_threads; i += chunk_size, thread_id++)
		  spawnKernelFor(thread_id, i, i + chunk_size);
  }
  _kernel_args[0].i_begin	= i;
  _kernel_args[0].i_end		= N;
#if	0
	printf("Thread %d i = %d-%d\n", 0, i, N);
	fflush(stdout);
#endif
  doKernelFor(&_kernel_args[0]);
  if (chunk_size > 0) {
	  for (thread_id = 1; thread_id < num_threads; thread_id++) {
		  micro_thread_wait(&_mt[thread_id]);
		  x += _kernel_args[thread_id].x;
	  }
  }
  x 			+= _kernel_args[0].x;
#else
  for (i = 0; i < N; i+=4)
    {
#ifdef SIMD
      vec_q0  = vec_div(vec_A[i/2+0], vec_n);
      vec_q1  = vec_div(vec_A[i/2+1], vec_n);
      vec_c0  = vec_cmpgt(vec_q0, vec_zero);
      vec_c1  = vec_cmpgt(vec_q1, vec_zero);
      vec_T[0]= vec_sel(vec_one, vec_q0, vec_c0);
      vec_T[1]= vec_sel(vec_one, vec_q1, vec_c1);
      vlog(T1, T, &vlen);
      vec_t0  = vec_mul(vec_T[0], vec_T1[0]);
      vec_t1  = vec_mul(vec_T[1], vec_T1[1]);
      vec_X0  = vec_add(vec_X0, vec_t0);
      vec_X1  = vec_add(vec_X1, vec_t1);
#else
      E = 0;
      T[0] = (A[i+0]>0)?((double)A[i+0]/(double)n):1.0;
      T[1] = (A[i+1]>0)?((double)A[i+1]/(double)n):1.0;
      T[2] = (A[i+2]>0)?((double)A[i+2]/(double)n):1.0;
      T[3] = (A[i+3]>0)?((double)A[i+3]/(double)n):1.0;
      vlog(B, T, &vlen);
      E -= (T[0]*B[0]+T[1]*B[1]+T[2]*B[2]+T[3]*B[3]);
      x += E;
#endif
    }
#endif	/* PARALLEL */
#ifdef SIMD
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = -1.0 * vec_extract(vec_X0, 0);
#endif
#elif (UNROLL == 2)
  int vlen = 2;
  for (i = 0; i < N; i+=2)
    {
#ifdef SIMD
      vec_q0  = vec_div(vec_A[i/2+0], vec_n);
      vec_c0  = vec_cmpgt(vec_q0, vec_zero);
      vec_T[0]= vec_sel(vec_one, vec_q0, vec_c0);
      vlog(T1, T, &vlen);
      vec_t0  = vec_mul(vec_T[0], vec_T1[0]);
      vec_X0  = vec_add(vec_X0, vec_t0);
#else
      E = 0;
      T[0] = (A[i+0]>0)?((double)A[i+0]/(double)n):1.0;
      T[1] = (A[i+1]>0)?((double)A[i+1]/(double)n):1.0;
      vlog(B, T, &vlen);
      E -= (T[0]*B[0]+T[1]*B[1]);
      x += E;
#endif
    }
#ifdef SIMD
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = -1.0 * vec_extract(vec_X0, 0);
#endif
#else
  int vlen = 1;
  for (i = 0; i < N; i++)
    {
      E = 0;
      T[0] = (double)A[i]/(double)n;
      if (T[0] > 0)
	{
	  vlog(B, T, &vlen);
	  E -= T[0] * B[0];
	}
      x += E;
    }
#endif
#else
  // scalar version
#if (UNROLL == 8)
  printf("not implemented\n");
  for (i = 0; i < N; i+=8)
    {
    }
#elif (UNROLL == 4)
  printf("not implemented\n");
  for (i = 0; i < N; i+=4)
    {
    }
#elif (UNROLL == 2)
  printf("not implemented\n");
  for (i = 0; i < N; i+=2)
    {
    }
#else
  for (i = 0; i < N; i++)
    {
      E = 0;
      q = (double)A[i]/(double)n;
      if (q > 0)
#ifdef	FAST_LOG
	E -= q * flog(q);
#else
	E -= q * log(q);
#endif
      x += E;
    }
#endif
#endif
  return x;
}
#endif
#endif

#ifdef	TIGHTNESS_2
double simple_tightness_2(int N, double *A, double *B)
{
  int i;
  double x = 0;
#if defined(SIMD) && !defined(VECTOR)
  // simd version
  vector double *vec_A = (vector double *)A;
  vector double *vec_B = (vector double *)B;
  vector double vec_X0 = vzero, vec_X1 = vzero, vec_X2 = vzero, vec_X3 = vzero;
  vector double vec_X4 = vzero, vec_X5 = vzero, vec_X6 = vzero, vec_X7 = vzero;
  vector double vec_one = vec_splats(1.0);
  vector double vec_d0, vec_d1, vec_d2, vec_d3, vec_d4, vec_d5, vec_d6, vec_d7;
  vector double vec_a0, vec_a1, vec_a2, vec_a3, vec_a4, vec_a5, vec_a6, vec_a7;
  vector double vec_l0, vec_l1, vec_l2, vec_l3, vec_l4, vec_l5, vec_l6, vec_l7;
  vector double vec_S0;
#if (UNROLL == 8)
  for (i = 0; i < N/2; i+=8)
    {
      vec_d0 = vec_div(vec_A[i+0], vec_B[i+0]);
      vec_d1 = vec_div(vec_A[i+1], vec_B[i+1]);
      vec_d2 = vec_div(vec_A[i+2], vec_B[i+2]);
      vec_d3 = vec_div(vec_A[i+3], vec_B[i+3]);
      vec_d4 = vec_div(vec_A[i+4], vec_B[i+4]);
      vec_d5 = vec_div(vec_A[i+5], vec_B[i+5]);
      vec_d6 = vec_div(vec_A[i+6], vec_B[i+6]);
      vec_d7 = vec_div(vec_A[i+7], vec_B[i+7]);
      vec_l0 = log1pd2(vec_d0);
      vec_l1 = log1pd2(vec_d1);
      vec_l2 = log1pd2(vec_d2);
      vec_l3 = log1pd2(vec_d3);
      vec_l4 = log1pd2(vec_d4);
      vec_l5 = log1pd2(vec_d5);
      vec_l6 = log1pd2(vec_d6);
      vec_l7 = log1pd2(vec_d7);
      vec_X0 = vec_add(vec_X0, vec_l0);
      vec_X1 = vec_add(vec_X1, vec_l1);
      vec_X2 = vec_add(vec_X2, vec_l2);
      vec_X3 = vec_add(vec_X3, vec_l3);
      vec_X4 = vec_add(vec_X4, vec_l4);
      vec_X5 = vec_add(vec_X5, vec_l5);
      vec_X6 = vec_add(vec_X6, vec_l6);
      vec_X7 = vec_add(vec_X7, vec_l7);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X4 = vec_add(vec_X4, vec_X5);
  vec_X6 = vec_add(vec_X6, vec_X7);

  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_X4 = vec_add(vec_X4, vec_X6);
  vec_X0 = vec_add(vec_X0, vec_X4);

  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 4)
  for (i = 0; i < N/2; i+=4)
    {
      vec_d0 = vec_div(vec_A[i+0], vec_B[i+0]);
      vec_d1 = vec_div(vec_A[i+1], vec_B[i+1]);
      vec_d2 = vec_div(vec_A[i+2], vec_B[i+2]);
      vec_d3 = vec_div(vec_A[i+3], vec_B[i+3]);
      vec_l0 = log1pd2(vec_d0);
      vec_l1 = log1pd2(vec_d1);
      vec_l2 = log1pd2(vec_d2);
      vec_l3 = log1pd2(vec_d3);
      vec_X0 = vec_add(vec_X0, vec_l0);
      vec_X1 = vec_add(vec_X1, vec_l1);
      vec_X2 = vec_add(vec_X2, vec_l2);
      vec_X3 = vec_add(vec_X3, vec_l3);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 2)
  for (i = 0; i < N/2; i+=2)
    {
      vec_d0 = vec_div(vec_A[i+0], vec_B[i+0]);
      vec_d1 = vec_div(vec_A[i+1], vec_B[i+1]);
      vec_l0 = log1pd2(vec_d0);
      vec_l1 = log1pd2(vec_d1);
      vec_X0 = vec_add(vec_X0, vec_l0);
      vec_X1 = vec_add(vec_X1, vec_l1);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#else
  for (i = 0; i < N/2; i++)
    {
      vec_d0 = vec_div(vec_A[i+0], vec_B[i+0]);
      vec_l0 = log1pd2(vec_d0);
      vec_X0 = vec_add(vec_X0, vec_l0);
    }
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif
#elif defined(VECTOR)
  // vector version
#ifdef SIMD
  vector double *vec_A = (vector double *)A;
  vector double *vec_B = (vector double *)B;
  vector double vec_X0 = vzero, vec_X1 = vzero, vec_X2 = vzero, vec_X3 = vzero;
  vector double vec_S0;
  vector double *vec_T = (vector double *)T;
  vector double *vec_T1= (vector double *)T1;
#endif
#if (UNROLL == 8)
  int vlen = 8;
  for (i = 0; i < N; i+=8)
    {
#ifdef SIMD
      vec_T[0] = vec_div(vec_A[i/2+0], vec_B[i/2+0]);
      vec_T[1] = vec_div(vec_A[i/2+1], vec_B[i/2+1]);
      vec_T[2] = vec_div(vec_A[i/2+2], vec_B[i/2+2]);
      vec_T[3] = vec_div(vec_A[i/2+3], vec_B[i/2+3]);
#else
      //vdiv(T, A+i, B+i, &vlen);
      T[0] = A[i+0]/B[i+0];
      T[1] = A[i+1]/B[i+1];
      T[2] = A[i+2]/B[i+2];
      T[3] = A[i+3]/B[i+3];
      T[4] = A[i+4]/B[i+4];
      T[5] = A[i+5]/B[i+5];
      T[6] = A[i+6]/B[i+6];
      T[7] = A[i+7]/B[i+7];
#endif
      vlog1p(T1, T, &vlen);
#ifdef SIMD
      vec_X0 = vec_add(vec_X0, vec_T1[0]);
      vec_X1 = vec_add(vec_X1, vec_T1[1]);
      vec_X2 = vec_add(vec_X2, vec_T1[2]);
      vec_X3 = vec_add(vec_X3, vec_T1[3]);
#else
      x += (T1[0]+T1[1]+T1[2]+T1[3]+T1[4]+T1[5]+T1[6]+T1[7]);
#endif
    }
#ifdef SIMD
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif  
#elif (UNROLL == 4)
  int vlen = 4;
  for (i = 0; i < N; i+=4)
    {
#ifdef SIMD
      vec_T[0] = vec_div(vec_A[i/2+0], vec_B[i/2+0]);
      vec_T[1] = vec_div(vec_A[i/2+1], vec_B[i/2+1]);
#else
      T[0] = A[i+0]/B[i+0];
      T[1] = A[i+1]/B[i+1];
      T[2] = A[i+2]/B[i+2];
      T[3] = A[i+3]/B[i+3];
#endif
      vlog1p(T1, T, &vlen);
#ifdef SIMD
      vec_X0 = vec_add(vec_X0, vec_T1[0]);
      vec_X1 = vec_add(vec_X1, vec_T1[1]);
#else
      x += (T1[0]+T1[1]+T1[2]+T1[3]);
#endif
    }
#ifdef SIMD
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif  
#elif (UNROLL == 2)
  int vlen = 2;
  for (i = 0; i < N; i+=2)
    {
#ifdef SIMD
      vec_T[0] = vec_div(vec_A[i/2+0], vec_B[i/2+0]);
#else
      T[0] = A[i+0]/B[i+0];
      T[1] = A[i+1]/B[i+1];
#endif
      vlog1p(T1, T, &vlen);
#ifdef SIMD
      vec_X0 = vec_add(vec_X0, vec_T1[0]);
#else
      x += (T1[0]+T1[1]);
#endif
    }
#ifdef SIMD
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif  
#else
  int vlen = 1;
  for (i = 0; i < N; i++)
    {
      T[0] = A[i+0]/B[i+0];
      vlog1p(T1, T, &vlen);
      x += (T1[0]);
    }
#endif
#else
  // scalar version
#if (UNROLL == 8)
  for (i = 0; i < N; i+=8)
    {
#ifdef	FAST_LOG
      x += flog(1.0 + A[i+0]/B[i+0]);
      x += flog(1.0 + A[i+1]/B[i+1]);
      x += flog(1.0 + A[i+2]/B[i+2]);
      x += flog(1.0 + A[i+3]/B[i+3]);
      x += flog(1.0 + A[i+4]/B[i+4]);
      x += flog(1.0 + A[i+5]/B[i+5]);
      x += flog(1.0 + A[i+6]/B[i+6]);
      x += flog(1.0 + A[i+7]/B[i+7]);
#else
      x += log(1.0 + A[i+0]/B[i+0]);
      x += log(1.0 + A[i+1]/B[i+1]);
      x += log(1.0 + A[i+2]/B[i+2]);
      x += log(1.0 + A[i+3]/B[i+3]);
      x += log(1.0 + A[i+4]/B[i+4]);
      x += log(1.0 + A[i+5]/B[i+5]);
      x += log(1.0 + A[i+6]/B[i+6]);
      x += log(1.0 + A[i+7]/B[i+7]);
#endif
    }
#elif (UNROLL == 4)
  for (i = 0; i < N; i+=4)
    {
#ifdef	FAST_LOG
      x += flog(1.0 + A[i+0]/B[i+0]);
      x += flog(1.0 + A[i+1]/B[i+1]);
      x += flog(1.0 + A[i+2]/B[i+2]);
      x += flog(1.0 + A[i+3]/B[i+3]);
#else
      x += log(1.0 + A[i+0]/B[i+0]);
      x += log(1.0 + A[i+1]/B[i+1]);
      x += log(1.0 + A[i+2]/B[i+2]);
      x += log(1.0 + A[i+3]/B[i+3]);
#endif
    }
#elif (UNROLL == 2)
  for (i = 0; i < N; i+=2)
    {
#ifdef	FAST_LOG
      x += flog(1.0 + A[i+0]/B[i+0]);
      x += flog(1.0 + A[i+1]/B[i+1]);
#else
      x += log(1.0 + A[i+0]/B[i+0]);
      x += log(1.0 + A[i+1]/B[i+1]);
#endif
    }
#else
  for (i = 0; i < N; i++)
    {
#ifdef	FAST_LOG
      x += flog(1.0 + A[i]/B[i]);
#else
      x += log(1.0 + A[i]/B[i]);
#endif
    }
#endif
#endif
  return x;
}
#endif

#ifdef	TIGHTNESS_3
double simple_tightness_3(int N, double *A, double *B)
{
  int i;
#if defined(SIMD) && !defined(VECTOR)
  // simd version
  vector double *vec_A = (vector double *)A;
  vector double *vec_B = (vector double *)B;
#if (UNROLL == 8)
  for (i = 0; i < N/2; i+=8)
    {
      vec_B[i+0] = logd2(vec_A[i+0]);
      vec_B[i+1] = logd2(vec_A[i+1]);
      vec_B[i+2] = logd2(vec_A[i+2]);
      vec_B[i+3] = logd2(vec_A[i+3]);
      vec_B[i+4] = logd2(vec_A[i+4]);
      vec_B[i+5] = logd2(vec_A[i+5]);
      vec_B[i+6] = logd2(vec_A[i+6]);
      vec_B[i+7] = logd2(vec_A[i+7]);
    }
#elif (UNROLL == 4)
  for (i = 0; i < N/2; i+=4)
    {
      vec_B[i+0] = logd2(vec_A[i+0]);
      vec_B[i+1] = logd2(vec_A[i+1]);
      vec_B[i+2] = logd2(vec_A[i+2]);
      vec_B[i+3] = logd2(vec_A[i+3]);
    }
#elif (UNROLL == 2)
  for (i = 0; i < N/2; i+=2)
    {
      vec_B[i+0] = logd2(vec_A[i+0]);
      vec_B[i+1] = logd2(vec_A[i+1]);
    }
#else
  for (i = 0; i < N/2; i++)
    {
      vec_B[i+0] = logd2(vec_A[i+0]);
    }
#endif
#elif defined(VECTOR)
  // vector version
#if (UNROLL == 8)
  int vlen = 8;
  for (i = 0; i < N; i+=8)
    {
      vlog(B+i, A+i, &vlen);
    }
#elif (UNROLL == 4)
  int vlen = 4;
  for (i = 0; i < N; i+=4)
    {
      vlog(B+i, A+i, &vlen);
    }
#elif (UNROLL == 2)
  int vlen = 2;
  for (i = 0; i < N; i+=2)
     x
    {
      vlog(B+i, A+i, &vlen);
    }
#else
  int vlen = 1;
  for (i = 0; i < N; i++)
    {
      vlog(B+i, A+i, &vlen);
    }
#endif
#else
  // scalar version
#if (UNROLL == 8)
  for (i = 0; i < N; i+=8)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
      B[i+2] = flog(A[i+2]);
      B[i+3] = flog(A[i+3]);
      B[i+4] = flog(A[i+4]);
      B[i+5] = flog(A[i+5]);
      B[i+6] = flog(A[i+6]);
      B[i+7] = flog(A[i+7]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
      B[i+2] = log(A[i+2]);
      B[i+3] = log(A[i+3]);
      B[i+4] = log(A[i+4]);
      B[i+5] = log(A[i+5]);
      B[i+6] = log(A[i+6]);
      B[i+7] = log(A[i+7]);
#endif
    }
#elif (UNROLL == 4)
  for (i = 0; i < N; i+=4)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
      B[i+2] = flog(A[i+2]);
      B[i+3] = flog(A[i+3]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
      B[i+2] = log(A[i+2]);
      B[i+3] = log(A[i+3]);
#endif
    }
#elif (UNROLL == 2)
  for (i = 0; i < N; i+=2)
    {
#ifdef	FAST_LOG
      B[i+0] = flog(A[i+0]);
      B[i+1] = flog(A[i+1]);
#else
      B[i+0] = log(A[i+0]);
      B[i+1] = log(A[i+1]);
#endif
    }
#else
  for (i = 0; i < N; i++)
    {
#ifdef	FAST_LOG
      B[i] = flog(A[i]);
#else
      B[i] = log(A[i]);
#endif
    }
#endif
#endif
  return 1.0;
}
#endif

#ifdef	TIGHTNESS_4
double simple_tightness_4(int N, double *A, double *B)
{
  int i;
  double x = 0;
#if defined(SIMD) && !defined(VECTOR)
  // simd version
  vector double *vec_A = (vector double *)A;
  vector double vec_X0 = vzero, vec_X1 = vzero, vec_X2 = vzero, vec_X3 = vzero;
  vector double vec_X4 = vzero, vec_X5 = vzero, vec_X6 = vzero, vec_X7 = vzero;
  vector double vec_T0, vec_T1, vec_T2, vec_T3;
  vector double vec_T4, vec_T5, vec_T6, vec_T7;
  vector double vec_S0;
#if (UNROLL == 8)
  for (i = 0; i < N/2; i+=8)
    {
      vec_T0 = logd2(vec_A[i+0]);
      vec_T1 = logd2(vec_A[i+1]);
      vec_T2 = logd2(vec_A[i+2]);
      vec_T3 = logd2(vec_A[i+3]);
      vec_T4 = logd2(vec_A[i+4]);
      vec_T5 = logd2(vec_A[i+5]);
      vec_T6 = logd2(vec_A[i+6]);
      vec_T7 = logd2(vec_A[i+7]);
      vec_X0 = vec_add(vec_X0, vec_T0);
      vec_X1 = vec_add(vec_X1, vec_T1);
      vec_X2 = vec_add(vec_X2, vec_T2);
      vec_X3 = vec_add(vec_X3, vec_T3);
      vec_X4 = vec_add(vec_X4, vec_T4);
      vec_X5 = vec_add(vec_X5, vec_T5);
      vec_X6 = vec_add(vec_X6, vec_T6);
      vec_X7 = vec_add(vec_X7, vec_T7);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X4 = vec_add(vec_X4, vec_X5);
  vec_X6 = vec_add(vec_X6, vec_X7);

  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_X4 = vec_add(vec_X4, vec_X6);
  vec_X0 = vec_add(vec_X0, vec_X4);

  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 4)
  for (i = 0; i < N/2; i+=4)
    {
      vec_T0 = logd2(vec_A[i+0]);
      vec_T1 = logd2(vec_A[i+1]);
      vec_T2 = logd2(vec_A[i+2]);
      vec_T3 = logd2(vec_A[i+3]);
      vec_X0 = vec_add(vec_X0, vec_T0);
      vec_X1 = vec_add(vec_X1, vec_T1);
      vec_X2 = vec_add(vec_X2, vec_T2);
      vec_X3 = vec_add(vec_X3, vec_T3);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_X2 = vec_add(vec_X2, vec_X3);
  vec_X0 = vec_add(vec_X0, vec_X2);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#elif (UNROLL == 2)
  for (i = 0; i < N/2; i+=2)
    {
      vec_T0 = logd2(vec_A[i+0]);
      vec_X0 = vec_add(vec_X0, vec_T0);
      vec_T1 = logd2(vec_A[i+1]);
      vec_X1 = vec_add(vec_X1, vec_T1);
    }
  vec_X0 = vec_add(vec_X0, vec_X1);
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#else
  for (i = 0; i < N/2; i++)
    {
      vec_T0 = logd2(vec_A[i+0]);
      vec_X0 = vec_add(vec_X0, vec_T0);
    }
  vec_S0 = vec_mergel(vec_X0, vec_X0);
  vec_X0 = vec_add(vec_S0, vec_X0);
  x = vec_extract(vec_X0, 0);
#endif
#elif defined(VECTOR)
  // vector version
#if (UNROLL == 8)
  int vlen = 8;
  for (i = 0; i < N; i+=8)
    {
      vlog(B, A+i, &vlen);
      x += (B[0]+B[1]+B[2]+B[3]+B[4]+B[5]+B[6]+B[7]);
    }
#elif (UNROLL == 4)
  int vlen = 4;
  for (i = 0; i < N; i+=4)
    {
      vlog(B, A+i, &vlen);
      x += (B[0]+B[1]+B[2]+B[3]);
    }
#elif (UNROLL == 2)
  int vlen = 2;
  for (i = 0; i < N; i+=2)
    {
      vlog(B, A+i, &vlen);
      x += (B[0]+B[1]);
    }
#else
  int vlen = 1;
  for (i = 0; i < N; i++)
    {
      vlog(B, A+i, &vlen);
      x += B[0];
    }
#endif
#else
  // scalar version
#if (UNROLL == 8)
  for (i = 0; i < N; i+=8)
    {
#ifdef	FAST_LOG
      x += flog(A[i+0]);
      x += flog(A[i+1]);
      x += flog(A[i+2]);
      x += flog(A[i+3]);
      x += flog(A[i+4]);
      x += flog(A[i+5]);
      x += flog(A[i+6]);
      x += flog(A[i+7]);
#else
      x += log(A[i+0]);
      x += log(A[i+1]);
      x += log(A[i+2]);
      x += log(A[i+3]);
      x += log(A[i+4]);
      x += log(A[i+5]);
      x += log(A[i+6]);
      x += log(A[i+7]);
#endif
    }
#elif (UNROLL == 4)
  for (i = 0; i < N; i+=4)
    {
#ifdef	FAST_LOG
      x += flog(A[i+0]);
      x += flog(A[i+1]);
      x += flog(A[i+2]);
      x += flog(A[i+3]);
#else
      x += log(A[i+0]);
      x += log(A[i+1]);
      x += log(A[i+2]);
      x += log(A[i+3]);
#endif
    }
#elif (UNROLL == 2)
  for (i = 0; i < N; i+=2)
    {
#ifdef	FAST_LOG
      x += flog(A[i+0]);
      x += flog(A[i+1]);
#else
      x += log(A[i+0]);
      x += log(A[i+1]);
#endif
    }
#else
  for (i = 0; i < N; i++)
    {
#ifdef	FAST_LOG
      x += flog(A[i]);
#else
      x += log(A[i]);
#endif
    }
#endif
#endif
  return x;
}
#endif

#if 0
// original code in C++
double LogLikelihoodDist::LogLikelihoodDistImp::
GetTightness(const CFRecord* j) const {
  MCASSERT(j != 0, "LogLikelihoodDist::LogLikelihoodDistImp::GetTightness: j != 0");
  double t1 = 0, t2 = 0, mu = 0, sigma2 = 0, delta = 0, q = 0, E = 0;
  int k = 0, l = 0;
  int n = j->GetN();
  const CFRecord::ContValArray& vars = j->GetVariances();
  const CFRecord::CatValArray& nb = j->GetNB();

  MCASSERT(n > 0, "LogLikelihoodDist::LogLikelihoodDistImp::GetTightness: n > 0");
  if ( vars.size() > 0 ) {
    for (k = 0; k < vars.size(); k++) {
      double newVariance = vars[ k ];
      MCASSERT(fVariances[k] > 0, 
	       "LogLikelihoodDist::LogLikelihoodDistImp::GetTightness: fVariances[k] > 0");
      t1 += log(1.0 + newVariance/fVariances[k]);
    }
  }

  if (nb.size() > 0) {
    for (k = 0; k < nb.size(); k++) {
      E = 0;
      q = (double)nb[k] / (double)n;
      if (q > 0) {
	E -= q * log(q);
      }
      t2 += E;
    }
  }

  return 0.5 * t1 + t2;
}
#endif
