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

#define MELT_SCALE (SCREENHEIGHT / ORIGINAL_SCREEN_HEIGHT)
#define PIXELS_PER_COLUMN (2 * MELT_SCALE)

/* when zero, stop the wipe */
static d_bool  go = 0;

static colourindex_t*    wipe_scr_start;
static colourindex_t*    wipe_scr_end;
static colourindex_t*    wipe_scr;


static int
wipe_initColorXForm
( int   ticks )
{
	(void)ticks;

	memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(colourindex_t));
	return 0;
}

static int
wipe_doColorXForm
( int   ticks )
{
	d_bool      changed;
	colourindex_t*       w;
	colourindex_t*       e;
	int         newval;

	changed = d_false;
	w = wipe_scr;
	e = wipe_scr_end;

	while (w!=wipe_scr+SCREENWIDTH*SCREENHEIGHT)
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


static int     y[MAXIMUM_SCREENWIDTH];

static int
wipe_initMelt
( int   ticks )
{
	int i, r;

	(void)ticks;

	/* copy start screen to main screen */
	memcpy(wipe_scr, wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(colourindex_t));

	/* setup initial column positions */
	/* (y<0 => not ready to scroll yet) */
	y[0] = -(M_Random()%16);
	for (i=1;i<SCREENWIDTH;i++)
	{
		r = (M_Random()%3)-(3/2);
		y[i] = y[i-1] + r;
		if (y[i] > 0) y[i] = 0;
		else if (y[i] == -16) y[i] = -(16-1);
	}
	for (i=0;i<SCREENWIDTH;i++)
		y[i] *= MELT_SCALE;

	return 0;
}

static int
wipe_doMelt
( int   ticks )
{
	int         i;
	int         j;

	d_bool      done = d_true;

	while (ticks--)
	{
		for (i=0;i<SCREENWIDTH/PIXELS_PER_COLUMN;i++)
		{
			if (y[i]<0)
			{
				y[i] += 1*MELT_SCALE; done = d_false;
			}
			else if (y[i] < SCREENHEIGHT)
			{
				int dy, y_offset_incremented;
				const int y_offset = y[i];

				dy = (y_offset < 16 * MELT_SCALE) ? y_offset + 1 * MELT_SCALE : 8 * MELT_SCALE;
				if (y_offset + dy >= SCREENHEIGHT) dy = SCREENHEIGHT - y[i];
				y_offset_incremented = y_offset + dy;

				for (j=0;j<PIXELS_PER_COLUMN;++j)
				{
					const int x_offset = (i * PIXELS_PER_COLUMN + j) * SCREENHEIGHT;

					memcpy(&wipe_scr[x_offset + y_offset], &wipe_scr_end[x_offset + y_offset], dy * sizeof(colourindex_t));
					memcpy(&wipe_scr[x_offset + y_offset_incremented], &wipe_scr_start[x_offset], (SCREENHEIGHT - y_offset_incremented) * sizeof(colourindex_t));
				}

				y[i] += dy;
				done = d_false;
			}
		}
	}

	return done;

}

void wipe_StartScreen(void)
{
	wipe_scr_start = screens[SCREEN_WIPE_START];
	I_ReadScreen(wipe_scr_start);
}

void wipe_EndScreen(void)
{
	wipe_scr_end = screens[SCREEN_WIPE_END];
	I_ReadScreen(wipe_scr_end);
	memcpy(screens[SCREEN_FRAMEBUFFER], wipe_scr_start, SCREENWIDTH*SCREENHEIGHT*sizeof(colourindex_t)); /* restore start scr. */
}

int
wipe_ScreenWipe
( int   wipeno,
  int   ticks )
{
	int rc;
	static int (* const wipes[][2])(int) =
	{
		{wipe_initColorXForm, wipe_doColorXForm},
		{wipe_initMelt, wipe_doMelt}
	};

	/* initial stuff */
	if (!go)
	{
		go = 1;
		/*wipe_scr = (lighttable_t*) Z_Malloc(SCREENWIDTH*SCREENHEIGHT*sizeof(lighttable_t), PU_STATIC, NULL);*/ /* DEBUG */
		wipe_scr = screens[SCREEN_FRAMEBUFFER];
		wipes[wipeno][0](ticks);
	}

	/* do a piece of wipe-in */
	rc = wipes[wipeno][1](ticks);
	/*V_DrawBlock(0, 0, SCREEN_FRAMEBUFFER, SCREENWIDTH, SCREENHEIGHT, wipe_scr);*/ /* DEBUG */

	/* final stuff */
	if (rc)
		go = 0;

	return !go;
}
