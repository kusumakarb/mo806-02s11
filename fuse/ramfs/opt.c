#include "ramfs.h"

static dentry_t* droot;

int ilink(dentry_t* d, inode_t* in)
{  
   dentry_t* dn;

   if (!d || d->in || !in)
      return -1;

    in->nlink++;

    if ( !in->dentries )
       in->dentries = d;
    else
    {
       for ( dn = in->dentries; dn->dinode; dn = dn->dinode )
          ;

       dn->dinode = d;
    }

    d->in = in;

    return 0;
}

int iunlink(dentry_t* d, inode_t* in)
{
   dentry_t* dn;

   if (!d || !in || d->in != in || in->nlink == 0)
      return -1;

   if ( in->dentries == d )
   {
      in->dentries = d->dinode;
   }
   else
   {
      for ( dn = in->dentries; dn->dinode && dn->dinode != d; dn = dn->dinode )
         ;

      if (dn->in == NULL)
         return -1;

      dn->dinode = d->dinode;
   }
   
   d->in = NULL;
   free_dentry(d);
   in->nlink--;

   if (!in->nlink)
      free_inode(in);

   return 0;
}

inode_t* alloc_inode(mode_t mode)
{
   inode_t* in;

   in = (inode_t*)malloc(sizeof(inode_t));

   if (in)
   {
      in->data = NULL;
      in->len = 0;
      in->nlink = 0;
      in->mode = mode;
      in->dentries = NULL;
   }

   return in;
}

int free_inode(inode_t* in)
{
   if (in)
   {
      if (in->nlink)
         return -1;

      if (in->data)
         free(in->data);

      free(in);
   }

   return 0;
}

dentry_t* alloc_dentry(char* name, inode_t* in)
{
   dentry_t* d;
   int nlen;

   d = (dentry_t*)malloc(sizeof(dentry_t));

   if (d)
   {
      d->in = NULL;
      d->dnext = NULL;
      d->dchild = NULL;
      d->dinode = NULL;

      if (!ilink(d, in))
      {
         nlen = strlen(name);
         d->name = (char*)malloc(sizeof(char)*nlen);
         strcpy(d->name, name);
      }
   }

   return d;
}

int free_dentry(dentry_t* d)
{
   if (d)
   {
      if (d->dnext || d->dchild || d->dinode)
         return -1;

      if (d->name)
         free(d->name);

      free(d);
   }

   return 0;
}

dentry_t* get_dentry(dentry_t* dparent, const char* name)
{
   dentry_t* d;

   for (d = dparent->dchild; d != NULL; d = d->dnext)
      if (!strcmp(d->name, name))
          break;

   return d;
}

static dentry_t* __get_path(char* path)
{
   dentry_t* d;
   char* t;

   d = droot;

   if (!path[1])
      return d;

   t = strtok(path, "/");

   if ( (d = get_dentry(d, t)) == NULL )
      return NULL;

   while ( (t = strtok(NULL, "/")) )
   {
      if ( (d = get_dentry(d, t)) == NULL )
         break;
   }

   return d;
}

dentry_t* get_parent(const char* path)
{
   char p[MAX_PATH_SIZE];
   char* l;
   size_t len;

   l = rindex(path, '/');
   len = (size_t)l - (size_t)path;

   memcpy(p, path, len);
   p[len] = '\0';
   
   return __get_path(p);
}

dentry_t* get_path(const char* path)
{
   char p[MAX_PATH_SIZE];
  
   strncpy(p, path, MAX_PATH_SIZE);

   return __get_path(p);
}

void get_filename(const char* path, char* name)
{
   char p[MAX_PATH_SIZE];
   char *t;
   char* prev;

   strncpy(p, path, MAX_PATH_SIZE);

   t = strtok(p, "/");

   do
      prev = t; 
   while ( ( t = strtok(NULL, "/") ) );
   
   strcpy(name, prev);

   return;
}

int d_addchild(dentry_t* dparent, dentry_t* child)
{
   dentry_t* d;

   if (!dparent || !child)
      return -1;

   if (!dparent->dchild)
      dparent->dchild = child;
   else
   {
      for( d = dparent->dchild; d->dnext; d = d->dnext )
         ;
      
      d->dnext = child;
   } 
   
   return 0;
}

void ramfs_opt_init()
{
   droot = alloc_dentry("", alloc_inode(S_IFDIR | 0755));

   droot->dchild = alloc_dentry("andre", alloc_inode(S_IFREG | 0755));
   droot->dchild->in->data = malloc(4);
   memcpy(droot->dchild->in->data, "hehe", 4);
   droot->dchild->in->len = 4;
}
