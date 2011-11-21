#include "cps_dummy.h"

/**
   Dummy compression has a compression rate of 1:1,
   i.e., it does nothing.

 **/

#define BSIZE

static int dummy_compress(int infd, int outfd)
{
   char buf[BSIZE];
   ssize_t n;
   int r;

   r = 0;

   while ( 1 )
   {
      n = read(infd, buf, bsize);
      
      if ( n <= 0 || write(outfd, buf, n) != n )
      {
         r = -1;
         break;
      }
   }

   return r;
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
