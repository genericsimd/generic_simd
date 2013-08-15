#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "chk_demod.h"

int main(int argc, char *argv[])
{
  int  i;
  FILE *fh;
  float *ant1_r;
  float *ant1_i;
  float *ant2_r;
  float *ant2_i;
  float *hest1_r;
  float *hest1_i;
  float *hest2_r;
  float *hest2_i;
  float *noise_power1;
  float *noise_power2;
  float *softbit;
  float *softbit_p7;
  struct union_burst_ie para; 

  posix_memalign((void **)&ant1_r, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&ant1_i, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&ant2_r, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&ant2_i, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&hest1_r, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&hest1_i, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&hest2_r, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&hest2_i, 16, sizeof(float)*48*31*6);
  posix_memalign((void **)&noise_power1, 16, sizeof(float)*8);
  posix_memalign((void **)&noise_power2, 16, sizeof(float)*8);
  posix_memalign((void **)&softbit, 16, sizeof(float)*48*31*30);
  posix_memalign((void **)&softbit_p7, 16, sizeof(float)*48*31*30*2);

  memset(&para, 0, sizeof(para));
  para.buf_in_len = 384;
  para.buf_out_len = 384;
  para.uiuc = 1;
  para.slots_num = 8;
  para.repetition_coding_indication = 1;
  //para.code_id = CC_QAM64_34;
  

  // read input
  fh = fopen("bbu8_burstdata_ant1_r.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &ant1_r[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata_ant1_i.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &ant1_i[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata_ant2_r.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &ant2_r[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata_ant2_i.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &ant2_i[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata3_ant1_r.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &hest1_r[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata3_ant1_i.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &hest1_i[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata3_ant2_r.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &hest2_r[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_burstdata3_ant2_i.dat", "rt");
  for (i=0; i<384; i++) {
    fscanf(fh, "%f", &hest2_i[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_noise_est_ant1.dat", "rt");
  for (i=0; i<8; i++) {
    fscanf(fh, "%f", &noise_power1[i]);
  }
  fclose(fh);

  fh = fopen("bbu8_noise_est_ant2.dat", "rt");
  for (i=0; i<8; i++) {
    fscanf(fh, "%f", &noise_power2[i]);
  }
  fclose(fh);

  //read output
  fh = fopen("bbu8_demodulation.dat", "rt");
  for (i=0; i<768; i++) {
    fscanf(fh, "%f", &softbit[i]);
  }
  fclose(fh);

  int j;
  int trial = 100;
  struct timezone tz;
  struct timeval now,last,intev;

  gettimeofday(&last, &tz);
  for( j = 0; j < 1000; j++ )
  { 
	  for( i = 0; i < trial; i++ )
	  {
	    phy_ul_demodulation(    &para,
				    ant1_r,
				    ant1_i,
				    ant2_r,
				    ant2_i,
				    hest1_r,
				    hest1_i,
				    hest2_r,
				    hest2_i,
				    noise_power1,
				    noise_power2,
				    softbit_p7);
          }
   }

  gettimeofday(&now, &tz);
 
  int nerrors = 0;
  for (i=0; i<768; i++) {
    if ((abs(softbit_p7[i] - softbit[i]) > 1.E-4)) {
      printf("OUTPUT: %f mismatch with expected value %f at %d\n", softbit_p7[i], softbit[i], i);
      nerrors++;
    }
  }

  if (nerrors > 0 ) {
    printf("Verification failed: nerrors = %d\n", nerrors);
  } else {
    intev.tv_sec  = now.tv_sec-last.tv_sec;
    intev.tv_usec = now.tv_usec-last.tv_usec;

    printf("Passed w/ elapse time %lf seconds\n", (double)(intev.tv_sec) + (double)(intev.tv_usec)/1000000 );
    //printf("%lf\n", (double)(intev.tv_sec) + (double)(intev.tv_usec)/1000000 );
  }

  free(ant1_r);
  free(ant1_i);
  free(ant2_r);
  free(ant2_i);
  free(hest1_r);
  free(hest1_i);
  free(hest2_r);
  free(hest2_i);
  free(noise_power1);
  free(noise_power2);
  free(softbit);
  free(softbit_p7);

  return 0;
}
