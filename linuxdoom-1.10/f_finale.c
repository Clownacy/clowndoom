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
        Game completion, final screen animation.

******************************************************************************/


#include <ctype.h>

/* Functions. */
#include "i_system.h"
#include "m_swap.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"

/* Data. */
#include "dstrings.h"
#include "sounds.h"

#include "doomstat.h"
#include "r_state.h"

/* ? */
/* #include "doomstat.h" */
/* #include "r_local.h" */
/* #include "f_finale.h" */

/* Stage of animation: */
/*  0 = text, 1 = art screen, 2 = character cast */
int             finalestage;

int             finalecount;

#define TEXTSPEED       3
#define TEXTWAIT        250

const char*     e1text = E1TEXT;
const char*     e2text = E2TEXT;
const char*     e3text = E3TEXT;
const char*     e4text = E4TEXT;

static const char c2text[] = C2TEXT;

const char* ctext[] = {
	C1TEXT,
	c2text,
	C3TEXT,
	C4TEXT,
	C5TEXT,
	C6TEXT
};

const char* ptext[] = {
	P1TEXT,
	P2TEXT,
	P3TEXT,
	P4TEXT,
	P5TEXT,
	P6TEXT
};

const char* ttext[] = {
	T1TEXT,
	T2TEXT,
	T3TEXT,
	T4TEXT,
	T5TEXT,
	T6TEXT
};

const char*     finaletext;
const char*     finaleflat;

void    F_StartCast (void);
void    F_CastTicker (void);
d_bool F_CastResponder (const event_t *ev);
void    F_CastDrawer (void);

/* F_StartFinale */
void F_StartFinale (void)
{
	unsigned int finaletextindex = 0;

	gameaction = ga_nothing;
	gamestate = GS_FINALE;
	viewactive = d_false;
	automapactive = d_false;

	/* Okay - IWAD dependend stuff. */
	/* This has been changed severly, and */
	/*  some stuff might have changed in the process. */
	switch ( gamemode )
	{

	  /* DOOM 1 - E1, E3 or E4, but each nine missions */
	  case shareware:
	  case registered:
	  case retail:
	  {
		S_ChangeMusic(mus_victor, d_true);

		switch (gameepisode)
		{
		  case 1:
			finaleflat = "FLOOR4_8";
			finaletext = e1text;
			break;
		  case 2:
			finaleflat = "SFLR6_1";
			finaletext = e2text;
			break;
		  case 3:
			finaleflat = "MFLR8_4";
			finaletext = e3text;
			break;
		  case 4:
			finaleflat = "MFLR8_3";
			finaletext = e4text;
			break;
		  default:
			/* Ouch. */
			break;
		}
		break;
	  }

	  /* DOOM II and missions packs with E1, M34 */
	  case commercial:
	  {
		  S_ChangeMusic(mus_read_m, d_true);

		  switch (gamemap)
		  {
			case 6:
			  finaleflat = "SLIME16";
			  finaletextindex = 0;
			  break;
			case 11:
			  finaleflat = "RROCK14";
			  finaletextindex = 1;
			  break;
			case 20:
			  finaleflat = "RROCK07";
			  finaletextindex = 2;
			  break;
			case 30:
			  finaleflat = "RROCK17";
			  finaletextindex = 3;
			  break;
			case 15:
			  finaleflat = "RROCK13";
			  finaletextindex = 4;
			  break;
			case 31:
			  finaleflat = "RROCK19";
			  finaletextindex = 5;
			  break;
			default:
			  /* Ouch. */
			  break;
		  }

		  switch (gamemission)
		  {
			case doom2:
			  finaletext = ctext[finaletextindex];
			  break;
			case pack_tnt:
			  finaletext = ttext[finaletextindex];
			  break;
			case pack_plut:
			  finaletext = ptext[finaletextindex];
			  break;
			default:
			  /* Ouch. */
			  break;
		  }

		  break;
	  }


	  /* Indeterminate. */
	  default:
		S_ChangeMusic(mus_read_m, d_true);
		finaleflat = "F_SKY1"; /* Not used anywhere else. */
		finaletext = ctext[0];  /* FIXME - other text, music? */
		break;
	}

	finalestage = 0;
	finalecount = 0;

}



d_bool F_Responder (const event_t *event)
{
	if (finalestage == 2)
		return F_CastResponder (event);

	return d_false;
}


