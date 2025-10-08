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

#include "clownlibs/dictionary.h"





/* GLOBALS */

Dictionary_State        lump_dictionary;

/* Location of each lump on disk. */
lumpinfo_t*             lumpinfo;
size_t                  numlumps;

void**                  lumpcache;


static void
ExtractFileBase
( const char*   path,
  char*         dest )
{
	const char* src;
	int         length;

	src = path + strlen(path) - 1;

	/* back up until a \ or the start */
	while (src != path
		   && *(src-1) != '\\'
		   && *(src-1) != '/')
	{
		src--;
	}

	/* copy up to eight characters */
	memset (dest,'\0',8);
	length = 0;

	while (*src != '\0' && *src != '.')
	{
		if (++length == 9)
			I_Error ("Filename base of %s >8 chars",path);

		*dest++ = toupper(*src++);
	}
}

static unsigned long Read32LE(I_File* handle)
{
	unsigned char bytes[4];
	unsigned long value;
	unsigned int i;

	I_FileRead(handle, bytes, 4);

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

size_t                  reloadlump;
const char*             reloadname;


void W_AddFile (const char *filename)
{
	wadinfo_t           header;
	lumpinfo_t*         lump_p;
	size_t              i;
	I_File*             handle;
	size_t              startlump;
	lumpinfo_t          singleinfo;
	I_File*             storehandle;
	d_bool              singlelump;
	Dictionary_Entry*   dictionary_entry;

	/* open the file and add to directory */

	/* handle reload indicator. */
	if (filename[0] == '~')
	{
		filename++;
		reloadname = filename;
		reloadlump = numlumps;
	}

	if ((handle = I_FileOpen (filename,I_FILE_MODE_READ)) == NULL)
	{
		I_Info(" couldn't open %s\n",filename);
		return;
	}

	I_Info(" adding %s\n",filename);
	startlump = numlumps;

	singlelump = M_strcasecmp(filename+strlen(filename)-3 , "wad" );

	if (singlelump)
	{
		/* single lump file */
		singleinfo.handle = handle;
		singleinfo.position = 0;
		singleinfo.size = I_FileSize(handle);
		ExtractFileBase (filename, singleinfo.name);
		numlumps++;
	}
	else
	{
		/* WAD file */
		I_FileRead(handle, header.identification, sizeof(header.identification));
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
		I_FileSeek (handle, header.infotableofs, I_FILE_POSITION_START);
		numlumps += header.numlumps;
	}


	/* Fill in lumpinfo */
	lumpinfo = (lumpinfo_t*)realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

	if (lumpinfo == NULL)
		I_Error ("Couldn't realloc lumpinfo");

	lump_p = &lumpinfo[startlump];

	storehandle = reloadname ? NULL : handle;

	if (singlelump)
	{
		*lump_p = singleinfo;

		/* overwrite existing entries so patch lump files take precedence */
		if (!Dictionary_LookUpAndCreateIfNotExist(&lump_dictionary, singleinfo.name, 8, &dictionary_entry))
			I_Error("Could not create lump dictionary entry");

		dictionary_entry->shared.unsigned_long = startlump;
	}
	else
	{
		for (i=startlump; i<numlumps; i++, lump_p++)
		{
			lump_p->handle = storehandle;
			lump_p->position = Read32LE(storehandle);
			lump_p->size = Read32LE(storehandle);
			I_FileRead(storehandle, lump_p->name, 8);

			/* overwrite existing entries so patch lump files take precedence */
			if (!Dictionary_LookUpAndCreateIfNotExist(&lump_dictionary, lump_p->name, 8, &dictionary_entry))
				I_Error("Could not create lump dictionary entry");

			dictionary_entry->shared.unsigned_long = i;
		}
	}

	if (reloadname)
		I_FileClose(handle);
}




/* W_Reload */
/* Flushes any of the reloadable lumps in memory */
/*  and reloads the directory. */
void W_Reload (void)
{
	lumpinfo_t*         lump_p;
	size_t              i;
	I_File*             handle;
	size_t              numlumps;
	size_t              infotableofs;

	if (!reloadname)
		return;

	if ((handle = I_FileOpen(reloadname,I_FILE_MODE_READ)) == NULL)
		I_Error ("W_Reload: couldn't open %s",reloadname);

	I_FileSeek(handle, 4, I_FILE_POSITION_CURRENT);
	numlumps = Read32LE(handle);
	infotableofs = Read32LE(handle);
	I_FileSeek (handle, infotableofs, I_FILE_POSITION_START);

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
		I_FileSeek(handle, 8, I_FILE_POSITION_CURRENT);
	}

	I_FileClose(handle);
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
	size_t      size;

	if (!Dictionary_Init(&lump_dictionary, cc_false))
		I_Error("Couldn't initialise dictionary");

	/* open all the files, load headers, and count lumps */
	numlumps = 0;

	/* will be realloced as lumps are added */
	lumpinfo = NULL;

	for ( ; *filenames ; filenames++)
		W_AddFile (*filenames);

	if (!numlumps)
		I_Error ("W_InitFiles: no files found");

	/* set up caching */
	size = numlumps * sizeof(*lumpcache);
	lumpcache = (void**)malloc (size);

	if (!lumpcache)
		I_Error ("Couldn't allocate lumpcache");

	memset (lumpcache,0, size);
}




