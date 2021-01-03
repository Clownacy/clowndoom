// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//
//-----------------------------------------------------------------------------


#ifndef __W_WAD__
#define __W_WAD__


#ifdef __GNUG__
#pragma interface
#endif

#include <stddef.h>
#include <stdio.h>

//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];		
    size_t		numlumps;
    size_t		infotableofs;
    
} wadinfo_t;


//
// WADFILE I/O related stuff.
//
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
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
