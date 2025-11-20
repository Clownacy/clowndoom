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
        Archiving: SaveGame I/O.

******************************************************************************/

#include "i_system.h"
#include "z_zone.h"
#include "p_local.h"

/* State. */
#include "doomstat.h"
#include "r_state.h"

#define PADSAVEP()      index += (D_ALIGNMENT - (index % D_ALIGNMENT)) % D_ALIGNMENT



/* P_ArchivePlayers */
size_t P_ArchivePlayers (unsigned char* const buffer, size_t index)
{
	int         i;
	int         j;
	player_t*   dest;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;

		PADSAVEP();

		if (buffer != NULL)
		{
			dest = (player_t *)&buffer[index];
			*dest = players[i];
			for (j=0 ; j<NUMPSPRITES ; j++)
			{
				if (dest->psprites[j].state)
				{
					dest->psprites[j].state
						= (state_t *)(dest->psprites[j].state-states);
				}
			}
		}

		index += sizeof(player_t);
	}

	return index;
}



/* P_UnArchivePlayers */
size_t P_UnArchivePlayers (const unsigned char* const buffer, size_t index)
{
	int         i;
	int         j;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;

		PADSAVEP();

		players[i] = *(player_t*)&buffer[index];
		index += sizeof(player_t);

		/* will be set when unarc thinker */
		players[i].mo = NULL;
		players[i].message = NULL;
		players[i].attacker = NULL;

		for (j=0 ; j<NUMPSPRITES ; j++)
		{
			if (players[i]. psprites[j].state)
			{
				players[i]. psprites[j].state
					= &states[ (size_t)players[i].psprites[j].state ];
			}
		}
	}

	return index;
}


/* P_ArchiveWorld */
size_t P_ArchiveWorld (unsigned char* const buffer, size_t index)
{
	int                 i;
	int                 j;
	sector_t*           sec;
	line_t*             li;
	side_t*             si;
	short*              put;

	put = buffer == NULL ? NULL : (short *)&buffer[index];

	/* do sectors */
	for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
	{
		if (put != NULL)
		{
			*put++ = sec->floorheight >> FRACBITS;
			*put++ = sec->ceilingheight >> FRACBITS;
			*put++ = sec->floorpic;
			*put++ = sec->ceilingpic;
			*put++ = sec->lightlevel;
			*put++ = sec->special;          /* needed? */
			*put++ = sec->tag;              /* needed? */
		}

		index += 7 * sizeof(short);
	}


	/* do lines */
	for (i=0, li = lines ; i<numlines ; i++,li++)
	{
		if (put != NULL)
		{
			*put++ = li->flags;
			*put++ = li->special;
			*put++ = li->tag;
		}

		index += 3 * sizeof(short);

		for (j=0 ; j<2 ; j++)
		{
			if (li->sidenum[j] == -1)
				continue;

			if (put != NULL)
			{
				si = &sides[li->sidenum[j]];

				*put++ = si->textureoffset >> FRACBITS;
				*put++ = si->rowoffset >> FRACBITS;
				*put++ = si->toptexture;
				*put++ = si->bottomtexture;
				*put++ = si->midtexture;
			}

			index += 5 * sizeof(short);
		}
	}

	return index;
}



/* P_UnArchiveWorld */
size_t P_UnArchiveWorld (const unsigned char* const buffer, size_t index)
{
	int                 i;
	int                 j;
	sector_t*           sec;
	line_t*             li;
	side_t*             si;
	short*              get;

	get = (short *)&buffer[index];

	/* do sectors */
	for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
	{
		sec->floorheight = *get++ << FRACBITS;
		sec->ceilingheight = *get++ << FRACBITS;
		sec->floorpic = *get++;
		sec->ceilingpic = *get++;
		sec->lightlevel = *get++;
		sec->special = *get++;          /* needed? */
		sec->tag = *get++;              /* needed? */
		sec->specialdata = 0;
		sec->soundtarget = 0;

		index += 7 * sizeof(short);
	}

	/* do lines */
	for (i=0, li = lines ; i<numlines ; i++,li++)
	{
		li->flags = *get++;
		li->special = *get++;
		li->tag = *get++;
		index += 3 * sizeof(short);
		for (j=0 ; j<2 ; j++)
		{
			if (li->sidenum[j] == -1)
				continue;
			si = &sides[li->sidenum[j]];
			si->textureoffset = *get++ << FRACBITS;
			si->rowoffset = *get++ << FRACBITS;
			si->toptexture = *get++;
			si->bottomtexture = *get++;
			si->midtexture = *get++;
			index += 5 * sizeof(short);
		}
	}
	return index;
}





