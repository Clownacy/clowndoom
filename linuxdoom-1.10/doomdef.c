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
    DoomDef - basic defines for DOOM, e.g. Version, game mode
     and skill level, and display parameters.

******************************************************************************/


#include "doomdef.h"

/* Location for any defines turned variables. */

int resolution_scale = 2;

/* Try to keep this to powers of two to prevent rounding errors. */
int hud_scale = 2;
