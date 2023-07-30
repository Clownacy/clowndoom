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

******************************************************************************/

#include "SDL.h"

#include "../doomdef.h"

#include "../ib_system.h"


/* IB_GetTime */
/* returns time in 1/70th second tics */
int  IB_GetTime (void)
{
	return SDL_GetTicks() * TICRATE / 1000;
}


/* IB_Init */
void IB_Init (void)
{
	SDL_Init(0);
}


/* IB_Quit */
void IB_Quit (void)
{
	SDL_Quit();
}


void IB_WaitVBL(int count)
{
	SDL_Delay(count * 1000 / 70);
}


void IB_Sleep(void)
{
	SDL_Delay(1);
}
