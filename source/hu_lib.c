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

   DESCRIPTION:  heads-up text and input code

******************************************************************************/

#include <ctype.h>

#include "doomdef.h"
#include "doomstat.h"

#include "v_video.h"
#include "m_swap.h"

#include "hu_lib.h"
#include "r_local.h"
#include "r_draw.h"

/* boolean : whether the screen is always erased */
#define noterased viewwindowx

void HUlib_init(void)
{
}

void HUlib_clearTextLine(hu_textline_t* t)
{
	t->len = 0;
	t->l[0] = '\0';
	t->needsupdate = cc_true;
}

void
HUlib_initTextLine
( hu_textline_t*        t,
  int                   x,
  int                   y,
  patch_t**             f,
  int                   sc )
{
	t->x = x;
	t->y = y;
	t->f = f;
	t->sc = sc;
	HUlib_clearTextLine(t);
}

void
HUlib_positionTextLine
( hu_textline_t*        t,
  int                   x,
  int                   y )
{
	t->x = x;
	t->y = y;
}

cc_bool
HUlib_addCharToTextLine
( hu_textline_t*        t,
  char                  ch )
{

	if (t->len == HU_MAXLINELENGTH)
		return cc_false;
	else
	{
		t->l[t->len++] = ch;
		t->l[t->len] = '\0';
		t->needsupdate = 4;
		return cc_true;
	}

}

cc_bool HUlib_delCharFromTextLine(hu_textline_t* t)
{

	if (!t->len) return cc_false;
	else
	{
		t->l[--t->len] = '\0';
		t->needsupdate = 4;
		return cc_true;
	}

}

void
HUlib_drawTextLine
( hu_textline_t*        l,
  cc_bool               drawcursor )
{

	int                 i;
	int                 w;
	int                 x;
	unsigned char       c;

	/* draw the new stuff */
	x = l->x;
	for (i=0;i<l->len;i++)
	{
		c = toupper(l->l[i]);
		if (c != ' '
			&& c >= l->sc
			&& c <= '_')
		{
			w = SHORT(l->f[c - l->sc]->width) * HUD_SCALE;
			if (x+w > SCREENWIDTH)
				break;
			V_DrawPatch(x, l->y, SCREEN_FRAMEBUFFER, l->f[c - l->sc]);
			x += w;
		}
		else
		{
			x += 4 * HUD_SCALE;
			if (x >= SCREENWIDTH)
				break;
		}
	}

	/* draw the cursor if requested */
	if (drawcursor
		&& x + SHORT(l->f['_' - l->sc]->width) * HUD_SCALE <= SCREENWIDTH)
	{
		V_DrawPatch(x, l->y, SCREEN_FRAMEBUFFER, l->f['_' - l->sc]);
	}
}


/* sorta called by HU_Erase and just better darn get things straight */
void HUlib_eraseTextLine(hu_textline_t* l)
{
	int                 x;

	/* Only erases when NOT in automap and the screen is reduced, */
	/* and the text must either need updating or refreshing */
	/* (because of a recent change back from the automap) */

	if (!automapactive &&
		viewwindowx && l->needsupdate)
	{
		int lh, top, bottom, count, x_offset;

		lh = (SHORT(l->f[0]->height) + 1) * HUD_SCALE;
		top = l->y;
		bottom = top + lh;

		if (top < viewwindowy || top >= viewwindowy + viewheight)
			top = viewwindowy + viewheight;

		if (bottom < viewwindowy || bottom >= viewwindowy + viewheight)
			bottom = viewwindowy;

		count = bottom - top;

		x_offset = 0;

		/* erase left border */
		for (x = 0; x < viewwindowx; ++x)
		{
			R_VideoErase(l->y + x_offset, lh);
			x_offset += SCREENHEIGHT;
		}
		/* erase middle part (which may overlap the screen) */
		if (count > 0)
		{
			for (; x < viewwindowx + scaledviewwidth; ++x)
			{
				R_VideoErase(top + x_offset, count);
				x_offset += SCREENHEIGHT;
			}
		}
		/* erase right border */
		for (; x < SCREENWIDTH; ++x)
		{
			R_VideoErase(l->y + x_offset, lh);
			x_offset += SCREENHEIGHT;
		}
	}

	if (l->needsupdate) l->needsupdate--;

}

