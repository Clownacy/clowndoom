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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "ib_system.h"

#include "d_net.h"
#include "g_game.h"

#include "i_system.h"




void
I_Tactile
( int   on,
  int   off,
  int   total )
{
  /* UNUSED. */
  (void)on;
  (void)off;
  (void)total;
}

static ticcmd_t emptycmd;
ticcmd_t*       I_BaseTiccmd(void)
{
	return &emptycmd;
}


unsigned char* I_ZoneBase (size_t* size)
{
	/* According to PCDOOM, the DOS version of Doom would use between 1.5MiB and 8MiB of zone memory, depending on what was available. */
	static unsigned char buffer[6 * 1024 * 1024]; /* 6MiB of zone memory! */

	*size = sizeof(buffer);
	return buffer;
}



/* I_GetTime */
/* returns time in 1/70th second tics */
int  I_GetTime (void)
{
	return IB_GetTime();
}



/* I_Init */
void I_Init (void)
{
	IB_Init();

	I_StartupSound();
	I_InitGraphics();
}

/* I_Quit */
void I_Quit (void)
{
	D_QuitNetGame ();
	I_ShutdownSound();
	M_SaveDefaults ();
	I_ShutdownGraphics();

	IB_Quit();

	exit(0);
}

void I_WaitVBL(int count)
{
	IB_WaitVBL(count);
}

void I_BeginRead(void)
{
	/* This used to be for drawing the disk icon - see the Hexen source code */
}

void I_EndRead(void)
{
}


/* I_Error */
extern d_bool demorecording;

void I_Error (const char *error, ...)
{
	va_list     argptr;

	/* Message first. */
	va_start (argptr,error);
	fputs ("Error: ", stderr);
	vfprintf (stderr,error,argptr);
	fputc ('\n', stderr);
	va_end (argptr);

	fflush( stderr );

	/* Shutdown. Here might be other errors. */
	if (demorecording)
		G_CheckDemoStatus();

	D_QuitNetGame ();
	I_ShutdownGraphics();

	exit(-1);
}

void I_Info (const char *error, ...)
{
	va_list     argptr;

	va_start (argptr,error);
	vfprintf (stderr,error,argptr);
	va_end (argptr);

	fflush( stderr );
}

void I_Sleep(void)
{
	IB_Sleep();
}
