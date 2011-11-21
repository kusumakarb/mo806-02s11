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

#define GET_FD(FI) ((struct cfile*)FI->fh)->fd

struct compression_operations
{
   /* compression algorithm name */
   char cname[DEFAULT_STRING_SIZE];

   /* compression handlers  */
   void* (*compress) (void*,size_t);
   void* (*decompress) (void*,size_t);
};

struct cfile
{
   int fd;
   int count;
   
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
};


#endif /*__COMRPESSED_FS_H__*/
