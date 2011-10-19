#include "ramfs.h"

static FILE* flog;

void init_log(const char* path)
{
   flog = fopen(path, "w");
}

void do_log(const char* str)
{
   fwrite(str, strlen(str), 1, flog);
   fwrite("\n", 1, 1, flog);
   fflush(flog);
}
