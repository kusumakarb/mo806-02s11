/**

   compressed, FUSE implementation
   This is free software, distributed without any warranties.
   Use at your own risk.
   Andre Esteve <andre.esteve@students.ic.unicamp.br>
   Zhenlei Ji <zhenlei.ji@students.ic.unicamp.br>

 **/

#include "compressedfs.h"

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
   return stat(BP, stbuf);
}
  
static int compressionfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{ 
   DIR *dirp;
   struct dirent* dir;
   BPATH(path);

   dirp = opendir(BP);

   if (!dirp)
      return errno;

   errno = 0;

   while ( ( dir = readdir(dirp) ) != NULL )
      filler(buf, dir->d_name, NULL, dir->d_off);

   if (errno != 0)
      return errno;

   return 0;
}
  
static int compressionfs_open(const char *path, struct fuse_file_info *fi)
{
   // things start to get messy here
   // if we open the file on backstore the filesystem userspace process
   // will keep the file open, not the user process (using this fs)
   // there is a limit for open files and keeping too many files open
   // can get really messy
   // what we do is:
   // * verify if user can open the file
   // * keep metadata about file for fast access - this should be returned
   // to fuse and we should keep a reference on some list to be able to
   // free it, if the user process happens to die unexpectedly

   /** TODO **/

   BPATH(path);
   return 0;
}
  
static int compressionfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
   return 0;
}

static int compressionfs_write(const char* path, const char* buf, size_t size, off_t offset,
                       struct fuse_file_info *fi)
{
   return size;
}

static int compressionfs_mknod(const char * path, mode_t mode, dev_t dev)
{
   return 0;
}

static int compressionfs_mkdir(const char * path, mode_t mode)
{
   return 0;
}

static int compressionfs_unlink(const char* path)
{
   return 0;
}

static int compressionfs_rmdir(const char* path)
{
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

   success = 1;

   if (!strcmp(optarg, DEFAULT_COMPRESSION_TYPE))
   {

   }
   else
   {
      success = 0;
      fprintf(stderr, "Compression type %s not available\n", optarg);
   }


   return success;
}

static int parse(int *argc, char **argv[])
{
   int hasbs; //backstore
   int hastype; // algorithm type
   int success;
   int isparam;
   int opt;
   char **newargv;

   cinfo.bs_path = NULL;
   hasbs = hastype = 0;
   success = 1;

   while ( success && (opt = getopt(*argc, *argv, "b:t:") ) != -1) 
   {
      switch (opt) 
      {
         // backstore path
         case 'b':
            hasbs = 1;
            cinfo.bs_len = strlen(optarg);
            cinfo.bs_path = (char*)malloc(sizeof(char)*cinfo.bs_len);
            strcpy(cinfo.bs_path, optarg);
            break;

         case 't':
            hastype = 1;
            success = set_compression_type(optarg);
            break;

         default:
            success = 0;
            break;
      }
   }

   if (!hastype)
   {
      fprintf(stderr, "Using default compression type.\n");
      success = set_compression_type(DEFAULT_COMPRESSION_TYPE);
   }

   if (!hasbs)
   {
      fprintf(stderr, "Must inform back store path.\n");
      success = 0;
   }

   if (success)
   {
      newargv = (char**)malloc(sizeof(char*)**argc);
      isparam = 0;

      // TODO remove parameters

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
   if (parse(&argc, &argv))
       // fuse will parse mount options
       success = fuse_main(argc, argv, &compressionfs_oper, NULL);
   else
      success = -1;
   
   return success;
}
