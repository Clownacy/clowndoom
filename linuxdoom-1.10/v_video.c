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
        Gamma correction LUT stuff.
        Functions to draw patches (by post) directly to screen.
        Functions to blit a block to the screen.

******************************************************************************/


#include "i_system.h"
#include "r_local.h"

#include "doomdef.h"
#include "doomdata.h"

#include "m_bbox.h"
#include "m_swap.h"
#include "w_wad.h"
#include "z_zone.h"

#include "v_video.h"


/* Technically, the status bar buffer should only be `ST_WIDTH * ST_HEIGHT`, but every screen needs to be `SCREENHEIGHT` tall due to hardcoded assumptions. */
/* TODO: Fix this. */
unsigned char screens[5][SCREENWIDTH * SCREENHEIGHT];



/* Now where did these came from? */
const unsigned char gammatable[5][256] =
{
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	 17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	 33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	 49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	 65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
	 81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
	 97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	 113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

	{2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
	 32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
	 56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
	 78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
	 99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
	 115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
	 130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
	 146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
	 161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
	 175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
	 190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
	 205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
	 219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
	 233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
	 247,248,249,250,251,252,252,253,254,255},

	{4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
	 43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
	 70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
	 94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
	 113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	 129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
	 144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
	 160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
	 174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
	 188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
	 202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
	 216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
	 229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
	 242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
	 255},

	{8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
	 57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
	 86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
	 108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
	 125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
	 141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
	 155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
	 169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
	 183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
	 195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
	 207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
	 219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
	 231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
	 242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
	 253,253,254,254,255},

	{16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
	 78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
	 107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
	 125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
	 142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
	 156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
	 169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
	 182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
	 193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
	 204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
	 214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
	 224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
	 234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
	 243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
	 251,252,252,253,254,254,255,255}
};



int     usegamma;


/* V_CopyRect */
void
V_CopyRect
( int           srcx,
  int           srcy,
  screen_t      srcscrn,
  int           width,
  int           height,
  int           destx,
  int           desty,
  screen_t      destscrn )
{
	const unsigned char* src;
	unsigned char*       dest;

#ifdef RANGECHECK
	if (srcx<0
		||srcx+width >SCREENWIDTH
		|| srcy<0
		|| srcy+height>SCREENHEIGHT
		||destx<0||destx+width >SCREENWIDTH
		|| desty<0
		|| desty+height>SCREENHEIGHT
		|| (unsigned)srcscrn>4
		|| (unsigned)destscrn>4)
	{
		I_Error ("Bad V_CopyRect");
	}
#endif

	src = screens[srcscrn]+srcy+SCREENHEIGHT*srcx;
	dest = screens[destscrn]+desty+SCREENHEIGHT*destx;

	for ( ; width>0 ; width--)
	{
		memcpy (dest, src, height);
		src += SCREENHEIGHT;
		dest += SCREENHEIGHT;
	}
}


static void
V_DrawPatchColumnInternal
( unsigned char* desttop,
  const patch_t* patch,
  int            col )
{
	column_t*      column;
	unsigned char* source;
	unsigned char* dest;
	int            count;

	column = (column_t *)((unsigned char *)patch + LONG(patch->columnofs[col]));

	/* step through the posts in a column */
	while (column->topdelta != 0xff )
	{
		int x;

		for (x = 0; x < HUD_SCALE; ++x)
		{
			source = (unsigned char *)column + 3;
			dest = desttop + column->topdelta*HUD_SCALE + x*SCREENHEIGHT;
			count = column->length;

			while (count--)
			{
				int y;

				const unsigned char pixel = *source++;

				for (y = 0; y < HUD_SCALE; ++y)
					*dest++ = pixel;
			}
		}

		column = (column_t *)((unsigned char *)column + column->length + 4);
	}
}


