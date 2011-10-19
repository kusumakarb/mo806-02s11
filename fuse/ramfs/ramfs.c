/**

   ramfs, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Este <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

/**

   main implementation is here
   if everything else is in opt.c

   HOW TO MOUNT (look at makefile):
   ./ramfs [options] mount_point

 **/

/**

   NOTE: this was coded for learning/teaching purpouses only.
   It's not (even nearly) efficient (compared to kernel ramfs).

   There's no dentry cache, every function has to parse the path.
   You could improve performance using fuse_file_info structure
   to profit a little from the kernel dentry cache.

   This code is not threaded aware. You should mount it with
   -s fuse option, i.e., synchronized calls. This is yet
   another performance penalty.

   ramfs_write is specially slow. Take a look at the comments
   there so you know how to improve it.

 **/

/**

   NOTE2: Please, refer to <fuse.h> for each operation description.
   Usually they match a kernel syscall. Try man <opt_name>, e.g.
   "man rename" if you want to know what to do whithin fuse rename
   operation.
   <fuse.h> could be at /usr/include/fuse/fuse.h

 **/

/**

   NOTE3: All paths FUSE passes to us start from the root '/', dispite
   the mount point. e.g. mount point is /myfs
   A open call to /myfs/foo will issue a path like this: /foo

 **/

#include "ramfs.h"

static int ramfs_getattr(const char *path, struct stat *stbuf)
{
   int res;
   dentry_t* d;

   res = 0;
   // get dentry for this path
   d = get_path(path);

   if (!d)
   {
      // invalid path
      res = -ENOENT;
   }
   else
   {
      // pass inode info
      memset(stbuf, 0, sizeof(struct stat));
      stbuf->st_mode = d->in->mode;
      stbuf->st_nlink = d->in->nlink;
      stbuf->st_size = d->in->len;
   }

   return res;
}
  
static int ramfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
   (void) offset;
   (void) fi;
  
   dentry_t* d;

   if(!( d = get_path(path) ))
      return -ENOENT;
  
   // filler fills buf with each directory entry
   filler(buf, ".", NULL, 0);
   filler(buf, "..", NULL, 0);
  
   for (d = d->dchild; d; d = d->dnext)
      filler(buf, d->name, NULL, 0);
  
   return 0;
}
  
static int ramfs_open(const char *path, struct fuse_file_info *fi)
{
   dentry_t* d;

   d = get_path(path);

   if (!d)
      return -ENOENT;
  
   // you could test access permitions here
   // take a look at man open for possible errors
  
   return 0;
}
  
static int ramfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
   size_t len;
   dentry_t* d;
   (void) fi;

   d = get_path(path);

   if(!d)
      return -ENOENT;
  
   len = d->in->len;

   if (offset < len) 
   {
      if (offset + size > len)
         size = len - offset;

      // copy data to be read to buf from inode data
      memcpy(buf, d->in->data + offset, size);
   }
   else
      size = 0;
  
   return size;
}

static int ramfs_write(const char* path, const char* buf, size_t size, off_t offset,
                       struct fuse_file_info *fi)
{
   dentry_t* d;
   inode_t* in;
   void* tmp;
   (void) fi;

   d = get_path(path);

   if(!d)
      return -ENOENT;

   in = d->in;

   // do we have to make inode data larger?
   if (size + offset > in->len)
   {
      // can't use realloc, if it fails, we'd lost data
      // this is a huge performance penalty
      // to improve performance a smarter memory management algorithm should be used

      tmp = malloc(in->len + size);
      
      if (tmp)
      {
         if (in->data)
         {
            memcpy(tmp, in->data, in->len);
            free(in->data);
         }

         in->data = tmp;
         in->len += size;
      }
      else
         return -EFBIG;
   }

   memcpy(in->data + offset, buf, size);

   return size;
}

// generic operations to create file and directories
static int __ramfs_mkentry(const char * path, mode_t mode)
{
   dentry_t* d;
   dentry_t* nd;
   char name[MAX_PATH_SIZE];

   d = get_parent(path);

   if (!d)
      return -ENOENT;

   // get filename
   get_filename(path, name);

   // check if it already exists
   if (get_dentry(d, name))
      return -EEXIST;

   // create new instance
   nd = alloc_dentry(name, alloc_inode(mode));

   // add dentry to parent list
   if (d_addchild(d, nd))
   {
      // if fails, rollbak
      iunlink(nd, nd->in);
      return -ENOSPC;
   }

   return 0;
}

static int ramfs_mknod(const char * path, mode_t mode, dev_t dev)
{
   // new file
   return __ramfs_mkentry(path, mode);
}

static int ramfs_mkdir(const char * path, mode_t mode)
{
   // new dir
   return __ramfs_mkentry(path, mode | S_IFDIR);
}

static int ramfs_unlink(const char* path)
{
   dentry_t* d;

   d = get_path(path);

   if (!d)
      return -ENOENT;

   /* linux calls rmdir if target is a directory */

   return iunlink(d, d->in);
}

static int ramfs_rmdir(const char* path)
{
   dentry_t* d;

   d = get_path(path);

   if (!d)
      return -ENOENT;

   // removes dentry reference to inode object
   return iunlink(d, d->in);
}

static void* ramfs_init(struct fuse_conn_info *conn)
{
   (void) conn;

   ramfs_opt_init();

   return NULL;
}

static void ramfs_destroy(void* v)
{
   ramfs_opt_destroy();
}

  
// function mapping
static struct fuse_operations ramfs_oper = {
   .init = ramfs_init,
   .destroy = ramfs_destroy,
   .getattr   = ramfs_getattr,
   .readdir = ramfs_readdir,
   .open   = ramfs_open,
   .read   = ramfs_read,
   .write = ramfs_write,
   .mknod = ramfs_mknod,
   .mkdir = ramfs_mkdir,
   .unlink = ramfs_unlink,
   .rmdir = ramfs_rmdir,
};
  
int main(int argc, char *argv[])
{
   // fuse will parse mount options
   return fuse_main(argc, argv, &ramfs_oper, NULL);
}