/* W_InitFile */
/* Just initialize from a single file. */
void W_InitFile (const char* filename)
{
	const char* names[2];

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
	Dictionary_Entry *dictionary_entry;

	strncpy (name_upper,name,8);

	/* in case the name was a full 8 chars */
	name_upper[8] = '\0';

	/* case insensitive */
	M_strupr(name_upper);

	dictionary_entry = Dictionary_LookUp(&lump_dictionary, name_upper, 8);

	if (dictionary_entry != NULL)
		return dictionary_entry->shared.unsigned_long;

	/* TFB. Not found. */
	return -1;
}




/* W_GetNumForName */
/* Calls W_CheckNumForName, but bombs out if not found. */
int W_GetNumForName (const char* name)
{
	int i;

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
		I_Error ("W_LumpLength: %li >= numlumps",(unsigned long)lump);

	return lumpinfo[lump].size;
}



/* W_ReadLump */
/* Loads the lump into the given buffer, */
/*  which must be >= W_LumpLength(). */
void
W_ReadLump
( size_t        lump,
  void*         dest )
{
	size_t      c;
	lumpinfo_t* l;
	I_File*     handle;

	if (lump >= numlumps)
		I_Error ("W_ReadLump: %li >= numlumps",(unsigned long)lump);

	l = lumpinfo+lump;

	/* ??? I_BeginRead (); */

	if (l->handle == NULL)
	{
		/* reloadable file, so use open / read / close */
		if ( (handle = I_FileOpen (reloadname,I_FILE_MODE_READ)) == NULL)
			I_Error ("W_ReadLump: couldn't open %s",reloadname);
	}
	else
		handle = l->handle;

	I_FileSeek (handle, l->position, I_FILE_POSITION_START);
	c = I_FileRead (handle, dest, l->size);

	if (c < l->size)
		I_Error ("W_ReadLump: only read %li of %li on lump %li",
				 (unsigned long)c,(unsigned long)l->size,(unsigned long)lump);

	if (l->handle == NULL)
		I_FileClose (handle);

	/* ??? I_EndRead (); */
}




/* W_CacheLumpNum */
void*
W_CacheLumpNum
( size_t        lump,
  int           tag )
{
	if (lump >= numlumps)
		I_Error ("W_CacheLumpNum: %i >= numlumps",(unsigned int)lump);

	if (!lumpcache[lump])
	{
		/* read the lump in */

/* I_Info ("cache miss on lump %i\n",lump); */
		Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
		W_ReadLump (lump, lumpcache[lump]);
	}
	else
	{
/* I_Info ("cache hit on lump %i\n",lump); */
		Z_ChangeTag (lumpcache[lump],tag);
	}

	return lumpcache[lump];
}



/* W_CacheLumpName */
void*
W_CacheLumpName
( const char*   name,
  int           tag )
{
	return W_CacheLumpNum (W_GetNumForName(name), tag);
}


/* W_Profile */
int             info[2500][10];
size_t          profilecount;

void W_Profile (void)
{
	size_t      i;
	memblock_t* block;
	void*       ptr;
	char        ch;
	FILE*       f;
	size_t      j;
	char        name[9];


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
			block = (memblock_t *) ( (unsigned char *)ptr - sizeof(memblock_t));
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


