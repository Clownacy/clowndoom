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
        The status bar widget code.

******************************************************************************/


#include <ctype.h>

#include "doomdef.h"
#include "doomstat.h"

#include "z_zone.h"
#include "v_video.h"

#include "m_swap.h"

#include "i_system.h"

#include "w_wad.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"


/* Hack display negative frags. */
/*  Loads and store the stminus lump. */
patch_t*                sttminus;

void STlib_init(void)
{
	sttminus = (patch_t *) W_CacheLumpName("STTMINUS", PU_STATIC);
}


/* ? */
void
STlib_initNum
( st_number_t*          n,
  int                   x,
  int                   y,
  patch_t**             pl,
  int*                  num,
  cc_bool*              on,
  int                   width )
{
	n->xRaw        = x;
	n->yRaw        = y;
	n->oldnum   = 0;
	n->width    = width;
	n->num      = num;
	n->on       = on;
	n->p        = pl;
}


/* A fairly efficient way to draw a number */
/*  based on differences from the old number. */
/* Note: worth the trouble? */
void
STlib_drawNum
( st_number_t*  n,
  cc_bool       refresh )
{

	int         numdigits = n->width;
	int         num = *n->num;

	int         w = SHORT(n->p[0]->width) * HUD_SCALE;
	int         h = SHORT(n->p[0]->height) * HUD_SCALE;
	int         x = X_CENTRE(n->xRaw);
	int         y = Y_BOTTOM(n->yRaw);

	int         neg;

	(void)refresh;

	n->oldnum = *n->num;

	neg = num < 0;

	if (neg)
	{
		if (numdigits == 2 && num < -9)
			num = -9;
		else if (numdigits == 3 && num < -99)
			num = -99;

		num = -num;
	}

	/* clear the area */
	x = X_CENTRE(n->xRaw) - numdigits*w;

	if (y - ST_Y < 0)
		I_Error("drawNum: n->y - ST_Y < 0");

	V_CopyRect(x, y - ST_Y, SCREEN_STATUS_BAR, w*numdigits, h, x, y, SCREEN_FRAMEBUFFER);

	/* if non-number, do not draw it */
	if (num == 1994)
		return;

	x = X_CENTRE(n->xRaw);

	/* in the special case of 0, you draw 0 */
	if (!num)
		V_DrawPatch(x - w, y, SCREEN_FRAMEBUFFER, n->p[ 0 ]);

	/* draw the new number */
	while (num && numdigits--)
	{
		x -= w;
		V_DrawPatch(x, y, SCREEN_FRAMEBUFFER, n->p[ num % 10 ]);
		num /= 10;
	}

	/* draw a minus sign if necessary */
	if (neg)
		V_DrawPatch(x - 8, y, SCREEN_FRAMEBUFFER, sttminus);
}


void
STlib_updateNum
( st_number_t*          n,
  cc_bool               refresh )
{
	if (*n->on) STlib_drawNum(n, refresh);
}


void
STlib_initPercent
( st_percent_t*         p,
  int                   x,
  int                   y,
  patch_t**             pl,
  int*                  num,
  cc_bool*              on,
  patch_t*              percent )
{
	STlib_initNum(&p->n, x, y, pl, num, on, 3);
	p->p = percent;
}




void
STlib_updatePercent
( st_percent_t*         per,
  int                   refresh )
{
	if (refresh && *per->n.on)
		V_DrawPatch(X_CENTRE(per->n.xRaw), Y_BOTTOM(per->n.yRaw), SCREEN_FRAMEBUFFER, per->p);

	STlib_updateNum(&per->n, refresh);
}



void
STlib_initMultIcon
( st_multicon_t*        i,
  int                   x,
  int                   y,
  patch_t**             il,
  int*                  inum,
  cc_bool*              on )
{
	i->xRaw     = x;
	i->yRaw     = y;
	i->oldinum  = -1;
	i->inum     = inum;
	i->on       = on;
	i->p        = il;
}



void
STlib_updateMultIcon
( st_multicon_t*        mi,
  cc_bool               refresh )
{
	int                 w;
	int                 h;
	int                 x;
	int                 y;

	if (*mi->on
		&& (mi->oldinum != *mi->inum || refresh)
		&& (*mi->inum!=-1))
	{
		if (mi->oldinum != -1)
		{
			x = X_CENTRE(mi->xRaw) - SHORT(mi->p[mi->oldinum]->leftoffset) * HUD_SCALE;
			y = Y_BOTTOM(mi->yRaw) - SHORT(mi->p[mi->oldinum]->topoffset) * HUD_SCALE;
			w = SHORT(mi->p[mi->oldinum]->width) * HUD_SCALE;
			h = SHORT(mi->p[mi->oldinum]->height) * HUD_SCALE;

			if (y - ST_Y < 0)
				I_Error("updateMultIcon: y - ST_Y < 0");

			V_CopyRect(x, y-ST_Y, SCREEN_STATUS_BAR, w, h, x, y, SCREEN_FRAMEBUFFER);
		}
		V_DrawPatch(X_CENTRE(mi->xRaw), Y_BOTTOM(mi->yRaw), SCREEN_FRAMEBUFFER, mi->p[*mi->inum]);
		mi->oldinum = *mi->inum;
	}
}



void
STlib_initBinIcon
( st_binicon_t*         b,
  int                   x,
  int                   y,
  patch_t*              i,
  cc_bool*              val,
  cc_bool*              on )
{
	b->xRaw     = x;
	b->yRaw     = y;
	b->oldval   = cc_false;
	b->val      = val;
	b->on       = on;
	b->p        = i;
}



void
STlib_updateBinIcon
( st_binicon_t*         bi,
  cc_bool               refresh )
{
	int                 x;
	int                 y;
	int                 w;
	int                 h;

	if (*bi->on
		&& (bi->oldval != *bi->val || refresh))
	{
		x = X_CENTRE(bi->xRaw) - SHORT(bi->p->leftoffset) * HUD_SCALE;
		y = Y_BOTTOM(bi->yRaw) - SHORT(bi->p->topoffset) * HUD_SCALE;
		w = SHORT(bi->p->width) * HUD_SCALE;
		h = SHORT(bi->p->height) * HUD_SCALE;

		if (y - ST_Y < 0)
			I_Error("updateBinIcon: y - ST_Y < 0");

		if (*bi->val)
			V_DrawPatch(X_CENTRE(bi->xRaw), Y_BOTTOM(bi->yRaw), SCREEN_FRAMEBUFFER, bi->p);
		else
			V_CopyRect(x, y-ST_Y, SCREEN_STATUS_BAR, w, h, x, y, SCREEN_FRAMEBUFFER);

		bi->oldval = *bi->val;
	}

}

