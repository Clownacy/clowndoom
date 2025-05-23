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
        System specific interface stuff.

******************************************************************************/


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include <stddef.h>

#include "d_ticcmd.h"
#include "d_event.h"


/* Called by DoomMain. */
void I_Init (void);

/* Called by startup code */
/* to get the ammount of memory to malloc */
/* for the zone management. */
unsigned char*   I_ZoneBase (size_t *size);


/* Called by D_DoomLoop, */
/* returns current time in tics. */
int I_GetTime (void);


/* Asynchronous interrupt functions should maintain private queues */
/* that are read by the synchronous functions */
/* to be converted into events. */

/* Either returns a null ticcmd, */
/* or calls a loadable driver to build it. */
/* This ticcmd will then be modified by the gameloop */
/* for normal input. */
ticcmd_t* I_BaseTiccmd (void);


/* Called by M_Responder when quit is selected. */
/* Clean exit, displays sell blurb. */
void I_Quit (void);


void I_Tactile (int on, int off, int total);


#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2))
__attribute__((format(printf, 1, 2)))
#endif
void I_Error (const char *error, ...);
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2))
__attribute__((format(printf, 1, 2)))
#endif
void I_Info(const char *error, ...);


void I_Sleep(void);

size_t I_GetConfigPath(char *buffer, size_t size);

void I_BeginRead (void);
void I_EndRead (void);


#endif
