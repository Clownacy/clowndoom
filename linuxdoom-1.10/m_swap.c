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
        Endianess handling, swapping 16bit and 32bit.

******************************************************************************/

#include "m_swap.h"


/* Needed for big endian. */
#ifdef PLATFORM_IS_BIG_ENDIAN

/* Swap 16bit, that is, MSB and LSB byte. */
unsigned short SwapSHORT(unsigned short x)
{
	return
		((x>>8) & 0xff)
		| ((x<<8) & 0xff00);
}

/* Swapping 32bit. */
unsigned long SwapLONG( unsigned long x)
{
	return
		((x>>24) & 0xff)
		| ((x>>8) & 0xff00)
		| ((x<<8) & 0xff0000)
		| ((x<<24) & 0xff000000);
}


#endif


