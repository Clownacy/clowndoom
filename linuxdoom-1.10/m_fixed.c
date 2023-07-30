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
  	Fixed point implementation.

******************************************************************************/


#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"

typedef struct SplitInteger
{
	unsigned long splits[3];
} SplitInteger;

static void Subtract(SplitInteger * const minuend, const SplitInteger * const subtrahend)
{
    unsigned long carry;
    unsigned int i;

    carry = 0;

    for (i = 3; i-- != 0; )
    {
        const unsigned long difference = minuend->splits[i] - subtrahend->splits[i] + carry;

        /* Isolate and sign-extend the overflow to obtain the new carry. */
        carry = difference >> 16;
        carry = (carry & 0x7FFF) - (carry & 0x8000);

        /* Store result. */
        minuend->splits[i] = difference & 0xFFFF;
    }
}

static boolean GreaterThan(const SplitInteger * const a, const SplitInteger * const b)
{
    unsigned int i;

    for (i = 0; i < 3; ++i)
    {
        if (a->splits[i] > b->splits[i])
            return true;
        else if (a->splits[i] < b->splits[i])
            return false;
    }

    return false;
}

static void LeftShift(SplitInteger * const value)
{
    unsigned long carry;
    unsigned int i;

    carry = 0;

    for (i = 3; i-- != 0; )
    {
        const unsigned long new_carry = value->splits[i] >> 15;
        value->splits[i] <<= 1;
	value->splits[i] |= carry;
	carry = new_carry;
        value->splits[i] &= 0xFFFF;
    }
}

static void RightShift(SplitInteger * const value)
{
    unsigned long carry;
    unsigned int i;

    carry = 0;

    for (i = 0; i < 3; ++i)
    {
        const unsigned long new_carry = value->splits[i] << 15;
        value->splits[i] >>= 1;
	value->splits[i] |= carry;
	carry = new_carry;
        value->splits[i] &= 0xFFFF;
    }
}



/* Fixme. __USE_C_FIXED__ or something. */

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
    const unsigned long a_absolute = labs(a);
    const unsigned long b_absolute = labs(b);

    const unsigned long a_upper = a_absolute / FRACUNIT;
    const unsigned long a_lower = a_absolute % FRACUNIT;
    const unsigned long b_upper = b_absolute / FRACUNIT;
    const unsigned long b_lower = b_absolute % FRACUNIT;

    const long result = (long)((a_upper * b_upper * FRACUNIT) + (a_upper * b_lower) + (a_lower * b_upper) + (a_lower * b_lower / FRACUNIT));

    return (a < 0) != (b < 0) ? -result : result;

#if 0
    /* Old legacy code. */
    return ((long long) a * (long long) b) >> FRACBITS;
#endif
}



/* FixedDiv, C version. */

fixed_t
FixedDiv
( fixed_t	a,
  fixed_t	b )
{
    if ( (abs(a)>>14) >= abs(b))
	return (a^b)<0 ? MININT : MAXINT;
    return FixedDiv2 (a,b);
}



fixed_t
FixedDiv2
( fixed_t	a,
  fixed_t	b )
{
    /* Horrific fixed point division using only 32-bit integers. */
    SplitInteger dividend, divisor;
    unsigned int shift_amount;
    unsigned long result;

    const unsigned long a_absolute = labs(a);
    const unsigned long b_absolute = labs(b);

    /* Note that this sneakily multiplies by FRACUNIT. */
    dividend.splits[0] = (a_absolute >> 16) & 0xFFFF;
    dividend.splits[1] = a_absolute & 0xFFFF;
    dividend.splits[2] = 0;

    divisor.splits[0] = 0;
    divisor.splits[1] = (b_absolute >> 16) & 0xFFFF;
    divisor.splits[2] = b_absolute & 0xFFFF;

    shift_amount = 0;

    while (GreaterThan(&dividend, &divisor))
    {
        LeftShift(&divisor);
        ++shift_amount;
    }

    result = 0;

    for (;;)
    {
        do
        {
            if (shift_amount == 0)
                return (a < 0) != (b < 0) ? -(long)result : (long)result;

            RightShift(&divisor);
            --shift_amount;

            result <<= 1;
        } while (GreaterThan(&divisor, &dividend));

        do
        {
            Subtract(&dividend, &divisor);
            ++result;
        } while (!GreaterThan(&divisor, &dividend));
    }
}



#if 0
/* Old legacy code. */
fixed_t
FixedDiv2
( fixed_t	a,
  fixed_t	b )
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
