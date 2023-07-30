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
  	WAD I/O functions.

******************************************************************************/


#ifndef __W_WAD__
#define __W_WAD__


#include <stddef.h>
#include <stdio.h>

/* TYPES */
typedef struct
{
    /* Should be "IWAD" or "PWAD". */
    char		identification[4];
    size_t		numlumps;
    size_t		infotableofs;

} wadinfo_t;


/* WADFILE I/O related stuff. */
typedef struct
{
    char	name[8];
    FILE*	handle;
    size_t	position;
    size_t	size;
} lumpinfo_t;


extern	void**		lumpcache;
extern	lumpinfo_t*	lumpinfo;
extern	size_t		numlumps;

void    W_InitMultipleFiles (const char** filenames);
void    W_Reload (void);

int	W_CheckNumForName (const char* name);
int	W_GetNumForName (const char* name);

int	W_LumpLength (size_t lump);
void    W_ReadLump (size_t lump, void *dest);

void*	W_CacheLumpNum (size_t lump, int tag);
void*	W_CacheLumpName (const char* name, int tag);




#endif
