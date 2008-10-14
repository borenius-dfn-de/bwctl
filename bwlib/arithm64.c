/*
 *      $Id$
 */
/************************************************************************
 *									*
 *			     Copyright (C)  2003			*
 *				Internet2				*
 *			     All Rights Reserved			*
 *									*
 ************************************************************************/
/*
 *	File:		arithm64.c
 *
 *	Author:		Jeff W. Boote
 *			Internet2
 *
 *	Date:		Tue Sep 16 14:25:16 MDT 2003
 *
 *	Description:
 *		Arithmatic and conversion functions for the BWLNum64
 *		type.
 *
 * BWLNum64 is interpreted as 32bits of "seconds" and 32bits of
 * "fractional seconds".
 * The byte ordering is defined by the hardware for this value. 4 MSBytes are
 * seconds, 4 LSBytes are fractional. Each set of 4 Bytes is pulled out
 * via shifts/masks as a 32bit unsigned int when needed independently.
 *
 *    License:
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the following copyright notice,
 *       this list of conditions and the disclaimer below.
 * 
 *        Copyright (c) 2003-2008, Internet2
 * 
 *                              All rights reserved.
 * 
 *     * Redistribution in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 * 
 *    *  Neither the name of Internet2 nor the names of its contributors may be
 *       used to endorse or promote products derived from this software without
 *       explicit prior written permission.
 * 
 * You are under no obligation whatsoever to provide any enhancements to Internet2,
 * or its contributors.  If you choose to provide your enhancements, or if you
 * choose to otherwise publish or distribute your enhancement, in source code form
 * without contemporaneously requiring end users to enter into a separate written
 * license agreement for such enhancements, then you thereby grant Internet2, its
 * contributors, and its members a non-exclusive, royalty-free, perpetual license
 * to copy, display, install, use, modify, prepare derivative works, incorporate
 * into the software or other computer software, distribute, and sublicense your
 * enhancements or derivative works thereof, in binary and source code form.
 * 
 * DISCLAIMER - THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * “AS IS” AND WITH ALL FAULTS.  THE UNIVERSITY OF DELAWARE, INTERNET2, ITS CONTRI-
 * BUTORS, AND ITS MEMBERS DO NOT IN ANY WAY WARRANT, GUARANTEE, OR ASSUME ANY RES-
 * PONSIBILITY, LIABILITY OR OTHER UNDERTAKING WITH RESPECT TO THE SOFTWARE. ANY E-
 * XPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRAN-
 * TIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
 * ARE HEREBY DISCLAIMED AND THE ENTIRE RISK OF SATISFACTORY QUALITY, PERFORMANCE,
 * ACCURACY, AND EFFORT IS WITH THE USER THEREOF.  IN NO EVENT SHALL THE COPYRIGHT
 * OWNER, CONTRIBUTORS, OR THE UNIVERSITY CORPORATION FOR ADVANCED INTERNET DEVELO-
 * PMENT, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTIT-
 * UTE GOODS OR SERVICES; REMOVAL OR REINSTALLATION LOSS OF USE, DATA, SAVINGS OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILIT-
 * Y, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHE-
 * RWISE) ARISING IN ANY WAY OUT OF THE USE OR DISTRUBUTION OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <bwlib/bwlib.h>

#define MASK32(x) ((x) & 0xFFFFFFFFUL)
#define BILLION 1000000000UL
#define MILLION 1000000UL
#define	EXP2POW32	0x100000000ULL

/************************************************************************
 *									*	
 *			Arithmetic functions				*
 *									*	
 ************************************************************************/

/*
 * Function:	BWLNum64Mult
 *
 * Description:	
 *	Multiplication. Allows overflow. Straightforward implementation
 *	of Knuth vol.2 Algorithm 4.3.1.M (p.268)
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
BWLNum64
BWLNum64Mult(
        BWLNum64	x,
        BWLNum64	y
        )
{
    unsigned long w[4];
    uint64_t xdec[2];
    uint64_t ydec[2];

    int i, j;
    uint64_t k, t;
    BWLNum64 ret;

    xdec[0] = MASK32(x);
    xdec[1] = MASK32(x>>32);
    ydec[0] = MASK32(y);
    ydec[1] = MASK32(y>>32);

    for (j = 0; j < 4; j++)
        w[j] = 0; 

    for (j = 0;  j < 2; j++) {
        k = 0;
        for (i = 0; ; ) {
            t = k + (xdec[i]*ydec[j]) + w[i + j];
            w[i + j] = (uint32_t)MASK32(t%EXP2POW32);
            k = t/EXP2POW32;
            if (++i < 2)
                continue;
            else {
                w[j + 2] = (uint32_t)MASK32(k);
                break;
            }
        }
    }

    ret = w[2];
    ret <<= 32;
    return w[1] + ret;
}

/************************************************************************
 *									*	
 *			Conversion functions				*
 *									*	
 ************************************************************************/

