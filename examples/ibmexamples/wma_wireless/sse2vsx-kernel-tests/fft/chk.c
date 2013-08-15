#include <stdio.h>
#include <stdlib.h>

#include "flog.h"
#include "viterbicore.h"

int main(int argc, char *argv[])
{
  struct va_ctx *vp;
  int len, nbits;
  unsigned char *syms;
  FILE *fh;

  len = 500;
  nbits = 288;

  posix_memalign((void **)&syms, 16, sizeof(unsigned char)*640);
  fh = fopen("bbu8_fecdecoding_vit.in", "rb");
  fread(syms, sizeof(unsigned char), 576, fh);
  fclose(fh);

  vp = create_viterbi(len);
  update_viterbi_blk((void *)vp, syms, nbits);

  fh = fopen("bbu8_fecdecoding_vit.out", "wb");
  fwrite((void *)(vp->decisions), sizeof(unsigned char), 18432, fh);
  fclose(fh);

  delete_viterbi((void *)vp);
  free(syms);

  return 0;
}
