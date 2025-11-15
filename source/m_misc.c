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
        Main loop menu stuff.
        Default Config File.
        PCX Screenshots.

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <ctype.h>


#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "s_sound.h"
#include "v_video.h"

#include "hu_stuff.h"
#include "r_main.h"

/* State. */
#include "doomstat.h"

/* Data. */
#include "dstrings.h"

#include "m_misc.h"

/* M_DrawText */
/* Returns the final X coordinate */
/* HU_Init must have been called to init the font */
extern patch_t*         hu_font[HU_FONTSIZE];

int
M_DrawText
( int           x,
  int           y,
  char*         string )
{
	int         c;
	int         w;

	while (*string)
	{
		c = toupper(*string) - HU_FONTSTART;
		string++;
		if (c < 0 || c> HU_FONTSIZE)
		{
			x += 4*HUD_SCALE;
			continue;
		}

		w = SHORT (hu_font[c]->width)*HUD_SCALE;
		if (x+w > SCREENWIDTH)
			break;
		V_DrawPatch(x, y, SCREEN_FRAMEBUFFER, hu_font[c]);
		x+=w;
	}

	return x;
}




/* M_WriteFile */
d_bool
M_WriteFile
( char const*   name,
  void*         source,
  int           length )
{
	I_File*     handle;
	int         count;

	handle = I_FileOpen ( name, I_FILE_MODE_WRITE);

	if (handle == NULL)
		return d_false;

	count = I_FileWrite (handle, source, length);
	I_FileClose (handle);

	if (count < length)
		return d_false;

	return d_true;
}


/* M_ReadFile */
int
M_ReadFile
( char const*   name,
  unsigned char**        buffer )
{
	I_File*     handle;
	long        count, length;
	unsigned char                *buf;

	handle = I_FileOpen (name, I_FILE_MODE_READ);
	if (handle != NULL)
	{
		length = I_FileSize(handle);
		if (length != -1)
		{
			buf = (unsigned char*)Z_Malloc (length, PU_STATIC, NULL);
			count = I_FileRead (handle, buf, length);
			I_FileClose (handle);

			if (count == length)
			{
				*buffer = buf;
				return length;
			}
		}
	}

	I_Error ("Couldn't read file %s", name);
	return -1;
}


/* DEFAULTS */
/* Show messages has default, 0 = off, 1 = on */
int             showMessages = 1;

int             novert = 1;
int             always_run = 1;
int             always_strafe = 1;
int             default_compatibility_level = 3; /* Default to Ultimate Doom, since the Linux Doom source release is the closest to it. */

/* controls (have defaults) */
int             key_right = 'd';
int             key_left = 'a';
int             key_up = 'w';
int             key_down = 's';

int             key_strafeleft = ',';
int             key_straferight = '.';

int             key_fire = KEY_RCTRL;
int             key_use = ' ';
int             key_strafe = KEY_RALT;
int             key_speed = KEY_RSHIFT;

int             mousebfire = 0;
int             mousebstrafe = 1;
int             mousebforward = 2;
int             mouseSensitivity = 5;

int             joybfire = 11;
int             joybstrafe = 2;
int             joybuse = 3;
int             joybspeed = 10;
int             joybweaponprevious = 1;
int             joybweaponnext = 0;

int             screenblocks = 10;
/* Blocky mode, has default, 0 = high, 1 = normal */
int             detailLevel = 0;
int             usegamma = 0;
int             aspect_ratio_correction = 1;
int             full_colour = 0;
int             prototype_light_amplification_visor_effect = 0;

int             musicVolume = 8;
int             sfxVolume = 8;
int             numChannels = 8;
const char     *wildmidi_config_path = "wildmidi.cfg";

const char     *chat_macros[10] =
{
	HUSTR_CHATMACRO0,
	HUSTR_CHATMACRO1,
	HUSTR_CHATMACRO2,
	HUSTR_CHATMACRO3,
	HUSTR_CHATMACRO4,
	HUSTR_CHATMACRO5,
	HUSTR_CHATMACRO6,
	HUSTR_CHATMACRO7,
	HUSTR_CHATMACRO8,
	HUSTR_CHATMACRO9
};


