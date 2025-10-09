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
        DOOM graphics stuff for X11, UNIX.

******************************************************************************/

#include "../ib_video.h"

/* IB_StartTic */
void IB_StartTic (void)
{
	
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	(void)pixels;
	(void)pitch;
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	
}


void IB_GetColor(unsigned char *bytes, unsigned char red, unsigned char green, unsigned char blue)
{
	(void)bytes;
	(void)red;
	(void)green;
	(void)blue;
}


void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback_p)(size_t width, size_t height))
{
	(void)title;
	(void)screen_width;
	(void)screen_height;
	(void)bytes_per_pixel;
	(void)output_size_changed_callback_p;
}


void IB_ShutdownGraphics(void)
{
	
}


void IB_GrabMouse(d_bool grab)
{
	(void)grab;
}

void IB_ToggleFullscreen(void)
{
	
}
