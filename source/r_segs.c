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
        All the clipping: columns, horizontal spans, sky columns.

******************************************************************************/

#include <limits.h>
#include <stdlib.h>

#include "i_system.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"

/* OPTIMIZE: closed two sided lines as single sided */

/* True if any of the segs textures might be visible. */
d_bool          segtextured;

/* False if the back side is the same plane. */
d_bool          markfloor;
d_bool          markceiling;

d_bool          maskedtexture;
int             toptexture;
int             bottomtexture;
int             midtexture;


angle_t         rw_normalangle;
/* angle to line origin */
angle_t         rw_angle1;

/* regular wall */
int             rw_x;
int             rw_stopx;
angle_t         rw_centerangle;
fixed_t         rw_offset;
fixed_t         rw_distance;
fixed_t         rw_scale;
fixed_t         rw_scalestep;
fixed_t         rw_midtexturemid;
fixed_t         rw_toptexturemid;
fixed_t         rw_bottomtexturemid;

int             worldtop;
int             worldbottom;
int             worldhigh;
int             worldlow;

fixed_t         pixhigh;
fixed_t         pixlow;
fixed_t         pixhighstep;
fixed_t         pixlowstep;

fixed_t         topfrac;
fixed_t         topstep;

fixed_t         bottomfrac;
fixed_t         bottomstep;


colourindex_t** walllights;

short*          maskedtexturecol;

/*
 * R_FixWiggle()
 * Dynamic wall/texture rescaler, AKA "WiggleHack II"
 *  by Kurt "kb1" Baumgardner ("kb") and Andrey "Entryway" Budko ("e6y")
 *
 *  [kb] When the rendered view is positioned, such that the viewer is
 *   looking almost parallel down a wall, the result of the scale
 *   calculation in R_ScaleFromGlobalAngle becomes very large. And, the
 *   taller the wall, the larger that value becomes. If these large
 *   values were used as-is, subsequent calculations would overflow,
 *   causing full-screen HOM, and possible program crashes.
 *
 *  Therefore, vanilla Doom clamps this scale calculation, preventing it
 *   from becoming larger than 0x400000 (64*FRACUNIT). This number was
 *   chosen carefully, to allow reasonably-tight angles, with reasonably
 *   tall sectors to be rendered, within the limits of the fixed-point
 *   math system being used. When the scale gets clamped, Doom cannot
 *   properly render the wall, causing an undesirable wall-bending
 *   effect that I call "floor wiggle". Not a crash, but still ugly.
 *
 *  Modern source ports offer higher video resolutions, which worsens
 *   the issue. And, Doom is simply not adjusted for the taller walls
 *   found in many PWADs.
 *
 *  This code attempts to correct these issues, by dynamically
 *   adjusting the fixed-point math, and the maximum scale clamp,
 *   on a wall-by-wall basis. This has 2 effects:
 *
 *  1. Floor wiggle is greatly reduced and/or eliminated.
 *  2. Overflow is no longer possible, even in levels with maximum
 *     height sectors (65535 is the theoretical height, though Doom
 *     cannot handle sectors > 32767 units in height.
 *
 *  The code is not perfect across all situations. Some floor wiggle can
 *   still be seen, and some texture strips may be slightly misaligned in
 *   extreme cases. These effects cannot be corrected further, without
 *   increasing the precision of various renderer variables, and, 
 *   possibly, creating a noticable performance penalty.
 */

static int max_rwscale = 64 * FRACUNIT;
static int heightbits = 12;
static int heightunit = 1 << 12;
static int invhgtbits = 4;
 
static const struct
{
	int clamp;
	int heightbits;
} scale_values[8] = {
	/* TODO: Implement an equivalent to Killough's 'centeryfrac' fix so that this hack is not necessary. */
	{/*2048*/1024 * FRACUNIT, 12}, /* Hack to prevent crash when going out-of-bounds. */
	{1024 * FRACUNIT, 12},
	{1024 * FRACUNIT, 11},
	{ 512 * FRACUNIT, 11},
	{ 512 * FRACUNIT, 10},
	{ 256 * FRACUNIT, 10},
	{ 256 * FRACUNIT,  9},
	{ 128 * FRACUNIT,  9}
};

