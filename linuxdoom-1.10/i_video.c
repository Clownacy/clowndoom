// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

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

#define SCALER

static unsigned char *colors;

static unsigned int bytes_per_pixel;

static unsigned char *colored_screen;

#ifdef SCALER
static unsigned char *upscale_x_deltas;
static unsigned char *upscale_y_deltas;
#endif

static size_t output_width;
static size_t output_height;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?
}


//
// I_StartTic
//
void I_StartTic (void)
{
    IB_StartTic();
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}


//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    
    }

    // Step 1. Color the screen
    unsigned char *indexed_pixels = screens[0];
    unsigned char *colored_screen_pointer = colored_screen;

    for (size_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
    {
	unsigned char *color = &colors[*indexed_pixels++ * bytes_per_pixel];

	for (unsigned int j = 0; j < bytes_per_pixel; ++j)
	    *colored_screen_pointer++ = color[j];
    }

    unsigned char *pixels;
    size_t pitch;
    IB_GetFramebuffer(&pixels, &pitch);

//    for (size_t y = 0; y < SCREENHEIGHT; ++y)
//	memcpy(&pixels[y * pitch], &colored_screen[y * SCREENWIDTH * bytes_per_pixel], SCREENWIDTH * bytes_per_pixel);

#ifdef SCALER
    const unsigned char *src_pointer = colored_screen;
    for (size_t y = 0; y < output_height; ++y)
    {
	if (upscale_y_deltas[y])
	{
	    unsigned char *upscale_line_buffer_pointer = &pixels[y * pitch];

	    for (size_t x = 0; x < output_width; ++x)
	    {
		for (unsigned int i = 0; i < bytes_per_pixel; ++i)
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
#else
    // scales the screen size before blitting it
    if (multiply == 1)
    {
	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	    memcpy(&pixels[y * pitch], &colored_screen[y * SCREENWIDTH * bytes_per_pixel], SCREENWIDTH * bytes_per_pixel);
    }
    else
    {
	const unsigned char *src_pointer = colored_screen;

	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	{
	    unsigned char *dst_row = &pixels[y * multiply * pitch];
	    unsigned char *dst_pointer = dst_row;

	    for (size_t x = 0; x < SCREENWIDTH; ++x)
	    {
		for (int i = 0; i < multiply; ++i)
		    for (unsigned int j = 0; j < bytes_per_pixel; ++j)
			*dst_pointer++ = src_pointer[j];

		src_pointer += bytes_per_pixel;
	    }

	    for (int i = 1; i < multiply; ++i)
		memcpy(&dst_row[i * pitch], dst_row, output_width * bytes_per_pixel);
	}
    }
#endif

    IB_FinishUpdate();
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (const byte* palette)
{
    register int	i;
    const unsigned char* gamma = gammatable[usegamma];

    for (i=0 ; i<256 ; i++)
    {
	IB_GetColor(&colors[i * bytes_per_pixel], gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
	palette += 3;
    }
}


void I_InitGraphics(void)
{
    // TODO - get rid of this junk
    static int		firsttime=1;

    if (!firsttime)
	return;
    firsttime = 0;

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

    output_width = SCREENWIDTH * multiply;
#ifdef SCALER
    output_height = SCREENWIDTH * multiply * (3.0/4.0);
#else
    output_height = SCREENHEIGHT * multiply;
#endif

    IB_InitGraphics(output_width, output_height, &bytes_per_pixel);

    I_GrabMouse(true);

    // TODO - handle failed allocations
    colors = malloc(256 * bytes_per_pixel);
    colored_screen = malloc(SCREENWIDTH * SCREENHEIGHT * bytes_per_pixel);

#ifdef SCALER
    // Create LUTs for the upscaler
    upscale_x_deltas = malloc(output_width);
    upscale_y_deltas = malloc(output_height);

    for (size_t last = 0, i = 0; i < output_height; ++i)
    {
	size_t current = i * SCREENHEIGHT / output_height;

	upscale_y_deltas[i] = last != current;

	last = current;
    }

    upscale_y_deltas[0] = 1;	// Force a redraw on the first line

    for (size_t last = 0, i = 0; i < output_width; ++i)
    {
	size_t current = (i + 1) * SCREENWIDTH / output_width;	// The +1 here is deliberate, to avoid distortions at 320x240

	upscale_x_deltas[i] = last != current;

	last = current;
    }
#endif
}


void I_ShutdownGraphics(void)
{
    free(colors);
    free(colored_screen);

#ifdef SCALER
    free(upscale_x_deltas);
    free(upscale_y_deltas);
#endif

    IB_ShutdownGraphics();
}


void I_GrabMouse(boolean grab)
{
    IB_GrabMouse(grab);
}
