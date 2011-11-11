#include "cps_dummy.h"

/**
   Dummy compression has a compression rate of 1:1,
   i.e., it does nothing.

 **/

static void* dummy_compress(void* data, size_t size)
{
   return data;
}

static void* dummy_decompress(void* data, size_t size)
{
   return data;
}

void dummy_init(struct compression_operations* opt)
{
   strcpy(opt->cname, DUMMY_NAME);
   opt->compress = dummy_compress;
   opt->decompress = dummy_decompress;
}
