#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#include "flog.h"
#include "viterbicore.h"

int main(int argc, char *argv[])
{
  struct va_ctx *vp;
  int len, nbits;
  unsigned char *syms;
  FILE *fh;
  int i;
  struct timezone tz;
  struct timeval now,last,intev;
  int trial = 1000;


  if( argc > 1 )
  {
  	 trial = atoi( argv[1] );
  }

  len = 500;
  nbits = 288;

  posix_memalign((void **)&syms, 16, sizeof(unsigned char)*640);
  fh = fopen("bbu8_fecdecoding_vit.in", "rb");
  fread(syms, sizeof(unsigned char), 576, fh);
  fclose(fh);

  vp = create_viterbi(len);
  //warm up
  for( i = 0; i < 6000; i++ )
  {
	  init_viterbi(vp,0);
	  update_viterbi_blk((void *)vp, syms, nbits);
  }

  int j;
  gettimeofday(&last, &tz);

  for( j = 0; j < 30; j++ )
  { 
	  for( i = 0; i < trial; i++ )
	  {
		  init_viterbi(vp,0);
		  update_viterbi_blk((void *)vp, syms, nbits);
	  }
  }

  gettimeofday(&now, &tz);
  intev.tv_sec  = now.tv_sec-last.tv_sec;
  intev.tv_usec = now.tv_usec-last.tv_usec;
  
  printf("Elapse time:= %lf seconds\n", (double)(intev.tv_sec) + (double)(intev.tv_usec)/1000000 );
  //printf("%lf\n", (double)(intev.tv_sec) + (double)(intev.tv_usec)/1000000 );

  fh = fopen("bbu8_fecdecoding_vit.out", "wb");
  fwrite((void *)(vp->decisions), sizeof(unsigned char), 18432, fh);
  fclose(fh);

  delete_viterbi((void *)vp);
  free(syms);

  return 0;
}