/* F_Ticker */
void F_Ticker (void)
{
	int         i;

	/* check for skipping */
	if ( (gamemode == commercial)
	  && ( finalecount > 50) )
	{
	  /* go on to the next level */
	  for (i=0 ; i<MAXPLAYERS ; i++)
		if (players[i].cmd.buttons)
		  break;

	  if (i < MAXPLAYERS)
	  {
		if (gamemap == 30)
		  F_StartCast ();
		else
		  gameaction = ga_worlddone;
	  }
	}

	/* advance animation */
	finalecount++;

	if (finalestage == 2)
	{
		F_CastTicker ();
		return;
	}

	if ( gamemode == commercial)
		return;

	if (!finalestage && finalecount>(int)strlen (finaletext)*TEXTSPEED + TEXTWAIT)
	{
		finalecount = 0;
		finalestage = 1;
		wipegamestate = GS_FORCEWIPE;             /* force a wipe */
		if (gameepisode == 3)
			S_StartMusic (mus_bunny);
	}
}



/* F_TextWrite */

#include "hu_stuff.h"
extern  patch_t *hu_font[HU_FONTSIZE];

void F_TextWrite (void)
{
	int         w;
	int         count;
	const char* ch;
	int         c;
	int         cx;
	int         cy;

	/* erase the entire screen to a tiled background */
	V_FillScreenWithPattern(finaleflat, 0, SCREENHEIGHT);

	/* draw some of the text onto the screen */
	cx = X_CENTRE(10);
	cy = Y_CENTRE(10);
	ch = finaletext;

	count = (finalecount - 10)/TEXTSPEED;
	if (count < 0)
		count = 0;
	for ( ; count ; count-- )
	{
		c = *ch++;
		if (!c)
			break;
		if (c == '\n')
		{
			cx = X_CENTRE(10);
			cy += 11*SCREEN_MUL;
			continue;
		}

		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c> HU_FONTSIZE)
		{
			cx += 4*SCREEN_MUL;
			continue;
		}

		w = SHORT (hu_font[c]->width)*SCREEN_MUL;
		if (cx+w > SCREENWIDTH)
			break;
		V_DrawPatchScaled(cx, cy, 0, hu_font[c]);
		cx+=w;
	}

}

/* Final DOOM 2 animation */
/* Casting by id Software. */
/*   in order of appearance */
typedef struct
{
	const char          *name;
	mobjtype_t  type;
} castinfo_t;

castinfo_t      castorder[] = {
	{CC_ZOMBIE, MT_POSSESSED},
	{CC_SHOTGUN, MT_SHOTGUY},
	{CC_HEAVY, MT_CHAINGUY},
	{CC_IMP, MT_TROOP},
	{CC_DEMON, MT_SERGEANT},
	{CC_LOST, MT_SKULL},
	{CC_CACO, MT_HEAD},
	{CC_HELL, MT_KNIGHT},
	{CC_BARON, MT_BRUISER},
	{CC_ARACH, MT_BABY},
	{CC_PAIN, MT_PAIN},
	{CC_REVEN, MT_UNDEAD},
	{CC_MANCU, MT_FATSO},
	{CC_ARCH, MT_VILE},
	{CC_SPIDER, MT_SPIDER},
	{CC_CYBER, MT_CYBORG},
	{CC_HERO, MT_PLAYER},

	{NULL,(mobjtype_t)0}
};

int             castnum;
int             casttics;
state_t*        caststate;
d_bool         castdeath;
int             castframes;
int             castonmelee;
d_bool         castattacking;


/* F_StartCast */
extern  gamestate_t     wipegamestate;


void F_StartCast (void)
{
	wipegamestate = GS_FORCEWIPE;         /* force a screen wipe */
	castnum = 0;
	caststate = &states[mobjinfo[castorder[castnum].type].seestate];
	casttics = caststate->tics;
	castdeath = d_false;
	finalestage = 2;
	castframes = 0;
	castonmelee = 0;
	castattacking = d_false;
	S_ChangeMusic(mus_evil, d_true);
}


