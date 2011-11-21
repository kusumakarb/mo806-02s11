#include "cps_dummy.h"

/**
   Dummy compression has a compression rate of 1:1,
   i.e., it does nothing.

 **/

#define BSIZE 2048

static int cpy(int infd, int outfd)
{
   char buf[BSIZE];
   ssize_t n;
   int r;

   r = 0;

   if (lseek(infd, 0, SEEK_SET) == -1 ||
       lseek(outfd, 0, SEEK_SET) == -1)
      return -1;

   while ( 1 )
   {
      n = read(infd, buf, BSIZE);
      
      if ( n == -1 || write(outfd, buf, n) != n )
      {
         r = -1;
         break;
      }
      else if ( n < BSIZE )
         break;
   }

   return r;
}

static int dummy_compress(int infd, int outfd)
{
   return cpy(infd, outfd);
}

static int dummy_decompress(int infd, int outfd)
{
   return cpy(infd, outfd);
}

void dummy_init(struct compression_operations* opt)
{
   strcpy(opt->cname, DUMMY_NAME);
   opt->compress = dummy_compress;
   opt->decompress = dummy_decompress;
}