void
V_DrawPatchFlipped
( int            x,
  int            y,
  screen_t       scrn,
  const patch_t* patch,
  d_bool         flip )
{

	int         col;
	unsigned char*       desttop;
	int         w;

#ifdef RANGECHECK
	const int scaled_width = SHORT(patch->width) * HUD_SCALE;
	const int scaled_height = SHORT(patch->height) * HUD_SCALE;
#endif

	y -= SHORT(patch->topoffset) * HUD_SCALE;
	x -= SHORT(patch->leftoffset) * HUD_SCALE;
#ifdef RANGECHECK
	if (x<0
		||x+scaled_width >SCREENWIDTH
		|| y<0
		|| y+scaled_height>SCREENHEIGHT
		|| (unsigned)scrn>4)
	{
	  I_Info("Patch at %d,%d exceeds LFB\n", x,y );
	  /* TODO: No I_Error abort - what is up with TNT.WAD? */
	  I_Info("V_DrawPatch: bad patch (ignored)\n");
	  return;
	}
#endif

	col = 0;
	desttop = screens[scrn]+y+x*SCREENHEIGHT;

	w = SHORT(patch->width);

	for ( ; col<w ; col++, desttop += SCREENHEIGHT*HUD_SCALE)
		V_DrawPatchColumnInternal(desttop, patch, flip ? w - 1 - col : col);
}



void
V_DrawPatchColumn
( int            x,
  int            y,
  screen_t       scrn,
  const patch_t* patch,
  int            col )
{
	V_DrawPatchColumnInternal(screens[scrn]+y+x*SCREENHEIGHT, patch, col);
}



/* V_DrawBlock */
/* Draw a linear block of pixels into the view buffer. */
void
V_DrawBlock
( int           x,
  int           y,
  screen_t      scrn,
  int           width,
  int           height,
  const unsigned char*           src )
{
	unsigned char*       dest;

#ifdef RANGECHECK
	if (x<0
		||x+width >SCREENWIDTH
		|| y<0
		|| y+height>SCREENHEIGHT
		|| (unsigned)scrn>4 )
	{
		I_Error ("Bad V_DrawBlock");
	}
#endif

	dest = screens[scrn] + y+x*SCREENHEIGHT;

	while (width--)
	{
		memcpy (dest, src, height);
		src += height;
		dest += SCREENHEIGHT;
	}
}



/* V_GetBlock */
/* Gets a linear block of pixels from the view buffer. */
void
V_GetBlock
( int           x,
  int           y,
  screen_t      scrn,
  int           width,
  int           height,
  unsigned char*         dest )
{
	const unsigned char* src;

#ifdef RANGECHECK
	if (x<0
		||x+width >SCREENWIDTH
		|| y<0
		|| y+height>SCREENHEIGHT
		|| (unsigned)scrn>4 )
	{
		I_Error ("Bad V_DrawBlock");
	}
#endif

	src = screens[scrn] + y+x*SCREENHEIGHT;

	while (width--)
	{
		memcpy (dest, src, height);
		src += SCREENHEIGHT;
		dest += height;
	}
}

#define BG_TILE_SRC_SIZE 64
#define BG_TILE_DST_SIZE (BG_TILE_SRC_SIZE*HUD_SCALE)

void
V_FillScreenWithPattern
( const char* const lump_name,
  const screen_t screen,
  const int height )
{
	int x,y,w;

	const unsigned char* const src = (unsigned char*)W_CacheLumpName ( lump_name , PU_CACHE);
	unsigned char *dest = screens[screen];

	for (x=0 ; x<(SCREENWIDTH+(HUD_SCALE-1))/HUD_SCALE ; x++)
	{
		static unsigned char line_buffer[BG_TILE_DST_SIZE];

		/* Upscale a row of pixels. */
		for (y=0 ; y<BG_TILE_SRC_SIZE ; y++)
			for (w=0 ; w<HUD_SCALE; w++)
				line_buffer[y*HUD_SCALE+w] = src[y*BG_TILE_SRC_SIZE+x%BG_TILE_SRC_SIZE];

		/* Repeatedly copy the upscaled row to the screen. */
		for (w=0 ; w<D_MIN(HUD_SCALE,SCREENWIDTH-x*HUD_SCALE); w++)
		{
			for (y=0 ; y<height; y+=BG_TILE_DST_SIZE)
			{
				const int bytes_to_do = D_MIN(BG_TILE_DST_SIZE, height - y);
				memcpy (dest, line_buffer, bytes_to_do);
				dest += bytes_to_do;
			}
			dest += SCREENHEIGHT - height;
		}
	}
}




/* V_Init */
void V_Init (void)
{
	/* The first four framebuffers used to be allocated here. */
}
