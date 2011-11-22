/************************************************************************
 * SCZ_CORE.c - Core routines for SCZ compression methods.		*
 ************************************************************************/

#ifndef SCZ_CORE
#define SCZ_CORE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SCZ_MAX_BUF  16777215
#define SCZFREELSTSZ 2048
#define nil 0

struct scz_item		/* Data structure for holding buffer items. */
 {
   unsigned char ch;
   struct scz_item *nxt;
 };

struct scz_amalgam	/* Data structure for holding markers and phrases. */
 {
   unsigned char phrase[2];
   int value;
 };

struct scz_item2		/* Data structure for holding buffer items with index. */
 {
   unsigned char ch;
   int j;
   struct scz_item2 *nxt;
};

struct scz_block_alloc		/* List for holding references to blocks of allocated memory. */
{
 void *mem_block;
 struct scz_block_alloc *next_block;
};




struct scz_item *new_scz_item();
void sczfree( struct scz_item *tmppt );
struct scz_item2 *new_scz_item2();
void sczfree2( struct scz_item2 *tmppt );
void scz_cleanup();
void scz_add_item( struct scz_item **hd, struct scz_item **tl, unsigned char ch );

#endif
