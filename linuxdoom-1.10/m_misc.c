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
//
// $Log:$
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

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

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int
M_DrawText
( int		x,
  int		y,
  boolean	direct,
  char*		string )
{
    int 	c;
    int		w;

    while (*string)
    {
	c = toupper(*string) - HU_FONTSTART;
	string++;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    x += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (x+w > SCREENWIDTH)
	    break;
	if (direct)
	    V_DrawPatchDirect(x, y, 0, hu_font[c]);
	else
	    V_DrawPatch(x, y, 0, hu_font[c]);
	x+=w;
    }

    return x;
}




//
// M_WriteFile
//
boolean
M_WriteFile
( char const*	name,
  void*		source,
  int		length )
{
    FILE*	handle;
    int		count;
	
    handle = fopen ( name, "wb");

    if (handle == NULL)
	return false;

    count = fwrite (source, 1, length, handle);
    fclose (handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//
int
M_ReadFile
( char const*	name,
  byte**	buffer )
{
    FILE*	handle;
    long	count, length;
    byte		*buf;
	
    handle = fopen (name, "rb");
    if (handle != NULL)
    {
        fseek(handle, 0, SEEK_END);
        length = ftell(handle);
        if (length != -1)
        {
            buf = Z_Malloc (length, PU_STATIC, NULL);
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


//
// DEFAULTS
//
int		usemouse;
int		usejoystick;
int		bmp_screenshots;

extern int	key_right;
extern int	key_left;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;

extern int	mousebfire;
extern int	mousebstrafe;
extern int	mousebforward;

extern int	joybfire;
extern int	joybstrafe;
extern int	joybuse;
extern int	joybspeed;

extern int	viewwidth;
extern int	viewheight;

extern int	mouseSensitivity;
extern int	showMessages;

extern int	novert;
extern int	always_run;

extern int	detailLevel;

extern int	screenblocks;

extern int	aspect_ratio_correction;

extern int	showMessages;

// machine-independent sound params
extern	int	numChannels;

extern const char*	wildmidi_config_path;

extern const char*	chat_macros[];



typedef struct
{
    const char*	name;
    int*	location;
    size_t	defaultvalue;
    boolean	is_string;
//    int		scantranslate;		// PC scan code hack
//    int		untranslated;		// lousy hack
} default_t;

default_t	defaults[] =
{
    /* General */
    {"show_messages",&showMessages, 1, false},
    {"novert",&novert, 1, false},
    {"always_run",&always_run, 1, false},
    {"bmp_screenshots",&bmp_screenshots, 1, false},

    /* Keyboard */
    /* Movement */
    {"key_right",&key_right, ',', false},
    {"key_left",&key_left, '.', false},
    {"key_up",&key_up, 'w', false},
    {"key_down",&key_down, 's', false},
    {"key_strafeleft",&key_strafeleft, 'a', false},
    {"key_straferight",&key_straferight, 'd', false},
    /* Actions */
    {"key_fire",&key_fire, KEY_RCTRL, false},
    {"key_use",&key_use, ' ', false},
    {"key_strafe",&key_strafe, KEY_RALT, false},
    {"key_speed",&key_speed, KEY_RSHIFT, false},

    /* Mouse */
    {"use_mouse",&usemouse, 1, false},
    {"mouseb_fire",&mousebfire,0, false},
    {"mouseb_strafe",&mousebstrafe,1, false},
    {"mouseb_forward",&mousebforward,2, false},
    {"mouse_sensitivity",&mouseSensitivity, 5, false},

    /* Joystick */
    {"use_joystick",&usejoystick, 0, false},
    {"joyb_fire",&joybfire,0, false},
    {"joyb_strafe",&joybstrafe,1, false},
    {"joyb_use",&joybuse,3, false},
    {"joyb_speed",&joybspeed,2, false},

    /* Video */
    {"screenblocks",&screenblocks, 10, false},
    {"detaillevel",&detailLevel, 0, false},
    {"usegamma",&usegamma, 0, false},
    {"aspect_ratio_correction",&aspect_ratio_correction, 1, false},

    /* Audio */
    {"music_volume",&musicVolume, 8, false},
    {"sfx_volume",&sfxVolume, 8, false},
    {"snd_channels",&numChannels, 8, false},
    {"wildmidi_config_path", (int*)&wildmidi_config_path, (size_t)"wildmidi.cfg", true },

    /* Chat macros */
    {"chatmacro0", (int *) &chat_macros[0], (size_t) HUSTR_CHATMACRO0, true },
    {"chatmacro1", (int *) &chat_macros[1], (size_t) HUSTR_CHATMACRO1, true },
    {"chatmacro2", (int *) &chat_macros[2], (size_t) HUSTR_CHATMACRO2, true },
    {"chatmacro3", (int *) &chat_macros[3], (size_t) HUSTR_CHATMACRO3, true },
    {"chatmacro4", (int *) &chat_macros[4], (size_t) HUSTR_CHATMACRO4, true },
    {"chatmacro5", (int *) &chat_macros[5], (size_t) HUSTR_CHATMACRO5, true },
    {"chatmacro6", (int *) &chat_macros[6], (size_t) HUSTR_CHATMACRO6, true },
    {"chatmacro7", (int *) &chat_macros[7], (size_t) HUSTR_CHATMACRO7, true },
    {"chatmacro8", (int *) &chat_macros[8], (size_t) HUSTR_CHATMACRO8, true },
    {"chatmacro9", (int *) &chat_macros[9], (size_t) HUSTR_CHATMACRO9, true }
};

int	numdefaults;
char*	defaultfile;


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
    int		i;
    int		v;
    FILE*	f;
	
    f = fopen (defaultfile, "w");
    if (!f)
	return; // can't write the file, but don't complain
		
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


//
// M_LoadDefaults
//
extern byte	scantokey[128];

void M_LoadDefaults (void)
{
    int		i;
    int		len;
    FILE*	f;
    char	def[80];
    char	strparm[100];
    char*	newstring;
    int		parm;
    boolean	isstring;
    
    // set everything to base values
    numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i=0 ; i<numdefaults ; i++)
    {
	if (defaults[i].is_string)
	    *(const char**)defaults[i].location = (const char*)defaults[i].defaultvalue;
	else
	    *defaults[i].location = defaults[i].defaultvalue;
    }
    
    // check for a custom default file
    i = M_CheckParm ("-config");
    if (i && i<myargc-1)
    {
	defaultfile = myargv[i+1];
	printf ("	default file: %s\n",defaultfile);
    }
    else
	defaultfile = basedefault;
    
    // read the file in, overriding any set defaults
    f = fopen (defaultfile, "r");
    if (f)
    {
	while (!feof(f))
	{
	    isstring = false;
	    if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
	    {
		if (strparm[0] == '"')
		{
		    // get a string default
		    isstring = true;
		    len = strlen(strparm);
		    newstring = (char *) malloc(len);
		    strparm[len-1] = 0;
		    strcpy(newstring, strparm+1);
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


//
// SCREEN SHOTS
//


static void WriteLE(unsigned char *pointer, unsigned long value, unsigned int total_bytes)
{
    unsigned int i;

    for (i = 0; i < total_bytes; ++i)
        *pointer++ = (value >> (8 * i)) & 0xFF;
}

#define WriteU16LE(pointer, value) WriteLE(pointer, value, 2)
#define WriteU32LE(pointer, value) WriteLE(pointer, value, 4)


//
// WritePCXfile
//
void
WritePCXfile
( char*		filename,
  byte*		data,
  unsigned int	width,
  unsigned int	height,
  byte*		palette )
{
    int		i;
    size_t	length;
    byte*	pcx;
    byte*	pack;

    length = 0x80 + 1 + (1 << 8) * 3;

    pcx = Z_Malloc(length + width * height * 2, PU_STATIC, NULL);

    /* Manufacturer */
    pcx[0] = 0x0A;      /* PCX ID */
    /* Version */
    pcx[1] = 5;         /* 256 color */
    /* Encoding */
    pcx[2] = 1;         /* Uncompressed */
    /* Bits per pixel */
    pcx[3] = 8;         /* 256 colors */
    /* X minimum */
    WriteU16LE(&pcx[4], 0);
    /* Y minimum */
    WriteU16LE(&pcx[6], 0);
    /* X maximum */
    WriteU16LE(&pcx[8], width-1);
    /* Y maximum */
    WriteU16LE(&pcx[0xA], height-1);
    /* Horizontal resolution */
    WriteU16LE(&pcx[0xC], width);
    /* Vertical resolution */
    WriteU16LE(&pcx[0xE], height);
    /* Palette */
    memset(&pcx[0x10], 0, 0x30);
    /* Reserved */
    pcx[0x40] = 0;
    /* Color planes */
    pcx[0x41] = 1;      /* Chunky image */
    /* Bytes per line */
    WriteU16LE(&pcx[0x42], width);
    /* Palette type */
    WriteU16LE(&pcx[0x44], 2); /* Not a grey scale */
    /* Filler */
    memset(&pcx[0x46], 0, 0x3A);

    /* Pack the image */
    pack = &pcx[0x80];

    for (i = 0; i < width * height; ++i)
    {
        if ((*data & 0xC0) == 0xC0)
        {
            *pack++ = 0xC1;
            ++length;
        }

        *pack++ = *data++;
        ++length;
    }

    /* Write the palette */
    *pack++ = 0x0C; /* Palette ID byte */
    memcpy(pack, palette, (1 << 8) * 3);

    /* Write output file */
    M_WriteFile(filename, pcx, length);

    Z_Free(pcx);
}


//
// WriteBMPfile
//
void
WriteBMPfile
( char*		filename,
  byte*		data,
  unsigned int	width,
  unsigned int	height,
  byte*		palette )
{
    unsigned int i;
    byte *bmp, *bmp_pointer;

    const unsigned long rounded_width = (width + (4 - 1)) / 4 * 4; /* Pad width to a multiple of 4, as required by the BMP file format. */
    const unsigned long bitmap_offset = 0x1A + 0x100 * 3;
    const unsigned long length = bitmap_offset + rounded_width * height;

    bmp = Z_Malloc(length, PU_STATIC, NULL);

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
    WriteU16LE(&bmp[0x18], 8);

    /* Color table */

    bmp_pointer = &bmp[0x1A];

    for (i = 0; i < 1 << 8; ++i)
    {
        const byte blue  = *palette++;
        const byte green = *palette++;
        const byte red   = *palette++;

        *bmp_pointer++ = red;
        *bmp_pointer++ = green;
        *bmp_pointer++ = blue;
    }

    /* Pixel array */

    for (i = 0; i < height; ++i)
    {
        unsigned int j;

        bmp_pointer = &bmp[bitmap_offset + rounded_width * (height - i - 1)];

        memcpy(bmp_pointer, data, width);
        memset(bmp_pointer + width, 0, rounded_width - width);
    }

    /* Write output file */
    M_WriteFile(filename, bmp, length);

    Z_Free(bmp);
}


//
// M_ScreenShot
//
void M_ScreenShot (void)
{
    int		i;
    byte*	linear;
    char	lbmname[12];
    
    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen (linear);
    
    // find a file name to save it to
    strcpy(lbmname,"DOOM00.pcx");
    if (bmp_screenshots)
        strcpy(&lbmname[7],"bmp");
		
    for (i=0 ; i<=99 ; i++)
    {
	lbmname[4] = i/10 + '0';
	lbmname[5] = i%10 + '0';
	if (!M_FileExists(lbmname))
	    break;	// file doesn't exist
    }
    if (i==100)
	I_Error ("M_ScreenShot: Couldn't create a screenshot");
    
    // save the screenshot file
    (bmp_screenshots ? WriteBMPfile : WritePCXfile) (lbmname, linear,
		  SCREENWIDTH, SCREENHEIGHT,
		  W_CacheLumpName ("PLAYPAL",PU_CACHE));
	
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

boolean M_FileExists(const char* const filename)
{
    FILE* const file = fopen(filename, "rb");

    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }
}
