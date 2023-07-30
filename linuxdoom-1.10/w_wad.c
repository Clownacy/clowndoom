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
  	Handles WAD file header, directory, lump I/O.
  
******************************************************************************/

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "m_misc.h"

#include "w_wad.h"






/* GLOBALS */

/* Location of each lump on disk. */
lumpinfo_t*		lumpinfo;		
size_t			numlumps;

void**			lumpcache;


/* This was originally called 'strupr', but it conflicted with the standard library so I renamed it. */
static void MakeStringUppercase (char* s)
{
    while (*s) { *s = toupper(*s); s++; }
}

static int filelength (FILE* handle) 
{ 
    long previous_position, length;

    previous_position = ftell(handle);
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);
    fseek(handle, previous_position, SEEK_SET);

    return length;
}


static void
ExtractFileBase
( const char*	path,
  char*		dest )
{
    const char*	src;
    int		length;

    src = path + strlen(path) - 1;
    
    /* back up until a \ or the start */
    while (src != path
	   && *(src-1) != '\\'
	   && *(src-1) != '/')
    {
	src--;
    }
    
    /* copy up to eight characters */
    memset (dest,0,8);
    length = 0;
    
    while (*src && *src != '.')
    {
	if (++length == 9)
	    I_Error ("Filename base of %s >8 chars",path);

	*dest++ = toupper((int)*src++);
    }
}

static unsigned long Read32LE(FILE* handle)
{
    unsigned char bytes[4];
    unsigned long value;
    unsigned int i;

    fread(bytes, 1, 4, handle);

    value = 0;

    for (i = 0; i < 4; ++i)
        value |= (unsigned long)bytes[i] << (8 * i);

    return value;
}





/* LUMP BASED ROUTINES. */

/* W_AddFile */
/* All files are optional, but at least one file must be */
/*  found (PWAD, if all required lumps are present). */
/* Files with a .wad extension are wadlink files */
/*  with multiple lumps. */
/* Other files are single lumps with the base filename */
/*  for the lump name. */
/* If filename starts with a tilde, the file is handled */
/*  specially to allow map reloads. */
/* But: the reload feature is a fragile hack... */

size_t			reloadlump;
const char*		reloadname;


