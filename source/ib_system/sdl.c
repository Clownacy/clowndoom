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

******************************************************************************/

#include <stdlib.h>

#include "SDL.h"

#if SDL_MAJOR_VERSION >= 2
#include "SDL_main.h"
#endif

#include "../doomdef.h"
#include "../d_main.h"
#include "../m_misc.h"

#include "../ib_system.h"

int main(int argc, char **argv)
{
	D_DoomMain(argc, argv);
	return 0;
}

/* IB_GetTime */
/* returns time in 1/35th second tics */
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
void IB_Quit (int exit_code)
{
	SDL_Quit();
	exit(exit_code);
}


void IB_WaitFrames(int count)
{
	SDL_Delay(count * 1000 / TICRATE);
}


void IB_Sleep(void)
{
	SDL_Delay(1);
}


size_t IB_GetConfigPath(char* const buffer, const size_t size)
{
	char* const path = SDL_GetPrefPath("clownacy", "clowndoom");
	const size_t path_length = M_StringCopy(buffer, size, path);
	SDL_free(path);
	return path_length;
}