#ifndef __LIBRETRO__
typedef struct
{
	const char* name;
	int*        location;
	d_bool      is_string;
	/*int               scantranslate;*/        /* PC scan code hack */
	/*int               untranslated;*/         /* lousy hack */
} default_t;

static const default_t       defaults[] =
{
	/* General */
	{"show_messages",&showMessages, d_false},
	{"novert",&novert, d_false},
	{"always_run",&always_run, d_false},
	{"always_strafe",&always_strafe, d_false},
	{"default_compatibility_level",&default_compatibility_level, d_false},

	/* Keyboard */
	/* Movement */
	{"key_right",&key_right, d_false},
	{"key_left",&key_left, d_false},
	{"key_up",&key_up, d_false},
	{"key_down",&key_down, d_false},
	{"key_strafeleft",&key_strafeleft, d_false},
	{"key_straferight",&key_straferight, d_false},
	/* Actions */
	{"key_fire",&key_fire, d_false},
	{"key_use",&key_use, d_false},
	{"key_strafe",&key_strafe, d_false},
	{"key_speed",&key_speed, d_false},

	/* Mouse */
	{"mouseb_fire",&mousebfire, d_false},
	{"mouseb_strafe",&mousebstrafe, d_false},
	{"mouseb_forward",&mousebforward, d_false},
	{"mouse_sensitivity",&mouseSensitivity, d_false},

	/* Joystick */
	{"joyb_fire",&joybfire, d_false},
	{"joyb_strafe",&joybstrafe, d_false},
	{"joyb_use",&joybuse, d_false},
	{"joyb_speed",&joybspeed, d_false},
	{"joyb_weaponprevious",&joybweaponprevious, d_false},
	{"joyb_weaponnext",&joybweaponnext, d_false},

	/* Video */
	{"screenblocks",&screenblocks, d_false},
	{"detaillevel",&detailLevel, d_false},
	{"usegamma",&usegamma, d_false},
	{"aspect_ratio_correction",&aspect_ratio_correction, d_false},
	{"full_colour",&full_colour, d_false},
	{"prototype_light_amplification_visor_effect",&prototype_light_amplification_visor_effect, d_false},
	{"screen_width",&SCREENWIDTH, d_false},
	{"screen_height",&SCREENHEIGHT, d_false},
	{"hud_scale",&HUD_SCALE, d_false},
	{"field_of_view",&field_of_view, d_false},

	/* Audio */
	{"music_volume",&musicVolume, d_false},
	{"sfx_volume",&sfxVolume, d_false},
	{"snd_channels",&numChannels, d_false},
	{"wildmidi_config_path", (int*)&wildmidi_config_path, d_true },

	/* Chat macros */
	{"chatmacro0", (int *) &chat_macros[0], d_true },
	{"chatmacro1", (int *) &chat_macros[1], d_true },
	{"chatmacro2", (int *) &chat_macros[2], d_true },
	{"chatmacro3", (int *) &chat_macros[3], d_true },
	{"chatmacro4", (int *) &chat_macros[4], d_true },
	{"chatmacro5", (int *) &chat_macros[5], d_true },
	{"chatmacro6", (int *) &chat_macros[6], d_true },
	{"chatmacro7", (int *) &chat_macros[7], d_true },
	{"chatmacro8", (int *) &chat_macros[8], d_true },
	{"chatmacro9", (int *) &chat_macros[9], d_true }
};
#endif

char*   defaultfile;


/* M_SaveDefaults */
void M_SaveDefaults (void)
{
#ifndef __LIBRETRO__
	int         i;
	int         v;
	FILE*       f;

	f = fopen (defaultfile, "w");
	if (!f)
		return; /* can't write the file, but don't complain */

	for (i=0 ; i<(int)D_COUNT_OF(defaults) ; i++)
	{
		if (!defaults[i].is_string)
		{
			v = *defaults[i].location;
			fprintf (f,"%s\t\t%i\n",defaults[i].name,v);
		} else {
			fprintf (f,"%s\t\t\"%s\"\n",defaults[i].name,
					 * (const char **) (defaults[i].location));
		}
	}

	fclose (f);
#endif
}