/* F_CastTicker */
void F_CastTicker (void)
{
	int         st;
	int         sfx;

	if (--casttics > 0)
		return;                 /* not time to change state yet */

	if (caststate->tics == -1 || caststate->nextstate == S_NULL)
	{
		/* switch from deathstate to next monster */
		castnum++;
		castdeath = d_false;
		if (castorder[castnum].name == NULL)
			castnum = 0;
		if (mobjinfo[castorder[castnum].type].seesound)
			S_StartSound (NULL, mobjinfo[castorder[castnum].type].seesound);
		caststate = &states[mobjinfo[castorder[castnum].type].seestate];
		castframes = 0;
	}
	else
	{
		/* just advance to next state in animation */
		if (caststate == &states[S_PLAY_ATK1])
			goto stopattack;    /* Oh, gross hack! */
		st = caststate->nextstate;
		caststate = &states[st];
		castframes++;

		/* sound hacks.... */
		switch (st)
		{
		  case S_PLAY_ATK1:     sfx = sfx_dshtgn; break;
		  case S_POSS_ATK2:     sfx = sfx_pistol; break;
		  case S_SPOS_ATK2:     sfx = sfx_shotgn; break;
		  case S_VILE_ATK2:     sfx = sfx_vilatk; break;
		  case S_SKEL_FIST2:    sfx = sfx_skeswg; break;
		  case S_SKEL_FIST4:    sfx = sfx_skepch; break;
		  case S_SKEL_MISS2:    sfx = sfx_skeatk; break;
		  case S_FATT_ATK8:
		  case S_FATT_ATK5:
		  case S_FATT_ATK2:     sfx = sfx_firsht; break;
		  case S_CPOS_ATK2:
		  case S_CPOS_ATK3:
		  case S_CPOS_ATK4:     sfx = sfx_shotgn; break;
		  case S_TROO_ATK3:     sfx = sfx_claw; break;
		  case S_SARG_ATK2:     sfx = sfx_sgtatk; break;
		  case S_BOSS_ATK2:
		  case S_BOS2_ATK2:
		  case S_HEAD_ATK2:     sfx = sfx_firsht; break;
		  case S_SKULL_ATK2:    sfx = sfx_sklatk; break;
		  case S_SPID_ATK2:
		  case S_SPID_ATK3:     sfx = sfx_shotgn; break;
		  case S_BSPI_ATK2:     sfx = sfx_plasma; break;
		  case S_CYBER_ATK2:
		  case S_CYBER_ATK4:
		  case S_CYBER_ATK6:    sfx = sfx_rlaunc; break;
		  case S_PAIN_ATK3:     sfx = sfx_sklatk; break;
		  default: sfx = 0; break;
		}

		if (sfx)
			S_StartSound (NULL, sfx);
	}

	if (castframes == 12)
	{
		/* go into attack frame */
		castattacking = d_true;
		if (castonmelee)
			caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
		else
			caststate=&states[mobjinfo[castorder[castnum].type].missilestate];
		castonmelee ^= 1;
		if (caststate == &states[S_NULL])
		{
			if (castonmelee)
				caststate=
					&states[mobjinfo[castorder[castnum].type].meleestate];
			else
				caststate=
					&states[mobjinfo[castorder[castnum].type].missilestate];
		}
	}

	if (castattacking)
	{
		if (castframes == 24
			||  caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
		{
		  stopattack:
			castattacking = d_false;
			castframes = 0;
			caststate = &states[mobjinfo[castorder[castnum].type].seestate];
		}
	}

	casttics = caststate->tics;
	if (casttics == -1)
		casttics = 15;
}


/* F_CastResponder */

d_bool F_CastResponder (const event_t* ev)
{
	if (ev->type != ev_keydown)
		return d_false;

	if (castdeath)
		return d_true;                    /* already in dying frames */

	/* go into death frame */
	castdeath = d_true;
	caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
	casttics = caststate->tics;
	castframes = 0;
	castattacking = d_false;
	if (mobjinfo[castorder[castnum].type].deathsound)
		S_StartSound (NULL, mobjinfo[castorder[castnum].type].deathsound);

	return d_true;
}


void F_CastPrint (const char* text)
{
	const char* ch;
	int         c;
	int         cx;
	int         w;
	int         width;

	/* find width */
	ch = text;
	width = 0;

	while (ch)
	{
		c = *ch++;
		if (!c)
			break;
		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c> HU_FONTSIZE)
		{
			width += 4*SCREEN_MUL;
			continue;
		}

		w = SHORT (hu_font[c]->width)*SCREEN_MUL;
		width += w;
	}

	/* draw it */
	cx = SCREENWIDTH/2-width/2;
	ch = text;
	while (ch)
	{
		c = *ch++;
		if (!c)
			break;
		c = toupper(c) - HU_FONTSTART;
		if (c < 0 || c> HU_FONTSIZE)
		{
			cx += 4*SCREEN_MUL;
			continue;
		}

		w = SHORT (hu_font[c]->width)*SCREEN_MUL;
		V_DrawPatchScaled(cx, Y_CENTRE(180), 0, hu_font[c]);
		cx+=w;
	}

}


