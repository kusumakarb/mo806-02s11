#include "scz_main.h"

/**
   Simple Compression Zip 
 **/

static int scz_compress(int infd, int outfd)
{
   return 0;
}

static int scz_decompress(int infd, int outfd)
{
   return 0;
}

void scz_init(struct compression_operations* opt)
{
   strcpy(opt->cname, DUMMY_NAME);
   opt->compress = scz_compress;
   opt->decompress = scz_decompress;
}
