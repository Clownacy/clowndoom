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
        The actual span/column drawing functions.
        Here find the main potential for optimization,
         e.g. inline assembly, different algorithms.

******************************************************************************/


#include "doomdef.h"

#include "i_system.h"
#include "st_stuff.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

/* Needs access to LFB (guess what). */
#include "v_video.h"

/* State. */
#include "doomstat.h"


/* ? */
#define MAXWIDTH                MAXIMUM_SCREENWIDTH/*1120*/
#define MAXHEIGHT               MAXIMUM_SCREENHEIGHT/*832*/

/* status bar height at bottom of screen */
#define SBARHEIGHT              ST_HEIGHT

/* All drawing to the view buffer is accomplished in this file. */
/* The other refresh files only know about ccordinates, */
/*  not the architecture of the frame buffer. */
/* Conveniently, the frame buffer is a linear one, */
/*  and we need only the base address, */
/*  and the total size == width*height*depth/8., */


unsigned char*  viewimage;
int             viewwidth;
int             scaledviewwidth;
int             viewheight;
int             viewwindowx;
int             viewwindowy;
colourindex_t*   ylookup[MAXHEIGHT];
int             columnofs[MAXWIDTH];

/* Color tables for different players, */
/*  translate a limited part to another */
/*  (color ramps used for  suit colors). */
unsigned char   translations[3][256];




/* R_DrawColumn */
/* Source is the top of the column to scale. */
colourindex_t*           dc_colormap;
int                     dc_x;
int                     dc_yl;
int                     dc_yh;
fixed_t                 dc_iscale;
fixed_t                 dc_texturemid;

/* first pixel in a column (possibly virtual) */
unsigned char*         dc_source;

/* just for profiling */
int                     dccount;

/* A column is a vertical slice/span from a wall texture that, */
/*  given the DOOM style restrictions on the view orientation, */
/*  will always have constant z depth. */
/* Thus a special case loop for very fast rendering can */
/*  be used. It has also been used with Wolfenstein 3D. */
void R_DrawColumn (void)
{
	int                 count;
	colourindex_t*       dest;
	fixed_t             frac;
	fixed_t             fracstep;

	count = dc_yh - dc_yl;

	/* Zero length, column does not exceed a pixel. */
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0
		|| dc_yh >= SCREENHEIGHT)
		I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

	/* Framebuffer destination address. */
	/* Use ylookup LUT to avoid multiply with ScreenWidth. */
	/* Use columnofs LUT for subwindows? */
	dest = ylookup[dc_yl] + columnofs[dc_x];

	/* Determine scaling, */
	/*  which is the only mapping to be done. */
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	/* Inner loop that does the actual texture mapping, */
	/*  e.g. a DDA-lile scaling. */
	/* This is as fast as it gets. */
	do
	{
		/* Re-map color indices from wall texture column */
		/*  using a lighting/special effects LUT. */
		*dest++ = dc_colormap[dc_source[(frac>>FRACBITS)&127]];

		frac += fracstep;

	} while (count--);
}



void R_DrawColumnLow (void)
{
	int                 count;
	colourindex_t*       dest[2];
	fixed_t             frac;
	fixed_t             fracstep;

	count = dc_yh - dc_yl;

	/* Zero length. */
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0
		|| dc_yh >= SCREENHEIGHT)
	{

		I_Error ("R_DrawColumnLow: %i to %i at %i", dc_yl, dc_yh, dc_x);
	}
	/*  dccount++; */
#endif
	/* Blocky mode, need to multiply by 2. */
	dest[0] = ylookup[dc_yl] + columnofs[dc_x*2];
	dest[1] = dest[0] + SCREENHEIGHT;

	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	do
	{
		*dest[0]++ = *dest[1]++ = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
		frac += fracstep;

	} while (count--);
}


/* Spectre/Invisibility. */
#define FUZZTABLE 50
/* TODO: This is a bit awkward. Should resolution increases be limited to integer multiples to prevent this from becoming a problem? */
#define FUZZOFF CC_DIVIDE_ROUND(SCREENHEIGHT, ORIGINAL_SCREEN_HEIGHT) /* TODO: This will break when alternate FOVs are added. */


