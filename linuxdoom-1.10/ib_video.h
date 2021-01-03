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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __IB_VIDEO__
#define __IB_VIDEO__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

void IB_ShutdownGraphics(void);
void IB_StartTic (void);
void IB_FinishUpdate (void);
void IB_SetPalette (const unsigned char* palette);
void IB_InitGraphics(void);
void IB_GrabMouse(boolean);


#endif
