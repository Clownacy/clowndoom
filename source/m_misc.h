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


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
/* MISC */

extern int showMessages;
extern int novert;
extern int always_run;
extern int always_strafe;
extern int default_compatibility_level;

extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;

extern int key_strafeleft;
extern int key_straferight;

extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;
extern int mouseSensitivity;

extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;
extern int joybweaponprevious;
extern int joybweaponnext;

extern int screenblocks;
extern int detailLevel;
extern int usegamma;
extern int aspect_ratio_correction;
extern int full_colour;
extern int prototype_light_amplification_visor_effect;

extern int musicVolume;
extern int sfxVolume;
extern int numChannels;
extern const char *wildmidi_config_path;

extern const char *chat_macros[10];

d_bool
M_WriteFile
( char const*   name,
  void*         source,
  int           length );

int
M_ReadFile
( char const*   name,
  unsigned char**        buffer );

void M_ScreenShot (void);

void M_LoadDefaults (void);

void M_SaveDefaults (void);

void M_ChangedShowMessages (void);
void M_ChangedMouseSensitivity (void);
void M_ChangedGraphicDetail (void);
void M_ChangedScreenBlocks (void);
void M_ChangedUseGamma (void);
void M_ChangedAspectRatioCorrection (void);
void M_ChangedFullColour (void);
void M_ChangedPrototypeLightAmplificationVisorEffect (void);
void M_ChangedSFXVolume (void);
void M_ChangedMusicVolume (void);


int
M_DrawText
( int           x,
  int           y,
  char*         string );

int M_strncasecmp(const char *s1, const char *s2, size_t n);
#define M_strcasecmp(s1, s2) M_strncasecmp(s1, s2, (size_t)-1)

char* M_strupr(char *string);
char* M_strndup(const char *src, size_t size);
char* M_strdup(const char *src);
const char* M_basename(const char *path);
size_t M_StringCopy(char *dest, size_t destsz, const char *src);
size_t M_StringCopyOffset(char *dest, size_t destsz, size_t dest_offset, const char *src);

d_bool M_FileExists(const char* const filename);

int M_BytesToShort(const unsigned char * const data);
long M_BytesToLong(const unsigned char * const data);

const char* M_GetSaveFilePath(int i);

#endif
