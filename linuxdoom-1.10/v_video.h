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
        Gamma correction LUT.
        Functions to draw patches (by post) directly to screen.
        Functions to blit a block to the screen.

******************************************************************************/


#ifndef __V_VIDEO__
#define __V_VIDEO__

#include <string.h>

#include "doomtype.h"

#include "doomdef.h"

/* Needed because we are refering to patches. */
#include "r_data.h"

/* VIDEO */

#define CENTERY                 (SCREENHEIGHT/2)


/* Screen 0 is the screen updated by I_Update screen. */
/* Screen 1 is an extra buffer. */

typedef enum screen_t
{
    SCREEN_FRAMEBUFFER,
    SCREEN_BACK,
    SCREEN_WIPE_START,
    SCREEN_WIPE_END,
    SCREEN_STATUS_BAR
} screen_t;

extern  unsigned char screens[5][SCREENWIDTH * SCREENHEIGHT];

extern  const unsigned char      gammatable[5][256];
extern  int     usegamma;



/* Allocates buffer screens, call before R_Init. */
void V_Init (void);


void
V_CopyRect
( int           srcx,
  int           srcy,
  screen_t      srcscrn,
  int           width,
  int           height,
  int           destx,
  int           desty,
  screen_t      destscrn );

void
V_DrawPatchColumnInternal
(unsigned char* desttop,
    const patch_t* patch,
    int            col);

#define V_DrawPatchColumn(x, y, scrn, patch, col) V_DrawPatchColumnInternal(&screens[scrn][(y)+(x)*SCREENHEIGHT], patch, col)

void
V_DrawPatchFlipped
( int            x,
  int            y,
  screen_t       scrn,
  const patch_t* patch,
  d_bool         flip );

/* V_DrawPatch */
/* Masks a column based masked pic to the screen. */
#define V_DrawPatch(x, y, scrn, patch) V_DrawPatchFlipped(x, y, scrn, patch, d_false)

void
V_FillScreenWithPattern
( const char *lump_name,
  screen_t    screen,
  int         height );

#define V_ClearScreen(scrn) if (SCREENWIDTH != ORIGINAL_SCREEN_WIDTH * HUD_SCALE || SCREENHEIGHT != ORIGINAL_SCREEN_HEIGHT * HUD_SCALE) memset(screens[scrn], 0, SCREENWIDTH*SCREENHEIGHT);

/* Draw a linear block of pixels into the view buffer. */
void
V_DrawBlock
( int           x,
  int           y,
  screen_t      scrn,
  int           width,
  int           height,
  const unsigned char*           src );

/* Reads a linear block of pixels into the view buffer. */
void
V_GetBlock
( int           x,
  int           y,
  screen_t      scrn,
  int           width,
  int           height,
  unsigned char*         dest );

#endif