static const signed char fuzzoffset[FUZZTABLE] =
{
	 1,-1, 1,-1, 1,
	 1,-1, 1, 1,-1,
	 1, 1, 1,-1, 1,
	 1, 1,-1,-1,-1,
	-1, 1,-1,-1, 1,
	 1, 1, 1,-1, 1,
	-1, 1, 1,-1,-1,
	 1, 1,-1,-1,-1,
	-1, 1, 1, 1, 1,
	-1, 1, 1,-1, 1
};

static int GetFuzzOffset(void)
{
	static int fuzzpos = 0;

	const int value = FUZZOFF * fuzzoffset[fuzzpos];

	/* Clamp table lookup index. */
	if (++fuzzpos == FUZZTABLE)
		fuzzpos = 0;

	return value;
}


/* Framebuffer postprocessing. */
/* Creates a fuzzy image by copying pixels */
/*  from adjacent ones to left and right. */
/* Used with an all black colormap, this */
/*  could create the SHADOW effect, */
/*  i.e. spectres and invisible players. */
void R_DrawFuzzColumn (void)
{
	int                 count;
	colourindex_t*       dest;
	fixed_t             frac;
	fixed_t             fracstep;

	/* Adjust borders. Low... */
	if (dc_yl < FUZZOFF)
		dc_yl = FUZZOFF;

	/* .. and high. */
	if (dc_yh > viewheight - 1 - FUZZOFF)
		dc_yh = viewheight - 1 - FUZZOFF;

	count = dc_yh - dc_yl;

	/* Zero length. */
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0 || dc_yh >= SCREENHEIGHT)
	{
		I_Error ("R_DrawFuzzColumn: %i to %i at %i",
				 dc_yl, dc_yh, dc_x);
	}
#endif

	/* Does not work with blocky mode. */
	dest = ylookup[dc_yl] + columnofs[dc_x];

	/* Looks familiar. */
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	/* Looks like an attempt at dithering, */
	/*  using the colormap #6 (of 0-31, a bit */
	/*  brighter than average). */
	do
	{
		/* Lookup framebuffer, and retrieve */
		/*  a pixel that is either one column */
		/*  left or right of the current one. */
		/* Add index from colormap to index. */
		/* This wacky new logic converts from full-colour back down to 256-colour to authentically recreate the fuzz effect. */
		const unsigned int pixel = dest[GetFuzzOffset()];
		*dest = colormaps[FUZZCOLORMAPS + 6][colormaps[FUZZCOLORMAPS + pixel / 0x100 / NUMLIGHTCOLORMAPS_MUL][pixel % 0x100]];

		++dest;
		frac += fracstep;
	} while (count--);
}

void R_DrawFuzzColumnLow (void)
{
	int                 count;
	colourindex_t*       dest[2];
	fixed_t             frac;
	fixed_t             fracstep;

	/* Adjust borders. Low... */
	if (dc_yl < FUZZOFF)
		dc_yl = FUZZOFF;

	/* .. and high. */
	if (dc_yh > viewheight - 1 - FUZZOFF)
		dc_yh = viewheight - 1 - FUZZOFF;

	count = dc_yh - dc_yl;

	/* Zero length. */
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0 || dc_yh >= SCREENHEIGHT)
	{
		I_Error ("R_DrawFuzzColumnLow: %i to %i at %i",
				 dc_yl, dc_yh, dc_x);
	}
#endif

	dest[0] = ylookup[dc_yl] + columnofs[dc_x*2];
	dest[1] = dest[0] + SCREENHEIGHT;

	/* Looks familiar. */
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	/* Looks like an attempt at dithering, */
	/*  using the colormap #6 (of 0-31, a bit */
	/*  brighter than average). */
	do
	{
		/* Lookup framebuffer, and retrieve */
		/*  a pixel that is either one column */
		/*  left or right of the current one. */
		/* Add index from colormap to index. */
		/* This wacky new logic converts from full-colour back down to 256-colour to authentically recreate the fuzz effect. */
		const unsigned int pixel = dest[0][GetFuzzOffset()];
		*dest[0] = *dest[1] = colormaps[FUZZCOLORMAPS + 6][colormaps[FUZZCOLORMAPS + pixel / 0x100 / NUMLIGHTCOLORMAPS_MUL][pixel % 0x100]];

		++dest[0];
		++dest[1];
		frac += fracstep;
	} while (count--);
}




