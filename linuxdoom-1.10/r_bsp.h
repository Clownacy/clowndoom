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
        Refresh module, BSP traversal and handling.

******************************************************************************/


#ifndef __R_BSP__
#define __R_BSP__


extern seg_t*           curline;
extern side_t*          sidedef;
extern line_t*          linedef;
extern sector_t*        frontsector;
extern sector_t*        backsector;

extern int              rw_x;
extern int              rw_stopx;

extern d_bool           segtextured;

/* false if the back side is the same plane */
extern d_bool           markfloor;
extern d_bool           markceiling;

extern d_bool           skymap;

extern drawseg_t        drawsegs[MAXDRAWSEGS];
extern drawseg_t*       ds_p;

extern colourindex_t**   hscalelight;
extern colourindex_t**   vscalelight;
extern colourindex_t**   dscalelight;


typedef void (*drawfunc_t) (int start, int stop);


/* BSP? */
void R_ClearClipSegs (void);
void R_ClearDrawSegs (void);


void R_RenderBSPNode (int bspnum);


#endif
