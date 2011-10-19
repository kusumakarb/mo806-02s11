#ifndef __RAM_FS_H__
#define __RAM_FS_H__

#define FUSE_USE_VERSION  26
   
#define MAX_PATH_SIZE 256

#include <fuse.h>  
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

typedef __nlink_t r_nlink_t;
typedef __mode_t r_mode_t;


typedef struct inode
{
   void* data;
   size_t len;   
   r_nlink_t nlink;
   r_mode_t mode;
} inode_t;


typedef struct dentry
{
   inode_t* in;
   char* name;
   struct dentry* dnext; /* next dentry in parent linked list */
   struct dentry* dchild; /* children dentries linked list head */
} dentry_t;

/* log */

void init_log(const char*);
void do_log(const char*);

#endif /*__RAM_FS_H__*/
