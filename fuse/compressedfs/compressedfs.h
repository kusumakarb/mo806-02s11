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

#define DEBUG 

#include <sys/types.h>
#include <fuse.h>  
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

struct compression_operations
{
   /* compression algorithm name */
   char cname[DEFAULT_STRING_SIZE];

   /* compression handlers  */
   void* (*compress) (void*,size_t);
   void* (*decompress) (void*,size_t);
};

struct compression_info
{
   /* compression operations */
   struct compression_operations opt;
   /* backstore path */
   char* bs_path;
   /* strlen(bs_path) */
   size_t bs_len;
};


#endif /*__COMRPESSED_FS_H__*/
