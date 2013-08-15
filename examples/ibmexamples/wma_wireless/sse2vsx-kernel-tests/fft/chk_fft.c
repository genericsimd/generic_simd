#include <stdio.h>
#include <stdlib.h>

#include "fft.h"
#define OFDMA_SYMBOL_SIZE 1024

int main(int argc, char *argv[])
{
  int len, i;
  float *syms_tr, *syms_ti;
  float *p7_fr,   *p7_fi;
  float *syms_fr, *syms_fi;
  float *syms_itr, *syms_iti;
  float *p7_ifr,   *p7_ifi;
  float *syms_ifr, *syms_ifi;
  FILE *fh;

  float **XX;
  float **x;
  float **X;

  posix_memalign((void **)&syms_tr, 16, sizeof(float)*1024);
  posix_memalign((void **)&syms_ti, 16, sizeof(float)*1024);
  posix_memalign((void **)&p7_fr,   16, sizeof(float)*1024);
  posix_memalign((void **)&p7_fi,   16, sizeof(float)*1024);
  posix_memalign((void **)&syms_fr, 16, sizeof(float)*1024);
  posix_memalign((void **)&syms_fi, 16, sizeof(float)*1024);

  posix_memalign((void **)&syms_itr, 16, sizeof(float)*1024);
  posix_memalign((void **)&syms_iti, 16, sizeof(float)*1024);
  posix_memalign((void **)&p7_ifr,   16, sizeof(float)*1024);
  posix_memalign((void **)&p7_ifi,   16, sizeof(float)*1024);
  posix_memalign((void **)&syms_ifr, 16, sizeof(float)*1024);
  posix_memalign((void **)&syms_ifi, 16, sizeof(float)*1024);

  // read input
  fh = fopen("bbu8_fft_input_tr.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_tr[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_fft_input_ti.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_ti[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_shift_r.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_itr[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_shift_i.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_iti[i]);
  }
  fclose(fh);

  // read x86 output
  fh = fopen("bbu8_fft_output_fr.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_fr[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_fft_output_fi.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_fi[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_ifft_r.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_ifr[i]);
  }
  fclose(fh);
  fh = fopen("bbu8_ifft_i.dat", "rt");
  for (i=0; i<1024; i++) {
    fscanf(fh, "%f", &syms_ifi[i]);
  }
  fclose(fh);

for(i = 0; i < 1; i++ )
{
  fft_init(OFDMA_SYMBOL_SIZE, &XX, &x, &X);
  ifft(OFDMA_SYMBOL_SIZE, p7_ifr, p7_ifi, syms_itr, syms_iti, XX, x, X);
  fft(OFDMA_SYMBOL_SIZE, syms_tr, syms_ti, p7_fr, p7_fi, XX, x, X);
  fft(OFDMA_SYMBOL_SIZE, syms_tr, syms_ti, p7_fr, p7_fi, XX, x, X);
  fft_quit(XX, x, X);
}
#if 1
  for (i=0; i<1024; i++) {
    if ((abs(syms_fr[i] - p7_fr[i]) > 1.E-5) || (abs(syms_fi[i] - p7_fi[i]) > 1.E-5)) {
      printf("FFT Info: (%f, %f) mismatch with expected value (%f, %f) at %d\n", p7_fr[i], p7_fi[i], syms_fr[i], syms_fi[i], i);
    }
  }
  for (i=0; i<1024; i++) {
    if ((abs(syms_ifr[i] - p7_ifr[i]) > 1.E-5) || (abs(syms_ifi[i] - p7_ifi[i]) > 1.E-5)) {
      printf("IFFT Info: (%f, %f) mismatch with expected value (%f, %f) at %d\n", p7_ifr[i], p7_ifi[i], syms_ifr[i], syms_ifi[i], i);
    }
  }
#endif
  free(syms_tr);
  free(syms_ti);
  free(p7_fr);
  free(p7_fi);
  free(syms_fr);
  free(syms_fi);
  free(syms_itr);
  free(syms_iti);
  free(p7_ifr);
  free(p7_ifi);
  free(syms_ifr);
  free(syms_ifi);

  return 0;
}