/* F_CastDrawer */
void F_CastDrawer (void)
{
	spritedef_t*        sprdef;
	spriteframe_t*      sprframe;
	int                 lump;
	d_bool             flip;
	patch_t*            patch;

	/* erase the entire screen to a background */
	V_DrawPatchScaled (X_CENTRE(0),Y_CENTRE(0),0, (patch_t*)W_CacheLumpName ("BOSSBACK", PU_CACHE));

	F_CastPrint (castorder[castnum].name);

	/* draw the current frame in the middle of the screen */
	sprdef = &sprites[caststate->sprite];
	sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
	lump = sprframe->lump[0];
	flip = (d_bool)sprframe->flip[0];

	patch = (patch_t*)W_CacheLumpNum (lump+firstspritelump, PU_CACHE);
	if (flip)
		V_DrawPatchFlippedScaled (X_CENTRE(160),Y_CENTRE(170),0,patch);
	else
		V_DrawPatchScaled (X_CENTRE(160),Y_CENTRE(170),0,patch);
}




/* F_BunnyScroll */
void F_BunnyScroll (void)
{
	int         scrolled;
	int         x;
	patch_t*    p1;
	patch_t*    p2;
	char        name[5];
	int         stage;
	static int  laststage;

	p1 = (patch_t*)W_CacheLumpName ("PFUB2", PU_LEVEL);
	p2 = (patch_t*)W_CacheLumpName ("PFUB1", PU_LEVEL);

	scrolled = 320 - (finalecount-230)/2;
	if (scrolled > 320)
		scrolled = 320;
	if (scrolled < 0)
		scrolled = 0;

	for ( x=0 ; x<320 ; x++)
	{
		if (x+scrolled < 320)
			V_DrawPatchColumn (X_CENTRE(x), Y_CENTRE(0), 0, p1, SCREEN_MUL, x+scrolled);
		else
			V_DrawPatchColumn (X_CENTRE(x), Y_CENTRE(0), 0, p2, SCREEN_MUL, x+scrolled - 320);
	}

	if (finalecount < 1130)
		return;
	if (finalecount < 1180)
	{
		V_DrawPatchScaled (X_CENTRE((ORIGINAL_SCREEN_WIDTH-13*8)/2),
					 Y_CENTRE((ORIGINAL_SCREEN_HEIGHT-8*8)/2),0, (patch_t*)W_CacheLumpName ("END0",PU_CACHE));
		laststage = 0;
		return;
	}

	stage = (finalecount-1180) / 5;
	if (stage > 6)
		stage = 6;
	if (stage > laststage)
	{
		S_StartSound (NULL, sfx_pistol);
		laststage = stage;
	}

	name[0] = 'E';
	name[1] = 'N';
	name[2] = 'D';
	name[3] = '0' + stage;
	name[4] = '\0';
	V_DrawPatchScaled (X_CENTRE((ORIGINAL_SCREEN_WIDTH-13*8)/2), Y_CENTRE((ORIGINAL_SCREEN_HEIGHT-8*8)/2),0, (patch_t*)W_CacheLumpName (name,PU_CACHE));
}


/* F_Drawer */
void F_Drawer (void)
{
	if (finalestage == 2)
	{
		F_CastDrawer ();
		return;
	}

	if (!finalestage)
		F_TextWrite ();
	else
	{
		switch (gameepisode)
		{
		  case 1:
			if ( gamemode == retail )
			  V_DrawPatchScaled (X_CENTRE(0),Y_CENTRE(0),0,
						 (patch_t*)W_CacheLumpName("CREDIT",PU_CACHE));
			else
			  V_DrawPatchScaled(X_CENTRE(0),Y_CENTRE(0),0,
						 (patch_t*)W_CacheLumpName("HELP2",PU_CACHE));
			break;
		  case 2:
			V_DrawPatchScaled(X_CENTRE(0),Y_CENTRE(0),0,
						(patch_t*)W_CacheLumpName("VICTORY2",PU_CACHE));
			break;
		  case 3:
			F_BunnyScroll ();
			break;
		  case 4:
			V_DrawPatchScaled (X_CENTRE(0),Y_CENTRE(0),0,
						 (patch_t*)W_CacheLumpName("ENDPIC",PU_CACHE));
			break;
		}
	}

}


