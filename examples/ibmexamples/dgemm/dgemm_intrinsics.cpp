/****************************************/
/* IBM Confidential - Do not distribute */
/****************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <timing.h>
//#include <time.h>

// define STATIC     - use static memory for matrices
// define MEMALIGN   - use posix_memalign to allocate matrices
// define HUGE_PAGE  - use mmap 
// define ONE_MALLOC - call malloc one time to allocate three matrices
// (default)         - use malloc to allocate matrices

//#define STATIC
//#define MEMALIGN
//#define ONE_MALLOC
//#define HUGE_PAGE

// initialize malloced areas to avoid synonyms
#define MEMINIT

// When HUGE_PAGE is defined, make sure to set up hugepage file system before running this
//
// check the number of pages assigned
// cat /proc/sys/vm/nr_hugepages 
// if this is not large enough to hold 1.5MB data (typically 1 page),
// make it larger (n is 1 or larger)
// echo n > /proc/sys/vm/nr_hugepages 
// mkdir /mnt/hugepage
// mount -t hugetlbfs none /mnt/hugepage

#ifdef HUGE_PAGE
//#include <sys/types.h>
//#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#define FILE_NAME "/mnt/hugepage/hugepagefile"
#endif

// select one of the following DGEMM implementations
// GEMM_BLAS calls an external library, like ESSL

//#define GEMM42 1
#define GEMM44 1
//#define GEMM_BLAS 1

// SIMD assumes p7 vsx and works only for GEMM44
//#define SIMD 1

//#define MATSIZE 1024
//#define MATSIZE 128
//#define MATSIZE 64
//#define MATSIZE 32
//#define MATSIZE 16
//#define MATSIZE 8
int MATSIZE=1024;
#define VERIFY

#ifdef	PARALLEL
#include		"mt.h"
#define NUM_THREADS_M 4
#define NUM_THREADS_N 4
unsigned int num_threads_m = 1;
unsigned int num_threads_n = SMT_LEVEL;
unsigned int num_threads;
#define	thread_id(thread_m, thread_n)	(thread_m * num_threads_n + thread_n)

//#define	NUM_THREADS	(NUM_THREADS_M * NUM_THREADS_N)
//#define	thread_id(thread_m, thread_n)	(thread_m * NUM_THREADS_M + thread_n)
//#define	thread_id(thread_m, thread_n)	(thread_m * NUM_THREADS_N + thread_n)
extern micro_thread_t	_mt[SMT_LEVEL];
#endif

int M;
int N;
int K;
int ldb;
int ldc;

void
gemmsk (/*int M,
        int N,
        int K,
        double *A,
        double *B,
        double *C,
        int ldb,
        int ldc,
        int *mind,
        int *nind*/);

#ifdef GEMM_BLAS
static void
gemms_BLAS (int    M,
            int    N,
            int    K,
            double *A,
            double *B,
            double *C,
            int    ldb,
            int    ldc,
            int    *mind,
            int    *nind);
#endif

#ifdef STATIC
double A[MATSIZE*MATSIZE];
double B[MATSIZE*MATSIZE];
double C[MATSIZE*MATSIZE];
#else
double *A;
double *B;
double *C;
#endif
int *mind;
int *nind;

void init_mat(int M, int N, int K, double *A, double *B, double *C)
{
  int i, j;
  for (i = 0; i < M; i++) {
    for (j = 0; j < K; j++) {
      A[K*i+j] = (double)(j+i);
    }
  }
  for (i = 0; i < N; i++) {
    for (j = 0; j < K; j++) {
      B[K*i+j] = (double)(j*i);
    }
  }
  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      C[N*i+j] = (double)(0);
    }
  }
}

void verify_mul(int MAX_COUNT, int M, int N, int K, double *A, double *B, double *C)
{
  int i, j, k, c;
#if 0
  for (i = 0; i < M; i++) {
    for (j = 0; j < K; j++) {
      printf ("%d %d %f\n", i, j, A[K*i+j]);
    }
  }
  for (i = 0; i < N; i++) {
    for (j = 0; j < K; j++) {
      printf ("%d %d %f\n", i, j, B[K*i+j]);
    }
  }
  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      printf ("%d %d %f\n", i, j, C[N*i+j]);
    }
  }
#endif
  double *D = (double *)malloc((M*N+1)*sizeof(double));
  if (((unsigned long)D)&(sizeof(double)*2-1)) D++;
  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      D[N*i+j] = (double)(0);
    }
  }
  for (c = 0; c < MAX_COUNT; c++) {
    for (i = 0; i < M; i++) {
      for (j = 0;  j < N; j++) {
	for (k = 0; k < K; k++) {
	  D[N*i+j] += (A[K*i+k] * B[K*j+k]);
	}
      }
    }
  }
  int count = 0;
  double diff = 0.0;
  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      if (C[N*i+j] != D[N*i+j]) {
	diff += fabs((C[N*i+j] - D[N*i+j]));
	count++;
      }
      //printf ("%d %d %f %f\n", i, j, C[N*i+j],  D[N*i+j]);
    }
  }
  printf ("count=%d diff=%f\n", count, diff);
}

