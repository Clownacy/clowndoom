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

#include <string.h>

#include "doomstat.h"
#include "i_system.h"
#include "ib_video.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#include "i_video.h"


typedef struct Color8Bit
{
    unsigned char x[1];
} Color8Bit;

typedef struct Color16Bit
{
    unsigned char x[2];
} Color16Bit;

typedef struct Color24Bit
{
    unsigned char x[3];
} Color24Bit;

typedef struct Color32Bit
{
    unsigned char x[4];
} Color32Bit;

static union
{
    Color8Bit Bit8[256];
    Color16Bit Bit16[256];
    Color24Bit Bit24[256];
    Color32Bit Bit32[256];
} colors;

static unsigned int bytes_per_pixel;

static unsigned char colored_screen[SCREENWIDTH * SCREENHEIGHT * 4];

#ifdef SCALER
static unsigned char **upscale_lut;
#endif

static size_t output_width;
static size_t output_height;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


void I_ShutdownGraphics(void)
{
    IB_ShutdownGraphics();
}


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
    Color8Bit *colored_screen_8bit = (Color8Bit*)colored_screen;
    Color16Bit *colored_screen_16bit = (Color16Bit*)colored_screen;
    Color24Bit *colored_screen_24bit = (Color24Bit*)colored_screen;
    Color32Bit *colored_screen_32bit = (Color32Bit*)colored_screen;

    switch (bytes_per_pixel)
    {
	case 1:
	    for (size_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
		*colored_screen_8bit++ = colors.Bit8[*indexed_pixels++];

	    break;

	case 2:
	    for (size_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
		*colored_screen_16bit++ = colors.Bit16[*indexed_pixels++];

	    break;

	case 3:
	    for (size_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
		*colored_screen_24bit++ = colors.Bit24[*indexed_pixels++];

	    break;

	case 4:
	    for (size_t i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
		*colored_screen_32bit++ = colors.Bit32[*indexed_pixels++];

	    break;
    }

    unsigned char *pixels;
    size_t pitch;
    IB_GetFramebuffer(&pixels, &pitch);

    for (size_t y = 0; y < output_height; ++y)
	memcpy(&pixels[y * pitch], &colored_screen[y * SCREENWIDTH * bytes_per_pixel], SCREENWIDTH * bytes_per_pixel);
/*
#ifdef SCALER
    unsigned char **upscale_lut_pointer = upscale_lut;

    for (size_t y = 0; y < output_height; ++y)
    {
	unsigned char *dst_row = &pixels[y * pitch];

	for (size_t x = 0; x < output_width; ++x)
	{
	    *dst_row++ = **upscale_lut_pointer++;
	}
    }
#else
    // scales the screen size before blitting it
    if (multiply == 1)
    {
	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	    memcpy(&pixels[y * pitch], &screens[0][y * SCREENWIDTH], SCREENWIDTH);
    }
    else
    {
	const unsigned char *src_pointer = screens[0];

	for (size_t y = 0; y < SCREENHEIGHT; ++y)
	{
	    unsigned char *dst_row = &pixels[y * multiply * pitch];
	    unsigned char *dst_pointer = dst_row;

	    for (size_t x = 0; x < SCREENWIDTH; ++x)
	    {
		const unsigned char pixel = *src_pointer++;

		for (int i = 0; i < multiply; ++i)
		    *dst_pointer++ = pixel;
	    }

	    for (int i = 1; i < multiply; ++i)
		memcpy(&dst_row[i * SCREENWIDTH * multiply], dst_row, SCREENWIDTH * multiply);
	}
    }
#endif
*/
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
	switch (bytes_per_pixel)
	{
	    case 1:
		IB_GetColor(colors.Bit8[i].x, gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
		break;

	    case 2:
		IB_GetColor(colors.Bit16[i].x, gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
		break;

	    case 3:
		IB_GetColor(colors.Bit24[i].x, gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
		break;

	    case 4:
		IB_GetColor(colors.Bit32[i].x, gamma[palette[0]], gamma[palette[1]], gamma[palette[2]]);
		break;

	}

	palette += 3;
    }
}


void I_InitGraphics(void)
{
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

#ifdef SCALER
    // Let's create the upscale LUT
    upscale_lut = malloc(output_width * output_height * sizeof(*upscale_lut));
    unsigned char **upscale_lut_pointer = upscale_lut;

    for (size_t y = 0; y < output_height; ++y)
	for (size_t x = 0; x < output_width; ++x)
	    *upscale_lut_pointer++ = &screens[0][((y * SCREENHEIGHT / output_height) * SCREENWIDTH) + (x * SCREENWIDTH / output_width)];
#endif

    IB_InitGraphics(output_width, output_height, &bytes_per_pixel);

    I_GrabMouse(true);
}


void I_GrabMouse(boolean grab)
{
    IB_GrabMouse(grab);
}
