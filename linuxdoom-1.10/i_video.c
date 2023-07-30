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

#include <stdlib.h>
#include <string.h>

#include "doomstat.h"
#include "i_system.h"
#include "ib_video.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#include "i_video.h"

int aspect_ratio_correction;

static unsigned char *colors;

static size_t bytes_per_pixel;

static unsigned char *colored_screen;

static unsigned char *upscale_x_deltas;
static unsigned char *upscale_y_deltas;

static size_t output_width;
static size_t output_height;
static size_t offset_x, offset_y;


/* I_StartFrame */
void I_StartFrame (void)
{
	/* er? */
}


/* I_StartTic */
void I_StartTic (void)
{
	IB_StartTic();
}


/* I_UpdateNoBlit */
void I_UpdateNoBlit (void)
{
	/* what is this? */
}


/* I_FinishUpdate */
void I_FinishUpdate (void)
{
	static int lasttic;
	int tics;
	size_t i,x,y;
	unsigned char *indexed_pixels;
	unsigned char *colored_screen_pointer;
	unsigned char *pixels;
	size_t pitch;
	const unsigned char *src_pointer;

	/* draws little dots on the bottom of the screen */
	if (devparm)
	{
		int i,x,y;

		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20) tics = 20;

		for (i=0 ; i<tics*2*HUD_SCALE ; i+=2*HUD_SCALE)
			for (x=0 ; x<HUD_SCALE ; ++x)
				for (y=0 ; y<HUD_SCALE ; ++y)
					screens[SCREEN_FRAMEBUFFER][(i+x)*SCREENHEIGHT + SCREENHEIGHT-1-y] = 0xff;
		for ( ; i<20*2*HUD_SCALE ; i+=2*HUD_SCALE)
			for (x=0 ; x<HUD_SCALE ; ++x)
				for (y=0 ; y<HUD_SCALE ; ++y)
					screens[SCREEN_FRAMEBUFFER][(i+x)*SCREENHEIGHT + SCREENHEIGHT-1-y] = 0x0;
	}

	/* Step 1. Color the screen */
	indexed_pixels = screens[SCREEN_FRAMEBUFFER];
	colored_screen_pointer = colored_screen;

	for (y = 0; y < SCREENHEIGHT; ++y)
	{
		for (x = 0; x < SCREENWIDTH; ++x)
		{
			size_t j;

			const unsigned char * const color = &colors[indexed_pixels[x * SCREENHEIGHT + y] * bytes_per_pixel];

			for (j = 0; j < bytes_per_pixel; ++j)
				*colored_screen_pointer++ = color[j];
		}
	}

	/* Step 2. Scale the screen */
	IB_GetFramebuffer(&pixels, &pitch);

	pixels += offset_x * bytes_per_pixel;
	pixels += offset_y * pitch;

	src_pointer = colored_screen;
	for (y = 0; y < output_height; ++y)
	{
		if (upscale_y_deltas[y])
		{
			size_t x;

			unsigned char *upscale_line_buffer_pointer = &pixels[y * pitch];

			for (x = 0; x < output_width; ++x)
			{
				for (i = 0; i < bytes_per_pixel; ++i)
					*upscale_line_buffer_pointer++ = src_pointer[i];

				if (upscale_x_deltas[x])
					src_pointer += bytes_per_pixel;
			}
		}
		else
		{
			memcpy(&pixels[y * pitch], &pixels[(y-1) * pitch], output_width * bytes_per_pixel);
		}
	}

	/* Step 3. Display the screen */
	IB_FinishUpdate();
}


/* I_ReadScreen */
void I_ReadScreen (unsigned char* scr)
{
	memcpy(scr, screens[SCREEN_FRAMEBUFFER], SCREENWIDTH*SCREENHEIGHT);
}


/* I_SetPalette */
void I_SetPalette (const unsigned char* palette)
{
	register int        i;
	const unsigned char* gamma = gammatable[usegamma];

	for (i=0 ; i<256 ; i++)
	{
		IB_GetColor(&colors[i * bytes_per_pixel], gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
		palette += 3;
	}
}


static void OutputSizeChanged(const size_t width, const size_t height)
{
	size_t last, i;

	const size_t aspect_ratio_w = SCREENWIDTH;
	const size_t aspect_ratio_h = aspect_ratio_correction ? SCREENHEIGHT * 6 / 5 : SCREENHEIGHT;
	const size_t alternate_width = height * aspect_ratio_w / aspect_ratio_h;
	const size_t alternate_height = width * aspect_ratio_h / aspect_ratio_w;

	if (width >= alternate_width)
	{
		output_width = alternate_width;
		output_height = height;
	}
	else
	{
		output_width = width;
		output_height = alternate_height;
	}

	offset_x = (width - output_width) / 2;
	offset_y = (height - output_height) / 2;

	/* Create LUTs for the upscaler */
	upscale_x_deltas = (unsigned char*)realloc(upscale_x_deltas, output_width);
	upscale_y_deltas = (unsigned char*)realloc(upscale_y_deltas, output_height);

	for (last = 0, i = 0; i < output_height; ++i)
	{
		const size_t current = i * SCREENHEIGHT / output_height;

		upscale_y_deltas[i] = last != current;

		last = current;
	}

	upscale_y_deltas[0] = 1;    /* Force a redraw on the first line */

	for (last = 0, i = 0; i < output_width; ++i)
	{
		const size_t current = (i + 1) * SCREENWIDTH / output_width;    /* The +1 here is deliberate, to avoid distortion at 320x240 */

		upscale_x_deltas[i] = last != current;

		last = current;
	}
}


void I_InitGraphics(void)
{
	int multiply;

	multiply = 1;

	if (M_CheckParm("-2"))
		multiply = 2;

	if (M_CheckParm("-3"))
		multiply = 3;

	if (M_CheckParm("-4"))
		multiply = 4;

	output_width = SCREENWIDTH * multiply;

	if (aspect_ratio_correction)
		output_height = SCREENWIDTH * multiply * 3 / 4;
	else
		output_height = SCREENHEIGHT * multiply;

	IB_InitGraphics("clowndoom", output_width, output_height, &bytes_per_pixel, OutputSizeChanged);

	I_GrabMouse(d_true);

	/* TODO - handle failed allocations */
	colors = (unsigned char*)malloc(256 * bytes_per_pixel);
	colored_screen = (unsigned char*)malloc(SCREENWIDTH * SCREENHEIGHT * bytes_per_pixel);

	OutputSizeChanged(output_width, output_height);
}


void I_ShutdownGraphics(void)
{
	free(colors);
	free(colored_screen);

	free(upscale_x_deltas);
	free(upscale_y_deltas);

	IB_ShutdownGraphics();
}


void I_GrabMouse(d_bool grab)
{
	IB_GrabMouse(grab);
}
