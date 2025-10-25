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





/* Thinkers */
typedef enum
{
	tc_end,
	tc_mobj

} thinkerclass_t;



/* P_ArchiveThinkers */
size_t P_ArchiveThinkers (unsigned char* const buffer, size_t index)
{
	thinker_t*          th;
	mobj_t*             mobj;

	/* save off the current thinkers */
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			if (buffer != NULL)
				buffer[index] = tc_mobj;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				mobj = (mobj_t *)&buffer[index];
				*mobj = *(mobj_t *)th;
				mobj->state = (state_t *)(mobj->state - states);

				if (mobj->player)
					mobj->player = (player_t *)((mobj->player-players) + 1);
			}
			index += sizeof(*mobj);
			continue;
		}

		/* I_Error ("P_ArchiveThinkers: Unknown thinker function"); */
	}

	/* add a terminating marker */
	if (buffer != NULL)
		buffer[index] = tc_end;
	++index;
	return index;
}



/* P_UnArchiveThinkers */
size_t P_UnArchiveThinkers (const unsigned char* const buffer, size_t index)
{
	thinkerclass_t      tclass;
	thinker_t*          currentthinker;
	thinker_t*          next;
	mobj_t*             mobj;

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

	/* read in saved thinkers */
	while (1)
	{
		tclass = (thinkerclass_t)buffer[index++];
		switch (tclass)
		{
		  case tc_end:
			return index;     /* end of list */

		  case tc_mobj:
			PADSAVEP();
			mobj = (mobj_t*)Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
			*mobj = *(mobj_t*)&buffer[index];
			index += sizeof(*mobj);
			mobj->state = &states[(size_t)mobj->state];
			mobj->target = NULL;
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
			break;

		  default:
			I_Error ("Unknown tclass %i in savegame",tclass);
		}

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
	ceiling_t*          ceiling;
	vldoor_t*           door;
	floormove_t*        floor;
	plat_t*             plat;
	lightflash_t*       flash;
	strobe_t*           strobe;
	glow_t*             glow;
	int                 i;

	/* save off the current thinkers */
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acv == (actionf_v)NULL)
		{
			for (i = 0; i < MAXCEILINGS;i++)
				if (activeceilings[i] == (ceiling_t *)th)
					break;

			if (i<MAXCEILINGS)
			{
				if (buffer != NULL)
					buffer[index] = tc_ceiling;
				++index;
				PADSAVEP();
				if (buffer != NULL)
				{
					ceiling = (ceiling_t *)&buffer[index];
					*ceiling = *(ceiling_t *)th;
					ceiling->sector = (sector_t *)(ceiling->sector - sectors);
				}
				index += sizeof(*ceiling);
			}
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
		{
			if (buffer != NULL)
				buffer[index] = tc_ceiling;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				ceiling = (ceiling_t *)&buffer[index];
				*ceiling = *(ceiling_t *)th;
				ceiling->sector = (sector_t *)(ceiling->sector - sectors);
			}
			index += sizeof(*ceiling);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_VerticalDoor)
		{
			if (buffer != NULL)
				buffer[index] = tc_door;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				door = (vldoor_t *)&buffer[index];
				*door = *(vldoor_t *)th;
				door->sector = (sector_t *)(door->sector - sectors);
			}
			index += sizeof(*door);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_MoveFloor)
		{
			if (buffer != NULL)
				buffer[index] = tc_floor;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				floor = (floormove_t *)&buffer[index];
				*floor = *(floormove_t *)th;
				floor->sector = (sector_t *)(floor->sector - sectors);
			}
			index += sizeof(*floor);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_PlatRaise)
		{
			if (buffer != NULL)
				buffer[index] = tc_plat;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				plat = (plat_t *)&buffer[index];
				*plat = *(plat_t *)th;
				plat->sector = (sector_t *)(plat->sector - sectors);
			}
			index += sizeof(*plat);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_LightFlash)
		{
			if (buffer != NULL)
				buffer[index] = tc_flash;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				flash = (lightflash_t *)&buffer[index];
				*flash = *(lightflash_t *)th;
				flash->sector = (sector_t *)(flash->sector - sectors);
			}
			index += sizeof(*flash);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
		{
			if (buffer != NULL)
				buffer[index] = tc_strobe;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				strobe = (strobe_t *)&buffer[index];
				*strobe = *(strobe_t *)th;
				strobe->sector = (sector_t *)(strobe->sector - sectors);
			}
			index += sizeof(*strobe);
			continue;
		}

		if (th->function.acp1 == (actionf_p1)T_Glow)
		{
			if (buffer != NULL)
				buffer[index] = tc_glow;
			++index;
			PADSAVEP();
			if (buffer != NULL)
			{
				glow = (glow_t *)&buffer[index];
				*glow = *(glow_t *)th;
				glow->sector = (sector_t *)(glow->sector - sectors);
			}
			index += sizeof(*glow);
			continue;
		}
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
	specials_e          tclass;
	ceiling_t*          ceiling;
	vldoor_t*           door;
	floormove_t*        floor;
	plat_t*             plat;
	lightflash_t*       flash;
	strobe_t*           strobe;
	glow_t*             glow;

	/* read in saved thinkers */
	while (1)
	{
		tclass = (specials_e)buffer[index++];
		switch (tclass)
		{
		  case tc_endspecials:
			return index;     /* end of list */

		  case tc_ceiling:
			PADSAVEP();
			ceiling = (ceiling_t*)Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
			*ceiling = *(ceiling_t*)&buffer[index];
			index += sizeof(*ceiling);
			ceiling->sector = &sectors[(size_t)ceiling->sector];
			ceiling->sector->specialdata = ceiling;

			if (ceiling->thinker.function.acp1)
				ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

			P_AddThinker (&ceiling->thinker);
			P_AddActiveCeiling(ceiling);
			break;

		  case tc_door:
			PADSAVEP();
			door = (vldoor_t*)Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
			*door = *(vldoor_t*)&buffer[index];
			index += sizeof(*door);
			door->sector = &sectors[(size_t)door->sector];
			door->sector->specialdata = door;
			door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
			P_AddThinker (&door->thinker);
			break;

		  case tc_floor:
			PADSAVEP();
			floor = (floormove_t*)Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
			*floor = *(floormove_t*)&buffer[index];
			index += sizeof(*floor);
			floor->sector = &sectors[(size_t)floor->sector];
			floor->sector->specialdata = floor;
			floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
			P_AddThinker (&floor->thinker);
			break;

		  case tc_plat:
			PADSAVEP();
			plat = (plat_t*)Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
			*plat = *(plat_t*)&buffer[index];
			index += sizeof(*plat);
			plat->sector = &sectors[(size_t)plat->sector];
			plat->sector->specialdata = plat;

			if (plat->thinker.function.acp1)
				plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

			P_AddThinker (&plat->thinker);
			P_AddActivePlat(plat);
			break;

		  case tc_flash:
			PADSAVEP();
			flash = (lightflash_t*)Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
			*flash = *(lightflash_t*)&buffer[index];
			index += sizeof(*flash);
			flash->sector = &sectors[(size_t)flash->sector];
			flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
			P_AddThinker (&flash->thinker);
			break;

		  case tc_strobe:
			PADSAVEP();
			strobe = (strobe_t*)Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
			*strobe = *(strobe_t*)&buffer[index];
			index += sizeof(*strobe);
			strobe->sector = &sectors[(size_t)strobe->sector];
			strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
			P_AddThinker (&strobe->thinker);
			break;

		  case tc_glow:
			PADSAVEP();
			glow = (glow_t*)Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
			*glow = *(glow_t*)&buffer[index];
			index += sizeof(*glow);
			glow->sector = &sectors[(size_t)glow->sector];
			glow->thinker.function.acp1 = (actionf_p1)T_Glow;
			P_AddThinker (&glow->thinker);
			break;

		  default:
			I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
					 "in savegame",tclass);
		}

	}

	return index;
}

