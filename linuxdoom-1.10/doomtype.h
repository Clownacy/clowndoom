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
        Simple basic typedefs, isolated here to make it easier
         separating modules.

******************************************************************************/


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#define D_MIN(a, b) ((a) < (b) ? (a) : (b))
#define D_MAX(a, b) ((a) > (b) ? (a) : (b))
#define D_CLAMP(min, max, value) D_MAX(min, D_MIN(max, value))
#define D_COUNT_OF(x) (sizeof(x) / sizeof(*x))

enum {d_false, d_true};
typedef unsigned int d_bool;

/* An indexed pixel. Currently `short` to enable true-colour rendering. */
typedef unsigned short colourindex_t;

#endif