/* M_LoadDefaults */
void M_LoadDefaults (void)
{
#ifdef __LIBRETRO__
	size_t IB_GetSystemDirectoryPath(char *buffer, size_t size);
	static char wildmidi_config_path_buffer[0x100];
	const size_t copy_length = IB_GetSystemDirectoryPath(wildmidi_config_path_buffer, D_COUNT_OF(wildmidi_config_path_buffer));
	M_StringCopyOffset(wildmidi_config_path_buffer, D_COUNT_OF(wildmidi_config_path_buffer), copy_length, "wildmidi/timidity.cfg");
	wildmidi_config_path = wildmidi_config_path_buffer;
#else
	int         i;
	FILE*       f;
	char        def[80];
	char        strparm[100];
	char*       newstring;
	int         parm;
	d_bool      isstring;

	/* check for a custom default file */
	i = M_CheckParm ("-config");
	if (i && i<myargc-1)
	{
		defaultfile = myargv[i+1];
		I_Info ("       default file: %s\n",defaultfile);
	}
	else
	{
		defaultfile = basedefault;
	}

	/* read the file in, overriding any set defaults */
	f = fopen (defaultfile, "r");
	if (f)
	{
		while (!feof(f))
		{
			isstring = d_false;
			if (fscanf (f, "%79s %99[^\n]\n", def, strparm) == 2)
			{
				if (strparm[0] == '"')
				{
					/* get a string default */
					isstring = d_true;
					newstring = M_strndup(strparm+1, strlen(strparm+1)-1);
				}
				else
				{
					sscanf(strparm, "%i", &parm);
				}

				for (i=0 ; i<(int)D_COUNT_OF(defaults) ; i++)
				{
					if (!strcmp(def, defaults[i].name))
					{
						if (!isstring)
							*defaults[i].location = parm;
						else
							*(const char**)defaults[i].location = newstring;

						break;
					}
				}
			}
		}

		fclose (f);
	}
#endif

	if (complevel == -1)
		complevel = default_compatibility_level;

	/* Clamp options to sane values. */
	SCREENWIDTH  = D_CLAMP(ORIGINAL_SCREEN_WIDTH,  MAXIMUM_SCREENWIDTH,  SCREENWIDTH);
	SCREENHEIGHT = D_CLAMP(ORIGINAL_SCREEN_HEIGHT, MAXIMUM_SCREENHEIGHT, SCREENHEIGHT);
	HUD_SCALE = D_CLAMP(1, D_MIN(SCREENWIDTH / ORIGINAL_SCREEN_WIDTH, SCREENHEIGHT / ORIGINAL_SCREEN_HEIGHT), HUD_SCALE);
}

void M_ChangedShowMessages(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedShowMessages(void);
	IB_ChangedShowMessages();
#endif
}

void M_ChangedMouseSensitivity(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedMouseSensitivity(void);
	IB_ChangedMouseSensitivity();
#endif
}

void M_ChangedGraphicDetail(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedGraphicDetail(void);
	IB_ChangedGraphicDetail();
#endif

	R_SetViewSize (screenblocks, detailLevel, SCREENWIDTH, SCREENHEIGHT, HUD_SCALE);
}

void M_ChangedScreenBlocks(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedScreenBlocks(void);
	IB_ChangedScreenBlocks();
#endif

	R_SetViewSize (screenblocks, detailLevel, SCREENWIDTH, SCREENHEIGHT, HUD_SCALE);
}

void M_ChangedUseGamma(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedUseGamma(void);
	IB_ChangedUseGamma();
#endif

	V_SetPalette(0);
}

void M_ChangedAspectRatioCorrection(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedAspectRatioCorrection(void);
	IB_ChangedAspectRatioCorrection();
#endif

	V_ReloadPalette();
	R_InitColormaps();
}

