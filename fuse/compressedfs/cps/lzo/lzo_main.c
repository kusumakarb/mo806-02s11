#include "lzo_main.h"

/**
   LZO COMPRESSION
 **/


/* Work-memory needed for compression. Allocate memory in units
 * of 'lzo_align_t' (instead of 'char') to make sure it is properly aligned.
 */

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

// block size
#define BSIZE 1024

/* We want to compress the data block at 'in' with length 'IN_LEN' to
 * the block at 'out'. Because the input block may be incompressible,
 * we must provide a little more output space in case that compression
 * is not possible.
 */
#define OUT_LEN     (BSIZE + BSIZE / 16 + 64 + 3)

typedef struct block_header
{
   unsigned char is_compressed;
   unsigned short block_size;
} header_t;

/*
  the compressed file is structured as follows:
  [0] 1 byte  (unsigned char):  1 if compressed, 0 otherwise
  [1~2] 2 bytes (unsigned short): block size
  [3~...]BLOCK_DATA
 */
#define BLOCK_OFFSET sizeof(struct block_header)

static int lzo_compress(int infd, int outfd)
{
   int r;
   int n;
   lzo_uint in_len;
   lzo_uint out_len;
   unsigned char __LZO_MMODEL  in[BSIZE + BLOCK_OFFSET];
   unsigned char __LZO_MMODEL  out[OUT_LEN];
   header_t header;

   // working memory
   HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);
   
   r = 0;

   // rewind fds
   if (lseek(infd, 0, SEEK_SET) == -1 ||
       lseek(outfd, 0, SEEK_SET) == -1)
      return -1;

   while ( 1 )
   {
      in_len = BSIZE;

      // read a block from in fd
      n = read(infd, in, in_len);
      
      if ( n == -1 )
      {
         r = -1;
         break;
      }

      if ( n < in_len )
      {
         // this is the last block, change block size
         in_len = n;
      }

      // compress
      if (lzo1x_1_compress(in, in_len, out + BLOCK_OFFSET, &out_len, wrkmem) != LZO_E_OK)
      {
         errno = EIO;
         r = -1;
         break;
      }

      // incompressible block
      if (out_len >= in_len)
      {
         // can't compress, it's better to stick to original data
         memcpy(out + BLOCK_OFFSET, in, in_len);

         header.is_compressed = 0;
      }
      else
         header.is_compressed = 1;

      // set block size
      header.block_size = (unsigned short)out_len;

      // copy header to out stream
      memcpy(out, &header, BLOCK_OFFSET);

      out_len += BLOCK_OFFSET;

      // write compressed data
      if ( write(outfd, out, out_len) != out_len )
      {
         r = -1;
         break;
      }
      else if ( n < BSIZE )
         break;
   }

   return r;
}

static int lzo_decompress(int infd, int outfd)
{
   int r;
   int n;
   lzo_uint in_len;
   lzo_uint out_len;
   unsigned char __LZO_MMODEL  in[BSIZE];
   unsigned char __LZO_MMODEL  out[OUT_LEN + BLOCK_OFFSET];
   header_t header;
   
   r = 0;
   errno = 0;

   // rewind fds
   if (lseek(infd, 0, SEEK_SET) == -1 ||
       lseek(outfd, 0, SEEK_SET) == -1)
      return -1;

   // read header from input
   while ( n = read(infd, &header, sizeof(header_t)) )
   {
      in_len = header.block_size;  

      if ( header.is_compressed )
      {
         // data is compressed

         // read block from input
         n = read(infd, in, in_len);

         if ( n != in_len )
         {
            r = -1;
            errno = EIO;
            break;
         }

         // decompress
         if (lzo1x_decompress(in, in_len, out, &out_len, NULL) != LZO_E_OK)
         {
            errno = EIO;
            r = -1;
            break;
         }
      }
      else
      {
         // data is not compressed, we can read it directly to out buffer
         n = read(infd, out, in_len);

         out_len = in_len;

         if ( n == in_len )
         {
            r = -1;
            errno = EIO;
            break;
         }
      }
      
      // write decompressed data to output
      if ( write(outfd, out, out_len) != out_len )
      {
         r = -1;
         if (errno == 0)
            errno = EIO;

         break;
      }
   }

   if (n == -1 && r == 0)
      r = -1;

   return r;
}

void minilzo_init(struct compression_operations* opt)
{
   strcpy(opt->cname, LZO_NAME);
   opt->compress = lzo_compress;
   opt->decompress = lzo_decompress;

   // init lzo lib
   lzo_init();
}
