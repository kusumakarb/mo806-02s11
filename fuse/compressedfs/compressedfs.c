/**

   compressedfs, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Esteve <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

#include "compressedfs.h"

#define BP bp
#define BPATH(PATH) char BP[DEFAULT_PATH_SIZE]; get_bs_path(PATH, BP);
#define GET_FD(FI) ((struct cfile*)FI->fh)->fd
#define IS_NULL(S) S[0] == '\0'

struct compression_info cinfo;

/**
   Returns 0 if path is not a special fs file
 **/
int special_file(const char *path)
{
   char *c;

   c = strrchr(path, '.');

   return c != NULL && strcmp(c+1, SPECIAL_EXT) == 0;
}

/**
   get corresponding path on backstore. stores it on p
   p should be a buffer with at least DEFAULT_PATH_SIZE bytes
 **/
char* get_bs_path(const char* path, char *p)
{
   if (path == NULL || path[0] != '/' || 
       special_file(path) ||
       cinfo.bs_len + strlen(path) > DEFAULT_PATH_SIZE)
   {
      p[0] = '\0';
      return NULL;
   }

   strcpy(p, cinfo.bs_path);
   strcat(p, path + 1);

   return p;
}

/**
   searches for already open file for path
   returns NULL if it's not open
 **/
struct cfile* get_cfile(char* path)
{
   struct cfile* f;

   pthread_mutex_lock(&cinfo.lock);

   for ( f = cinfo.ftable->next; f != NULL; f = f->next )
      if ( !strcmp(f->path, path) ) break;

   if (f)
   {
      pthread_mutex_lock(&f->lock);      

      f->count++;

      pthread_mutex_unlock(&f->lock);      
   }

   pthread_mutex_unlock(&cinfo.lock);

   return f;
}

struct cfile* new_cfile(int fd, int count, int compressed, int open_flags, 
                        mode_t mode, const char* path)
{
   struct cfile* f;
   struct cfile* n;
   pthread_mutexattr_t mattr;

   f = (struct cfile*)malloc(sizeof(struct cfile));
   f->fd = fd;
   f->count = count;
   f->compressed = compressed;
   f->oflag = open_flags;
   f->mode = mode;
   f->next = NULL;

   strcpy(f->path, path);

   pthread_mutexattr_init(&mattr);
   pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);

   pthread_mutex_init(&f->lock, &mattr);

   pthread_mutex_lock(&cinfo.lock);

   n = cinfo.ftable;

   while ( n->next != NULL )
      n = n->next;

   n->next = f;

   pthread_mutex_unlock(&cinfo.lock);

   return f;
}

void free_cfile(struct cfile* f)
{
   if (!f)
      return;

   pthread_mutex_destroy(&f->lock);

   free(f);
}

/**
   returns 0 on sucess and errno on failure
 **/
int compress(struct cfile* f)
{
   // f->fd is a working uncompressed file at f->path
   char dpath[DEFAULT_PATH_SIZE];
   int fd;
   int r;
   BPATH(f->path);

   pthread_mutex_lock(&f->lock);

   if (!f->compressed)
   {
      // concat special ext to path
      strcpy(dpath, BP);
      strcat(dpath, "."SPECIAL_EXT);

      // open working file for compression
      fd = creat(dpath, 0600);

      if (fd == -1)
         return errno;

      if (cinfo.opt.compress(f->fd, fd) != 0)
      {
         r = errno;

         // compression failled
         // undo work
         close(fd);
         unlink(BP);
      }
      else
      {
         // compression successful
         r = 0;
         f->compressed = 1;

         // remove decompressed file
         close(f->fd);
         unlink(f->path);

         // move compressed file to path
         close(fd);
         rename(dpath, f->path);

         // reopen compressed file
         f->fd = open(f->path, f->oflag, f->mode);

         // critical error
         if (f->fd == -1)
         {
            // by now we just ignore it
            // this should not happen
         }
      }
   }

   pthread_mutex_unlock(&f->lock);

   return r;
}

