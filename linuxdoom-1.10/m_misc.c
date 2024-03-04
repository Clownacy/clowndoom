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
#include "v_video.h"

#include "hu_stuff.h"

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
	FILE*       handle;
	int         count;

	handle = fopen ( name, "wb");

	if (handle == NULL)
		return d_false;

	count = fwrite (source, 1, length, handle);
	fclose (handle);

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
	FILE*       handle;
	long        count, length;
	unsigned char                *buf;

	handle = fopen (name, "rb");
	if (handle != NULL)
	{
		fseek(handle, 0, SEEK_END);
		length = ftell(handle);
		if (length != -1)
		{
			buf = (unsigned char*)Z_Malloc (length, PU_STATIC, NULL);
			rewind(handle);
			count = fread (buf, 1, length, handle);
			fclose (handle);

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
int             full_colour;
int             prototype_light_amplification_visor_effect;

extern int      key_right;
extern int      key_left;
extern int      key_up;
extern int      key_down;

extern int      key_strafeleft;
extern int      key_straferight;

extern int      key_fire;
extern int      key_use;
extern int      key_strafe;
extern int      key_speed;

extern int      mousebfire;
extern int      mousebstrafe;
extern int      mousebforward;

extern int      joybfire;
extern int      joybstrafe;
extern int      joybuse;
extern int      joybspeed;
extern int      joybweaponprevious;
extern int      joybweaponnext;

extern int      viewwidth;
extern int      viewheight;

extern int      mouseSensitivity;
extern int      showMessages;

extern int      novert;
extern int      always_run;
extern int      always_strafe;

extern int      detailLevel;

extern int      screenblocks;

extern int      aspect_ratio_correction;

/* machine-independent sound params */
extern  int     numChannels;

extern const char*      wildmidi_config_path;

extern const char*      chat_macros[];



typedef struct
{
	const char* name;
	int*        location;
	size_t      defaultvalue;
	d_bool      is_string;
	/*int               scantranslate;*/        /* PC scan code hack */
	/*int               untranslated;*/         /* lousy hack */
} default_t;

static const default_t       defaults[] =
{
	/* General */
	{"show_messages",&showMessages, 1, d_false},
	{"novert",&novert, 1, d_false},
	{"always_run",&always_run, 1, d_false},
	{"always_strafe",&always_strafe, 1, d_false},

	/* Keyboard */
	/* Movement */
	{"key_right",&key_right, 'd', d_false},
	{"key_left",&key_left, 'a', d_false},
	{"key_up",&key_up, 'w', d_false},
	{"key_down",&key_down, 's', d_false},
	{"key_strafeleft",&key_strafeleft, ',', d_false},
	{"key_straferight",&key_straferight, '.', d_false},
	/* Actions */
	{"key_fire",&key_fire, KEY_RCTRL, d_false},
	{"key_use",&key_use, ' ', d_false},
	{"key_strafe",&key_strafe, KEY_RALT, d_false},
	{"key_speed",&key_speed, KEY_RSHIFT, d_false},

	/* Mouse */
	{"mouseb_fire",&mousebfire,0, d_false},
	{"mouseb_strafe",&mousebstrafe,1, d_false},
	{"mouseb_forward",&mousebforward,2, d_false},
	{"mouse_sensitivity",&mouseSensitivity, 5, d_false},

	/* Joystick */
	{"joyb_fire",&joybfire,0, d_false},
	{"joyb_strafe",&joybstrafe,1, d_false},
	{"joyb_use",&joybuse,3, d_false},
	{"joyb_speed",&joybspeed,2, d_false},
	{"joyb_weaponprevious",&joybweaponprevious,4, d_false},
	{"joyb_weaponnext",&joybweaponnext,5, d_false},

	/* Video */
	{"screenblocks",&screenblocks, 10, d_false},
	{"detaillevel",&detailLevel, 0, d_false},
	{"usegamma",&usegamma, 0, d_false},
	{"aspect_ratio_correction",&aspect_ratio_correction, 1, d_false},
	{"full_colour",&full_colour, 1, d_false},
	{"prototype_light_amplification_visor_effect",&prototype_light_amplification_visor_effect, 1, d_false},
	{"screen_width",&SCREENWIDTH, ORIGINAL_SCREEN_WIDTH, d_false},
	{"screen_height",&SCREENHEIGHT, ORIGINAL_SCREEN_HEIGHT, d_false},
	{"hud_scale",&HUD_SCALE, 1, d_false},

	/* Audio */
	{"music_volume",&musicVolume, 8, d_false},
	{"sfx_volume",&sfxVolume, 8, d_false},
	{"snd_channels",&numChannels, 8, d_false},
	{"wildmidi_config_path", (int*)&wildmidi_config_path, (size_t)"wildmidi.cfg", d_true },

	/* Chat macros */
	{"chatmacro0", (int *) &chat_macros[0], (size_t) HUSTR_CHATMACRO0, d_true },
	{"chatmacro1", (int *) &chat_macros[1], (size_t) HUSTR_CHATMACRO1, d_true },
	{"chatmacro2", (int *) &chat_macros[2], (size_t) HUSTR_CHATMACRO2, d_true },
	{"chatmacro3", (int *) &chat_macros[3], (size_t) HUSTR_CHATMACRO3, d_true },
	{"chatmacro4", (int *) &chat_macros[4], (size_t) HUSTR_CHATMACRO4, d_true },
	{"chatmacro5", (int *) &chat_macros[5], (size_t) HUSTR_CHATMACRO5, d_true },
	{"chatmacro6", (int *) &chat_macros[6], (size_t) HUSTR_CHATMACRO6, d_true },
	{"chatmacro7", (int *) &chat_macros[7], (size_t) HUSTR_CHATMACRO7, d_true },
	{"chatmacro8", (int *) &chat_macros[8], (size_t) HUSTR_CHATMACRO8, d_true },
	{"chatmacro9", (int *) &chat_macros[9], (size_t) HUSTR_CHATMACRO9, d_true }
};

int     numdefaults;
char*   defaultfile;


/* M_SaveDefaults */
void M_SaveDefaults (void)
{
	int         i;
	int         v;
	FILE*       f;

	f = fopen (defaultfile, "w");
	if (!f)
		return; /* can't write the file, but don't complain */

	for (i=0 ; i<numdefaults ; i++)
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
}


/* M_LoadDefaults */
void M_LoadDefaults (void)
{
	int         i;
	FILE*       f;
	char        def[80];
	char        strparm[100];
	char*       newstring;
	int         parm;
	d_bool      isstring;

	/* set everything to base values */
	numdefaults = sizeof(defaults)/sizeof(defaults[0]);
	for (i=0 ; i<numdefaults ; i++)
	{
		if (defaults[i].is_string)
			*(const char**)defaults[i].location = (const char*)defaults[i].defaultvalue;
		else
			*defaults[i].location = defaults[i].defaultvalue;
	}

	/* check for a custom default file */
	i = M_CheckParm ("-config");
	if (i && i<myargc-1)
	{
		defaultfile = myargv[i+1];
		I_Info ("       default file: %s\n",defaultfile);
	}
	else
		defaultfile = basedefault;

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
					sscanf(strparm, "%i", &parm);
				for (i=0 ; i<numdefaults ; i++)
					if (!strcmp(def, defaults[i].name))
					{
						if (!isstring)
							*defaults[i].location = parm;
						else
							*(const char**)defaults[i].location =
								newstring;
						break;
					}
			}
		}

		fclose (f);
	}
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
	unsigned char ((* palette)[0x100][3]);

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

d_bool M_FileExists(const char* const filename)
{
	FILE* const file = fopen(filename, "rb");

	if (file != NULL)
	{
		fclose(file);
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
