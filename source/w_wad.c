/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
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
( char*         dest,
  size_t        dest_length,
  const char*   path )
{
	const char* src;
	size_t      length;

	src = path + strlen(path) - 1;

	/* back up until a '\' or the start */
	while (src != path
#ifdef _WIN32
		&& src[-1] != '\\'
#endif
		&& src[-1] != '/')
	{
		--src;
	}

	length = 0;

	while (*src != '\0' && *src != '.')
	{
		if (length++ == dest_length)
			I_Error("Filename base of '%s' is too long", path);

		*dest++ = toupper(*src++);
	}

	memset(dest, '\0', dest_length - length);
}

#define MAKE_U32LE(A, B, C, D) ((unsigned long)(A) << (8 * 0) | (unsigned long)(B) << (8 * 1) | (unsigned long)(C) << (8 * 2) | (unsigned long)(D) << (8 * 3))

static unsigned long ReadU32LE(I_File* handle)
{
	unsigned char bytes[4];
	I_FileRead(handle, bytes, sizeof(bytes));
	return MAKE_U32LE(bytes[0], bytes[1], bytes[2], bytes[3]);
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

	if ((handle = I_FileOpen(filename,I_FILE_MODE_READ)) == NULL)
	{
		I_Info(" couldn't open %s\n",filename);
		return;
	}

	I_Info(" adding %s\n",filename);
	startlump = numlumps;

	storehandle = reloadname ? NULL : handle;

	singlelump = M_strcasecmp(filename + strlen(filename) - 3 , "wad") != 0;

	if (singlelump)
	{
		/* single lump file */
		singleinfo.handle = storehandle;
		singleinfo.position = 0;
		singleinfo.size = I_FileSize(handle);
		ExtractFileBase(singleinfo.name, D_COUNT_OF(singleinfo.name), filename);
		numlumps++;
	}
	else
	{
		/* WAD file */
		const unsigned long identification    = ReadU32LE(handle);
		const unsigned long lumps_in_wad      = ReadU32LE(handle);
		const unsigned long info_table_offset = ReadU32LE(handle);

		I_FileSeek(handle, info_table_offset, I_FILE_POSITION_START);

		switch (identification)
		{
			case MAKE_U32LE('I', 'W', 'A', 'D'):
				/* Main game. */
				break;

			case MAKE_U32LE('P', 'W', 'A', 'D'):
				/* Homebrew levels. */
				/* ???modifiedgame = true; */
				break;

			default:
				I_Error("WAD file %s doesn't have IWAD or PWAD ID", filename);
				break;
		}

		numlumps += lumps_in_wad;
	}


	/* Fill in lumpinfo */
	lumpinfo = (lumpinfo_t*)realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));

	if (lumpinfo == NULL)
		I_Error("Couldn't realloc lumpinfo");

	lump_p = &lumpinfo[startlump];

	if (singlelump)
	{
		*lump_p = singleinfo;

		/* overwrite existing entries so patch lump files take precedence */
		if (!Dictionary_LookUpAndCreateIfNotExist(&lump_dictionary, singleinfo.name, D_COUNT_OF(singleinfo.name), &dictionary_entry))
			I_Error("Could not create lump dictionary entry");

		dictionary_entry->shared.unsigned_long = startlump;
	}
	else
	{
		for (i=startlump; i<numlumps; i++, lump_p++)
		{
			lump_p->handle   = storehandle;
			lump_p->position = ReadU32LE(handle);
			lump_p->size     = ReadU32LE(handle);
			I_FileRead(handle, lump_p->name, sizeof(lump_p->name));

			/* Emulate the behaviour of the original code, which fills everything after the terminator byte with null characters. */
			/* The original code used `strncpy` here, and PWADs such as Hell Revealed rely on its behaviour. */
			{
				size_t j = 0;

				while (j < D_COUNT_OF(lump_p->name))
					if (lump_p->name[j++] == '\0')
						break;

				while (j < D_COUNT_OF(lump_p->name))
					lump_p->name[j++] = '\0';
			}

			/* overwrite existing entries so patch lump files take precedence */
			if (!Dictionary_LookUpAndCreateIfNotExist(&lump_dictionary, lump_p->name, D_COUNT_OF(lump_p->name), &dictionary_entry))
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
	numlumps = ReadU32LE(handle);
	infotableofs = ReadU32LE(handle);
	I_FileSeek (handle, infotableofs, I_FILE_POSITION_START);

	/* Fill in lumpinfo */
	lump_p = &lumpinfo[reloadlump];

	for (i=reloadlump ;
		 i<reloadlump+numlumps ;
		 i++,lump_p++)
	{
		if (lumpcache[i] != NULL)
			Z_Free(lumpcache[i]);

		lump_p->position = ReadU32LE(handle);
		lump_p->size = ReadU32LE(handle);
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
	if (!Dictionary_Init(&lump_dictionary, cc_false))
		I_Error("Couldn't initialise dictionary");

	/* open all the files, load headers, and count lumps */
	numlumps = 0;

	/* will be realloced as lumps are added */
	lumpinfo = NULL;

	while (*filenames != NULL)
		W_AddFile (*filenames++);

	if (numlumps == 0)
		I_Error ("W_InitFiles: no files found");

	/* set up caching */
	lumpcache = (void**)calloc (numlumps, sizeof(*lumpcache));

	if (lumpcache == NULL)
		I_Error ("Couldn't allocate lumpcache");
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

#if 0
/* W_Profile */
void W_Profile (void)
{
	static int             info[2500][10];
	static size_t          profilecount;

	size_t      i;
	memblock_t* block;
	void*       ptr;
	char        ch;
	I_File*     f;
	size_t      j;
	char        name[8];


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

	f = I_FileOpen ("waddump.txt",I_FILE_MODE_WRITE);

	for (i=0 ; i<numlumps ; i++)
	{
		memcpy (name,lumpinfo[i].name,sizeof(name));

		for (j=0 ; j<D_COUNT_OF(name) ; j++)
			if (name[j] == '\0')
				break;

		for ( ; j<D_COUNT_OF(name) ; j++)
			name[j] = ' ';

		I_FileWrite (f,name,sizeof(name));
		I_FilePut (f,' ');

		for (j=0 ; j<profilecount ; j++)
		{
			I_FilePut (f,' ');
			I_FilePut (f,' ');
			I_FilePut (f,' ');
			I_FilePut (f,' ');
			I_FilePut (f,info[i][j]);
		}

		I_FilePut (f,'\n');
	}
	I_FileClose (f);
}
#endif