int decompress(struct cfile* f)
{
   // f->fd is the compressed file at path
   // we have to decompress it and change fd so any operation over fd
   // goes to the decompressed file

   char dpath[DEFAULT_PATH_SIZE];
   int fd;
   int r;
   BPATH(f->path);

   pthread_mutex_lock(&f->lock);

   if (f->compressed)
   {
      // concat special ext to path
      strcpy(dpath, BP);
      strcat(dpath, "."SPECIAL_EXT);

      // open working file for decompression
      fd = creat(dpath, 0600);

      if (fd == -1)
         return errno;

      if (cinfo.opt.decompress(f->fd, fd) != 0)
      {
         r = errno;

         // decompression failled
         // undo work
         close(fd);
         unlink(BP);
      }
      else
      {
         // decompression successful
         r = 0;
         f->compressed = 0;

         // remove compressed file
         close(f->fd);
         unlink(f->path);

         // move decompressed file to path
         close(fd);
         rename(dpath, f->path);

         // reopen decompressed file
         f->fd = open(f->path, f->oflag, f->mode);

         // critical error
         if (f->fd == -1)
         {
            // by now we just ignore it
            // this should not happen
         }
      }
   }

   pthread_mutex_unlock(&f->lock);

   return r;
}

/*
  ===============================
  FUSE OPERATIONS IMPLEMENTATION
  ===============================
 */

static int compressionfs_getattr(const char *path, struct stat *stbuf)
{
   BPATH(path); 

   if (!stat(BP, stbuf))
      return 0;
   else
      return -errno;
}
  
static int compressionfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{ 
   DIR *dirp;
   struct dirent* dir;
   BPATH(path);

   dirp = opendir(BP);

   if (!dirp)
      return -errno;

   errno = 0;

   while ( ( dir = readdir(dirp) ) != NULL )
   {
      /* only list files that are not special */
      if (!special_file(dir->d_name))
         filler(buf, dir->d_name, NULL, 0);
   }

   if (closedir(dirp) == -1)
      return -errno;

   if (errno != 0)
      return -errno;

   return 0;
}
  
static int compressionfs_open(const char *path, struct fuse_file_info *fi)
{
   int fd;
   struct cfile* f;
   mode_t mode;
   int r;
   BPATH(path);

   if (IS_NULL(BP))
      return -EACCES;

   r = 0;
   mode = 0640;

   // lock required, the same file may be opened at the same time
   pthread_mutex_lock(&cinfo.open_lock);

   // veifiry if it's openned already
   f = get_cfile(BP);

   // if it's null, then it's not openned yet   
   if (!f)
   {
      // open file on backstore
      fd = open(BP, fi->flags, mode);

      if (fd == -1)
         r = -errno;
      else
         f = new_cfile(fd, 1, 1, fi->flags, mode, path);
   }

   pthread_mutex_unlock(&cinfo.open_lock);

   // file's open, but compressed
   if (r == 0 && f->compressed)
   {
      if (decompress(f))
      {
         // decompression failled
         close(f->fd);
         free_cfile(f);
         
         // what the heck...
         r = -EIO;
      }
   }

   // on success, keeps file pointer on fuse special structure
   if (r == 0)
      fi->fh = (uint64_t)f;

   return r;
}

static int compressionfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
   int fd;
   struct cfile* f;
   int flags;
   BPATH(path);

   flags = O_CREAT | O_WRONLY;

   // open file on backstore
   // no lock needed, we are safe because open won't create a file twice
   fd = open(BP, flags, mode);

   if (fd == -1)
      return -errno;

   // new file was created

   f = new_cfile(fd, 1, 0, flags, mode, path);

   // and keeps it on fuse special structure
   fi->fh = (uint64_t)f;

   return 0;
}
 