void M_ChangedFullColour(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedFullColour(void);
	IB_ChangedFullColour();
#endif

	V_ReloadPalette();
	R_InitColormaps();
}

void M_ChangedPrototypeLightAmplificationVisorEffect(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedPrototypeLightAmplificationVisorEffect(void);
	IB_ChangedPrototypeLightAmplificationVisorEffect();
#endif

	R_InitColormaps();
}

void M_ChangedSFXVolume(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedSFXVolume(void);
	IB_ChangedSFXVolume();
#endif

	S_SetSfxVolume(sfxVolume);
}

void M_ChangedMusicVolume(void)
{
#ifdef __LIBRETRO__
	void IB_ChangedMusicVolume(void);
	IB_ChangedMusicVolume();
#endif

	S_SetMusicVolume(musicVolume);
}


/* SCREEN SHOTS */


static void WriteLE(unsigned char *pointer, unsigned long value, unsigned int total_bytes)
{
	unsigned int i;

	for (i = 0; i < total_bytes; ++i)
		*pointer++ = (value >> (8 * i)) & 0xFF;
}

#define WriteU16LE(pointer, value) WriteLE(pointer, value, 2)
#define WriteU32LE(pointer, value) WriteLE(pointer, value, 4)


/* WriteBMPfile */
static void
WriteBMPfile
( char*          filename,
  const colourindex_t* data,
  unsigned int   width,
  unsigned int   height,
  unsigned char  ((*palette)[3]) )
{
	unsigned int i;
	unsigned char *bmp, *bmp_pointer;

	const unsigned long rounded_width = (width * 3 + (4 - 1)) / 4 * 4; /* Pad width to a multiple of 4, as required by the BMP file format. */
	const unsigned long bitmap_offset = 0x1A;
	const unsigned long length = bitmap_offset + rounded_width * height;

	bmp = (unsigned char*)malloc(length);

	if (bmp != NULL)
	{
		/* BMP file header */

		/* Identifier */
		bmp[0] = 'B';
		bmp[1] = 'M';

		/* Size of file in bytes */
		WriteU32LE(&bmp[2], length);

		/* Reserved */
		WriteU16LE(&bmp[6], 0);

		/* Reserved */
		WriteU16LE(&bmp[8], 0);

		/* Offset of pixel array */
		WriteU32LE(&bmp[0xA], bitmap_offset);

		/* BITMAPCOREHEADER */

		/* Size of 'BITMAPCOREHEADER' */
		WriteU32LE(&bmp[0xE], 0x1A - 0xE);

		/* Width of bitmap in pixels */
		WriteU16LE(&bmp[0x12], width);

		/* Height of bitmap in pixels */
		WriteU16LE(&bmp[0x14], height);

		/* Number of color planes */
		WriteU16LE(&bmp[0x16], 1);

		/* Bits per pixel */
		WriteU16LE(&bmp[0x18], 24);

		/* Pixel array */
		bmp_pointer = &bmp[0x1A];

		for (i = 0; i < height; ++i)
		{
			unsigned int j;

			bmp_pointer = &bmp[bitmap_offset + rounded_width * (height - i - 1)];

			for (j = 0; j < width; ++j)
			{
				const colourindex_t pixel = data[i * width + j];

				*bmp_pointer++ = palette[pixel][2];
				*bmp_pointer++ = palette[pixel][1];
				*bmp_pointer++ = palette[pixel][0];
			}

			memset(bmp_pointer + width * 3, 0, rounded_width - width * 3);
		}

		/* Write output file */
		M_WriteFile(filename, bmp, length);

		free(bmp);
	}
}


