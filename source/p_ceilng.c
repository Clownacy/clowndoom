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

   DESCRIPTION:  Ceiling animation (lowering, crushing, raising)

******************************************************************************/

#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

/* State. */
#include "doomstat.h"
#include "r_state.h"

/* Data. */
#include "sounds.h"

/* CEILINGS */


ceiling_t*      activeceilings[MAXCEILINGS];


/* T_MoveCeiling */

void T_MoveCeiling (ceiling_t* ceiling)
{
	result_e    res;

	switch(ceiling->direction)
	{
	case 0:
		/* IN STASIS */
		break;
	case 1:
		/* UP */
		res = T_MovePlane(ceiling->sector,
						  ceiling->speed,
						  ceiling->topheight,
						  cc_false,1,ceiling->direction);

		if (!(leveltime&7))
		{
			switch(ceiling->type)
			{
			case ceiling_silentCrushAndRaise:
				break;
			default:
				S_StartSound((mobj_t *)&ceiling->sector->soundorg,
							 sfx_stnmov);
				/* ? */
				break;
			}
		}

		if (res == result_pastdest)
		{
			switch(ceiling->type)
			{
			case ceiling_raiseToHighest:
				P_RemoveActiveCeiling(ceiling);
				break;

			case ceiling_silentCrushAndRaise:
				S_StartSound((mobj_t *)&ceiling->sector->soundorg,
							 sfx_pstop);
				/* Fallthrough */
			case ceiling_fastCrushAndRaise:
			case ceiling_crushAndRaise:
				ceiling->direction = -1;
				break;

			default:
				break;
			}

		}
		break;

	case -1:
		/* DOWN */
		res = T_MovePlane(ceiling->sector,
						  ceiling->speed,
						  ceiling->bottomheight,
						  ceiling->crush,1,ceiling->direction);

		if (!(leveltime&7))
		{
			switch(ceiling->type)
			{
			case ceiling_silentCrushAndRaise: break;
			default:
				S_StartSound((mobj_t *)&ceiling->sector->soundorg,
							 sfx_stnmov);
			}
		}

		if (res == result_pastdest)
		{
			switch(ceiling->type)
			{
			case ceiling_silentCrushAndRaise:
				S_StartSound((mobj_t *)&ceiling->sector->soundorg,
							 sfx_pstop);
				/* Fallthrough */
			case ceiling_crushAndRaise:
				ceiling->speed = CEILSPEED;
				/* Fallthrough */
			case ceiling_fastCrushAndRaise:
				ceiling->direction = 1;
				break;

			case ceiling_lowerAndCrush:
			case ceiling_lowerToFloor:
				P_RemoveActiveCeiling(ceiling);
				break;

			default:
				break;
			}
		}
		else /* ( res != result_pastdest ) */
		{
			if (res == result_crushed)
			{
				switch(ceiling->type)
				{
				case ceiling_silentCrushAndRaise:
				case ceiling_crushAndRaise:
				case ceiling_lowerAndCrush:
					ceiling->speed = CEILSPEED / 8;
					break;

				default:
					break;
				}
			}
		}
		break;
	}
}


/* EV_DoCeiling */
/* Move a ceiling up/down and all around! */
int
EV_DoCeiling
( line_t*       line,
  ceiling_e     type )
{
	int         secnum;
	int         rtn;
	sector_t*   sec;
	ceiling_t*  ceiling;

	secnum = -1;
	rtn = 0;

	/*  Reactivate in-stasis ceilings...for certain types. */
	switch(type)
	{
	case ceiling_fastCrushAndRaise:
	case ceiling_silentCrushAndRaise:
	case ceiling_crushAndRaise:
		P_ActivateInStasisCeiling(line);
	default:
		break;
	}

	while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* new door thinker */
		rtn = 1;
		ceiling = (ceiling_t*)Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
		P_AddThinker (&ceiling->thinker);
		sec->specialdata = ceiling;
		ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
		ceiling->sector = sec;
		ceiling->crush = cc_false;

		switch(type)
		{
		case ceiling_fastCrushAndRaise:
			ceiling->crush = cc_true;
			ceiling->topheight = sec->ceilingheight;
			ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);
			ceiling->direction = -1;
			ceiling->speed = CEILSPEED * 2;
			break;

		case ceiling_silentCrushAndRaise:
		case ceiling_crushAndRaise:
			ceiling->crush = cc_true;
			ceiling->topheight = sec->ceilingheight;
			/* Fallthrough */
		case ceiling_lowerAndCrush:
		case ceiling_lowerToFloor:
			ceiling->bottomheight = sec->floorheight;
			if (type != ceiling_lowerToFloor)
				ceiling->bottomheight += 8*FRACUNIT;
			ceiling->direction = -1;
			ceiling->speed = CEILSPEED;
			break;

		case ceiling_raiseToHighest:
			ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
			ceiling->direction = 1;
			ceiling->speed = CEILSPEED;
			break;
		}

		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
	}
	return rtn;
}


/* Add an active ceiling */
void P_AddActiveCeiling(ceiling_t* c)
{
	int         i;

	for (i = 0; i < MAXCEILINGS;i++)
	{
		if (activeceilings[i] == NULL)
		{
			activeceilings[i] = c;
			return;
		}
	}
}



/* Remove a ceiling's thinker */
void P_RemoveActiveCeiling(ceiling_t* c)
{
	int         i;

	for (i = 0;i < MAXCEILINGS;i++)
	{
		if (activeceilings[i] == c)
		{
			activeceilings[i]->sector->specialdata = NULL;
			P_RemoveThinker (&activeceilings[i]->thinker);
			activeceilings[i] = NULL;
			break;
		}
	}
}



/* Restart a ceiling that's in-stasis */
void P_ActivateInStasisCeiling(line_t* line)
{
	int         i;

	for (i = 0;i < MAXCEILINGS;i++)
	{
		if (activeceilings[i]
			&& (activeceilings[i]->tag == line->tag)
			&& (activeceilings[i]->direction == 0))
		{
			activeceilings[i]->direction = activeceilings[i]->olddirection;
			activeceilings[i]->thinker.function.acp1
			  = (actionf_p1)T_MoveCeiling;
		}
	}
}



/* EV_CeilingCrushStop */
/* Stop a ceiling from crushing! */
int     EV_CeilingCrushStop(line_t      *line)
{
	int         i;
	int         rtn;

	rtn = 0;
	for (i = 0;i < MAXCEILINGS;i++)
	{
		if (activeceilings[i]
			&& (activeceilings[i]->tag == line->tag)
			&& (activeceilings[i]->direction != 0))
		{
			activeceilings[i]->olddirection = activeceilings[i]->direction;
			activeceilings[i]->thinker.function.acv = (actionf_v)NULL;
			activeceilings[i]->direction = 0;           /* in-stasis */
			rtn = 1;
		}
	}


	return rtn;
}