/* R_DrawTranslatedColumn */
/* Used to draw player sprites */
/*  with the green colorramp mapped to others. */
/* Could be used with different translation */
/*  tables, e.g. the lighter colored version */
/*  of the BaronOfHell, the HellKnight, uses */
/*  identical sprites, kinda brightened up. */
unsigned char   *dc_translation;
unsigned char   translationtables[3][0x100];

void R_DrawTranslatedColumn (void)
{
	int                 count;
	colourindex_t*       dest;
	fixed_t             frac;
	fixed_t             fracstep;

	count = dc_yh - dc_yl;
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0
		|| dc_yh >= SCREENHEIGHT)
	{
		I_Error ( "R_DrawTranslatedColumn: %i to %i at %i",
				  dc_yl, dc_yh, dc_x);
	}
#endif

	dest = ylookup[dc_yl] + columnofs[dc_x];

	/* Looks familiar. */
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	/* Here we do an additional index re-mapping. */
	do
	{
		/* Translation tables are used */
		/*  to map certain colorramps to other ones, */
		/*  used with PLAY sprites. */
		/* Thus the "green" ramp of the player 0 sprite */
		/*  is mapped to gray, red, black/indigo. */
		*dest++ = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];

		frac += fracstep;
	} while (count--);
}

void R_DrawTranslatedColumnLow (void)
{
	int                 count;
	colourindex_t*       dest[2];
	fixed_t             frac;
	fixed_t             fracstep;

	count = dc_yh - dc_yl;
	if (count < 0)
		return;

#ifdef RANGECHECK
	if (dc_x >= SCREENWIDTH
		|| dc_yl < 0
		|| dc_yh >= SCREENHEIGHT)
	{
		I_Error ( "R_DrawTranslatedColumnLow: %i to %i at %i",
				  dc_yl, dc_yh, dc_x);
	}

#endif

	dest[0] = ylookup[dc_yl] + columnofs[dc_x*2];
	dest[1] = dest[0] + SCREENHEIGHT;

	/* Looks familiar. */
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	/* Here we do an additional index re-mapping. */
	do
	{
		/* Translation tables are used */
		/*  to map certain colorramps to other ones, */
		/*  used with PLAY sprites. */
		/* Thus the "green" ramp of the player 0 sprite */
		/*  is mapped to gray, red, black/indigo. */
		*dest[0]++ = *dest[1]++ = dc_colormap[dc_translation[dc_source[frac >> FRACBITS]]];

		frac += fracstep;
	} while (count--);
}




/* R_InitTranslationTables */
/* Creates the translation tables to map */
/*  the green color ramp to gray, brown, red. */
/* Assumes a given structure of the PLAYPAL. */
/* Could be read from a lump instead. */
void R_InitTranslationTables (void)
{
	int         i;

	/* translate just the 16 green colors */
	for (i=0 ; i<0x100 ; i++)
	{
		if (i >= 0x70 && i<= 0x7f)
		{
			/* map green ramp to gray, brown, red */
			translationtables[0][i] = 0x60 + (i&0xf);
			translationtables[1][i] = 0x40 + (i&0xf);
			translationtables[2][i] = 0x20 + (i&0xf);
		}
		else
		{
			/* Keep all other colors as is. */
			translationtables[0][i] = translationtables[1][i]
				= translationtables[2][i] = i;
		}
	}
}




/* R_DrawSpan */
/* With DOOM style restrictions on view orientation, */
/*  the floors and ceilings consist of horizontal slices */
/*  or spans with constant z depth. */
/* However, rotation around the world z axis is possible, */
/*  thus this mapping, while simpler and faster than */
/*  perspective correct texture mapping, has to traverse */
/*  the texture at an angle in all but a few cases. */
/* In consequence, flats are not stored by column (like walls), */
/*  and the inner loop has to step in texture space u and v. */
int                     ds_y;
int                     ds_x1;
int                     ds_x2;

colourindex_t*           ds_colormap;

fixed_t                 ds_xfrac;
fixed_t                 ds_yfrac;
fixed_t                 ds_xstep;
fixed_t                 ds_ystep;

/* start of a 64*64 tile image */
unsigned char*          ds_source;

