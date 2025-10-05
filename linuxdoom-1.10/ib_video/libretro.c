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

#include "../ib_system/libretro-callbacks.h"

#include "../doomdef.h"

typedef uint16_t Pixel;

static Pixel framebuffer[MAXIMUM_SCREENWIDTH * MAXIMUM_SCREENHEIGHT];

/* IB_StartTic */
void IB_StartTic (void)
{
	
}


void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch)
{
	*pixels = (unsigned char*)framebuffer;
	*pitch = MAXIMUM_SCREENWIDTH * sizeof(*framebuffer);
}


/* IB_FinishUpdate */
void IB_FinishUpdate (void)
{
	libretro.video(framebuffer, MAXIMUM_SCREENWIDTH, MAXIMUM_SCREENHEIGHT, MAXIMUM_SCREENWIDTH * sizeof(*framebuffer));
}


static unsigned int Get5BitColourChannel(const unsigned int channel)
{
	const unsigned int bits_per_source_colour = 8;
	const unsigned int bits_per_destination_colour = 5;
	const unsigned int colour_shift = bits_per_source_colour - bits_per_destination_colour;
	const unsigned int colour_mask = (1 << bits_per_destination_colour) - 1;

	return (channel >> colour_shift) & colour_mask;
}

void IB_GetColor(unsigned char* const bytes, const unsigned char red, const unsigned char green, const unsigned char blue)
{
	const unsigned int bits_per_source_colour = 8;
	const unsigned int bits_per_destination_colour = 5;
	const unsigned int colour_shift = bits_per_source_colour - bits_per_destination_colour;

	const Pixel pixel = (Get5BitColourChannel(red)   << (bits_per_destination_colour * 2))
	                  | (Get5BitColourChannel(green) << (bits_per_destination_colour * 1))
	                  | (Get5BitColourChannel(blue)  << (bits_per_destination_colour * 0));

	/* Do this trick to write the pixel in native-endian byte ordering, as required by the libretro API. */
	*(Pixel*)bytes = pixel;
}


void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, void (*output_size_changed_callback_p)(size_t width, size_t height))
{
	(void)title;
	(void)screen_width;
	(void)screen_height;
	(void)output_size_changed_callback_p;

	*bytes_per_pixel = sizeof(Pixel);
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
