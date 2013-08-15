#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "phy_ul_channelest.h"

const char *file_input_r = "bbu8_subcarderandom_ant1_r.dat";
const char *file_input_i = "bbu8_subcarderandom_ant1_i.dat";
const char *file_output_r = "bbu8_chest_ant1_r.dat";
const char *file_output_i = "bbu8_chest_ant1_i.dat";

FILE *fp_input_r;
FILE *fp_input_i;
FILE *fp_output_r;
FILE *fp_output_i;

void open_files()
{
    fp_input_r = fopen(file_input_r, "r+");
    if(fp_input_r == NULL)
    {
        printf("open dump file failed \n");
        exit(1);
    }

    fp_input_i = fopen(file_input_i, "r+");
    if(fp_input_i == NULL)
    {
        printf("open dump file failed \n");
        exit(1);
    }

    fp_output_r = fopen(file_output_r, "r+");
    if(fp_output_r == NULL)
    {
        printf("open dump file failed \n");
        exit(1);
    }

    fp_output_i = fopen(file_output_i, "r+");
    if(fp_output_i == NULL)
    {
        printf("open dump file failed \n");
        exit(1);
    }

}

#define SUBCARRIER_NUM (840)

int main(int argc, char *argv[])
{
    int i;
    struct timezone tz;
    struct timeval now,last,intev;

    struct phy_ul_rx_syspara para;
    float subcarderand_r[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));
    float subcarderand_i[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));
    float h_r[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));
    float h_i[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));
    float h_r_p7[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));
    float h_i_p7[SUBCARRIER_NUM * 3] __attribute__((aligned(16)));

    open_files();

    para.numoftiles = 186;
    para.ofdma_nused_no_dc = 756;

    for(i = 0; i < para.ofdma_nused_no_dc * 3; i++)
    {
        fscanf(fp_input_r, "%f", &(subcarderand_r[i]));
        fscanf(fp_input_i, "%f", &(subcarderand_i[i]));

    }

    for(i = 0; i < para.ofdma_nused_no_dc * 3; i++)
    {
        fscanf(fp_output_r, "%f", &(h_r[i]));
        fscanf(fp_output_i, "%f", &(h_i[i]));

    }
  int j;
  int trial = 100;
  gettimeofday(&last, &tz);
  for( j = 0; j < 3000; j++ )
  { 
	  for( i = 0; i < trial; i++ )
	  {
	    if (phy_ul_single_chanlest(&para, subcarderand_r, subcarderand_i, h_r_p7, h_i_p7) != 0) 
	    {
	      printf("Channelest_ut failed\n");
	      return 0;
	    }
          }
   }
  gettimeofday(&now, &tz);
 
  int nerrors = 0;
  for (i=0; i<para.ofdma_nused_no_dc * 3; i++) {
    if ((abs(h_r[i] - h_r_p7[i]) > 1.E-4) || (abs(h_i[i] - h_i_p7[i]) > 1.E-4)) {
      printf("Info: (%f, %f) mismatch with expected value (%f, %f) at %d\n", h_r_p7[i], h_i_p7[i], h_r[i], h_i[i], i);
      nerrors++;
    }
  }

  if (nerrors > 0) {
    printf("Verification failed: nerrors = %d\n", nerrors);
  } else {
    intev.tv_sec  = now.tv_sec-last.tv_sec;
    intev.tv_usec = now.tv_usec-last.tv_usec;
    printf("Test passed w/ elapse time %.2lf seconds\n", (double)(intev.tv_sec) + (double)(intev.tv_usec)/1000000);
  }
  return 0;
}
