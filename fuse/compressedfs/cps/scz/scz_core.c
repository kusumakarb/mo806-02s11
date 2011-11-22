/************************************************************************
 * SCZ_CORE.c - Core routines for SCZ compression methods.		*
 ************************************************************************/

#include "scz_core.h"

struct scz_item *sczfreelist1 = 0;

struct scz_item2 *sczphrasefreq[256], *scztmpphrasefreq, *sczfreelist2=0;

struct scz_block_alloc *scz_allocated_blocks = 0, *scz_tmpblockalloc;

int sczbuflen = 4 * 1048576;


struct scz_item *new_scz_item()
{
 int j;
 struct scz_item *tmppt;

 if (sczfreelist1==nil)
  {
   sczfreelist1 = (struct scz_item *)malloc(SCZFREELSTSZ * sizeof(struct scz_item));
   tmppt = sczfreelist1;
   for (j=SCZFREELSTSZ-1; j!=0; j--) 
    {
     tmppt->nxt = tmppt + 1;	/* Pointer arithmetic. */
     tmppt = tmppt->nxt;
    }
   tmppt->nxt = nil;
   /* Record the block allocation for eventual freeing. */
   scz_tmpblockalloc = (struct scz_block_alloc *)malloc(sizeof(struct scz_block_alloc));
   scz_tmpblockalloc->mem_block = sczfreelist1;
   scz_tmpblockalloc->next_block = scz_allocated_blocks;
   scz_allocated_blocks = scz_tmpblockalloc;
  }
 tmppt = sczfreelist1;
 sczfreelist1 = sczfreelist1->nxt;
 return tmppt;
}

void sczfree( struct scz_item *tmppt )
{
 tmppt->nxt = sczfreelist1;
 sczfreelist1 = tmppt;
}


struct scz_item2 *new_scz_item2()
{
 int j;
 struct scz_item2 *tmppt;

 if (sczfreelist2==nil)
  {
   sczfreelist2 = (struct scz_item2 *)malloc(SCZFREELSTSZ * sizeof(struct scz_item2));
   tmppt = sczfreelist2;
   for (j=SCZFREELSTSZ-1; j!=0; j--) 
    {
     tmppt->nxt = tmppt + 1;	/* Pointer arithmetic. */
     tmppt = tmppt->nxt;
    }
   tmppt->nxt = nil;
   /* Record the block allocation for eventual freeing. */
   scz_tmpblockalloc = (struct scz_block_alloc *)malloc(sizeof(struct scz_block_alloc));
   scz_tmpblockalloc->mem_block = sczfreelist2;
   scz_tmpblockalloc->next_block = scz_allocated_blocks;
   scz_allocated_blocks = scz_tmpblockalloc;
  }
 tmppt = sczfreelist2;
 sczfreelist2 = sczfreelist2->nxt;
 return tmppt;
}

void sczfree2( struct scz_item2 *tmppt )
{
 tmppt->nxt = sczfreelist2;
 sczfreelist2 = tmppt;
}


void scz_cleanup()	/* Call after last SCZ call to free temporarily allocated */
{			/*  memory which will all be on the free-lists now. */
 while (scz_allocated_blocks!=0)
  {
   scz_tmpblockalloc = scz_allocated_blocks;
   scz_allocated_blocks = scz_allocated_blocks->next_block;
   free(scz_tmpblockalloc->mem_block);
   free(scz_tmpblockalloc);
  }
 sczfreelist1 = nil;
 sczfreelist2 = nil;
}


/*-----------------------*/
/* Add item to a buffer. */
/*-----------------------*/
void scz_add_item( struct scz_item **hd, struct scz_item **tl, unsigned char ch )
{
 struct scz_item *tmppt;

 tmppt = new_scz_item();
 tmppt->ch = ch;
 tmppt->nxt = 0;
 if (*hd==0) *hd = tmppt; else (*tl)->nxt = tmppt;
 *tl = tmppt;
}
