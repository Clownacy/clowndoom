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

   Revision 1.3  1997/01/29 20:10
   DESCRIPTION:
        Preparation of data for rendering,
        generation of lookups, caching, retrieval by name.

******************************************************************************/


#include "i_system.h"
#include "z_zone.h"

#include "m_swap.h"

#include "w_wad.h"

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"
#include "m_misc.h"

#include <string.h>


#include "r_data.h"

/* Graphics. */
/* DOOM graphics for walls and sprites */
/* is stored in vertical runs of opaque pixels (posts). */
/* A column is composed of zero or more posts, */
/* a patch or sprite is composed of zero or more columns. */



/* Texture definition. */
/* Each texture is composed of one or more patches, */
/* with patches being lumps stored in the WAD. */
/* The lumps are referenced by number, and patched */
/* into the rectangular texture space using origin */
/* and possibly other attributes. */
enum
{
	MAPPATCH_ORIGIN_X  = 0,
	MAPPATCH_ORIGIN_Y  = 2,
	MAPPATCH_PATCH     = 4,
	MAPPATCH_STEP_DIR  = 6,
	MAPPATCH_COLOR_MAP = 8,
	MAPPATCH_SIZEOF    = 10
};


/* Texture definition. */
/* A DOOM wall texture is a list of patches */
/* which are to be combined in a predefined order. */
enum
{
	MAPTEXTURE_NAME             = 0,
	MAPTEXTURE_MASKED           = 8,
	MAPTEXTURE_WIDTH            = 12,
	MAPTEXTURE_HEIGHT           = 14,
	MAPTEXTURE_COLUMN_DIRECTORY = 16,
	MAPTEXTURE_PATCH_COUNT      = 20,
	MAPTEXTURE_PATCHES          = 22
};


/* A single patch from a texture definition, */
/*  basically a rectangular area within */
/*  the texture rectangle. */
typedef struct
{
	/* Block origin (allways UL), */
	/* which has allready accounted */
	/* for the internal origin of the patch. */
	int         originx;
	int         originy;
	int         patch;
} texpatch_t;


/* A maptexturedef_t describes a rectangular texture, */
/*  which is composed of one or more mappatch_t structures */
/*  that arrange graphic patches. */
typedef struct
{
	/* Keep name for switch changing, etc. */
	char        name[8];
	short       width;
	short       height;

	/* All the patches[patchcount] */
	/*  are drawn back to front into the cached texture. */
	short       patchcount;
	texpatch_t  patches[1];

} texture_t;



int             firstflat;
int             lastflat;
int             numflats;

int             firstpatch;
int             lastpatch;
int             numpatches;

int             firstspritelump;
int             lastspritelump;
int             numspritelumps;

int             numtextures;
texture_t**     textures;


int*                    texturewidthmask;
/* needed for texture pegging */
fixed_t*                textureheight;
int*                    texturecompositesize;
short**                 texturecolumnlump;
unsigned short**        texturecolumnofs;
unsigned char**         texturecomposite;

/* for global animation */
int*            flattranslation;
int*            texturetranslation;

/* needed for pre rendering */
fixed_t*        spritewidth;
fixed_t*        spriteoffset;
fixed_t*        spritetopoffset;

colourindex_t    colormaps[NUMLIGHTCOLORMAPS*2+2][0x100];


/* MAPTEXTURE_T CACHING */
/* When a texture is first needed, */
/*  it counts the number of composite columns */
/*  required in the texture and allocates space */
/*  for a column directory and any new columns. */
/* The directory will simply point inside other patches */
/*  if there is only one patch in a given column, */
/*  but any columns with multiple patches */
/*  will have new column_ts generated. */



/* R_DrawColumnInCache */
/* Clip and draw a column */
/*  from a patch into a cached post. */
void
R_DrawColumnInCache
( column_t*      patch,
  unsigned char* cache,
  int            originy,
  int            cacheheight )
{
	int            count;
	int            position;
	unsigned char* source;

	while (patch->topdelta != 0xff)
	{
		source = (unsigned char *)patch + 3;
		count = patch->length;
		position = originy + patch->topdelta;

		if (position < 0)
		{
			count += position;
			position = 0;
		}

		if (position + count > cacheheight)
			count = cacheheight - position;

		if (count > 0)
			memcpy (cache + position, source, count);

		patch = (column_t *)(  (unsigned char *)patch + patch->length + 4);
	}
}