static int compressionfs_release(const char *path, struct fuse_file_info *fi)
{
   struct cfile* f;
   struct cfile* n;
   int count;
   int r;

   f = (struct cfile*)fi->fh;
   r = 0;

   pthread_mutex_lock(&f->lock);

   count = --f->count;

   if (count == 0)
   {
      // ok, we are the last one using this file
      // lets compress it and close it

      // compress file on disk
      if (compress(f) != 0)
      {
         // compression failled        
         // what the heck...
         r = -EIO;
      }

      // close file
      if (r == 0 && close(f->fd) == -1)
         r = -errno;

      // can't do much more

      pthread_mutex_lock(&cinfo.lock);
      // remove f from ftable
      n = cinfo.ftable;

      while ( n->next != f )
         n = n->next;

      n->next = n->next->next;
      // finished ftable removal
      pthread_mutex_unlock(&cinfo.lock);
   }

   pthread_mutex_unlock(&f->lock);

   if (count == 0)
      free_cfile(f);
      
   return r;
}
 
static int compressionfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
   int n;

   n = pread(GET_FD(fi), buf, size, offset);

   if (n == -1)
      return -errno;

   return n;
}

static int compressionfs_write(const char* path, const char* buf, size_t size, off_t offset,
                       struct fuse_file_info *fi)
{
   int n;

   n = pwrite(GET_FD(fi), buf, size, offset);

   if (n == -1)
      return -errno;
   
   return n;
}

static int compressionfs_mknod(const char * path, mode_t mode, dev_t dev)
{
   BPATH(path);

   if (mknod(BP, mode, dev) == -1)
      return -errno;

   return 0;
}

static int compressionfs_mkdir(const char * path, mode_t mode)
{
   BPATH(path);

   if (mkdir(BP, mode) == -1)
      return -errno;

   return 0;
}

static int compressionfs_unlink(const char* path)
{
   BPATH(path);

   if (unlink(BP) == -1)
      return -errno;

   return 0;
}

static int compressionfs_rmdir(const char* path)
{
   BPATH(path);

   if (rmdir(BP) == -1)
      return -errno;

   return 0;
}

static int compressionfs_setxattr(const char* path, const char* name, const char* value,
                                  size_t size, int flags)
{
   int r;
/*
   BPATH(path);

   r = setxattr(BP, name, value, size, flags);

   if (!r)
      r = -errno;
*/

   r = -ENOSYS;

   return r;
}

static int compressionfs_utime(const char* path, struct utimbuf *tv)
{
   int r;

   BPATH(path);

   r = utime(BP, tv);

   if (r != 0)
      r = -errno;

   return r;
}

static int compressionfs_truncate(const char* path, off_t offset)
{
   int r;

   BPATH(path);

   r = truncate(BP, offset);

   if (r != 0)
      r = -errno;

   return r;
}

static int compressionfs_chmod(const char* path, mode_t mode)
{
   int r;

   BPATH(path);

   r = chmod(BP, mode);

   if (r != 0)
      r = -errno;

   return r;
}

static void* compressionfs_init(struct fuse_conn_info *conn)
{
   return NULL;
}

static void compressionfs_destroy(void* v)
{
   if (cinfo.bs_path)
      free(cinfo.bs_path);

   pthread_mutex_destroy(&cinfo.lock);
   pthread_mutex_destroy(&cinfo.open_lock);
}

  
// function mapping
static struct fuse_operations compressionfs_oper = {
   .init = compressionfs_init,
   .destroy = compressionfs_destroy,
   .getattr   = compressionfs_getattr,
   .readdir = compressionfs_readdir,
   .open   = compressionfs_open,
   .create = compressionfs_create,
   .release = compressionfs_release,
   .read   = compressionfs_read,
   .write = compressionfs_write,
   .mknod = compressionfs_mknod,
   .mkdir = compressionfs_mkdir,
   .unlink = compressionfs_unlink,
   .rmdir = compressionfs_rmdir,
   .setxattr = compressionfs_setxattr,
   .utime = compressionfs_utime,
   .truncate = compressionfs_truncate,
   .chmod = compressionfs_chmod,
};

static void print_usage()
{
   puts("compressedfs -b backstore_path [-t compression_type] [fuse_args] mount_path");
}

