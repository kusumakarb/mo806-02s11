#include "ramfs.h"

static FILE* flog;

void log_init(const char* path)
{
   flog = fopen(path, "w");
}

void log_do(const char* str)
{
   fwrite(str, strlen(str), 1, flog);
   fwrite("\n", 1, 1, flog);
   fflush(flog);
}

void log_destroy()
{
   if (flog)
      fclose(flog);

   flog = NULL;
}