/* P_ArchiveThinkers */
size_t P_ArchiveThinkers (unsigned char* const buffer, size_t index)
{
	thinker_t*          th;
	mobj_t*             mobj;
	size_t              total_mobjs;

	total_mobjs = 0;

	/* temporarily replace the prev pointer with an index into the thinker list */
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
			th->prev = (thinker_t*)total_mobjs++;

	if (buffer != NULL)
		*(size_t*)&buffer[index] = total_mobjs;
	index += sizeof(size_t);
	PADSAVEP();

	/* save off the current thinkers */
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			if (buffer != NULL)
			{
				mobj = (mobj_t *)&buffer[index];
				*mobj = *(mobj_t *)th;
				mobj->state = (state_t *)(mobj->state - states);

				if (mobj->player)
					mobj->player = (player_t *)((mobj->player-players) + 1);

				if (mobj->target)
					mobj->target = (mobj_t *)mobj->target->thinker.prev;
			}
			index += sizeof(*mobj);
		}
	}

	/* restore the prev pointers */
	for (th = &thinkercap ; th != &thinkercap ; th=th->next)
		if (th->next->function.acp1 == (actionf_p1)P_MobjThinker)
			th->next->prev = th;

	return index;
}



/* P_UnArchiveThinkers */
size_t P_UnArchiveThinkers (const unsigned char* const buffer, size_t index)
{
	thinker_t*          currentthinker;
	thinker_t*          next;
	size_t              total_mobjs;

	/* remove all the current thinkers */
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		next = currentthinker->next;

		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
			P_RemoveMobj ((mobj_t *)currentthinker);
		else
			Z_Free (currentthinker);

		currentthinker = next;
	}
	P_InitThinkers ();

	total_mobjs = *(size_t*)&buffer[index];
	index += sizeof(size_t);
	PADSAVEP();

	if (total_mobjs != 0)
	{
		size_t i;

		mobj_t** const mobjs = (mobj_t**)Z_Malloc(sizeof(*mobjs) * total_mobjs, PU_STATIC, NULL);

		/* read in saved thinkers */
		for (i = 0; i < total_mobjs; ++i)
		{
			mobj_t* const mobj = mobjs[i] = (mobj_t*)Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
			*mobj = *(mobj_t*)&buffer[index];
			index += sizeof(*mobj);
			mobj->state = &states[(size_t)mobj->state];
			if (mobj->player)
			{
				mobj->player = &players[(size_t)mobj->player-1];
				mobj->player->mo = mobj;
			}
			P_SetThingPosition (mobj);
			mobj->info = &mobjinfo[mobj->type];
			mobj->floorz = mobj->subsector->sector->floorheight;
			mobj->ceilingz = mobj->subsector->sector->ceilingheight;
			mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
			P_AddThinker (&mobj->thinker);
		}

		/* BUGFIX: Restore the mobj's target. */
		for (i = 0; i < total_mobjs; ++i)
		{
			mobj_t* const mobj = mobjs[i];
			mobj->target = mobjs[(size_t)mobj->target];
		}

		Z_Free(mobjs);
	}

	return index;
}


/* P_ArchiveSpecials */
typedef enum
{
	tc_ceiling,
	tc_door,
	tc_floor,
	tc_plat,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_flicker, /* BUGFIX: Added missing special type. */
	tc_endspecials

} specials_e;