/* R_GenerateComposite */
/* Using the texture definition, */
/*  the composite texture is created from the patches, */
/*  and each column is cached. */
void R_GenerateComposite (int texnum)
{
	unsigned char*      block;
	texture_t*          texture;
	texpatch_t*         patch;
	patch_t*            realpatch;
	int                 x;
	int                 x1;
	int                 x2;
	int                 i;
	column_t*           patchcol;
	short*              collump;
	unsigned short*     colofs;

	texture = textures[texnum];

	block = (unsigned char*)Z_Malloc (texturecompositesize[texnum],
					  PU_STATIC,
					  &texturecomposite[texnum]);

	collump = texturecolumnlump[texnum];
	colofs = texturecolumnofs[texnum];

	/* Composite the columns together. */
	patch = texture->patches;

	for (i=0 , patch = texture->patches;
		 i<texture->patchcount;
		 i++, patch++)
	{
		realpatch = (patch_t*)W_CacheLumpNum (patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + SHORT(realpatch->width);

		if (x1<0)
			x = 0;
		else
			x = x1;

		if (x2 > texture->width)
			x2 = texture->width;

		for ( ; x<x2 ; x++)
		{
			/* Column does not have multiple patches? */
			if (collump[x] >= 0)
				continue;

			patchcol = (column_t *)((unsigned char *)realpatch
									+ LONG(realpatch->columnofs[x-x1]));
			R_DrawColumnInCache (patchcol,
								 block + colofs[x],
								 patch->originy,
								 texture->height);
		}

	}

	/* Now that the texture has been built in column cache, */
	/*  it is purgable from zone memory. */
	Z_ChangeTag (block, PU_CACHE);
}



/* R_GenerateLookup */
void R_GenerateLookup (int texnum)
{
	texture_t*          texture;
	unsigned char*      patchcount;     /* patchcount[texture->width] */
	texpatch_t*         patch;
	patch_t*            realpatch;
	int                 x;
	int                 x1;
	int                 x2;
	int                 i;
	short*              collump;
	unsigned short*     colofs;

	texture = textures[texnum];

	/* Composited texture not created yet. */
	texturecomposite[texnum] = NULL;

	texturecompositesize[texnum] = 0;
	collump = texturecolumnlump[texnum];
	colofs = texturecolumnofs[texnum];

	/* Now count the number of columns */
	/*  that are covered by more than one patch. */
	/* Fill in the lump / offset, so columns */
	/*  with only a single patch are all done. */
	patchcount = (unsigned char *)Z_Malloc (texture->width, PU_STATIC, NULL);
	memset(patchcount, 0, texture->width);
	patch = texture->patches;

	for (i=0 , patch = texture->patches;
		 i<texture->patchcount;
		 i++, patch++)
	{
		realpatch = (patch_t*)W_CacheLumpNum (patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + SHORT(realpatch->width);

		if (x1 < 0)
			x = 0;
		else
			x = x1;

		if (x2 > texture->width)
			x2 = texture->width;

		for ( ; x<x2 ; x++)
		{
			patchcount[x]++;
			collump[x] = patch->patch;
			colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
		}
	}

	for (x=0 ; x<texture->width ; x++)
	{
		if (!patchcount[x])
		{
			I_Info ("R_GenerateLookup: column without a patch (%s)\n",
					texture->name);
			break;
		}
		/* I_Error ("R_GenerateLookup: column without a patch"); */

		if (patchcount[x] > 1)
		{
			/* Use the cached block. */
			collump[x] = -1;
			colofs[x] = texturecompositesize[texnum];

			if (texturecompositesize[texnum] > 0x10000-texture->height)
			{
				I_Error ("R_GenerateLookup: texture %i is >64k",
						 texnum);
			}

			texturecompositesize[texnum] += texture->height;
		}
	}

	Z_Free(patchcount);
}




/* R_GetColumn */
unsigned char*
R_GetColumn
( int           tex,
  int           col )
{
	int         lump;
	int         ofs;

	col &= texturewidthmask[tex];
	lump = texturecolumnlump[tex][col];
	ofs = texturecolumnofs[tex][col];

	if (lump > 0)
		return (unsigned char *)W_CacheLumpNum(lump,PU_CACHE)+ofs;

	if (!texturecomposite[tex])
		R_GenerateComposite (tex);

	return texturecomposite[tex] + ofs;
}




/* R_InitTextures */
/* Initializes the texture list */
/*  with the textures from the world map. */
void R_InitTextures (void)
{
	unsigned char*      mtexture;
	texture_t*          texture;
	unsigned char*      mpatch;
	texpatch_t*         patch;

	int                 i;
	int                 j;

	unsigned char*      maptex;
	unsigned char*      maptex2;
	unsigned char*      maptex1;

	char                name[9];
	unsigned char*      names;
	const char*         name_p;

	int*                patchlookup;

	int                 totalwidth;
	int                 nummappatches;
	int                 offset;
	int                 maxoff;
	int                 maxoff2;
	int                 numtextures1;
	int                 numtextures2;

	unsigned char*      directory;

	int                 temp1;
	int                 temp2;
	int                 temp3;


	/* Load the patch names from pnames.lmp. */
	name[sizeof(name) - 1] = '\0';
	names = (unsigned char*)W_CacheLumpName ("PNAMES", PU_STATIC);
	nummappatches = M_BytesToLong(names);
	name_p = (const char*)names+4;
	patchlookup = (int*)Z_Malloc (nummappatches*sizeof(*patchlookup), PU_STATIC, NULL);

	for (i=0 ; i<nummappatches ; i++)
	{
		strncpy (name,name_p+i*8, 8);
		patchlookup[i] = W_CheckNumForName (name);
	}
	Z_Free (names);

	/* Load the map texture definitions from textures.lmp. */
	/* The data is contained in one or two lumps, */
	/*  TEXTURE1 for shareware, plus TEXTURE2 for commercial. */
	maptex = maptex1 = (unsigned char*)W_CacheLumpName ("TEXTURE1", PU_STATIC);
	numtextures1 = M_BytesToLong(maptex);
	maxoff = W_LumpLength (W_GetNumForName ("TEXTURE1"));
	directory = maptex+4;

	if (W_CheckNumForName ("TEXTURE2") != -1)
	{
		maptex2 = (unsigned char*)W_CacheLumpName ("TEXTURE2", PU_STATIC);
		numtextures2 = M_BytesToLong(maptex2);
		maxoff2 = W_LumpLength (W_GetNumForName ("TEXTURE2"));
	}
	else
	{
		maptex2 = NULL;
		numtextures2 = 0;
		maxoff2 = 0;
	}
	numtextures = numtextures1 + numtextures2;

	textures = (texture_t**)Z_Malloc (numtextures*sizeof(*textures), PU_STATIC, NULL);
	texturecolumnlump = (short**)Z_Malloc (numtextures*sizeof(*texturecolumnlump), PU_STATIC, NULL);
	texturecolumnofs = (unsigned short**)Z_Malloc (numtextures*sizeof(*texturecolumnofs), PU_STATIC, NULL);
	texturecomposite = (unsigned char**)Z_Malloc (numtextures*sizeof(*texturecomposite), PU_STATIC, NULL);
	texturecompositesize = (int*)Z_Malloc (numtextures*sizeof(*texturecompositesize), PU_STATIC, NULL);
	texturewidthmask = (int*)Z_Malloc (numtextures*sizeof(*texturewidthmask), PU_STATIC, NULL);
	textureheight = (fixed_t*)Z_Malloc (numtextures*sizeof(*textureheight), PU_STATIC, NULL);

	totalwidth = 0;

	/*  Really complex printing shit... */
	temp1 = W_GetNumForName ("S_START");  /* P_??????? */
	temp2 = W_GetNumForName ("S_END") - 1;
	temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);
	I_Info("[");
	for (i = 0; i < temp3; i++)
		I_Info(" ");
	I_Info("         ]");
	for (i = 0; i < temp3; i++)
		I_Info("\x8");
	I_Info("\x8\x8\x8\x8\x8\x8\x8\x8\x8\x8");

	for (i=0 ; i<numtextures ; i++, directory += 4)
	{
		if (!(i&63))
			I_Info (".");

		if (i == numtextures1)
		{
			/* Start looking in second texture file. */
			maptex = maptex2;
			maxoff = maxoff2;
			directory = maptex+4;
		}

		offset = M_BytesToLong(directory);

		if (offset > maxoff)
			I_Error ("R_InitTextures: bad texture directory");

		mtexture = maptex + offset;

		texture = textures[i] =
			(texture_t*)Z_Malloc (sizeof(texture_t)
					  + sizeof(texpatch_t)*(M_BytesToShort(&mtexture[MAPTEXTURE_PATCH_COUNT])-1),
					  PU_STATIC, NULL);

		texture->width = M_BytesToShort(&mtexture[MAPTEXTURE_WIDTH]);
		texture->height = M_BytesToShort(&mtexture[MAPTEXTURE_HEIGHT]);
		texture->patchcount = M_BytesToShort(&mtexture[MAPTEXTURE_PATCH_COUNT]);

		memcpy (texture->name, &mtexture[MAPTEXTURE_NAME], sizeof(texture->name));
		mpatch = &mtexture[MAPTEXTURE_PATCHES];
		patch = &texture->patches[0];

		for (j=0 ; j<texture->patchcount ; j++, mpatch += MAPPATCH_SIZEOF, patch++)
		{
			patch->originx = M_BytesToShort(&mpatch[MAPPATCH_ORIGIN_X]);
			patch->originy = M_BytesToShort(&mpatch[MAPPATCH_ORIGIN_Y]);
			patch->patch = patchlookup[M_BytesToShort(&mpatch[MAPPATCH_PATCH])];
			if (patch->patch == -1)
			{
				I_Error ("R_InitTextures: Missing patch in texture %s",
						 texture->name);
			}
		}
		texturecolumnlump[i] = (short*)Z_Malloc (texture->width*2, PU_STATIC, NULL);
		texturecolumnofs[i] = (unsigned short*)Z_Malloc (texture->width*2, PU_STATIC, NULL);

		j = 1;
		while (j*2 <= texture->width)
			j<<=1;

		texturewidthmask[i] = j-1;
		textureheight[i] = texture->height<<FRACBITS;

		totalwidth += texture->width;
	}

	Z_Free(patchlookup);

	Z_Free (maptex1);
	if (maptex2 != NULL)
		Z_Free (maptex2);

	/* Precalculate whatever possible. */
	for (i=0 ; i<numtextures ; i++)
		R_GenerateLookup (i);

	/* Create translation table for global animation. */
	texturetranslation = (int*)Z_Malloc ((numtextures+1)*sizeof(*texturetranslation), PU_STATIC, NULL);

	for (i=0 ; i<numtextures ; i++)
		texturetranslation[i] = i;
}



/* R_InitFlats */
void R_InitFlats (void)
{
	int         i;

	firstflat = W_GetNumForName ("F_START") + 1;
	lastflat = W_GetNumForName ("F_END") - 1;
	numflats = lastflat - firstflat + 1;

	/* Create translation table for global animation. */
	flattranslation = (int*)Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, NULL);

	for (i=0 ; i<numflats ; i++)
		flattranslation[i] = i;
}


