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


#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"



#define MAXWADFILES             20
extern const char*              wadfiles[MAXWADFILES];

void D_AddFile (const char *file);



/* D_DoomMain() */
/* Not a globally visible function, just included for source reference, */
/* calls all startup code, parses command line options. */
/* If not overrided by user input, calls N_AdvanceDemo. */
void D_DoomMain (void);

/* Called by IO functions when input is detected. */
void D_PostEvent (const event_t* ev);



/* BASE LEVEL */
void D_PageTicker (void);
void D_PageDrawer (void);
void D_AdvanceDemo (void);
void D_StartTitle (void);

#endif