/* just for profiling */
/*int                     dscount;*/

#define SPAN_DIMENSION_SHIFT 6
#define SPAN_DIMENSION (1 << SPAN_DIMENSION_SHIFT)
#define SPAN_DIMENSION_MASK (SPAN_DIMENSION - 1)

/* Draws the actual span. */
void R_DrawSpan (void)
{
	fixed_t             xfrac;
	fixed_t             yfrac;
	colourindex_t*       dest;
	int                 count;
	int                 spot;

#ifdef RANGECHECK
	if (ds_x2 < ds_x1
		|| ds_x1<0
		|| ds_x2>=SCREENWIDTH
		|| ds_y>SCREENHEIGHT)
	{
		I_Error( "R_DrawSpan: %i to %i at %i",
				 ds_x1,ds_x2,ds_y);
	}
/*      dscount++; */
#endif

	xfrac = ds_xfrac;
	yfrac = ds_yfrac;

	dest = ylookup[ds_y] + columnofs[ds_x1];

	/* We do not check for zero spans here? */
	count = ds_x2 - ds_x1;

	do
	{
		/* Current texture index in u,v. */
		spot = ((yfrac>>(16-SPAN_DIMENSION_SHIFT))&(SPAN_DIMENSION_MASK*SPAN_DIMENSION)) + ((xfrac>>16)&SPAN_DIMENSION_MASK);

		/* Lookup pixel from flat texture tile, */
		/*  re-index using light/colormap. */
		*dest = ds_colormap[ds_source[spot]];

		dest += SCREENHEIGHT; /* TODO: Is there a way to make this linear again? */

		/* Next step in u,v. */
		xfrac += ds_xstep;
		yfrac += ds_ystep;

	} while (count--);
}



/* Again.. */
void R_DrawSpanLow (void)
{
	fixed_t             xfrac;
	fixed_t             yfrac;
	colourindex_t*       dest;
	int                 count;
	int                 spot;

#ifdef RANGECHECK
	if (ds_x2 < ds_x1
		|| ds_x1<0
		|| ds_x2>=SCREENWIDTH
		|| ds_y>SCREENHEIGHT)
	{
		I_Error( "R_DrawSpanLow: %i to %i at %i",
				 ds_x1,ds_x2,ds_y);
	}
/*      dscount++; */
#endif

	xfrac = ds_xfrac;
	yfrac = ds_yfrac;

	/* Blocky mode, need to multiply by 2. */
	dest = ylookup[ds_y] + columnofs[ds_x1*2];

	count = ds_x2 - ds_x1;

	do
	{
		colourindex_t pixel;
		/* Current texture index in u,v. */
		spot = ((yfrac>>(16-SPAN_DIMENSION_SHIFT))&(SPAN_DIMENSION_MASK*SPAN_DIMENSION)) + ((xfrac>>16)&SPAN_DIMENSION_MASK);
		/* Lowres/blocky mode does it twice, */
		/*  while scale is adjusted appropriately. */
		pixel = ds_colormap[ds_source[spot]];
		*dest = pixel;
		dest += SCREENHEIGHT; /* TODO: Can this be made linear again? */
		*dest = pixel;
		dest += SCREENHEIGHT;

		/* Next step in u,v. */
		xfrac += ds_xstep;
		yfrac += ds_ystep;

	} while (count--);
}

/* R_InitBuffer */
/* Creats lookup tables that avoid */
/*  multiplies and other hazzles */
/*  for getting the framebuffer address */
/*  of a pixel to draw. */
void
R_InitBuffer
( int           width,
  int           height )
{
	int         i;

	/* Handle resize, */
	/*  e.g. smaller view windows */
	/*  with border and/or status bar. */
	viewwindowx = (SCREENWIDTH-width) >> 1;

	/* Column offset. For windows. */
	for (i=0 ; i<width ; i++)
		columnofs[i] = (viewwindowx + i) * SCREENHEIGHT;

	/* Same with base row offset. */
	if (width == SCREENWIDTH)
		viewwindowy = 0;
	else
		viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1;

	/* Preclaculate all row offsets. */
	for (i=0 ; i<height ; i++)
		ylookup[i] = screens[SCREEN_FRAMEBUFFER] + viewwindowy + i;
}




