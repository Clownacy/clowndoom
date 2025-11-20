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
        Cheat code checking.

******************************************************************************/


#ifndef __M_CHEAT__
#define __M_CHEAT__

/* CHEAT SEQUENCE PACKAGE */

#include "doomtype.h"

typedef struct
{
	char*      sequence;
	char*      p;

} cheatseq_t;

cc_bool
cht_CheckCheat
( cheatseq_t*           cht,
  char                  key );


void
cht_GetParam
( cheatseq_t*           cht,
  char*                 buffer );


#endif
