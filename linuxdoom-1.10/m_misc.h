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


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
/* MISC */



d_bool
M_WriteFile
( char const*   name,
  void*         source,
  int           length );

int
M_ReadFile
( char const*   name,
  unsigned char**        buffer );

void
M_WriteBMPfile
( char*          filename,
  const colourindex_t* data,
  unsigned int   width,
  unsigned int   height );

void M_ScreenShot (void);

void M_LoadDefaults (void);

void M_SaveDefaults (void);


int
M_DrawText
( int           x,
  int           y,
  char*         string );

int M_strncasecmp(const char *s1, const char *s2, size_t n);
#define M_strcasecmp(s1, s2) M_strncasecmp(s1, s2, (size_t)-1)

char* M_strupr(char *string);
char* M_strndup(const char *src, const size_t size);
char* M_strdup(const char *src);

d_bool M_FileExists(const char* const filename);

int M_BytesToShort(const unsigned char * const data);
long M_BytesToLong(const unsigned char * const data);

#endif
