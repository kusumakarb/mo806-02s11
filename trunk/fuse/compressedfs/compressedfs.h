/**

   compressed, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Esteve <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

#ifndef __COMRPESSED_FS_H__
#define __COMRPESSED_FS_H__

#define FUSE_USE_VERSION  26
   
#define DEFAULT_STRING_SIZE 64
#define DEFAULT_PATH_SIZE   128

// usual files cannot end with SPECIAL_EXT, those are reserved for fs
// special operations
#define SPECIAL_EXT "compfs"

#define DEBUG 

#include <sys/time.h>
#include <sys/types.h>
/*#include <attr/xattr.h>*/
#include <fuse.h>  
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

struct compression_operations
{
   /* compression algorithm name */
   char cname[DEFAULT_STRING_SIZE];

   /* compression handlers  */
   /**
      void compress(int infd, int oufd);
      compress data from input fd to output fd
      return 0 on success
      return != 0 on failue and set errno
    **/
   int (*compress) (int, int);
   /**
      void decompress(int infd, int oufd);
      decompress data from input fd to output fd
      return 0 on success
      return != 0 on failue and set errno
    **/
   int (*decompress) (int, int);
};

struct cfile
{
   int fd;
   int count;
   char path[DEFAULT_PATH_SIZE];
   int compressed;
   
   int oflag; // open flags
   mode_t mode;

   pthread_mutex_t lock;

   struct cfile* next;
};

struct compression_info
{
   /* compression operations */
   struct compression_operations opt;
   /* backstore path */
   char* bs_path;
   /* strlen(bs_path) */
   size_t bs_len;

   struct cfile* ftable;
   pthread_mutex_t lock;
   pthread_mutex_t open_lock;
};




/*
  ===============================
   SUPPORTED COMPRESSION METHODS
  ===============================
*/
#include "cps/dummy/dummy.h"
#include "cps/scz/scz_main.h"
#include "cps/lzo/lzo_main.h"

#define DEFAULT_COMPRESSION LZO_NAME

/*
   **************************
   **DO NOT CODE BELOW HERE**
   **************************
 */

#endif /*__COMRPESSED_FS_H__*/