void W_AddFile (const char *filename)
{
    wadinfo_t		header;
    lumpinfo_t*		lump_p;
    size_t		i;
    FILE*		handle;
    size_t		startlump;
    lumpinfo_t		singleinfo;
    FILE*		storehandle;
    boolean		singlelump;
    
    /* open the file and add to directory */

    /* handle reload indicator. */
    if (filename[0] == '~')
    {
	filename++;
	reloadname = filename;
	reloadlump = numlumps;
    }
		
    if ((handle = fopen (filename,"rb")) == NULL)
    {
	printf(" couldn't open %s\n",filename);
	return;
    }

    printf(" adding %s\n",filename);
    startlump = numlumps;
	
    singlelump = M_strcasecmp(filename+strlen(filename)-3 , "wad" );

    if (singlelump)
    {
	/* single lump file */
	singleinfo.handle = handle;
	singleinfo.position = 0;
	singleinfo.size = filelength(handle);
	ExtractFileBase (filename, singleinfo.name);
	numlumps++;
    }
    else 
    {
	/* WAD file */
	fread(header.identification, 1, sizeof(header.identification), handle);
	header.numlumps = Read32LE(handle);
	header.infotableofs = Read32LE(handle);

	if (memcmp(header.identification,"IWAD",4))
	{
	    /* Homebrew levels? */
	    if (memcmp(header.identification,"PWAD",4))
	    {
		I_Error ("Wad file %s doesn't have IWAD "
			 "or PWAD id\n", filename);
	    }
	    
	    /* ???modifiedgame = true; */
	}
	fseek (handle, header.infotableofs, SEEK_SET);
	numlumps += header.numlumps;
    }

    
    /* Fill in lumpinfo */
    lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

    if (lumpinfo == NULL)
	I_Error ("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];
	
    storehandle = reloadname ? NULL : handle;
	
    if (singlelump)
    {
	*lump_p = singleinfo;
    }
    else
    {
	for (i=startlump; i<numlumps; i++, lump_p++)
	{
	    lump_p->handle = storehandle;
	    lump_p->position = Read32LE(storehandle);
	    lump_p->size = Read32LE(storehandle);
	    fread(lump_p->name, 1, 8, storehandle);
	}
    }
	
    if (reloadname)
	fclose(handle);
}




/* W_Reload */
/* Flushes any of the reloadable lumps in memory */
/*  and reloads the directory. */
void W_Reload (void)
{
    lumpinfo_t*		lump_p;
    size_t		i;
    FILE*		handle;
    size_t		numlumps;
    size_t		infotableofs;
	
    if (!reloadname)
	return;
		
    if ((handle = fopen(reloadname,"rb")) == NULL)
	I_Error ("W_Reload: couldn't open %s",reloadname);

    fseek(handle, 4, SEEK_CUR);
    numlumps = Read32LE(handle);
    infotableofs = Read32LE(handle);
    fseek (handle, infotableofs, SEEK_SET);
    
    /* Fill in lumpinfo */
    lump_p = &lumpinfo[reloadlump];
	
    for (i=reloadlump ;
	 i<reloadlump+numlumps ;
	 i++,lump_p++)
    {
	if (lumpcache[i] != NULL)
	    Z_Free(lumpcache[i]);

	lump_p->position = Read32LE(handle);
	lump_p->size = Read32LE(handle);
	fseek(handle, 8, SEEK_CUR);
    }
	
    fclose(handle);
}



/* W_InitMultipleFiles */
/* Pass a null terminated list of files to use. */
/* All files are optional, but at least one file */
/*  must be found. */
/* Files with a .wad extension are idlink files */
/*  with multiple lumps. */
/* Other files are single lumps with the base filename */
/*  for the lump name. */
/* Lump names can appear multiple times. */
/* The name searcher looks backwards, so a later file */
/*  does override all earlier ones. */
void W_InitMultipleFiles (const char** filenames)
{	
    size_t	size;
    
    /* open all the files, load headers, and count lumps */
    numlumps = 0;

    /* will be realloced as lumps are added */
    lumpinfo = malloc(1);	

    for ( ; *filenames ; filenames++)
	W_AddFile (*filenames);

    if (!numlumps)
	I_Error ("W_InitFiles: no files found");
    
    /* set up caching */
    size = numlumps * sizeof(*lumpcache);
    lumpcache = malloc (size);
    
    if (!lumpcache)
	I_Error ("Couldn't allocate lumpcache");

    memset (lumpcache,0, size);
}




/* W_InitFile */
/* Just initialize from a single file. */
void W_InitFile (const char* filename)
{
    const char*	names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles (names);
}



/* W_NumLumps */
int W_NumLumps (void)
{
    return numlumps;
}



/* W_CheckNumForName */
/* Returns -1 if name not found. */

int W_CheckNumForName (const char* name)
{
    char name_upper[9];
    
    lumpinfo_t*	lump_p;

    strncpy (name_upper,name,8);

    /* in case the name was a full 8 chars */
    name_upper[8] = '\0';

    /* case insensitive */
    MakeStringUppercase(name_upper);

    /* scan backwards so patch lump files take precedence */
    lump_p = lumpinfo + numlumps;

    while (lump_p-- != lumpinfo)
	if (!memcmp(lump_p->name, name_upper, 8))
	    return lump_p - lumpinfo;

    /* TFB. Not found. */
    return -1;
}




/* W_GetNumForName */
/* Calls W_CheckNumForName, but bombs out if not found. */
int W_GetNumForName (const char* name)
{
    int	i;

    i = W_CheckNumForName (name);
    
    if (i == -1)
      I_Error ("W_GetNumForName: %s not found!", name);
      
    return i;
}


/* W_LumpLength */
/* Returns the buffer size needed to load the given lump. */
int W_LumpLength (size_t lump)
{
    if (lump >= numlumps)
	I_Error ("W_LumpLength: %i >= numlumps",lump);

    return lumpinfo[lump].size;
}



/* W_ReadLump */
/* Loads the lump into the given buffer, */
/*  which must be >= W_LumpLength(). */
void
W_ReadLump
( size_t	lump,
  void*		dest )
{
    size_t	c;
    lumpinfo_t*	l;
    FILE*	handle;
	
    if (lump >= numlumps)
	I_Error ("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo+lump;
	
    /* ??? I_BeginRead (); */
	
    if (l->handle == NULL)
    {
	/* reloadable file, so use open / read / close */
	if ( (handle = fopen (reloadname,"rb")) == NULL)
	    I_Error ("W_ReadLump: couldn't open %s",reloadname);
    }
    else
	handle = l->handle;
		
    fseek (handle, l->position, SEEK_SET);
    c = fread (dest, 1, l->size, handle);

    if (c < l->size)
	I_Error ("W_ReadLump: only read %i of %i on lump %i",
		 c,l->size,lump);	

    if (l->handle == NULL)
	fclose (handle);
		
    /* ??? I_EndRead (); */
}




/* W_CacheLumpNum */
void*
W_CacheLumpNum
( size_t	lump,
  int		tag )
{
    if (lump >= numlumps)
	I_Error ("W_CacheLumpNum: %i >= numlumps",(unsigned int)lump);
		
    if (!lumpcache[lump])
    {
	/* read the lump in */
	
/* printf ("cache miss on lump %i\n",lump); */
	Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
	W_ReadLump (lump, lumpcache[lump]);
    }
    else
    {
/* printf ("cache hit on lump %i\n",lump); */
	Z_ChangeTag (lumpcache[lump],tag);
    }
	
    return lumpcache[lump];
}



/* W_CacheLumpName */
void*
W_CacheLumpName
( const char*	name,
  int		tag )
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}


/* W_Profile */
int		info[2500][10];
size_t		profilecount;

void W_Profile (void)
{
    size_t	i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    size_t	j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpcache[i];
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
	memcpy (name,lumpinfo[i].name,8);

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}


