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
        Player related stuff.
        Bobbing POV/weapon, movement.
        Pending weapon.

******************************************************************************/


#include "doomdef.h"
#include "d_event.h"

#include "p_local.h"

#include "doomstat.h"


/* Movement. */

/* 16 pixels of bob */
#define MAXBOB  0x100000

d_bool          onground;


/* P_Thrust */
/* Moves the given origin along a given angle. */
void
P_Thrust
( player_t*     player,
  angle_t       angle,
  fixed_t       move )
{
	angle >>= ANGLETOFINESHIFT;

	player->mo->momx += FixedMul(move,finecosine[angle]);
	player->mo->momy += FixedMul(move,finesine[angle]);
}




/* P_CalcHeight */
/* Calculate the walking / running height adjustment */
void P_CalcHeight (player_t* player)
{
	int         angle;
	fixed_t     bob;

	/* Regular movement bobbing */
	/* (needs to be calculated for gun swing */
	/* even if not on ground) */
	/* OPTIMIZE: tablify angle */
	/* Note: a LUT allows for effects */
	/*  like a ramp with low health. */
	player->bob =
		FixedMul (player->mo->momx, player->mo->momx)
		+ FixedMul (player->mo->momy,player->mo->momy);

	player->bob >>= 2;

	if (player->bob>MAXBOB)
		player->bob = MAXBOB;

	if ((player->cheats & CF_NOMOMENTUM) || !onground)
	{
		player->viewz = player->mo->z + VIEWHEIGHT;

		if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;

		player->viewz = player->mo->z + player->viewheight;
		return;
	}

	angle = (FINEANGLES/20*leveltime)&FINEMASK;
	bob = FixedMul ( player->bob/2, finesine[angle]);


	/* move viewheight */
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;

		if (player->viewheight > VIEWHEIGHT)
		{
			player->viewheight = VIEWHEIGHT;
			player->deltaviewheight = 0;
		}

		if (player->viewheight < VIEWHEIGHT/2)
		{
			player->viewheight = VIEWHEIGHT/2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if (player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT/4;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	player->viewz = player->mo->z + player->viewheight + bob;

	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		player->viewz = player->mo->ceilingz-4*FRACUNIT;
}



/* P_MovePlayer */
void P_MovePlayer (player_t* player)
{
	ticcmd_t*           cmd;

	cmd = &player->cmd;

	player->mo->angle += (cmd->angleturn<<16);

	/* Do not let the player control movement */
	/*  if not onground. */
	onground = (player->mo->z <= player->mo->floorz);

	if (cmd->forwardmove && onground)
		P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);

	if (cmd->sidemove && onground)
		P_Thrust (player, player->mo->angle-ANG90, cmd->sidemove*2048);

	if ( (cmd->forwardmove || cmd->sidemove)
		 && player->mo->state == &states[S_PLAY] )
	{
		P_SetMobjState (player->mo, S_PLAY_RUN1);
	}
}



/* P_DeathThink */
/* Fall on your face when dying. */
/* Decrease POV height to floor height. */
#define ANG5    (ANG90/18)

void P_DeathThink (player_t* player)
{
	angle_t             angle;
	angle_t             delta;

	P_MovePsprites (player);

	/* fall to the ground */
	if (player->viewheight > 6*FRACUNIT)
		player->viewheight -= FRACUNIT;

	if (player->viewheight < 6*FRACUNIT)
		player->viewheight = 6*FRACUNIT;

	player->deltaviewheight = 0;
	onground = (player->mo->z <= player->mo->floorz);
	P_CalcHeight (player);

	if (player->attacker && player->attacker != player->mo)
	{
		angle = R_PointToAngle2 (player->mo->x,
								 player->mo->y,
								 player->attacker->x,
								 player->attacker->y);

		delta = angle - player->mo->angle;

		if (delta < ANG5 || delta > (unsigned)-ANG5)
		{
			/* Looking at killer, */
			/*  so fade damage flash down. */
			player->mo->angle = angle;

			if (player->damagecount)
				player->damagecount--;
		}
		else if (delta < ANG180)
			player->mo->angle += ANG5;
		else
			player->mo->angle -= ANG5;
	}
	else if (player->damagecount)
		player->damagecount--;


	if (player->cmd.buttons & BT_USE)
		player->playerstate = PST_REBORN;
}



static d_bool WeaponValid(const player_t* const player, const weapontype_t weapon)
{
	if (!player->weaponowned[weapon])
		return d_false;

	/* Disable plasma rifle and BFG if playing shareware Doom. */
	if ((weapon == wp_plasma || weapon == wp_bfg) && gamemode == shareware)
		return d_false;

	/* Disable super shotgun if not playing Doom II. */
	if (weapon == wp_supershotgun && gamemode != commercial)
		return d_false;

	return d_true;
}



/* P_PlayerThink */
void P_PlayerThink (player_t* player)
{
	ticcmd_t*           cmd;
	weapontype_t        newweapon;

	/* fixme: do this in the cheat code */
	if (player->cheats & CF_NOCLIP)
		player->mo->flags |= MF_NOCLIP;
	else
		player->mo->flags &= ~MF_NOCLIP;

	/* chain saw run forward */
	cmd = &player->cmd;
	if (player->mo->flags & MF_JUSTATTACKED)
	{
		cmd->angleturn = 0;
		cmd->forwardmove = 0xc800/512;
		cmd->sidemove = 0;
		player->mo->flags &= ~MF_JUSTATTACKED;
	}


	if (player->playerstate == PST_DEAD)
	{
		P_DeathThink (player);
		return;
	}

	/* Move around. */
	/* Reactiontime is used to prevent movement */
	/*  for a bit after a teleport. */
	if (player->mo->reactiontime)
		player->mo->reactiontime--;
	else
		P_MovePlayer (player);

	P_CalcHeight (player);

	if (player->mo->subsector->sector->special)
		P_PlayerInSpecialSector (player);

	/* Check for weapon change. */

	/* A special event has no other buttons. */
	if (cmd->buttons & BT_SPECIAL)
		cmd->buttons = 0;

	if (cmd->buttons & BT_CHANGE)
	{
		/* The actual changing of the weapon is done */
		/*  when the weapon psprite can do it */
		/*  (read: not in the middle of an attack). */
		newweapon = (weapontype_t)((cmd->buttons>>BT_WEAPONSHIFT)&BT_WEAPONMASK);

		if (newweapon == wp_fist
			&& player->weaponowned[wp_chainsaw]
			&& !(player->readyweapon == wp_chainsaw
				 && player->powers[pw_strength]))
		{
			newweapon = wp_chainsaw;
		}

		if (newweapon == wp_shotgun
			&& WeaponValid(player, wp_supershotgun)
			&& player->readyweapon != wp_supershotgun)
		{
			newweapon = wp_supershotgun;
		}


		if (newweapon != player->readyweapon && WeaponValid(player, newweapon))
				player->pendingweapon = newweapon;
	}

	if (cmd->buttons & BT_CYCLE)
	{
		int index;

		static const weapontype_t order[] = {wp_fist, wp_chainsaw, wp_pistol, wp_shotgun, wp_supershotgun, wp_chaingun, wp_missile, wp_plasma, wp_bfg};
		static const int type_to_index[] = {0, 2, 3, 5, 6, 7, 8, 1, 4};

		index = type_to_index[player->readyweapon];

		do
		{
			if (cmd->buttons & 1)
				++index;
			else
				--index;

			if (index == -1)
				index = D_COUNT_OF(order) - 1;
			else if (index == D_COUNT_OF(order))
				index = 0;
		} while (!WeaponValid(player, order[index]));

		player->pendingweapon = order[index];
	}

	/* check for use */
	if (cmd->buttons & BT_USE)
	{
		if (!player->usedown)
		{
			P_UseLines (player);
			player->usedown = d_true;
		}
	}
	else
		player->usedown = d_false;

	/* cycle psprites */
	P_MovePsprites (player);

	/* Counters, time dependend power ups. */

	/* Strength counts up to diminish fade. */
	if (player->powers[pw_strength])
		player->powers[pw_strength]++;

	if (player->powers[pw_invulnerability])
		player->powers[pw_invulnerability]--;

	if (player->powers[pw_invisibility])
		if (! --player->powers[pw_invisibility] )
			player->mo->flags &= ~MF_SHADOW;

	if (player->powers[pw_infrared])
		player->powers[pw_infrared]--;

	if (player->powers[pw_ironfeet])
		player->powers[pw_ironfeet]--;

	if (player->damagecount)
		player->damagecount--;

	if (player->bonuscount)
		player->bonuscount--;


	/* Handling colormaps. */
	if (player->powers[pw_invulnerability])
	{
		if (player->powers[pw_invulnerability] > 4*32
			|| (player->powers[pw_invulnerability]&8) )
			player->fixedcolormap = INVERSECOLORMAP;
		else
			player->fixedcolormap = 0;
	}
	else if (player->powers[pw_infrared])
	{
		if (player->powers[pw_infrared] > 4*32
			|| (player->powers[pw_infrared]&8) )
			player->fixedcolormap = LIGHTAMPCOLORMAP;
		else
			player->fixedcolormap = 0;
	}
	else
		player->fixedcolormap = 0;
}


