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
        Mission begin melt/wipe screen special effect.

******************************************************************************/


#include <stdlib.h>

#include "z_zone.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"

#include "doomdef.h"

#include "f_wipe.h"

/*                       SCREEN WIPE PACKAGE */

#define PIXELS_PER_COLUMN (2 * SCREEN_MUL)

/* when zero, stop the wipe */
static d_bool  go = 0;

static unsigned char*    wipe_scr_start;
static unsigned char*    wipe_scr_end;
static unsigned char*    wipe_scr;


void
wipe_shittyColMajorXform
( unsigned char* array,
  int            width,
  int            height )
{
	int x,y,i;
	static unsigned char dest[SCREENWIDTH*SCREENHEIGHT];

	for(y=0;y<height;y++)
		for(x=0;x<width;x++)
			memcpy(&dest[(x*height+y)*PIXELS_PER_COLUMN], &array[(y*width+x)*PIXELS_PER_COLUMN], PIXELS_PER_COLUMN);

	memcpy(array, dest, width*height*PIXELS_PER_COLUMN);
}

int
wipe_initColorXForm
( int   width,
  int   height,
  int   ticks )
{
	(void)ticks;

	memcpy(wipe_scr, wipe_scr_start, width*height);
	return 0;
}

int
wipe_doColorXForm
( int   width,
  int   height,
  int   ticks )
{
	d_bool     changed;
	unsigned char*       w;
	unsigned char*       e;
	int         newval;

	changed = d_false;
	w = wipe_scr;
	e = wipe_scr_end;

	while (w!=wipe_scr+width*height)
	{
		if (*w != *e)
		{
			if (*w > *e)
			{
				newval = *w - ticks;
				if (newval < *e)
					*w = *e;
				else
					*w = newval;
				changed = d_true;
			}
			else if (*w < *e)
			{
				newval = *w + ticks;
				if (newval > *e)
					*w = *e;
				else
					*w = newval;
				changed = d_true;
			}
		}
		w++;
		e++;
	}

	return !changed;

}


static int     y[SCREENWIDTH];

int
wipe_initMelt
( int   width,
  int   height,
  int   ticks )
{
	int i, r;

	(void)ticks;

	/* copy start screen to main screen */
	memcpy(wipe_scr, wipe_scr_start, width*height);

	/* makes this wipe faster (in theory) */
	/* to have stuff in column-major format */
	wipe_shittyColMajorXform(wipe_scr_start, width/PIXELS_PER_COLUMN, height);
	wipe_shittyColMajorXform(wipe_scr_end, width/PIXELS_PER_COLUMN, height);

	/* setup initial column positions */
	/* (y<0 => not ready to scroll yet) */
	y[0] = -(M_Random()%16);
	for (i=1;i<width;i++)
	{
		r = (M_Random()%3)-(3/2);
		y[i] = y[i-1] + r;
		if (y[i] > 0) y[i] = 0;
		else if (y[i] == -16) y[i] = -(16-1);
	}
	for (i=0;i<width;i++)
		y[i] *= SCREEN_MUL;

	return 0;
}

int
wipe_doMelt
( int   width,
  int   height,
  int   ticks )
{
	int         i;
	int         j;
	int         dy;

	unsigned char *s, *d;
	d_bool     done = d_true;

	width/=PIXELS_PER_COLUMN;

	while (ticks--)
	{
		for (i=0;i<width;i++)
		{
			if (y[i]<0)
			{
				y[i] += 1*SCREEN_MUL; done = d_false;
			}
			else if (y[i] < height)
			{
				dy = (y[i] < 16*SCREEN_MUL) ? y[i]+1*SCREEN_MUL : 8*SCREEN_MUL;
				if (y[i]+dy >= height) dy = height - y[i];
				s = &wipe_scr_end[(i*height+y[i])*PIXELS_PER_COLUMN];
				d = &wipe_scr[(y[i]*width+i)*PIXELS_PER_COLUMN];
				for (j=dy;j;j--)
				{
					memcpy(d, s, PIXELS_PER_COLUMN);
					s += PIXELS_PER_COLUMN;
					d += width*PIXELS_PER_COLUMN;
				}
				y[i] += dy;
				s = &wipe_scr_start[(i*height)*PIXELS_PER_COLUMN];
				d = &wipe_scr[(y[i]*width+i)*PIXELS_PER_COLUMN];
				for (j=height-y[i];j;j--)
				{
					memcpy(d, s, PIXELS_PER_COLUMN);
					s += PIXELS_PER_COLUMN;
					d += width*PIXELS_PER_COLUMN;
				}
				done = d_false;
			}
		}
	}

	return done;

}

int
wipe_StartScreen
( int   x,
  int   y,
  int   width,
  int   height )
{
	(void)x;
	(void)y;
	(void)width;
	(void)height;

	wipe_scr_start = screens[2];
	I_ReadScreen(wipe_scr_start);
	return 0;
}

int
wipe_EndScreen
( int   x,
  int   y,
  int   width,
  int   height )
{
	wipe_scr_end = screens[3];
	I_ReadScreen(wipe_scr_end);
	V_DrawBlock(x, y, 0, width, height, wipe_scr_start); /* restore start scr. */
	return 0;
}

int
wipe_ScreenWipe
( int   wipeno,
  int   x,
  int   y,
  int   width,
  int   height,
  int   ticks )
{
	int rc;
	static const int (*wipes[][2])(int, int, int) =
	{
		{wipe_initColorXForm, wipe_doColorXForm},
		{wipe_initMelt, wipe_doMelt}
	};

	void V_MarkRect(int, int, int, int);

	(void)x;
	(void)y;

	/* initial stuff */
	if (!go)
	{
		go = 1;
		/*wipe_scr = (byte *) Z_Malloc(width*height, PU_STATIC, 0);*/ /* DEBUG */
		wipe_scr = screens[0];
		wipes[wipeno][0](width, height, ticks);
	}

	/* do a piece of wipe-in */
	V_MarkRect(0, 0, width, height);
	rc = wipes[wipeno][1](width, height, ticks);
	/*V_DrawBlock(x, y, 0, width, height, wipe_scr);*/ /* DEBUG */

	/* final stuff */
	if (rc)
		go = 0;

	return !go;

}