/* R_FillBackScreen */
/* Fills the back screen with a pattern */
/*  for variable screen sizes */
/* Also draws a beveled edge. */
void R_FillBackScreen (void)
{
	int         x;
	int         y;
	patch_t*    patch;

	if (scaledviewwidth == SCREENWIDTH)
		return;

	V_FillScreenWithPattern(gamemode == commercial ? "GRNROCK" : "FLOOR7_2", SCREEN_BACK, SCREENHEIGHT-SBARHEIGHT);

	patch = (patch_t*)W_CacheLumpName ("brdr_t",PU_CACHE);

	for (x=0 ; x<scaledviewwidth ; x+=BEVEL_SIZE)
		V_DrawPatch (viewwindowx+x,viewwindowy-BEVEL_SIZE,SCREEN_BACK,patch);
	patch = (patch_t*)W_CacheLumpName ("brdr_b",PU_CACHE);

	for (x=0 ; x<scaledviewwidth ; x+=BEVEL_SIZE)
		V_DrawPatch(viewwindowx+x,viewwindowy+viewheight,SCREEN_BACK,patch);
	patch = (patch_t*)W_CacheLumpName ("brdr_l",PU_CACHE);

	for (y=0 ; y<viewheight ; y+=BEVEL_SIZE)
		V_DrawPatch(viewwindowx-BEVEL_SIZE,viewwindowy+y,SCREEN_BACK,patch);
	patch = (patch_t*)W_CacheLumpName ("brdr_r",PU_CACHE);

	for (y=0 ; y<viewheight ; y+=BEVEL_SIZE)
		V_DrawPatch(viewwindowx+scaledviewwidth,viewwindowy+y,SCREEN_BACK,patch);


	/* Draw beveled edge. */
	V_DrawPatch (viewwindowx-BEVEL_SIZE,
				 viewwindowy-BEVEL_SIZE,
				 SCREEN_BACK,
				 (patch_t*)W_CacheLumpName ("brdr_tl",PU_CACHE));

	V_DrawPatch(viewwindowx+scaledviewwidth,
				 viewwindowy-BEVEL_SIZE,
				 SCREEN_BACK,
				 (patch_t*)W_CacheLumpName ("brdr_tr",PU_CACHE));

	V_DrawPatch(viewwindowx-BEVEL_SIZE,
				 viewwindowy+viewheight,
				 SCREEN_BACK,
				 (patch_t*)W_CacheLumpName ("brdr_bl",PU_CACHE));

	V_DrawPatch(viewwindowx+scaledviewwidth,
				 viewwindowy+viewheight,
				 SCREEN_BACK,
				 (patch_t*)W_CacheLumpName ("brdr_br",PU_CACHE));
}


/* Copy a screen buffer. */
void
R_VideoErase
( unsigned      ofs,
  int           count )
{
	/* LFB copy. */
	/* This might not be a good idea if memcpy */
	/*  is not optiomal, e.g. byte by byte on */
	/*  a 32bit CPU, as GNU GCC/Linux libc did */
	/*  at one point. */
	memcpy (screens[SCREEN_FRAMEBUFFER]+ofs, screens[SCREEN_BACK]+ofs, count*sizeof(colourindex_t));
}


/* R_DrawViewBorder */
/* Draws the border around the view */
/*  for different size windows? */
void R_DrawViewBorder (void)
{
	int         top;
	int         side;
	int         i;

	if (scaledviewwidth == SCREENWIDTH)
		return;

	top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2;
	side = (SCREENWIDTH-scaledviewwidth)/2;

	/* Copy left side. */
	for (i = 0; i < side; ++i)
		R_VideoErase(i * SCREENHEIGHT, SCREENHEIGHT - SBARHEIGHT);

	/* Copy top and bottom. */
	for (i = 0; i < scaledviewwidth; ++i)
	{
		R_VideoErase((side + i) * SCREENHEIGHT, top);
		R_VideoErase((side + i) * SCREENHEIGHT + top + viewheight, top);
	}

	/* Copy right side. */
	for (i = 0; i < side; ++i)
		R_VideoErase((side + scaledviewwidth + i) * SCREENHEIGHT, SCREENHEIGHT - SBARHEIGHT);
}
