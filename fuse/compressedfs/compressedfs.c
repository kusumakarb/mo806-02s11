/**

   compressed, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Esteve <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

#include "compressedfs.h"
#include "cps_dummy.h"


#define BP bp
#define BPATH(PATH) char BP[DEFAULT_PATH_SIZE]; get_bs_path(PATH, BP);

struct compression_info cinfo;

/**
   get corresponding path on backstore. stores it on p
   p should be a buffer with at least DEFAULT_PATH_SIZE bytes
 **/
char* get_bs_path(const char* path, char *p)
{
   if (path == NULL || path[0] != '/' || 
       cinfo.bs_len + strlen(path) > DEFAULT_PATH_SIZE)
      return NULL;

   strcpy(p, cinfo.bs_path);
   strcat(p, path + 1);

   return p;
}

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
      filler(buf, dir->d_name, NULL, dir->d_off);

   if (errno != 0)
      return -errno;

   return 0;
}
  
static int compressionfs_open(const char *path, struct fuse_file_info *fi)
{
   int fd;
   BPATH(path);

   // open file on backstore
   fd = open(BP, fi->flags, 0640);

   if (fd == -1)
      return -errno;

   // and keeps it on fuse special structure
   fi->fh = fd;

   return 0;
}

static int compressionfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
   int fd;
   BPATH(path);

   // open file on backstore
   fd = open(BP, fi->flags, mode);

   if (fd == -1)
      return -errno;

   // and keeps it on fuse special structure
   fi->fh = fd;

   return 0;
}
 
static int compressionfs_release(const char *path, struct fuse_file_info *fi)
{
   if (close(fi->fh))
      return 0;

   return -errno;
}
 
static int compressionfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
   int n;

   n = pread(fi->fh, buf, size, offset);

   if (n == -1)
      return -errno;

   return n;
}

static int compressionfs_write(const char* path, const char* buf, size_t size, off_t offset,
                       struct fuse_file_info *fi)
{
   int n;

   n = pwrite(fi->fh, buf, size, offset);

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

static void* compressionfs_init(struct fuse_conn_info *conn)
{
   return NULL;
}

static void compressionfs_destroy(void* v)
{
   if (cinfo.bs_path)
      free(cinfo.bs_path);
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
               strcpy(pwd + strlen(pwd), "/");
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