/* R_InitSpriteLumps */
/* Finds the width and hoffset of all sprites in the wad, */
/*  so the sprite does not need to be cached completely */
/*  just for having the header info ready during rendering. */
void R_InitSpriteLumps (void)
{
	int         i;
	patch_t     *patch;

	firstspritelump = W_GetNumForName ("S_START") + 1;
	lastspritelump = W_GetNumForName ("S_END") - 1;

	numspritelumps = lastspritelump - firstspritelump + 1;
	spritewidth = (fixed_t*)Z_Malloc (numspritelumps*sizeof(*spritewidth), PU_STATIC, NULL);
	spriteoffset = (fixed_t*)Z_Malloc (numspritelumps*sizeof(*spriteoffset), PU_STATIC, NULL);
	spritetopoffset = (fixed_t*)Z_Malloc (numspritelumps*sizeof(*spritetopoffset), PU_STATIC, NULL);

	for (i=0 ; i< numspritelumps ; i++)
	{
		if (!(i&63))
			I_Info (".");

		patch = (patch_t*)W_CacheLumpNum (firstspritelump+i, PU_CACHE);
		spritewidth[i] = SHORT(patch->width)<<FRACBITS;
		spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
		spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
	}
}



/* R_InitColormaps */
void R_InitColormaps (void)
{
	extern int full_colour;
	extern int prototype_light_amplification_visor_effect;

	size_t i, j;
	unsigned char(* const original_colour_maps)[0x100] = (unsigned char(*)[0x100])W_CacheLumpName("COLORMAP", PU_STATIC);

	/* Load-in the original light tables. */
	for (i = 0; i < NUMLIGHTCOLORMAPS; ++i)
		for (j = 0; j < D_COUNT_OF(colormaps[FUZZCOLORMAPS + i]); ++j)
			colormaps[FUZZCOLORMAPS + i][j] = original_colour_maps[i][j];

	/* Release the `COLORMAP` buffer. */
	Z_ChangeTag(original_colour_maps, PU_CACHE);

	if (full_colour)
	{
		/* Make the brightness-related colour maps pass-through straight to the underlying
		   expanded palette, bypassing colour-aliasing and enabling 'full-colour' rendering. */
		for (i = 0; i < NUMLIGHTCOLORMAPS; ++i)
			for (j = 0; j < D_COUNT_OF(colormaps[i]); ++j)
				colormaps[i][j] = i * D_COUNT_OF(colormaps[i]) + j;
	}
	else
	{
		/* Load the original colour maps. */
		memcpy(colormaps[0], colormaps[FUZZCOLORMAPS], sizeof(colormaps[0]) * NUMLIGHTCOLORMAPS);
	}

	/* Set-up the light-amplification visor colour map. */
	if (prototype_light_amplification_visor_effect)
	{
		/* Ripped from the Press Release Pre-Beta. */
		/* You don't need to be a lawyer to know that the output of a very generic colour-matching algorithm is not copyrightable. */
		static const unsigned char light_amplification_colour_map[D_COUNT_OF(colormaps[LIGHTAMPCOLORMAP])] = {
			0x00, 0x7F, 0x08, 0x7D, 0x75, 0x7F, 0x7F, 0x08, 0x00, 0x7E, 0x7F, 0x7F, 0x08, 0x7D, 0x7D, 0x7E,
			0x77, 0x77, 0x77, 0x78, 0x78, 0x78, 0x79, 0x7A, 0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C,
			0x7C, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
			0x75, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x78, 0x78, 0x79, 0x7A, 0x7A,
			0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C, 0x7C, 0x7D, 0x7D, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F,
			0x75, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x78, 0x78, 0x79, 0x79, 0x7A, 0x7A,
			0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7E, 0x7F, 0x7F,
			0x77, 0x77, 0x78, 0x79, 0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7C, 0x7D, 0x7D, 0x7E, 0x7F, 0x7F, 0x08,
			0x78, 0x78, 0x79, 0x7A, 0x7A, 0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C, 0x7D, 0x7D, 0x7D,
			0x7A, 0x7B, 0x7B, 0x7B, 0x7C, 0x7D, 0x7D, 0x7D, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C, 0x7D, 0x7D, 0x7D,
			0x75, 0x77, 0x77, 0x79, 0x7A, 0x7B, 0x7D, 0x7D, 0x75, 0x77, 0x77, 0x77, 0x78, 0x7A, 0x7B, 0x7C,
			0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
			0x75, 0x77, 0x77, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x08, 0x08,
			0x75, 0x75, 0x77, 0x77, 0x77, 0x77, 0x78, 0x79, 0x7A, 0x7A, 0x7A, 0x7B, 0x7B, 0x7B, 0x7C, 0x7C,
			0x75, 0x75, 0x75, 0x75, 0x75, 0x77, 0x77, 0x77, 0x7C, 0x7D, 0x7D, 0x7D, 0x7D, 0x7E, 0x7F, 0x7F,
			0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x77, 0x77, 0x7B, 0x7C, 0x7D, 0x7E, 0x7B
		};

		for (i = 0; i < D_COUNT_OF(colormaps[LIGHTAMPCOLORMAP]); ++i)
			colormaps[LIGHTAMPCOLORMAP][i] = light_amplification_colour_map[i];
	}
	else
	{
		/* Use the second-brightest regular colour map. */
		memcpy(&colormaps[LIGHTAMPCOLORMAP], &colormaps[1*NUMLIGHTCOLORMAPS_MUL], sizeof(colormaps[1*NUMLIGHTCOLORMAPS_MUL]));
	}
}