/*
 * Function:	BWLULongToNum64
 *
 * Description:	
 *	Convert an unsigned 32-bit integer into a BWLNum64 struct..
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
BWLNum64
BWLULongToNum64(uint32_t a)
{
    return ((uint64_t)a << 32);
}

/*
 * Function:	BWLI2numTToNum64
 *
 * Description:	
 *	Convert an unsigned 64-bit integer into a BWLNum64 struct..
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
BWLNum64
BWLI2numTToNum64(I2numT a)
{
    return (a << 32);
}


/*
 * Function:	BWLNum64toTimespec
 *
 * Description:	
 * 	Convert a time value in BWLNum64 representation to timespec
 * 	representation. These are "relative" time values. (Not absolutes - i.e.
 * 	they are not relative to some "epoch".) BWLNum64 values are
 * 	unsigned 64 integral types with the MS (Most Significant) 32 bits
 * 	representing seconds, and the LS (Least Significant) 32 bits
 * 	representing fractional seconds (at a resolution of 32 bits).
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
void
BWLNum64ToTimespec(
        struct timespec	*to,
        BWLNum64	from
        )
{
    /*
     * MS 32 bits represent seconds
     */
    to->tv_sec = (long)MASK32(from >> 32);

    /*
     * LS 32 bits represent fractional seconds, normalize them to nsecs:
     * frac/2^32 == nano/(10^9), so
     * nano = frac * 10^9 / 2^32
     */
    to->tv_nsec = (long)MASK32((MASK32(from)*BILLION) >> 32);

    while(to->tv_nsec >= (long)BILLION){
        to->tv_sec++;
        to->tv_nsec -= BILLION;
    }
}

/*
 * Function:	BWLTimespecToNum64
 *
 * Description:	
 *
 * 	Convert a time value in timespec representation to an BWLNum64
 * 	representation. These are "relative" time values. (Not absolutes - i.e.
 * 	they are not relative to some "epoch".) BWLNum64 values are
 * 	unsigned 64 integral types with the Most Significant 32 of those
 * 	64 bits representing seconds. The Least Significant 32 bits
 * 	represent fractional seconds at a resolution of 32 bits.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
void
BWLTimespecToNum64(
        BWLNum64	*to,
        struct timespec	*from
        )
{
    uint32_t	sec = from->tv_sec;
    uint32_t	nsec = from->tv_nsec;

    *to = 0;

    /*
     * Ensure nsec's is only fractional.
     */
    while(nsec >= BILLION){
        sec++;
        nsec -= BILLION;
    }

    /*
     * Place seconds in MS 32 bits.
     */
    *to = (uint64_t)MASK32(sec) << 32;
    /*
     * Normalize nsecs to 32bit fraction, then set that to LS 32 bits.
     */
    *to |= MASK32(((uint64_t)nsec << 32)/BILLION);

    return;
}
/*
 * Function:	BWLNum64toTimeval
 *
 * Description:	
 * 	Convert a time value in BWLNum64 representation to timeval
 * 	representation. These are "relative" time values. (Not absolutes - i.e.
 * 	they are not relative to some "epoch".) BWLNum64 values are
 * 	unsigned 64 integral types with the MS (Most Significant) 32 bits
 * 	representing seconds, and the LS (Least Significant) 32 bits
 * 	representing fractional seconds (at a resolution of 32 bits).
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
void
BWLNum64ToTimeval(
        struct timeval	*to,
        BWLNum64	from
        )
{
    /*
     * MS 32 bits represent seconds
     */
    to->tv_sec = (long)MASK32(from >> 32);

    /*
     * LS 32 bits represent fractional seconds, normalize them to usecs:
     * frac/2^32 == micro/(10^6), so
     * nano = frac * 10^6 / 2^32
     */
    to->tv_usec = (long)MASK32((MASK32(from)*MILLION) >> 32);

    while(to->tv_usec >= (long)MILLION){
        to->tv_sec++;
        to->tv_usec -= MILLION;
    }
}

/*
 * Function:	BWLTimevalToNum64
 *
 * Description:	
 *
 * 	Convert a time value in timeval representation to an BWLNum64
 * 	representation. These are "relative" time values. (Not absolutes - i.e.
 * 	they are not relative to some "epoch".) BWLNum64 values are
 * 	unsigned 64 integral types with the Most Significant 32 of those
 * 	64 bits representing seconds. The Least Significant 32 bits
 * 	represent fractional seconds at a resolution of 32 bits.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
void
BWLTimevalToNum64(
        BWLNum64	*to,
        struct timeval	*from
        )
{
    uint32_t	sec = from->tv_sec;
    uint32_t	usec = from->tv_usec;

    *to = 0;

    /*
     * Ensure usec's is only fractional.
     */
    while(usec >= MILLION){
        sec++;
        usec -= MILLION;
    }

    /*
     * Place seconds in MS 32 bits.
     */
    *to = (uint64_t)MASK32(sec) << 32;
    /*
     * Normalize usecs to 32bit fraction, then set that to LS 32 bits.
     */
    *to |= MASK32(((uint64_t)usec << 32)/MILLION);

    return;
}

/*
 * Function:	BWLNum64toDouble
 *
 * Description:	
 * 	Convert an BWLNum64 time value to a double representation. 
 * 	The double will contain the number of seconds with the fractional
 * 	portion of the BWLNum64 mapping to the portion of the double
 * 	represented after the radix point. This will obviously loose
 * 	some precision after the radix point, however - larger values
 * 	will be representable in double than an BWLNum64.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
double
BWLNum64ToDouble(
        BWLNum64	from
        )
{
    return (double)from / EXP2POW32;
}

/*
 * Function:	BWLDoubleToNum64
 *
 * Description:	
 * 	Convert a double value to an BWLNum64 representation.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
BWLNum64
BWLDoubleToNum64(
        double	from
        )
{
    if(from < 0){
        return 0;
    }
    return (BWLNum64)(from * EXP2POW32);
}

/*
 * Function:	BWLUsecToNum64
 *
 * Description:	
 * 	Convert an unsigned 32bit number representing some number of
 * 	microseconds to an BWLNum64 representation.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	
 * Returns:	
 * Side Effect:	
 */
    BWLNum64
BWLUsecToNum64(uint32_t usec)
{
    return ((uint64_t)usec << 32)/MILLION;
}
