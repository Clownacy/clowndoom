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
        System specific interface stuff.

******************************************************************************/


#ifndef __I_VIDEO__
#define __I_VIDEO__


#include "doomtype.h"


/* Called by D_DoomMain, */
/* determines the hardware configuration */
/* and sets up the video mode */
void I_InitGraphics (void);


void I_ShutdownGraphics(void);

/* Takes full 8 bit values. */
void I_SetPalette (unsigned char(* const palette)[0x100][3], const size_t total_palettes);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

/* Wait for vertical retrace or pause a bit. */
void I_WaitVBL(int count);

void I_ReadScreen (colourindex_t* scr);

/* Called by D_DoomLoop, */
/* called before processing any tics in a frame */
/* (just after displaying a frame). */
/* Time consuming syncronous operations */
/* are performed here (joystick reading). */
/* Can call D_PostEvent. */
void I_StartFrame (void);


/* Called by D_DoomLoop, */
/* called before processing each tic in a frame. */
/* Quick syncronous operations are performed here. */
/* Can call D_PostEvent. */
void I_StartTic (void);

void I_GrabMouse(d_bool grab);


#endif
