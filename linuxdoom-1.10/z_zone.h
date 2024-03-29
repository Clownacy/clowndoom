/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:
        Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
        Remark: this was the only stuff that, according
         to John Carmack, might have been useful for
         Quake.

******************************************************************************/



#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stddef.h>
#include <stdio.h>

/* ZONE MEMORY */
/* PU - purge tags. */
/* Tags < 100 are not overwritten until freed. */
#define PU_STATIC               1       /* static entire execution time */
#define PU_LEVEL                50      /* static until level exited */
/* Tags >= 100 are purgable whenever needed. */
#define PU_PURGELEVEL   100
#define PU_CACHE                101


void    Z_Init (void);
void*   Z_Malloc (size_t size, int tag, void *ptr);
void    Z_Free (void *ptr);
void    Z_FreeTags (int lowtag, int hightag);
void    Z_DumpHeap (int lowtag, int hightag);
void    Z_FileDumpHeap (FILE *f);
void    Z_CheckHeap (void);
void    Z_ChangeTag2 (void *ptr, int tag);
size_t  Z_FreeMemory (void);


typedef struct memblock_s
{
	size_t              size;   /* including the header and possibly tiny fragments */
	void**              user;   /* NULL if a free block */
	int                 tag;    /* purgelevel */
	int                 id;     /* should be ZONEID */
	struct memblock_s*  next;
	struct memblock_s*  prev;
} memblock_t;

#ifdef RANGECHECK
/* This is used to get the local FILE:LINE info from CPP */
/* prior to really call the function in question. */
#define Z_ChangeTag(p,t) \
{ \
	if (( (memblock_t *)( (unsigned char *)(p) - sizeof(memblock_t)))->id!=0x1d4a11) \
		I_Error("Z_CT at "__FILE__":%i",__LINE__); \
	Z_ChangeTag2(p,t); \
};
#else
#define Z_ChangeTag(p,t) Z_ChangeTag2(p,t)
#endif



#endif
