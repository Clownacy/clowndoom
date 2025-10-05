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

#include "../ib_system.h"

#include "../d_main.h"

int main(int argc, char **argv)
{
	D_DoomMain(argc, argv);
	return 0;
}


/* IB_GetTime */
/* returns time in 1/70th second tics */
int  IB_GetTime (void)
{
	return 0;
}


/* IB_Init */
void IB_Init (void)
{
	
}


/* IB_Quit */
void IB_Quit (void)
{
	
}


void IB_WaitVBL(int count)
{
	(void)count;
}


void IB_Sleep(void)
{
	
}


size_t IB_GetConfigPath(char* const buffer, const size_t size)
{
	(void)buffer;
	(void)size;

	return 0;
}
