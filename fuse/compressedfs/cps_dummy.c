#include "cps_dummy.h"

/**
   Dummy compression has a compression rate of 1:1,
   i.e., it does nothing.

 **/

static int dummy_compress(int infd, int outfd)
{
   return 0;
}

static int dummy_decompress(int infd, int outfd)
{
   return 0;
}

void dummy_init(struct compression_operations* opt)
{
   strcpy(opt->cname, DUMMY_NAME);
   opt->compress = dummy_compress;
   opt->decompress = dummy_decompress;
}
