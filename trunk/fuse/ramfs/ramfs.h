/**

   ramfs, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Este <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

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

#define ISROOT(D) d->dparent == NULL

typedef struct inode
{
   void* data;
   size_t len;   
   nlink_t nlink; /*link number - dentries count*/
   mode_t mode;

   struct dentry* dentries; /*dentries pointing to this inode (linked list) */
} inode_t;


typedef struct dentry
{
   inode_t* in;
   char* name;

   struct dentry* dparent; /* parent dentry, NULL for root*/
   struct dentry* dnext; /* next dentry in parent linked list */
   struct dentry* dchild; /* children dentries linked list head */
   struct dentry* dinode; /* linked list for inode reference */

} dentry_t;

/* operations */

/* links a dentry with a inode */
int ilink(dentry_t*, inode_t*);
/* unlinks a dentry - removes inode if no dentries reference it*/
int iunlink(dentry_t*, inode_t*);
/* i.e. new inode() */
inode_t* alloc_inode(mode_t);
/* frees an inode */
int free_inode(inode_t*);
/* new dentry. calls ilink to inode (may be null) */
dentry_t* alloc_dentry(char*, inode_t*);
/* frees a dentry */
int free_dentry(dentry_t*);
/* get dentry for a (full) path */
dentry_t* get_path(const char*);
/* given a dentry and a name, returns the child dentry corresponding to that name (null if none) */
dentry_t* get_dentry(dentry_t*, const char*);
/* given a path, returns the parent dentry */
dentry_t* get_parent(const char*);
/* returns token after last '/' */
void get_filename(const char*, char*);
/*
   d_addchild(parent, child);
   adds child to the parent dentry
*/
int d_addchild(dentry_t*, dentry_t*);
/* init */
void ramfs_opt_init();
/* destroy */
void ramfs_opt_destroy();

#endif /*__RAM_FS_H__*/