/* M_ScreenShot */
void M_ScreenShot (void)
{
	int i;
	colourindex_t* linear;
	char lbmname[12];
	palette_t* palette;

	/* find a file name to save it to */
	strcpy(lbmname,"DOOM00.bmp");

	for (i=0 ; i<=99 ; i++)
	{
		lbmname[4] = i/10 + '0';
		lbmname[5] = i%10 + '0';
		if (!M_FileExists(lbmname))
			break;      /* file doesn't exist */
	}
	if (i==100)
		I_Error ("M_ScreenShot: Couldn't create a screenshot");

	/* rotate the framebuffer */
	linear = screens[SCREEN_WIPE_START];

	for (i = 0; i < SCREENWIDTH; ++i)
	{
		int j;

		for (j = 0; j < SCREENHEIGHT; ++j)
			linear[j * SCREENWIDTH + i] = screens[SCREEN_FRAMEBUFFER][i * SCREENHEIGHT + j];
	}

	palette = V_GetPalette(NULL);

	/* save the screenshot file */
	WriteBMPfile(lbmname, linear,
				  SCREENWIDTH, SCREENHEIGHT,
				  palette[0]);

	players[consoleplayer].message = "screen shot";
}

int M_strncasecmp(const char *s1, const char *s2, size_t n)
{
	int delta = 0;

	while (n-- != 0)
	{
		const int c1 = *s1++;
		const int c2 = *s2++;

		delta = toupper(c1) - toupper(c2);

		if (delta != 0 || c1 == '\0')
			break;
	}

	return delta;
}

char* M_strupr(char* const string)
{
	char *s;
	for (s = string; *s != '\0'; ++s)
		*s = toupper(*s);
	return string;
}

char* M_strndup(const char* const src, const size_t size)
{
	char* const copy = (char*)malloc(size + 1);

	if (copy != NULL)
	{
		memcpy(copy, src, size);
		copy[size] = '\0';
	}

	return copy;
}

char* M_strdup(const char* const src)
{
	return M_strndup(src, strlen(src));
}

static const char* M_basename_Internal(const char* const path, const char separator)
{
	const char* const found = strrchr(path, separator);

	if (found == NULL)
		return path;

	return found + 1;
}

const char* M_basename(const char* const path)
{
	const char *basename = path;

	basename = M_basename_Internal(basename, '/');

#ifdef _WIN32
	basename = M_basename_Internal(basename, '\\');
#endif

	return basename;
}

size_t M_StringCopy(char* const dest, const size_t destsz, const char* const src)
{
	size_t src_length, copy_length;

	if (destsz == 0)
		return 0;

	src_length = strlen(src);
	copy_length = D_MIN(src_length, destsz - 1);

	memcpy(dest, src, copy_length);
	dest[copy_length] = '\0';

	return copy_length;
}

size_t M_StringCopyOffset(char* const dest, const size_t destsz, const size_t dest_offset, const char* const src)
{
	return M_StringCopy(dest + dest_offset, destsz - dest_offset, src);
}

d_bool M_FileExists(const char* const filename)
{
	I_File* const file = I_FileOpen(filename, I_FILE_MODE_READ);

	if (file != NULL)
	{
		I_FileClose(file);
		return d_true;
	}
	else
	{
		return d_false;
	}
}

#define SIGN_EXTEND(type, bit, value) (((value) & (((type)1 << (bit)) - 1)) - (value & ((type)1 << (bit))))
#define SIGN_EXTEND_SHORT(value) SIGN_EXTEND(unsigned int,  16 - 1, value)
#define SIGN_EXTEND_LONG(value)  SIGN_EXTEND(unsigned long, 32 - 1, value)

int M_BytesToShort(const unsigned char * const data)
{
	unsigned int result;
	unsigned int i;

	result = 0;

	for (i = 0; i < 2; ++i)
		result |= (unsigned int)data[i] << (8 * i);

	return (int)SIGN_EXTEND_SHORT(result);
}

long M_BytesToLong(const unsigned char * const data)
{
	unsigned long result;
	unsigned int i;

	result = 0;

	for (i = 0; i < 4; ++i)
		result |= (unsigned long)data[i] << (8 * i);

	return (long)SIGN_EXTEND_LONG(result);
}

const char* M_GetSaveFilePath(const int i)
{
	static char name[0x100];

	sprintf(name,SAVEGAMENAME"%d.dsg",i);

	return name;
}
