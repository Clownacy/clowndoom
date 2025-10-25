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
        System specific interface stuff.

******************************************************************************/


#ifndef __IB_VIDEO__
#define __IB_VIDEO__

#include <stddef.h>

#include "doomtype.h"

typedef void (*IB_OutputSizeChangedCallback)(size_t width, size_t height, d_bool aspect_ratio_correction);

void IB_StartTic (void);
void IB_GetFramebuffer(unsigned char **pixels, size_t *pitch);
void IB_FinishUpdate (void);
void IB_GetColor(unsigned char *bytes, unsigned char red, unsigned char green, unsigned char blue);
void IB_InitGraphics(const char *title, size_t screen_width, size_t screen_height, size_t *bytes_per_pixel, IB_OutputSizeChangedCallback output_size_changed_callback);
void IB_ShutdownGraphics(void);
void IB_GrabMouse(d_bool grab);
void IB_ToggleFullscreen(void);


#endif