void R_FixWiggle(sector_t* const sector)
{
	static int	lastheight = 0;
	int		height = (sector->ceilingheight - sector->floorheight) >> FRACBITS;

	/* Disallow negative heights. using 1 forces cache initialization. */
	if (height < 1)
		height = 1;

	/* Early out? */
	if (lastheight != height)
	{
		lastheight = height;

		/* Initialize, or handle moving sector. */
		if (sector->cachedheight != height)
		{
			sector->cachedheight = height;
			sector->scaleindex = 0;
			height /= 128;

			/* Calculate adjustment. */
			while ((height /= 2) != 0)
				++sector->scaleindex;
		}

		/* Fine-tune renderer for this wall. */
		max_rwscale = scale_values[sector->scaleindex].clamp;
		heightbits = scale_values[sector->scaleindex].heightbits;
		heightunit = 1 << heightbits;
		invhgtbits = FRACBITS - heightbits;
	}
}

/* R_RenderMaskedSegRange */
void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2)
{
	unsigned int index;
	column_t    *col;
	int          lightnum;
	int          texnum;

	/* Calculate light table. */
	/* Use different light tables */
	/*   for horizontal / vertical / diagonal. Diagonal? */
	/* OPTIMIZE: get rid of LIGHTSEGSHIFT globally */
	curline = ds->curline;
	frontsector = curline->frontsector;
	backsector = curline->backsector;
	texnum = texturetranslation[curline->sidedef->midtexture];

	lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight;

	if (curline->v1->y == curline->v2->y)
		lightnum--;
	else if (curline->v1->x == curline->v2->x)
		lightnum++;

	if (lightnum < 0)
		walllights = scalelight[0];
	else if (lightnum >= LIGHTLEVELS)
		walllights = scalelight[LIGHTLEVELS-1];
	else
		walllights = scalelight[lightnum];

	maskedtexturecol = ds->maskedtexturecol;

	rw_scalestep = ds->scalestep;
	spryscale = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
	mfloorclip = ds->sprbottomclip;
	mceilingclip = ds->sprtopclip;

	/* find positioning */
	if (curline->linedef->flags & ML_DONTPEGBOTTOM)
	{
		dc_texturemid = D_MAX(frontsector->floorheight, backsector->floorheight);
		dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
	}
	else
	{
		dc_texturemid = D_MIN(frontsector->ceilingheight, backsector->ceilingheight);
		dc_texturemid = dc_texturemid - viewz;
	}
	dc_texturemid += curline->sidedef->rowoffset;

	if (fixedcolormap)
		dc_colormap = fixedcolormap;

	/* draw the columns */
	for (dc_x = x1; dc_x <= x2; ++dc_x)
	{
		/* calculate lighting */
		if (maskedtexturecol[dc_x] != SHRT_MAX)
		{
			if (!fixedcolormap)
			{
				index = spryscale/light_y_scale;

				if (index >=  MAXLIGHTSCALE )
					index = MAXLIGHTSCALE - 1;

				dc_colormap = walllights[index];
			}

			sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);
			dc_iscale = 0xFFFFFFFFu / (unsigned int)spryscale;

			/* draw the texture */
			col = (column_t *)(R_GetColumn(texnum, maskedtexturecol[dc_x]) - 3);

			R_DrawMaskedColumn(col);
			maskedtexturecol[dc_x] = SHRT_MAX;
		}

		spryscale += rw_scalestep;
	}

}