void print_config()
{
  printf("SIMD=%s,GEMM=%s,%s,MEMINIT=%s\n",
#ifdef SIMD
	 "YES",
#else
	 "NO",
#endif
#ifdef GEMM_BLAS
	 "GEMM_BLASS"
#elif defined(GEMM44)
	 "GEMM44"
#elif defined(GEMM42)
	 "GEMM42"
#else
	 "UNKNOWN"
#endif
	 ,
#ifdef STATIC
	 "STATIC"
#elif defined(MEMALIGN)
	 "MEMALIGN"
#elif defined(HUGE_PAGE)
	 "HUGE_PAGE"
#elif defined(ONE_MALLOC)
	 "ONE_MALLOC"
#else
	 "MALLOC"
#endif
	 ,
#ifdef MEMINIT
	 "YES"
#else
	 "NO"
#endif
	 );
}

#ifdef HUGE_PAGE
int fd;
void *hugepage_heap;
int hugepage_length;
int offset;
void hugepage_map(int bytes)
{
  if ((fd = open(FILE_NAME, O_CREAT | O_RDWR, 0775)) < 0)
    {
      perror("cannot open file");
      exit(1);
    }
  int hugepage_size = 2*1024*1024; // 2MB
  hugepage_length = ((bytes/hugepage_size)+1) * hugepage_size;
  offset = 0;
  if ((hugepage_heap = mmap(0, hugepage_length, PROT_READ|PROT_WRITE, 
			    MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
      perror("cannot mmap");
      unlink(FILE_NAME);
      exit(1);
    }
  printf("hugepage %lx %d (0x%x)\n", 
	 (unsigned long)hugepage_heap, bytes, bytes);
}

void hugepage_unmap()
{
  munmap(hugepage_heap, hugepage_length);
  close(fd);
  unlink(FILE_NAME);
}

void *hugepage_malloc(int bytes)
{
  if (hugepage_length > (offset + bytes))
    {
      void *ret = (void *)((unsigned long)hugepage_heap + offset);
      offset += bytes;
      printf("malloc %lx %d (0x%x)\n", (unsigned long)ret, bytes, bytes);
      return ret;
    }
  return 0;
}
#endif

//#include	"funcAddr.h"

void usage(char *cmd)
{
  printf("usage: %s <repeat_count> [-num_threads_m <num_threads_m>] [-num_threads_n <num_threads_n>] [-matsize <matsize>] [-verify]\n", cmd);
  exit(-1);
}

int main(int argc, char *argv[])
{
  int verify = 0;
  int MAX_COUNT = 1; int count;
  int num_threads_m;
  int num_threads_n;
  
  if (argc > 1) {
    MAX_COUNT = atol(argv[1]);
    int c = 2;
    while (c < argc) {
      if (strcmp(argv[c], "-verify") == 0) {
	verify = 1;
	c++;
      } else if (strcmp(argv[c], "-num_threads_m") == 0) {
	c++;
	num_threads_m = atol(argv[c]);
	c++;
	/*if (num_threads_m > SMT_LEVEL) {
	  printf("num_threads_m %d should be no greater than %d\n", num_threads_m, SMT_LEVEL);
	  exit(-1);
	}*/
      } else if (strcmp(argv[c], "-num_threads_n") == 0) {
	c++;
	num_threads_n = atol(argv[c]);
	c++;
	/*if (num_threads_n > SMT_LEVEL) {
	  printf("num_threads_n %d should be no greater than %d\n", num_threads_n, SMT_LEVEL);
	  exit(-1);
	}*/
      } else if (strcmp(argv[c], "-matsize") == 0) {
	c++;
	MATSIZE = atol(argv[c]);
	c++;
      } else {
	usage(argv[0]);
      }
    }
  } else {
    usage(argv[0]);
  }
  /*printf("dgemm entry = %p\n",
#ifdef GEMM_BLAS
  funcAddr(gemms_BLAS)
#else
  funcAddr(gemmsk)
#endif
  );*/
  printf("MATSIZE=%d MAX_COUNT=%d\n", MATSIZE, MAX_COUNT);
  /*int*/ M = MATSIZE;
  /*int*/ N = MATSIZE;
  /*int*/ K = MATSIZE;
#ifdef HUGE_PAGE
  hugepage_map((M*K+K*N+M*N)*sizeof(double));
  A = (double *)hugepage_malloc((M*K)*sizeof(double));
  B = (double *)hugepage_malloc((K*N)*sizeof(double));
  C = (double *)hugepage_malloc((M*N)*sizeof(double));
#elif defined(MEMALIGN)
  int ret;
  ret = posix_memalign((void **)&A, sizeof(double)*2, (M*K)*sizeof(double));
  if (ret != 0) {
    perror("cannot malloc"); exit(1);
  }
  ret = posix_memalign((void **)&B, sizeof(double)*2, (K*N)*sizeof(double));
  if (ret != 0) {
    perror("cannot malloc"); exit(1);
  }
  ret = posix_memalign((void **)&C, sizeof(double)*2, (M*N)*sizeof(double));
  if (ret != 0) {
    perror("cannot malloc"); exit(1);
  }
#elif defined(ONE_MALLOC)
  double *m = (double *)malloc((M*K+K*N+M*N+1)*sizeof(double));
  if (((unsigned long)m)&(sizeof(double)*2-1)) m++;
  A = m;
  B = m + M*K;
  C = m + M*K + K*N;
#elif defined(STATIC)  
#else
  A = (double *)malloc((M*K+1)*sizeof(double));
  B = (double *)malloc((K*N+1)*sizeof(double));
  C = (double *)malloc((M*N+1)*sizeof(double));
  if (((unsigned long)A)&(sizeof(double)*2-1)) A++;
  if (((unsigned long)B)&(sizeof(double)*2-1)) B++;
  if (((unsigned long)C)&(sizeof(double)*2-1)) C++;
#endif
  printf ("%lx %lx %lx\n", (unsigned long)A, (unsigned long)B, (unsigned long)C);
  print_config();
#ifdef MEMINIT
  init_mat(M, N, K, A, B, C);
#elif defined(VERIFY)
  if (verify) init_mat(M, N, K, A, B, C);
#endif
  /*int*/ ldb = N;
  /*int*/ ldc = M;
  mind = (int *)malloc(M*sizeof(int));
  nind = (int *)malloc(N*sizeof(int));
  int i;
  for (i = 0; i < M; i++) mind[i] = i;
  for (i = 0; i < N; i++) nind[i] = i;
#ifdef	PARALLEL
  num_threads = num_threads_m * num_threads_n;
  if (num_threads > SMT_LEVEL) {
    printf("num_threads %d should be no greater than %d\n", num_threads, SMT_LEVEL);
    exit(-1);
  }
  printf("num_threads=%d num_threads_m=%d (outer-most) num_threads_n=%d (2nd outer-most)\n", num_threads, num_threads_m, num_threads_n);
  mt_symbols();
  mt_init(); // start collecting a trace
  for (i = 1; i < num_threads; i++)
	  micro_thread_create(&_mt[i]);
#endif	/* PARALLEL */
  reset_and_start_stimer();
  for (count = 0; count < MAX_COUNT; count++) {
    //clock_t c = clock();
#ifdef GEMM_BLAS
    gemms_BLAS (M, N, K, A, B, C, ldb, ldc, mind, nind);
#else
    gemmsk(/*M, N, K, A, B, C, ldb, ldc, mind, nind*/);
#endif
    //c = clock() - c;
    //printf ("%f\n", ((float)c) * 1000/CLOCKS_PER_SEC);

  }
  double dt = get_elapsed_seconds();
  printf("Time = %f seconds\n", dt);
#ifdef	PARALLEL
  for (i = 1; i < num_threads; i++)
	  micro_thread_stop(&_mt[i]);
  mt_exit(); // stop collecting a trace
#endif	/* PARALLEL */
#ifdef VERIFY
  if (verify) verify_mul(MAX_COUNT, M, N, K, A, B, C);
#endif
#ifdef HUGE_PAGE
  hugepage_unmap();
#endif
}

#ifdef SIMD
#include <altivec.h>
const vector double vzero = {0.0};
#endif

#if !defined(GEMM_BLAS)

void
gemmsk_single (int m0,
		 int M,
        int n0,
	int N,
        int K,
	double *A,
        double *B,
        double *C,
        int ldb,
        int ldc,
        int *mind,
        int *nind);

#ifdef	PARALLEL
typedef struct gemmsk_arg	{
	int		m0;
	int		M;
	int		n0;
	int		N;
	int		K;
	double	*A;
	double	*B;
	double	*C;
	int		ldb;
	int		ldc;
	int		*mind;
	int		*nind;
}	gemmsk_arg_t;

gemmsk_arg_t	gemmsk_args[NUM_THREADS_M][NUM_THREADS_N];

void	gemmsk_invoker(void *data) {
	gemmsk_arg_t	*args	= (gemmsk_arg_t *) data;
	gemmsk_single(args->n0, args->N);
	//gemmsk_single(args->m0, args->M, args->n0, args->N, args->K, args->A, args->B, args->C, args->ldb, args->ldc, args->mind, args->nind);
}
#endif	/* PARALLEL */

void
gemmsk (/*int M,
        int N,
        int K,
        double *A,
        double *B,
        double *C,
        int ldb,
        int ldc,
        int *mind,
        int *nind*/)
{
#ifdef	PARALLEL
#if defined(SMT4)
	int		n_stride	= N / 4;
	gemmsk_arg_t	*args;
	micro_thread_t *mt = _mt+1;
	args		= &gemmsk_args[0][1];
	//args->m0	= 0;
	//args->M	= M;
	args->n0	= n_stride * 1;
	args->N	        = n_stride * 2;
	//args->K	= K;
	//args->A	= A;
	//args->B	= B;
	//args->C	= C;
	//args->ldb	= ldb;
	//args->ldc	= ldc;
	//args->mind	= mind;
	//args->nind	= nind;
	//micro_thread_run(&_mt[thread_id(0, 1)], gemmsk_invoker, args);
	mt->fn    = (void (*)(void *))gemmsk_invoker;
	mt->data  = (void *)args;
	_unpause_store(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	args		= &gemmsk_args[0][2];
	//args->m0	= 0;
	//args->M	= M;
	args->n0	= n_stride * 2;
	args->N	        = n_stride * 3;
	//args->K	= K;
	//args->A	= A;
	//args->B       = B;
	//args->C	= C;
	//args->ldb	= ldb;
	//args->ldc	= ldc;
	//args->mind	= mind;
	//args->nind	= nind;
	//micro_thread_run(&_mt[thread_id(0, 2)], gemmsk_invoker, args);
	mt++;
	mt->fn    = (void (*)(void *))gemmsk_invoker;
	mt->data  = (void *)args;
	_unpause_store(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	args		= &gemmsk_args[0][3];
	//args->m0	= 0;
	//args->M	= M;
	args->n0	= n_stride * 3;
	args->N	        = n_stride * 4;
	//args->K	= K;
	//args->A	= A;
	//args->B	= B;
	//args->C	= C;
	//args->ldb	= ldb;
	//args->ldc	= ldc;
	//args->mind	= mind;
	//args->nind	= nind;
	//micro_thread_run(&_mt[thread_id(0, 3)], gemmsk_invoker, args);
	mt++;
	mt->fn    = (void (*)(void *))gemmsk_invoker;
	mt->data  = (void *)args;
	_unpause_store(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	//
	// Do my own task...
	//
	gemmsk_single (0, n_stride);
	//gemmsk_single (0, M, 0, n_stride, K, A, B, C, ldb, ldc, mind, nind);
	//
	// Wait for the workers...
	//
	//micro_thread_wait(&_mt[thread_id(0, 1)]);
	mt = _mt+1;
	cap(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	//micro_thread_wait(&_mt[thread_id(0, 2)]);
	mt = _mt+2;
	cap(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	//micro_thread_wait(&_mt[thread_id(0, 3)]);
	mt = _mt+3;
	cap(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
#elif defined(SMT2)
	int		n_stride	= N / 2;
	gemmsk_arg_t	*args;
	micro_thread_t *mt = _mt+1;
	args		= &gemmsk_args[0][1];
	//args->m0	= 0;
	//args->M	= M;
	args->n0	= n_stride * 1;
	args->N	        = n_stride * 2;
	//args->K	= K;
	//args->A	= A;
	//args->B	= B;
	//args->C	= C;
	//args->ldb	= ldb;
	//args->ldc	= ldc;
	//args->mind	= mind;
	//args->nind	= nind;
	//micro_thread_run(&_mt[thread_id(0, 1)], gemmsk_invoker, args);
	mt->fn    = (void (*)(void *))gemmsk_invoker;
	mt->data  = (void *)args;
	_unpause_store(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
	//
	// Do my own task...
	//
	gemmsk_single (0, n_stride);
	//gemmsk_single (0, M, 0, n_stride, K, A, B, C, ldb, ldc, mind, nind);
	//
	// Wait for the workers...
	//
	//micro_thread_wait(&_mt[thread_id(0, 1)]);
	mt = _mt+1;
	cap(&(mt->state), THREAD_RUNNING, MASTER_THREAD);
#elif defined(SMT1)
	int		n_stride	= N / 1;
	//
	// Do my own task...
	//
	gemmsk_single (0, n_stride);
#elif defined(SMT12)
	int		n_stride	= N / 2;
	gemmsk_arg_t	*args;
	args		= &gemmsk_args[0][1];
	args->n0	= n_stride * 1;
	args->N	        = n_stride * 2;
	gemmsk_invoker (args);
	gemmsk_single (0, n_stride);
#elif defined(SMT14)
	int		n_stride	= N / 4;
	gemmsk_arg_t	*args;
	args		= &gemmsk_args[0][1];
	args->n0	= n_stride * 1;
	args->N	        = n_stride * 2;
	gemmsk_invoker (args);
	args		= &gemmsk_args[0][2];
	args->n0	= n_stride * 2;
	args->N	        = n_stride * 3;
	gemmsk_invoker (args);
	args		= &gemmsk_args[0][3];
	args->n0	= n_stride * 3;
	args->N	        = n_stride * 4;
	gemmsk_invoker (args);
	gemmsk_single (0, n_stride);
#else
	int				m_stride	= M / num_threads_m;
	int				n_stride	= N / num_threads_n;
	int				m_start, n_start;
	int				thread_m, thread_n;
	gemmsk_arg_t	*args;
	int				m_start_master, m_end_master, n_start_master, n_end_master;

	//
	// Spawn workers...
	//
	for (thread_m = 0, m_start = 0; thread_m < num_threads_m; thread_m++, m_start += m_stride)
		for (thread_n = 0, n_start = 0; thread_n < num_threads_n; thread_n++, n_start += n_stride) {
			int	m_end	= thread_m < num_threads_m - 1 ? m_start + m_stride : M;
			int	n_end	= thread_n < num_threads_n - 1 ? n_start + n_stride : N;
			if (thread_m == 0 && thread_n == 0) {
				m_start_master	= m_start;
				m_end_master	= m_end;
				n_start_master	= n_start;
				n_end_master	= n_end;
				continue;
			}
			args		= &gemmsk_args[thread_m][thread_n];
			args->m0	= m_start;
			args->M		= m_end;
			args->n0	= n_start;
			args->N		= n_end;
			args->K		= K;
			args->A		= A;
			args->B		= B;
			args->C		= C;
			args->ldb	= ldb;
			args->ldc	= ldc;
			args->mind	= mind;
			args->nind	= nind;
			micro_thread_run(&_mt[thread_id(thread_m, thread_n)], gemmsk_invoker, args);
		}
	//
	// Do my own task...
	//
	gemmsk_single (m_start_master, m_end_master, n_start_master, n_end_master, K, A, B, C, ldb, ldc, mind, nind);

	//
	// Wait for the workers...
	//
	for (thread_m = 0; thread_m < num_threads_m; thread_m++)
		for (thread_n = 0; thread_n < num_threads_n; thread_n++) {
			if (thread_m == 0 && thread_n == 0) continue;
			micro_thread_wait(&_mt[thread_id(thread_m, thread_n)]);
		}
#endif
#else
	gemmsk_single (0, M, 0, N, K, A, B, C, ldb, ldc, mind, nind);
#endif
}

/*static void*/
void 
gemmsk_single
	   (int m0,
	      int M,
        int n0,
	int N,
        int K,
	double *A,
        double *B,
        double *C,
        int ldb,
        int ldc,
        int *mind,
        int *nind)
{
    m0 = 0;
   int i, j, k;
   int mindex0, mindex1, mindex2, mindex3;
   int nindex0, nindex1, nindex2, nindex3;
   double t00, t10, t20, t30;
   double t01, t11, t21, t31;
   double t02, t12, t22, t32;
   double t03, t13, t23, t33;
   double a0, a1, a2, b0, b1, b2;
#if defined(SIMD)
   vector double v00, v10, v20, v30;
   vector double v01, v11, v21, v31;
   vector double v02, v12, v22, v32;
   vector double v03, v13, v23, v33;
#endif

   for (i = m0; i < M; i+=4) {
      mindex0 = mind[i+0]*ldc;
      mindex1 = mind[i+1]*ldc;
      mindex2 = mind[i+2]*ldc;
      mindex3 = mind[i+3]*ldc;
#if defined(GEMM44)
#if defined(SIMD)
      for (j = n0; j < N; j+=4) {
	 v00 = vzero; v10 = vzero; v20 = vzero; v30 = vzero;
	 v01 = vzero; v11 = vzero; v21 = vzero; v31 = vzero;
	 v02 = vzero; v12 = vzero; v22 = vzero; v32 = vzero;
	 v03 = vzero; v13 = vzero; v23 = vzero; v33 = vzero;
         for (k = 0; k < K; k+=2) {
	    vector double *A0 = (vector double *)&A[(i+0)*K+k];
	    vector double *A1 = (vector double *)&A[(i+1)*K+k];
	    vector double *A2 = (vector double *)&A[(i+2)*K+k];
	    vector double *A3 = (vector double *)&A[(i+3)*K+k];
	    vector double *B0 = (vector double *)&B[(j+0)*ldb+k];
	    vector double *B1 = (vector double *)&B[(j+1)*ldb+k];
	    vector double *B2 = (vector double *)&B[(j+2)*ldb+k];
	    vector double *B3 = (vector double *)&B[(j+3)*ldb+k];
	    v00 = vec_madd(*A0, *B0, v00);
	    v10 = vec_madd(*A1, *B0, v10);
	    v20 = vec_madd(*A2, *B0, v20);
	    v30 = vec_madd(*A3, *B0, v30);
	    v01 = vec_madd(*A0, *B1, v01);
	    v11 = vec_madd(*A1, *B1, v11);
	    v21 = vec_madd(*A2, *B1, v21);
	    v31 = vec_madd(*A3, *B1, v31);
	    v02 = vec_madd(*A0, *B2, v02);
	    v12 = vec_madd(*A1, *B2, v12);
	    v22 = vec_madd(*A2, *B2, v22);
	    v32 = vec_madd(*A3, *B2, v32);
	    v03 = vec_madd(*A0, *B3, v03);
	    v13 = vec_madd(*A1, *B3, v13);
	    v23 = vec_madd(*A2, *B3, v23);
	    v33 = vec_madd(*A3, *B3, v33);
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         nindex2 = nind[j+2];
         nindex3 = nind[j+3];
	 vector double *C00 = (vector double *)&C[mindex0+nindex0];
	 vector double *C10 = (vector double *)&C[mindex1+nindex0];
         vector double *C20 = (vector double *)&C[mindex2+nindex0];
         vector double *C30 = (vector double *)&C[mindex3+nindex0];
	 vector double *C02 = (vector double *)&C[mindex0+nindex2];
	 vector double *C12 = (vector double *)&C[mindex1+nindex2];
         vector double *C22 = (vector double *)&C[mindex2+nindex2];
         vector double *C32 = (vector double *)&C[mindex3+nindex2];

	 vector double h00 = vec_mergeh(v00, v01);
	 vector double l00 = vec_mergel(v00, v01);
	 vector double h10 = vec_mergeh(v10, v11);
	 vector double l10 = vec_mergel(v10, v11);
	 vector double h20 = vec_mergeh(v20, v21);
	 vector double l20 = vec_mergel(v20, v21);
	 vector double h30 = vec_mergeh(v30, v31);
	 vector double l30 = vec_mergel(v30, v31);

	 vector double h02 = vec_mergeh(v02, v03);
	 vector double l02 = vec_mergel(v02, v03);
	 vector double h12 = vec_mergeh(v12, v13);
	 vector double l12 = vec_mergel(v12, v13);
	 vector double h22 = vec_mergeh(v22, v23);
	 vector double l22 = vec_mergel(v22, v23);
	 vector double h32 = vec_mergeh(v32, v33);
	 vector double l32 = vec_mergel(v32, v33);

	 vector double s00 = vec_add(h00, l00);
	 vector double s10 = vec_add(h10, l10);
	 vector double s20 = vec_add(h20, l20);
	 vector double s30 = vec_add(h30, l30);

	 vector double s02 = vec_add(h02, l02);
	 vector double s12 = vec_add(h12, l12);
	 vector double s22 = vec_add(h22, l22);
	 vector double s32 = vec_add(h32, l32);

	 *C00 = vec_add(*C00, s00);
	 *C10 = vec_add(*C10, s10);
	 *C20 = vec_add(*C20, s20);
	 *C30 = vec_add(*C30, s30);

	 *C02 = vec_add(*C02, s02);
	 *C12 = vec_add(*C12, s12);
	 *C22 = vec_add(*C22, s22);
	 *C32 = vec_add(*C32, s32);
      }
#else
      for (j = n0; j < N-3; j+=4) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         t01 = 0; t11 = 0; t21 = 0; t31 = 0;
         t02 = 0; t12 = 0; t22 = 0; t32 = 0;
         t03 = 0; t13 = 0; t23 = 0; t33 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t30 += A[(i+3)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
            t31 += A[(i+3)*K+k] * B[(j+1)*ldb+k];
            t02 += A[(i+0)*K+k] * B[(j+2)*ldb+k];
            t12 += A[(i+1)*K+k] * B[(j+2)*ldb+k];
            t22 += A[(i+2)*K+k] * B[(j+2)*ldb+k];
            t32 += A[(i+3)*K+k] * B[(j+2)*ldb+k];
            t03 += A[(i+0)*K+k] * B[(j+3)*ldb+k];
            t13 += A[(i+1)*K+k] * B[(j+3)*ldb+k];
            t23 += A[(i+2)*K+k] * B[(j+3)*ldb+k];
            t33 += A[(i+3)*K+k] * B[(j+3)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         nindex2 = nind[j+2];
         nindex3 = nind[j+3];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex3+nindex1] += t31;
         C[mindex0+nindex2] += t02;
         C[mindex1+nindex2] += t12;
         C[mindex2+nindex2] += t22;
         C[mindex3+nindex2] += t32;
         C[mindex0+nindex3] += t03;
         C[mindex1+nindex3] += t13;
         C[mindex2+nindex3] += t23;
         C[mindex3+nindex3] += t33;
      }
#endif
#endif
#if defined(GEMM42)
      for (j = n0; j < N-1; j+=2) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         t01 = 0; t11 = 0; t21 = 0; t31 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t30 += A[(i+3)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
            t31 += A[(i+3)*K+k] * B[(j+1)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex3+nindex1] += t31;
      }
#endif
   }
} /* END gemmsk */

#if 0
// original code
static void
gemmsk (int M,
        int N,
        int K,
        double *A,
        double *B,
        double *C,
        int ldb,
        int ldc,
        int *mind,
        int *nind)
{
   int i, j, k;
#ifdef FIX_WARNINGS
#if defined(GEMM22) || defined(GEMM21)
   int mindex0, mindex1;
   int nindex0;
   double t00, t10;
#if defined (GEMM22)
   int  nindex1;
   double t01, t11;
#endif
#elif defined(GEMM33) || defined(GEMM32) || defined(GEMM31)
   int mindex0, mindex1, mindex2;
   int nindex0;
   double t00, t10, t20;
#if defined(GEMM33) || defined(GEMM32)
   int  nindex1;
   double t01, t11, t21;
#endif
#if defined(GEMM33)
   int  nindex2;
   double t02, t12, t22;
   double a0, a1, a2, b0, b1, b2;
#endif
#elif defined(GEMM44) || defined(GEMM43) || defined(GEMM42) || defined(GEMM41)
   int mindex0, mindex1, mindex2, mindex3;
   int nindex0;
   double t00, t10, t20, t30;
#if defined(GEMM44) || defined(GEMM43) || defined(GEMM42)
   int nindex1;
   double t01, t11, t21, t31;
#endif
#if defined(GEMM44) || defined(GEMM43)
   int  nindex2;
   double t02, t12, t22, t32;
#endif
#if defined(GEMM44)
   int  nindex3;
   double t03, t13, t23, t33;
#endif
#else
   double t00;
   int mindex0;
#endif
#else
   int mindex0, mindex1, mindex2, mindex3;
   int nindex0, nindex1, nindex2, nindex3;
   double t00, t10, t20, t30;
   double t01, t11, t21, t31;
   double t02, t12, t22, t32;
   double t03, t13, t23, t33;
   double a0, a1, a2, b0, b1, b2;
#endif


   i = 0;
#if defined(GEMM44) || defined(GEMM43) || defined(GEMM42) || defined(GEMM41)
   for (; i < M-3; i+=4) {
      mindex0 = mind[i+0]*ldc;
      mindex1 = mind[i+1]*ldc;
      mindex2 = mind[i+2]*ldc;
      mindex3 = mind[i+3]*ldc;
      j = 0;
#if defined(GEMM44)
      for (; j < N-3; j+=4) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         t01 = 0; t11 = 0; t21 = 0; t31 = 0;
         t02 = 0; t12 = 0; t22 = 0; t32 = 0;
         t03 = 0; t13 = 0; t23 = 0; t33 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t30 += A[(i+3)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
            t31 += A[(i+3)*K+k] * B[(j+1)*ldb+k];
            t02 += A[(i+0)*K+k] * B[(j+2)*ldb+k];
            t12 += A[(i+1)*K+k] * B[(j+2)*ldb+k];
            t22 += A[(i+2)*K+k] * B[(j+2)*ldb+k];
            t32 += A[(i+3)*K+k] * B[(j+2)*ldb+k];
            t03 += A[(i+0)*K+k] * B[(j+3)*ldb+k];
            t13 += A[(i+1)*K+k] * B[(j+3)*ldb+k];
            t23 += A[(i+2)*K+k] * B[(j+3)*ldb+k];
            t33 += A[(i+3)*K+k] * B[(j+3)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         nindex2 = nind[j+2];
         nindex3 = nind[j+3];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex3+nindex1] += t31;
         C[mindex0+nindex2] += t02;
         C[mindex1+nindex2] += t12;
         C[mindex2+nindex2] += t22;
         C[mindex3+nindex2] += t32;
         C[mindex0+nindex3] += t03;
         C[mindex1+nindex3] += t13;
         C[mindex2+nindex3] += t23;
         C[mindex3+nindex3] += t33;
      }
#endif
#if defined(GEMM43)
      for (; j < N-2; j+=3) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         t01 = 0; t11 = 0; t21 = 0; t31 = 0;
         t02 = 0; t12 = 0; t22 = 0; t32 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t30 += A[(i+3)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
            t31 += A[(i+3)*K+k] * B[(j+1)*ldb+k];
            t02 += A[(i+0)*K+k] * B[(j+2)*ldb+k];
            t12 += A[(i+1)*K+k] * B[(j+2)*ldb+k];
            t22 += A[(i+2)*K+k] * B[(j+2)*ldb+k];
            t32 += A[(i+3)*K+k] * B[(j+2)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         nindex2 = nind[j+2];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex3+nindex1] += t31;
         C[mindex0+nindex2] += t02;
         C[mindex1+nindex2] += t12;
         C[mindex2+nindex2] += t22;
         C[mindex3+nindex2] += t32;
      }
#endif
#if defined(GEMM42)
      for (; j < N-1; j+=2) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         t01 = 0; t11 = 0; t21 = 0; t31 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t30 += A[(i+3)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
            t31 += A[(i+3)*K+k] * B[(j+1)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex3+nindex1] += t31;
      }
#endif
      for (; j < N; j++) {
         t00 = 0; t10 = 0; t20 = 0; t30 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[j*ldb+k];
            t10 += A[(i+1)*K+k] * B[j*ldb+k];
            t20 += A[(i+2)*K+k] * B[j*ldb+k];
            t30 += A[(i+3)*K+k] * B[j*ldb+k];
         }
         nindex0 = nind[j];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex3+nindex0] += t30;
      }
   }
#endif
#if defined(GEMM33) || defined(GEMM32) || defined(GEMM31)
   for (; i < M-2; i+=3) {
      mindex0 = mind[i+0]*ldc;
      mindex1 = mind[i+1]*ldc;
      mindex2 = mind[i+2]*ldc;
      j = 0;
#if defined(GEMM33)
      for (; j < N-2; j+=3) {
         t00 = 0; t10 = 0; t20 = 0;
         t01 = 0; t11 = 0; t21 = 0;
         t02 = 0; t12 = 0; t22 = 0;
         for (k = 0; k < K; k++) {
            /* stupid Sun compiler wants it to look like this... */
            a0 = A[(i+0)*K+k]; a1 = A[(i+1)*K+k]; a2 = A[(i+2)*K+k];
            b0 = B[k+(j+0)*ldb]; b1 = B[k+(j+1)*ldb]; b2 = B[k+(j+2)*ldb];
            t00 = t00 + a0*b0;
            t01 = t01 + a0*b1;
            t02 = t02 + a0*b2;
            t10 = t10 + a1*b0;
            t11 = t11 + a1*b1;
            t12 = t12 + a1*b2;
            t20 = t20 + a2*b0;
            t21 = t21 + a2*b1;
            t22 = t22 + a2*b2;
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         nindex2 = nind[j+2];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
         C[mindex0+nindex2] += t02;
         C[mindex1+nindex2] += t12;
         C[mindex2+nindex2] += t22;
      }
#endif
#if defined(GEMM32)
      for (; j < N-1; j+=2) {
         t00 = 0; t10 = 0; t20 = 0;
         t01 = 0; t11 = 0; t21 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t20 += A[(i+2)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
            t21 += A[(i+2)*K+k] * B[(j+1)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
         C[mindex2+nindex1] += t21;
      }
#endif
      for (; j < N; j++) {
         t00 = 0; t10 = 0; t20 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[j*ldb+k];
            t10 += A[(i+1)*K+k] * B[j*ldb+k];
            t20 += A[(i+2)*K+k] * B[j*ldb+k];
         }
         nindex0 = nind[j];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex2+nindex0] += t20;
      }
   }
#endif
#if defined(GEMM22) || defined(GEMM21)
   for (; i < M-1; i+=2) {
      mindex0 = mind[i+0]*ldc;
      mindex1 = mind[i+1]*ldc;
      j = 0;
#if defined(GEMM22)
      for (; j < N-1; j+=2) {
         t00 = 0; t10 = 0; t01 = 0; t11 = 0.0;
         for (k = 0; k < K-3; k+=4) {
            t00 += A[(i+0)*K+k+0] * B[(j+0)*ldb+k+0];
            t10 += A[(i+1)*K+k+0] * B[(j+0)*ldb+k+0];
            t01 += A[(i+0)*K+k+0] * B[(j+1)*ldb+k+0];
            t11 += A[(i+1)*K+k+0] * B[(j+1)*ldb+k+0];
            t00 += A[(i+0)*K+k+1] * B[(j+0)*ldb+k+1];
            t10 += A[(i+1)*K+k+1] * B[(j+0)*ldb+k+1];
            t01 += A[(i+0)*K+k+1] * B[(j+1)*ldb+k+1];
            t11 += A[(i+1)*K+k+1] * B[(j+1)*ldb+k+1];
            t00 += A[(i+0)*K+k+2] * B[(j+0)*ldb+k+2];
            t10 += A[(i+1)*K+k+2] * B[(j+0)*ldb+k+2];
            t01 += A[(i+0)*K+k+2] * B[(j+1)*ldb+k+2];
            t11 += A[(i+1)*K+k+2] * B[(j+1)*ldb+k+2];
            t00 += A[(i+0)*K+k+3] * B[(j+0)*ldb+k+3];
            t10 += A[(i+1)*K+k+3] * B[(j+0)*ldb+k+3];
            t01 += A[(i+0)*K+k+3] * B[(j+1)*ldb+k+3];
            t11 += A[(i+1)*K+k+3] * B[(j+1)*ldb+k+3];
         }
         for (; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[(j+0)*ldb+k];
            t10 += A[(i+1)*K+k] * B[(j+0)*ldb+k];
            t01 += A[(i+0)*K+k] * B[(j+1)*ldb+k];
            t11 += A[(i+1)*K+k] * B[(j+1)*ldb+k];
         }
         nindex0 = nind[j+0];
         nindex1 = nind[j+1];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
         C[mindex0+nindex1] += t01;
         C[mindex1+nindex1] += t11;
      }
#endif
      for (; j < N; j++) {
         t00 = 0; t10 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[(i+0)*K+k] * B[j*ldb+k];
            t10 += A[(i+1)*K+k] * B[j*ldb+k];
         }
         nindex0 = nind[j];
         C[mindex0+nindex0] += t00;
         C[mindex1+nindex0] += t10;
      }
   }
#endif
   for (; i < M; i++) {
      mindex0 = mind[i]*ldc;
      for (j = 0; j < N; j++) {
         t00 = 0;
         for (k = 0; k < K; k++) {
            t00 += A[i*K+k] * B[j*ldb+k];
         }

         C[mindex0+nind[j]] += t00;
      }
   }
} /* END gemmsk */
#endif 

#endif

#ifdef GEMM_BLAS
static void
gemms_BLAS (int    M,
            int    N,
            int    K,
            double *A,
            double *B,
            double *C,
            int    ldb,
            int    ldc,
            int    *mind,
            int    *nind)
{
   char   transa =  'n';
   char   transb =  't';
   double d_one  =  1.0;

   dgemm (&transb, &transa,
	  &N, &M, &K,
	  &d_one,
	  B, &ldb,
	  A, &K,
	  &d_one,
	  &C[mind[0]*ldc+nind[0]], &ldc);
}
#endif
