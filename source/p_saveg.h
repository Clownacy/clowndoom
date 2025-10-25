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
        Savegame I/O, archiving, persistence.

******************************************************************************/


#ifndef __P_SAVEG__
#define __P_SAVEG__

#include <stddef.h>

/* Persistent storage/archiving. */
/* These are the load / save game routines. */
size_t P_ArchivePlayers (unsigned char* const buffer, size_t index);
size_t P_UnArchivePlayers (const unsigned char* const buffer, size_t index);
size_t P_ArchiveWorld (unsigned char* const buffer, size_t index);
size_t P_UnArchiveWorld (const unsigned char* const buffer, size_t index);
size_t P_ArchiveThinkers (unsigned char* const buffer, size_t index);
size_t P_UnArchiveThinkers (const unsigned char* const buffer, size_t index);
size_t P_ArchiveSpecials (unsigned char* const buffer, size_t index);
size_t P_UnArchiveSpecials (const unsigned char* const buffer, size_t index);

#endif
