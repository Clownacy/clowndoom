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
        System specific interface stuff.

******************************************************************************/


#ifndef __IB_FILE__
#define __IB_FILE__

#include <stddef.h>

#include "doomtype.h"

typedef enum I_FileMode
{
	I_FILE_MODE_READ,
	I_FILE_MODE_WRITE
} I_FileMode;

typedef enum I_FilePosition
{
	I_FILE_POSITION_START,
	I_FILE_POSITION_CURRENT,
	I_FILE_POSITION_END
} I_FilePosition;

I_File* I_FileOpen(const char *path, I_FileMode mode);
void I_FileClose(I_File *file);
size_t I_FileSize(I_File *file);
size_t I_FileRead(I_File *file, void *buffer, size_t size);
size_t I_FileWrite(I_File *file, const void *buffer, size_t size);
cc_bool I_FilePut(I_File *file, char character);
cc_bool I_FileSeek(I_File *file, size_t offset, I_FilePosition position);

#endif