/* R_InitData */
/* Locates all the lumps */
/*  that will be used by all views */
/* Must be called after W_Init. */
void R_InitData (void)
{
	R_InitTextures ();
	I_Info ("\nInitTextures");
	R_InitFlats ();
	I_Info ("\nInitFlats");
	R_InitSpriteLumps ();
	I_Info ("\nInitSprites");
	R_InitColormaps ();
	I_Info ("\nInitColormaps");
}



/* R_FlatNumForName */
/* Retrieval, get a flat number for a flat name. */
int R_FlatNumForName (const char* name)
{
	int         i;
	char        namet[9];

	i = W_CheckNumForName (name);

	if (i == -1)
	{
		namet[8] = 0;
		memcpy (namet, name,8);
		I_Error ("R_FlatNumForName: %s not found",namet);
	}
	return i - firstflat;
}




/* R_CheckTextureNumForName */
/* Check whether texture is available. */
/* Filter out NoTexture indicator. */
int     R_CheckTextureNumForName (const char *name)
{
	int         i;

	/* "NoTexture" marker. */
	if (name[0] == '-')
		return 0;

	for (i=0 ; i<numtextures ; i++)
		if (!M_strncasecmp (textures[i]->name, name, 8) )
			return i;

	return -1;
}



/* R_TextureNumForName */
/* Calls R_CheckTextureNumForName, */
/*  aborts with error message. */
int     R_TextureNumForName (const char* name)
{
	int         i;

	i = R_CheckTextureNumForName (name);

	if (i==-1)
	{
		I_Error ("R_TextureNumForName: %s not found",
				 name);
	}
	return i;
}




