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
        Fixed point implementation.

******************************************************************************/


#include <limits.h>
#include <stdlib.h>

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"



fixed_t
FixedMul
( fixed_t       a,
  fixed_t       b )
{
	const cc_bool result_is_negative = (a < 0) != (b < 0);

	const unsigned int a_absolute = abs(a);
	const unsigned int b_absolute = abs(b);

	const unsigned int a_upper = a_absolute / FRACUNIT;
	const unsigned int a_lower = a_absolute % FRACUNIT;
	const unsigned int b_upper = b_absolute / FRACUNIT;
	const unsigned int b_lower = b_absolute % FRACUNIT;

	/* This code is burdened by having to recreate a rounding error in the original code that was caused by right-shifting a signed integer. Stupid. */
	const int result = (int)((a_upper * b_upper * FRACUNIT) + (a_upper * b_lower) + (a_lower * b_upper) + ((a_lower * b_lower + (result_is_negative ? FRACUNIT-1 : 0)) / FRACUNIT));

	return result_is_negative ? -result : result;

#if 0
	/* Old legacy code. */
	return ((long long) a * (long long) b) >> FRACBITS;
#endif
}



/* FixedDiv, C version. */

fixed_t
FixedDiv
( fixed_t       a,
  fixed_t       b )
{
	/* Horrific fixed point division using only 32-bit integers. */
	/* This particular algorithm is taken from the Atari Jaguar port. */
	const cc_bool result_is_negative = (a < 0) != (b < 0);

	unsigned int bit = FRACUNIT;
	unsigned int result = 0;
	unsigned int dividend_absolute = abs(a);
	unsigned int divisor_absolute = abs(b);

	if ((dividend_absolute >> 14) >= divisor_absolute)
		return result_is_negative ? INT_MIN : INT_MAX;

	while (dividend_absolute > divisor_absolute)
	{
		divisor_absolute <<= 1;
		bit <<= 1;
	}

	do
	{
		if (dividend_absolute >= divisor_absolute)
		{
			dividend_absolute -= divisor_absolute;
			result |= bit;
		}

		dividend_absolute <<= 1;
		bit >>= 1;
	} while (bit != 0 && dividend_absolute != 0);

	return result_is_negative ? -(int)result : (int)result;
}



#if 0
/* Old legacy code. */
fixed_t
FixedDiv
( fixed_t       a,
  fixed_t       b )
{
	if ( (abs(a)>>14) >= abs(b))
		return (a^b)<0 ? INT_MIN : INT_MAX;
	return FixedDiv2 (a,b);
}



fixed_t
FixedDiv2
( fixed_t       a,
  fixed_t       b )
{
#if 1
	long long c;
	c = ((long long)a<<16) / ((long long)b);
	return (fixed_t) c;
#else
	double c;

	c = ((double)a) / ((double)b) * FRACUNIT;

	if (c >= 2147483648.0 || c < -2147483648.0)
		I_Error("FixedDiv: divide by zero");
	return (fixed_t) c;
#endif
}
#endif
