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

#define MAX_PALETTES (32*8)

int aspect_ratio_correction;

static unsigned char *colors;

static size_t bytes_per_pixel;

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
	size_t x,y;
	unsigned char *pixels;
	size_t pitch;

	colourindex_t* const indexed_pixels = screens[SCREEN_FRAMEBUFFER];
	const colourindex_t *indexed_pixels_pointer;

	/* draws little dots on the bottom of the screen */
	if (devparm)
	{
		int i, x, y;

		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20) tics = 20;

		for (i=0 ; i<tics*2*HUD_SCALE ; i+=2*HUD_SCALE)
			for (x=0 ; x<HUD_SCALE ; ++x)
				for (y=0 ; y<HUD_SCALE ; ++y)
					indexed_pixels[(i+x)*SCREENHEIGHT + SCREENHEIGHT-1-y] = 0xff;
		for ( ; i<20*2*HUD_SCALE ; i+=2*HUD_SCALE)
			for (x=0 ; x<HUD_SCALE ; ++x)
				for (y=0 ; y<HUD_SCALE ; ++y)
					indexed_pixels[(i+x)*SCREENHEIGHT + SCREENHEIGHT-1-y] = 0x0;
	}

	/* Colour, rotate, and scale the screen. */
	IB_GetFramebuffer(&pixels, &pitch);

	pixels += offset_x * bytes_per_pixel;
	pixels += offset_y * pitch;

	indexed_pixels_pointer = indexed_pixels;

	for (y = 0; y < output_height; ++y)
	{
		if (upscale_y_deltas[y] != 0)
		{
			const colourindex_t *input_line_pointer = indexed_pixels_pointer;
			unsigned char *upscale_line_buffer_pointer = &pixels[y * pitch];

			indexed_pixels_pointer += upscale_y_deltas[y];

			for (x = 0; x < output_width; ++x)
			{
				const unsigned char *colour_pointer = &colors[*input_line_pointer * bytes_per_pixel];

				input_line_pointer += upscale_x_deltas[x] * SCREENHEIGHT;

				memcpy(upscale_line_buffer_pointer, colour_pointer, bytes_per_pixel);
				upscale_line_buffer_pointer += bytes_per_pixel;
			}
		}
		else
		{
			memcpy(&pixels[y * pitch], &pixels[(y-1) * pitch], output_width * bytes_per_pixel);
		}
	}

	/* Display the screen. */
	IB_FinishUpdate();
}


/* I_ReadScreen */
void I_ReadScreen (colourindex_t* scr)
{
	memcpy(scr, screens[SCREEN_FRAMEBUFFER], SCREENWIDTH*SCREENHEIGHT*sizeof(colourindex_t));
}


/* I_SetPalette */
void I_SetPalette (unsigned char(* const palette)[0x100][3], const size_t total_palettes)
{
	size_t i;
	const unsigned char* const gamma = gammatable[usegamma];

#ifdef RANGECHECK
	if (total_palettes > MAX_PALETTES)
		I_Error("Bad I_SetPalette");
#endif

	for (i = 0; i < total_palettes; ++i)
	{
		size_t j;

		for (j = 0; j < D_COUNT_OF(palette[i]); ++j)
		{
			IB_GetColor(&colors[(i * 0x100 + j) * bytes_per_pixel],
				gamma[palette[i][j][0]],
				gamma[palette[i][j][1]],
				gamma[palette[i][j][2]]);
		}
	}
}


static void OutputSizeChanged(const size_t width, const size_t height)
{
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

	I_RenderSizeChanged();
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

	IB_InitGraphics("clowndoom", 640, 480, &bytes_per_pixel, OutputSizeChanged);

	I_GrabMouse(d_true);

	/* TODO - handle failed allocations */
	colors = (unsigned char*)malloc(MAX_PALETTES * 0x100 * bytes_per_pixel);
}


void I_ShutdownGraphics(void)
{
	free(colors);

	free(upscale_x_deltas);
	free(upscale_y_deltas);

	IB_ShutdownGraphics();
}


void I_GrabMouse(d_bool grab)
{
	IB_GrabMouse(grab);
}

void I_ToggleFullscreen(void)
{
	IB_ToggleFullscreen();
}

void I_RenderSizeChanged(void)
{
	size_t last, i;

	/* Create LUTs for the upscaler */
	free(upscale_x_deltas);
	free(upscale_y_deltas);
	upscale_x_deltas = (unsigned char*)malloc(output_width);
	upscale_y_deltas = (unsigned char*)malloc(output_height);

	for (last = 0, i = 0; i < output_height; ++i)
	{
		const size_t current = i * SCREENHEIGHT / output_height;

		upscale_y_deltas[i] = current - last;

		last = current;
	}

	upscale_y_deltas[0] = 1;    /* Force a redraw on the first line */

	for (last = 0, i = 0; i < output_width; ++i)
	{
		const size_t current = i * SCREENWIDTH / output_width;

		upscale_x_deltas[i] = current - last;

		last = current;
	}

	upscale_x_deltas[0] = 1;    /* Force a redraw on the first pixel */
}
