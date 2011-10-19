#include "ramfs.h"

static int ramfs_getattr(const char *path, struct stat *stbuf)
{
   int res;
   dentry_t* d;

   res = 0;
   d = get_path(path);

   if (!d)
   {
      res = -ENOENT;
   }
   else
   {
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
  
//   if((fi->flags & 3) != O_RDONLY)
//      return -EACCES;
  
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

   if (size + offset > in->len)
   {
      tmp = malloc(in->len + size);
      
      if (tmp)
      {
         if (in->data)
         {
            memcpy(tmp, in->data, in->len);
            free(in->data);
         }

         in->data = tmp;
      }
      else
         return -EFBIG;
   }

   memcpy(in->data + offset, buf, size);
   in->len = size + offset;

   return size;
}

static int __ramfs_mkentry(const char * path, mode_t mode)
{
   dentry_t* d;
   dentry_t* nd;
   char name[MAX_PATH_SIZE];

   d = get_parent(path);

   if (!d)
      return -ENOENT;

   get_filename(path, name);

   if (get_dentry(d, name))
      return -EEXIST;

   nd = alloc_dentry(name, alloc_inode(mode));

   if (d_addchild(d, nd))
   {
      iunlink(nd, nd->in);
      return -ENOSPC;
   }

   return 0;
}

static int ramfs_mknod(const char * path, mode_t mode, dev_t dev)
{
   return __ramfs_mkentry(path, mode);
}

static int ramfs_mkdir(const char * path, mode_t mode)
{
   return __ramfs_mkentry(path, mode | S_IFDIR);
}

static void* ramfs_init(struct fuse_conn_info *conn)
{
   (void) conn;

   log_init("log.txt");

   ramfs_opt_init();

   return NULL;
}

static void ramfs_destroy(void* v)
{
   log_destroy();
}

  
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
};
  
int main(int argc, char *argv[])
{
   return fuse_main(argc, argv, &ramfs_oper, NULL);
}