/* R_RenderSegLoop */
/* Draws zero, one, or two textures (and possibly a masked */
/*  texture) for walls. */
/* Can draw or mark the starting pixel of floor and ceiling */
/*  textures. */
/* CALLED: CORE LOOPING ROUTINE. */
void R_RenderSegLoop (void)
{
	angle_t      angle;
	unsigned int index;
	int          yl;
	int          yh;
	int          mid;
	fixed_t      texturecolumn;
	int          top;
	int          bottom;

	texturecolumn = 0; /* shut up compiler warning */

	for (; rw_x < rw_stopx; ++rw_x)
	{
		/* mark floor / ceiling areas */
		yl = (topfrac + heightunit - 1) >> heightbits;

		/* no space above wall? */
		if (yl < ceilingclip[rw_x] + 1)
			yl = ceilingclip[rw_x] + 1;

		if (markceiling)
		{
			top = ceilingclip[rw_x] + 1;
			bottom = yl - 1;

			if (bottom >= floorclip[rw_x])
				bottom = floorclip[rw_x] - 1;

			if (top <= bottom)
			{
				ceilingplane->top[rw_x] = top;
				ceilingplane->bottom[rw_x] = bottom;
			}
		}

		yh = bottomfrac>>heightbits;

		if (yh >= floorclip[rw_x])
			yh = floorclip[rw_x] - 1;

		if (markfloor)
		{
			top = yh + 1;
			bottom = floorclip[rw_x] - 1;
			if (top <= ceilingclip[rw_x])
				top = ceilingclip[rw_x] + 1;
			if (top <= bottom)
			{
				floorplane->top[rw_x] = top;
				floorplane->bottom[rw_x] = bottom;
			}
		}

		/* texturecolumn and lighting are independent of wall tiers */
		if (segtextured)
		{
			/* calculate texture offset */
			angle = (rw_centerangle + xtoviewangle[rw_x]) >> ANGLETOFINESHIFT;
			texturecolumn = rw_offset - FixedMul(finetangent[angle & (FINEANGLES / 2 - 1)], rw_distance);
			texturecolumn >>= FRACBITS;
			/* calculate lighting */
			index = rw_scale/light_y_scale;

			if (index >=  MAXLIGHTSCALE )
				index = MAXLIGHTSCALE - 1;

			dc_colormap = walllights[index];
			dc_x = rw_x;
			dc_iscale = 0xffffffffu / (unsigned)rw_scale;
		}

		/* draw the wall tiers */
		if (midtexture)
		{
			/* single sided line */
			dc_yl = yl;
			dc_yh = yh;
			dc_texturemid = rw_midtexturemid;
			dc_source = R_GetColumn(midtexture, texturecolumn);
			colfunc();
			ceilingclip[rw_x] = viewheight;
			floorclip[rw_x] = -1;
		}
		else
		{
			/* two sided line */
			if (toptexture)
			{
				/* top wall */
				mid = pixhigh >> heightbits;
				pixhigh += pixhighstep;

				if (mid >= floorclip[rw_x])
					mid = floorclip[rw_x] - 1;

				if (mid >= yl)
				{
					dc_yl = yl;
					dc_yh = mid;
					dc_texturemid = rw_toptexturemid;
					dc_source = R_GetColumn(toptexture, texturecolumn);
					colfunc();
					ceilingclip[rw_x] = mid;
				}
				else
					ceilingclip[rw_x] = yl - 1;
			}
			else
			{
				/* no top wall */
				if (markceiling)
					ceilingclip[rw_x] = yl - 1;
			}

			if (bottomtexture)
			{
				/* bottom wall */
				mid = (pixlow + heightunit - 1) >> heightbits;
				pixlow += pixlowstep;

				/* no space above wall? */
				if (mid <= ceilingclip[rw_x])
					mid = ceilingclip[rw_x] + 1;

				if (mid <= yh)
				{
					dc_yl = mid;
					dc_yh = yh;
					dc_texturemid = rw_bottomtexturemid;
					dc_source = R_GetColumn(bottomtexture, texturecolumn);
					colfunc();
					floorclip[rw_x] = mid;
				}
				else
					floorclip[rw_x] = yh + 1;
			}
			else
			{
				/* no bottom wall */
				if (markfloor)
					floorclip[rw_x] = yh + 1;
			}

			if (maskedtexture)
			{
				/* save texturecol */
				/*  for backdrawing of masked mid texture */
				maskedtexturecol[rw_x] = texturecolumn;
			}
		}

		rw_scale += rw_scalestep;
		topfrac += topstep;
		bottomfrac += bottomstep;
	}
}

/* R_ScaleFromGlobalAngle */
/* Returns the texture mapping scale */
/*  for the current line (horizontal span) */
/*  at the given angle. */
/* rw_distance must be calculated first. */
static fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
	fixed_t             scale;
	angle_t             anglea;
	angle_t             angleb;
	int                 sinea;
	int                 sineb;
	fixed_t             num;
	int                 den;

	/* UNUSED */