/* Things to handle: */
/* T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list */
/* T_VerticalDoor, (vldoor_t: sector_t * swizzle), */
/* T_MoveFloor, (floormove_t: sector_t * swizzle), */
/* T_LightFlash, (lightflash_t: sector_t * swizzle), */
/* T_StrobeFlash, (strobe_t: sector_t *), */
/* T_Glow, (glow_t: sector_t *), */
/* T_PlatRaise, (plat_t: sector_t *), - active list */
size_t P_ArchiveSpecials (unsigned char* const buffer, size_t index)
{
	thinker_t*          th;
	int                 i;

	/* save off the current thinkers */
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		#define ARCHIVE_SPECIAL(TYPE, ENUMERATION) \
			do \
			{ \
				if (buffer != NULL) \
					buffer[index] = ENUMERATION; \
				++index; \
				PADSAVEP(); \
				if (buffer != NULL) \
				{ \
					TYPE* const special = (TYPE *)&buffer[index]; \
					*special = *(TYPE *)th; \
					special->sector = (sector_t *)(special->sector - sectors); \
				} \
				index += sizeof(TYPE); \
			} \
			while (0)

		#define CHECK_AND_ARCHIVE_SPECIAL(FUNCTION, TYPE, ENUMERATION) \
			if (th->function.acp1 == (actionf_p1)FUNCTION) \
			{ \
				ARCHIVE_SPECIAL(TYPE, ENUMERATION); \
				continue; \
			}

		if (th->function.acv == (actionf_v)NULL)
		{
			for (i = 0; i < MAXCEILINGS;i++)
				if (activeceilings[i] == (ceiling_t *)th)
					break;

			if (i<MAXCEILINGS)
				ARCHIVE_SPECIAL(ceiling_t, tc_ceiling);
			continue;
		}

		CHECK_AND_ARCHIVE_SPECIAL(T_MoveCeiling,  ceiling_t,     tc_ceiling);
		CHECK_AND_ARCHIVE_SPECIAL(T_VerticalDoor, vldoor_t,      tc_door);
		CHECK_AND_ARCHIVE_SPECIAL(T_MoveFloor,    floormove_t,   tc_floor);
		CHECK_AND_ARCHIVE_SPECIAL(T_PlatRaise,    plat_t,        tc_plat);
		CHECK_AND_ARCHIVE_SPECIAL(T_LightFlash,   lightflash_t,  tc_flash);
		CHECK_AND_ARCHIVE_SPECIAL(T_StrobeFlash,  strobe_t,      tc_strobe);
		CHECK_AND_ARCHIVE_SPECIAL(T_Glow,         glow_t,        tc_glow);
		CHECK_AND_ARCHIVE_SPECIAL(T_FireFlicker,  fireflicker_t, tc_flicker); /* BUGFIX: Added missing special type. */
	}

	/* add a terminating marker */
	if (buffer != NULL)
		buffer[index] = tc_endspecials;
	++index;
	return index;
}


/* P_UnArchiveSpecials */
size_t P_UnArchiveSpecials (const unsigned char* const buffer, size_t index)
{
	/* read in saved thinkers */
	while (1)
	{
		#define UNARCHIVE_SPECIAL(TYPE) \
			TYPE *special; \
			PADSAVEP(); \
			special = (TYPE*)Z_Malloc (sizeof(*special), PU_LEVEL, NULL); \
			*special = *(TYPE*)&buffer[index]; \
			index += sizeof(*special); \
			special->sector = &sectors[(size_t)special->sector]; \
			P_AddThinker (&special->thinker)

		const specials_e tclass = (specials_e)buffer[index++];
		switch (tclass)
		{
		case tc_endspecials:
			return index;     /* end of list */

		case tc_ceiling:
		{
			UNARCHIVE_SPECIAL(ceiling_t);
			special->sector->specialdata = special;

			if (special->thinker.function.acp1)
				special->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

			P_AddActiveCeiling(special);
			break;
		}

		case tc_door:
		{
			UNARCHIVE_SPECIAL(vldoor_t);
			special->sector->specialdata = special;
			special->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
			break;
		}

		case tc_floor:
		{
			UNARCHIVE_SPECIAL(floormove_t);
			special->sector->specialdata = special;
			special->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
			break;
		}

		case tc_plat:
		{
			UNARCHIVE_SPECIAL(plat_t);
			special->sector->specialdata = special;

			if (special->thinker.function.acp1)
				special->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

			P_AddActivePlat(special);
			break;
		}

		case tc_flash:
		{
			UNARCHIVE_SPECIAL(lightflash_t);
			special->thinker.function.acp1 = (actionf_p1)T_LightFlash;
			break;
		}

		case tc_strobe:
		{
			UNARCHIVE_SPECIAL(strobe_t);
			special->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
			break;
		}

		case tc_glow:
		{
			UNARCHIVE_SPECIAL(glow_t);
			special->thinker.function.acp1 = (actionf_p1)T_Glow;
			break;
		}

		/* BUGFIX: Added missing special type. */
		case tc_flicker:
		{
			UNARCHIVE_SPECIAL(fireflicker_t);
			special->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
			break;
		}

		default:
			I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
					 "in savegame",tclass);
		}

	}

	return index;
}