/* R_PrecacheLevel */
/* Preloads all relevant graphics for the level. */
int             flatmemory;
int             texturememory;
int             spritememory;

void R_PrecacheLevel (void)
{
	char*               flatpresent;
	char*               texturepresent;
	char*               spritepresent;

	int                 i;
	int                 j;
	int                 k;
	int                 lump;

	texture_t*          texture;
	thinker_t*          th;
	spriteframe_t*      sf;

	if (demoplayback)
		return;

	/* Precache flats. */
	flatpresent = (char*)Z_Malloc(numflats, PU_STATIC, NULL);
	memset(flatpresent, d_false, numflats);

	for (i=0 ; i<numsectors ; i++)
	{
		flatpresent[sectors[i].floorpic] = d_true;
		flatpresent[sectors[i].ceilingpic] = d_true;
	}

	flatmemory = 0;

	for (i=0 ; i<numflats ; i++)
	{
		if (flatpresent[i])
		{
			lump = firstflat + i;
			flatmemory += lumpinfo[lump].size;
			W_CacheLumpNum(lump, PU_CACHE);
		}
	}

	Z_Free(flatpresent);

	/* Precache textures. */
	texturepresent = (char*)Z_Malloc(numtextures, PU_STATIC, NULL);
	memset(texturepresent, d_false, numtextures);

	for (i=0 ; i<numsides ; i++)
	{
		texturepresent[sides[i].toptexture] = d_true;
		texturepresent[sides[i].midtexture] = d_true;
		texturepresent[sides[i].bottomtexture] = d_true;
	}

	/* Sky texture is always present. */
	/* Note that F_SKY1 is the name used to */
	/*  indicate a sky floor/ceiling as a flat, */
	/*  while the sky texture is stored like */
	/*  a wall texture, with an episode dependend */
	/*  name. */
	texturepresent[skytexture] = d_true;

	texturememory = 0;
	for (i=0 ; i<numtextures ; i++)
	{
		if (!texturepresent[i])
			continue;

		texture = textures[i];

		for (j=0 ; j<texture->patchcount ; j++)
		{
			lump = texture->patches[j].patch;
			texturememory += lumpinfo[lump].size;
			W_CacheLumpNum(lump , PU_CACHE);
		}
	}

	Z_Free(texturepresent);

	/* Precache sprites. */
	spritepresent = (char*)Z_Malloc(numsprites, PU_STATIC, NULL);
	memset(spritepresent, d_false, numsprites);

	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
			spritepresent[((mobj_t *)th)->sprite] = d_true;
	}

	spritememory = 0;
	for (i=0 ; i<numsprites ; i++)
	{
		if (!spritepresent[i])
			continue;

		for (j=0 ; j<sprites[i].numframes ; j++)
		{
			sf = &sprites[i].spriteframes[j];
			for (k=0 ; k<8 ; k++)
			{
				lump = firstspritelump + sf->lump[k];
				spritememory += lumpinfo[lump].size;
				W_CacheLumpNum(lump , PU_CACHE);
			}
		}
	}

	Z_Free(spritepresent);
}