#if 0
{
	fixed_t             dist;
	fixed_t             z;
	fixed_t             sinv;
	fixed_t             cosv;

	sinv = finesine[(visangle-rw_normalangle)>>ANGLETOFINESHIFT];
	dist = FixedDiv (rw_distance, sinv);
	cosv = finecosine[(viewangle-visangle)>>ANGLETOFINESHIFT];
	z = labs(FixedMul (dist, cosv));
	scale = FixedDiv(projection, z);
	return scale;
}
#endif

	anglea = ANG90 + (visangle-viewangle);
	angleb = ANG90 + (visangle-rw_normalangle);

	/* both sines are allways positive */
	sinea = finesine[anglea>>ANGLETOFINESHIFT];
	sineb = finesine[angleb>>ANGLETOFINESHIFT];
	num = FixedMul(projection,sineb)<<detailshift;
	den = FixedMul(rw_distance,sinea);

	if (den > num>>16)
	{
		scale = FixedDiv (num, den);

		/* [kb] When this evaluates True, the scale is clamped,
		   and there will be some wiggling. */
		if (scale > max_rwscale)
			scale = max_rwscale;
		else if (scale < 256)
			scale = 256;
	}
	else
		scale = max_rwscale;

	return scale;
}

/* R_StoreWallRange */
/* A wall segment will be drawn */
/*  between start and stop pixels (inclusive). */
void R_StoreWallRange(int start, int stop)
{
	fixed_t hyp;
	fixed_t sineval;
	angle_t distangle, offsetangle;
	fixed_t vtop;
	int     lightnum;

	/* don't overflow and crash */
	if (ds_p == &drawsegs[MAXDRAWSEGS])
		return;

#ifdef RANGECHECK
	if (start >=viewwidth || start > stop)
		I_Error("Bad R_RenderWallRange: %i to %i", start, stop);
#endif

	sidedef = curline->sidedef;
	linedef = curline->linedef;

	/* mark the segment as visible for auto map */
	linedef->flags |= ML_MAPPED;

	/* calculate rw_distance for scale calculation */
	rw_normalangle = curline->angle + ANG90;
	offsetangle = rw_normalangle-rw_angle1;

	if (offsetangle > ANG180)
		offsetangle = -offsetangle;

	if (offsetangle > ANG90)
		offsetangle = ANG90;

	distangle = ANG90 - offsetangle;
	hyp = R_PointToDist(curline->v1->x, curline->v1->y);
	sineval = finesine[distangle >> ANGLETOFINESHIFT];
	rw_distance = FixedMul(hyp, sineval);


	ds_p->x1 = rw_x = start;
	ds_p->x2 = stop;
	ds_p->curline = curline;
	rw_stopx = stop+1;

	R_FixWiggle(frontsector);

	/* calculate scale at both ends and step */
	ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle(viewangle + xtoviewangle[start]);

	if (stop > start )
	{
		ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
		ds_p->scalestep = rw_scalestep = (ds_p->scale2 - rw_scale) / (stop - start);
	}
	else
	{
		/* UNUSED: try to fix the stretched line bug */
#if 0
		if (rw_distance < FRACUNIT/2)
		{
			fixed_t trx,try;
			fixed_t gxt,gyt;

			trx = curline->v1->x - viewx;
			try = curline->v1->y - viewy;

			gxt = FixedMul(trx,viewcos);
			gyt = -FixedMul(try,viewsin);
			ds_p->scale1 = FixedDiv(projection, gxt-gyt) << detailshift;
		}
#endif
		ds_p->scale2 = ds_p->scale1;
	}

	/* calculate texture boundaries */
	/*  and decide if floor / ceiling marks are needed */
	worldtop = frontsector->ceilingheight - viewz;
	worldbottom = frontsector->floorheight - viewz;

	midtexture = toptexture = bottomtexture = maskedtexture = 0;
	ds_p->maskedtexturecol = NULL;

	if (!backsector)
	{
		/* single sided line */
		midtexture = texturetranslation[sidedef->midtexture];
		/* a single sided line is terminal, so it must mark ends */
		markfloor = markceiling = d_true;
		if (linedef->flags & ML_DONTPEGBOTTOM)
		{
			vtop = frontsector->floorheight + textureheight[sidedef->midtexture];
			/* bottom of texture at bottom */
			rw_midtexturemid = vtop - viewz;
		}
		else
		{
			/* top of texture at top */
			rw_midtexturemid = worldtop;
		}
		rw_midtexturemid += sidedef->rowoffset;

		ds_p->silhouette = SIL_BOTH;
		ds_p->sprtopclip = screenheightarray;
		ds_p->sprbottomclip = negonearray;
		ds_p->bsilheight = INT_MAX;
		ds_p->tsilheight = INT_MIN;
	}
	else
	{
		/* two sided line */
		ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
		ds_p->silhouette = 0;

		if (frontsector->floorheight > backsector->floorheight)
		{
			ds_p->silhouette = SIL_BOTTOM;
			ds_p->bsilheight = frontsector->floorheight;
		}
		else if (backsector->floorheight > viewz)
		{
			ds_p->silhouette = SIL_BOTTOM;
			ds_p->bsilheight = INT_MAX;
			/* ds_p->sprbottomclip = negonearray; */
		}

		if (frontsector->ceilingheight < backsector->ceilingheight)
		{
			ds_p->silhouette |= SIL_TOP;
			ds_p->tsilheight = frontsector->ceilingheight;
		}
		else if (backsector->ceilingheight < viewz)
		{
			ds_p->silhouette |= SIL_TOP;
			ds_p->tsilheight = INT_MIN;
			/* ds_p->sprtopclip = screenheightarray; */
		}

		if (backsector->ceilingheight <= frontsector->floorheight)
		{
			ds_p->sprbottomclip = negonearray;
			ds_p->bsilheight = INT_MAX;
			ds_p->silhouette |= SIL_BOTTOM;
		}

		if (backsector->floorheight >= frontsector->ceilingheight)
		{
			ds_p->sprtopclip = screenheightarray;
			ds_p->tsilheight = INT_MIN;
			ds_p->silhouette |= SIL_TOP;
		}

		worldhigh = backsector->ceilingheight - viewz;
		worldlow = backsector->floorheight - viewz;

		/* hack to allow height changes in outdoor areas */
		if (frontsector->ceilingpic == skyflatnum
			&& backsector->ceilingpic == skyflatnum)
		{
			worldtop = worldhigh;
		}


		if (worldlow != worldbottom
			|| backsector->floorpic != frontsector->floorpic
			|| backsector->lightlevel != frontsector->lightlevel)
		{
			markfloor = d_true;
		}
		else
		{
			/* same plane on both sides */
			markfloor = d_false;
		}


		if (worldhigh != worldtop
			|| backsector->ceilingpic != frontsector->ceilingpic
			|| backsector->lightlevel != frontsector->lightlevel)
		{
			markceiling = d_true;
		}
		else
		{
			/* same plane on both sides */
			markceiling = d_false;
		}

		if (backsector->ceilingheight <= frontsector->floorheight
			|| backsector->floorheight >= frontsector->ceilingheight)
		{
			/* closed door */
			markceiling = markfloor = d_true;
		}


		if (worldhigh < worldtop)
		{
			/* top texture */
			toptexture = texturetranslation[sidedef->toptexture];
			if (linedef->flags & ML_DONTPEGTOP)
			{
				/* top of texture at top */
				rw_toptexturemid = worldtop;
			}
			else
			{
				vtop = backsector->ceilingheight + textureheight[sidedef->toptexture];

				/* bottom of texture */
				rw_toptexturemid = vtop - viewz;
			}
		}
		if (worldlow > worldbottom)
		{
			/* bottom texture */
			bottomtexture = texturetranslation[sidedef->bottomtexture];

			if (linedef->flags & ML_DONTPEGBOTTOM)
			{
				/* bottom of texture at bottom */
				/* top of texture at top */
				rw_bottomtexturemid = worldtop;
			}
			else
			{
				/* top of texture at top */
				rw_bottomtexturemid = worldlow;
			}
		}
		rw_toptexturemid += sidedef->rowoffset;
		rw_bottomtexturemid += sidedef->rowoffset;

		/* allocate space for masked texture tables */
		if (sidedef->midtexture)
		{
			/* masked midtexture */
			maskedtexture = d_true;
			ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
			lastopening += rw_stopx - rw_x;
		}
	}

	/* calculate rw_offset (only needed for textured lines) */
	segtextured = midtexture || toptexture || bottomtexture || maskedtexture;

	if (segtextured)
	{
		offsetangle = rw_normalangle-rw_angle1;

		if (offsetangle > ANG180)
			offsetangle = -offsetangle;

		if (offsetangle > ANG90)
			offsetangle = ANG90;

		sineval = finesine[offsetangle >> ANGLETOFINESHIFT];
		rw_offset = FixedMul (hyp, sineval);

		if (rw_normalangle-rw_angle1 < ANG180)
			rw_offset = -rw_offset;

		rw_offset += sidedef->textureoffset + curline->offset;
		rw_centerangle = ANG90 + viewangle - rw_normalangle;

		/* calculate light table */
		/*  use different light tables */
		/*  for horizontal / vertical / diagonal */
		/* OPTIMIZE: get rid of LIGHTSEGSHIFT globally */
		if (!fixedcolormap)
		{
			lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT) + extralight;

			/* fake contrast */
			if (curline->v1->y == curline->v2->y)
				lightnum--;
			else if (curline->v1->x == curline->v2->x)
				lightnum++;

			if (lightnum < 0)
				walllights = scalelight[0];
			else if (lightnum >= LIGHTLEVELS)
				walllights = scalelight[LIGHTLEVELS - 1];
			else
				walllights = scalelight[lightnum];
		}
	}

	/* if a floor / ceiling plane is on the wrong side */
	/*  of the view plane, it is definitely invisible */
	/*  and doesn't need to be marked. */

	if (frontsector->floorheight >= viewz)
	{
		/* above view plane */
		markfloor = d_false;
	}

	if (frontsector->ceilingheight <= viewz && frontsector->ceilingpic != skyflatnum)
	{
		/* below view plane */
		markceiling = d_false;
	}


	/* calculate incremental stepping values for texture edges */
	worldtop >>= invhgtbits;
	worldbottom >>= invhgtbits;

	topstep = -FixedMul(rw_scalestep, worldtop);
	topfrac = (centeryfrac >> invhgtbits) - FixedMul(worldtop, rw_scale);

	bottomstep = -FixedMul(rw_scalestep, worldbottom);
	bottomfrac = (centeryfrac >> invhgtbits) - FixedMul(worldbottom, rw_scale);

	if (backsector)
	{
		worldhigh >>= invhgtbits;
		worldlow >>= invhgtbits;

		if (worldhigh < worldtop)
		{
			pixhigh = (centeryfrac >> invhgtbits) - FixedMul(worldhigh, rw_scale);
			pixhighstep = -FixedMul(rw_scalestep, worldhigh);
		}

		if (worldlow > worldbottom)
		{
			pixlow = (centeryfrac >> invhgtbits) - FixedMul(worldlow, rw_scale);
			pixlowstep = -FixedMul(rw_scalestep, worldlow);
		}
	}

	/* render it */
	if (markceiling)
		ceilingplane = R_CheckPlane(ceilingplane, rw_x, rw_stopx-1);

	if (markfloor)
		floorplane = R_CheckPlane(floorplane, rw_x, rw_stopx-1);

	R_RenderSegLoop ();


	/* save sprite clipping info */
	if (((ds_p->silhouette & SIL_TOP) || maskedtexture) && !ds_p->sprtopclip)
	{
		memcpy(lastopening, ceilingclip + start, 2 * (rw_stopx - start));
		ds_p->sprtopclip = lastopening - start;
		lastopening += rw_stopx - start;
	}

	if (((ds_p->silhouette & SIL_BOTTOM) || maskedtexture) && !ds_p->sprbottomclip)
	{
		memcpy(lastopening, floorclip + start, 2 * (rw_stopx - start));
		ds_p->sprbottomclip = lastopening - start;
		lastopening += rw_stopx - start;
	}

	if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
	{
		ds_p->silhouette |= SIL_TOP;
		ds_p->tsilheight = INT_MIN;
	}
	if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
	{
		ds_p->silhouette |= SIL_BOTTOM;
		ds_p->bsilheight = INT_MAX;
	}
	ds_p++;
}
