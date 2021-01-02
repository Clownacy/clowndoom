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
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#include "SDL.h"

#include "../doomdef.h"
#include "../m_misc.h"
#include "../i_video.h"
#include "../i_sound.h"

#include "../d_net.h"
#include "../g_game.h"

#ifdef __GNUG__
#pragma implementation "../i_system.h"
#endif
#include "../i_system.h"




static size_t	mb_used = 6;


void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  (void)on;
  (void)off;
  (void)total;
}

static ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


size_t  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (size_t*	size)
{
    *size = mb_used*1024*1024;
    return (byte *) malloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
    return SDL_GetTicks() * TICRATE / 1000;
}



//
// I_Init
//
void I_Init (void)
{
    I_StartupSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    exit(0);
}

void I_WaitVBL(int count)
{
    SDL_Delay(count * 1000 / 70);
}

void I_BeginRead(void)
{
    // This used to be for drawing the disk icon - see the Hexen source code
}

void I_EndRead(void)
{
}

byte*	I_AllocLow(size_t length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (const char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
    
    exit(-1);
}

void I_Sleep(void)
{
    SDL_Delay(1);
}