static int set_compression_type(char *optarg)
{
   int success;
   struct compression_operations* opt;

   success = 1;
   opt = &cinfo.opt;

   if (!strcmp(optarg, DUMMY_NAME))
   {
      dummy_init(opt);
   }
   else
   {
      success = 0;
      fprintf(stderr, "Compression type %s not available\n", optarg);
   }


   return success;
}

static int parse(int *argc, char *argv[])
{
   int hasbs; //backstore
   int hastype; // algorithm type
   int success;
   int opt;
   char* ag;
   char **newargv;
   int newargc;
   int skipnext;
   char pwd[256];
   pthread_mutexattr_t mattr;

   cinfo.bs_path = NULL;
   hasbs = hastype = 0;
   success = 1;

   newargv = (char**)malloc(sizeof(char*)**argc);
   newargc = 1;

   // disable getopt errors
   opterr = 0;

   // getopt changes argv order, let's save it
   for (opt = 0; opt < *argc; opt++)
      newargv[opt] = argv[opt];

   while ( success && (opt = getopt(*argc, argv, "b:t:") ) != -1) 
   {
      switch (opt) 
      {
         // backstore path
         case 'b':
            hasbs = 1;

            if (optarg[0] == '/')
            {
               pwd[0] = '\0';
            }
            else
            {
               getcwd(pwd, 256);
               strcat(pwd, "/");
            }

            cinfo.bs_len = strlen(optarg) + strlen(pwd);
            cinfo.bs_path = (char*)malloc(sizeof(char)*cinfo.bs_len);
            strcpy(cinfo.bs_path, pwd);
            strcat(cinfo.bs_path, optarg);

            break;

         case 't':
            hastype = 1;
            success = set_compression_type(optarg);
            break;

         default:
            // maybe fuse parameters
            break;
      }
   }

   if (!hastype)
   {
      fprintf(stderr, "Using default compression type.\n");
      success = set_compression_type(DUMMY_NAME);
   }

   if (!hasbs)
   {
      fprintf(stderr, "Must inform back store path.\n");
      success = 0;
   }

   if (success)
   {

      // getopt has changed argv order, let's restore it
      for (opt = 0; opt < *argc; opt++)
         argv[opt] = newargv[opt];

      skipnext = 0;

      // getopt is for shot options only, let's copy all other args
      for (opt = 1; opt < *argc; opt++)
      {
         if (skipnext)
         {
            skipnext = 0;
            continue;
         }

         ag = argv[opt];

         // choose whether or not to skip this arg
         switch (strlen(ag))
         {
            case 0:
               continue;
            case 1:
               if (ag[0] == '-')
                  continue;
               break;
            default: // >= 2
               if (ag[0] == '-' && ( ag[1] == 'b' || ag[1] == 't') )
               {
                  skipnext = 1;
                  continue;                  
               }
               break;
         }

         // copy
         newargv[newargc++] = argv[opt];
      }
      
      // copy back to the pointer
      *argc = newargc;
      for (opt = 0; opt < newargc; opt++)
         argv[opt] = newargv[opt];

      free(newargv);
    
      // init recursive lock
      pthread_mutexattr_init(&mattr);
      pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE_NP);

      pthread_mutex_init(&cinfo.lock, &mattr);
      pthread_mutex_init(&cinfo.open_lock, &mattr);

      pthread_mutexattr_destroy(&mattr);

      cinfo.ftable = (struct cfile*)malloc(sizeof(struct cfile));
      cinfo.ftable->next = NULL;
   }
   else
   {
      // undo what is needed

      if (cinfo.bs_path)
         free(cinfo.bs_path);

      print_usage();
   }

   return success;
}
  
int main(int argc, char *argv[])
{
   int success;

   // parse own arguments
   if (parse(&argc, argv))
   {
      puts(cinfo.bs_path);

       // fuse will parse mount options
       success = fuse_main(argc, argv, &compressionfs_oper, NULL);
   }
   else
      success = -1;
   
   return success;
}
