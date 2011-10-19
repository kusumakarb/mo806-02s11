#include "ramfs.h"

static dentry_t* droot;

static int ilink(dentry_t* d, inode_t* in)
{  
   if (!d || d->in || !in)
      return -1;

    in->nlink++;

    d->in = in;

    return 0;
}

static inode_t* alloc_inode(r_mode_t mode)
{
   inode_t* in;

   in = (inode_t*)malloc(sizeof(inode_t));

   if (in)
   {
      in->data = NULL;
      in->len = 0;
      in->nlink = 0;
      in->mode = mode;
   }

   return in;
}

static dentry_t* alloc_dentry(char* name, inode_t* in)
{
   dentry_t* d;

   d = (dentry_t*)malloc(sizeof(dentry_t));

   if (d)
   {
      d->name = name;
      d->in = NULL;
      d->dnext = NULL;
      d->dchild = NULL;

      if (ilink(d, in))
      {
         free(d);
         d = NULL;
      }
   }

   return d;
}

static dentry_t* get_dentry(dentry_t* dparent, const char* name)
{
   dentry_t* d;

   for (d = dparent->dchild; d != NULL; d = d->dnext)
      if (!strcmp(d->name, name))
          break;

   return d;
}

static dentry_t* get_path(const char* path)
{
   dentry_t* d;
   char* t;
   char p[MAX_PATH_SIZE];

   if (!strcmp("/", path))
      return droot;

   strncpy(p, path, MAX_PATH_SIZE);
   d = droot;

   t = strtok(p, "/");

   if ( t == NULL || (d = get_dentry(d, t)) == NULL )
      return NULL;

   while ( (t = strtok(NULL, "/") ))
   {
      if ( (d = get_dentry(d, t)) == NULL )
         break;
   }

   return d;
}

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
  
static struct fuse_operations hello_oper = {
   .getattr   = ramfs_getattr,
   .readdir = ramfs_readdir,
   .open   = ramfs_open,
   .read   = ramfs_read,
   .write = ramfs_write,
};

static void init_fs()
{
   init_log("log.txt");

   droot = alloc_dentry("", alloc_inode(S_IFDIR | 0755));

   droot->dchild = alloc_dentry("andre", alloc_inode(S_IFREG | 0755));
   droot->dchild->in->data = malloc(4);
   memcpy(droot->dchild->in->data, "hehe", 4);
   droot->dchild->in->len = 4;
}
  
int main(int argc, char *argv[])
{
   init_fs();

   return fuse_main(argc, argv, &hello_oper, NULL);
}
