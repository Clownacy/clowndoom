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
        Rendering of moving objects, sprites.

******************************************************************************/


#ifndef __R_THINGS__
#define __R_THINGS__


#define MAXVISSPRITES   4096 /* Formerly 128, but Master Levels' Bad Dream level hits this limit. */

/* Constant arrays used for psprite clipping */
/*  and initializing clipping. */
extern int              negonearray[MAXIMUM_SCREENWIDTH];
extern int              screenheightarray[MAXIMUM_SCREENWIDTH];

/* vars for R_DrawMaskedColumn */
extern int*             mfloorclip;
extern int*             mceilingclip;
extern fixed_t          spryscale;
extern fixed_t          sprtopscreen;

extern fixed_t          pspritescale;
extern fixed_t          pspriteiscale;


void R_DrawMaskedColumn (column_t* column);


void R_AddSprites (sector_t* sec);
void R_AddPSprites (void);
void R_DrawSprites (void);
void R_InitSprites (const char* const *namelist);
void R_ClearSprites (void);
void R_DrawMasked (void);

void
R_ClipVisSprite
( vissprite_t*          vis,
  int                   xl,
  int                   xh );


#endif