void
HUlib_initSText
( hu_stext_t*   s,
  int           x,
  int           y,
  int           h,
  patch_t**     font,
  int           startchar,
  cc_bool*      on )
{

	int i;

	s->h = h;
	s->on = on;
	s->laston = cc_true;
	s->cl = 0;
	for (i=0;i<h;i++)
		HUlib_initTextLine(&s->l[i],
						   x, y - i*(SHORT(font[0]->height)+1),
						   font, startchar);

}

void HUlib_addLineToSText(hu_stext_t* s)
{

	int i;

	/* add a clear line */
	if (++s->cl == s->h)
		s->cl = 0;
	HUlib_clearTextLine(&s->l[s->cl]);

	/* everything needs updating */
	for (i=0 ; i<s->h ; i++)
		s->l[i].needsupdate = 4;

}

void
HUlib_addMessageToSText
( hu_stext_t*   s,
  const char*   prefix,
  const char*   msg )
{
	HUlib_addLineToSText(s);
	if (prefix)
		while (*prefix)
			HUlib_addCharToTextLine(&s->l[s->cl], *(prefix++));

	while (*msg)
		HUlib_addCharToTextLine(&s->l[s->cl], *(msg++));
}

void HUlib_drawSText(hu_stext_t* s)
{
	int i, idx;
	hu_textline_t *l;

	if (!*s->on)
		return; /* if not on, don't draw */

	/* draw everything */
	for (i=0 ; i<s->h ; i++)
	{
		idx = s->cl - i;
		if (idx < 0)
			idx += s->h; /* handle queue of lines */

		l = &s->l[idx];

		/* need a decision made here on whether to skip the draw */
		HUlib_drawTextLine(l, cc_false); /* no cursor, please */
	}

}

void HUlib_eraseSText(hu_stext_t* s)
{

	int i;

	for (i=0 ; i<s->h ; i++)
	{
		if (s->laston && !*s->on)
			s->l[i].needsupdate = 4;
		HUlib_eraseTextLine(&s->l[i]);
	}
	s->laston = *s->on;

}

void
HUlib_initIText
( hu_itext_t*   it,
  int           x,
  int           y,
  patch_t**     font,
  int           startchar,
  cc_bool*      on )
{
	it->lm = 0; /* default left margin is start of text */
	it->on = on;
	it->laston = cc_true;
	HUlib_initTextLine(&it->l, x, y, font, startchar);
}


/* The following deletion routines adhere to the left margin restriction */
void HUlib_delCharFromIText(hu_itext_t* it)
{
	if (it->l.len != it->lm)
		HUlib_delCharFromTextLine(&it->l);
}

void HUlib_eraseLineFromIText(hu_itext_t* it)
{
	while (it->lm != it->l.len)
		HUlib_delCharFromTextLine(&it->l);
}

/* Resets left margin as well */
void HUlib_resetIText(hu_itext_t* it)
{
	it->lm = 0;
	HUlib_clearTextLine(&it->l);
}

void
HUlib_addPrefixToIText
( hu_itext_t*   it,
  const char*   str )
{
	while (*str)
		HUlib_addCharToTextLine(&it->l, *(str++));
	it->lm = it->l.len;
}

/* wrapper function for handling general keyed input. */
/* returns true if it ate the key */
cc_bool
HUlib_keyInIText
( hu_itext_t*   it,
  unsigned char ch )
{

	if (ch >= ' ' && ch <= '_')
		HUlib_addCharToTextLine(&it->l, (char) ch);
	else
		if (ch == KEY_BACKSPACE)
			HUlib_delCharFromIText(it);
		else
			if (ch != KEY_ENTER)
				return cc_false; /* did not eat key */

	return cc_true; /* ate the key */

}

void HUlib_drawIText(hu_itext_t* it)
{

	hu_textline_t *l = &it->l;

	if (!*it->on)
		return;
	HUlib_drawTextLine(l, cc_true); /* draw the line w/ cursor */

}

void HUlib_eraseIText(hu_itext_t* it)
{
	if (it->laston && !*it->on)
		it->l.needsupdate = 4;
	HUlib_eraseTextLine(&it->l);
	it->laston = *it->on;
}

