// source code taken from: http://www.taygeta.com/random.xml
//                     and http://www.taygeta.com/random/gaussian.html

/* r250.c	the r250 uniform random number algorithm

		Kirkpatrick, S., and E. Stoll, 1981; "A Very Fast
		Shift-Register Sequence Random Number Generator",
		Journal of Computational Physics, V.40

		also:

		see W.L. Maier, DDJ May 1991
*/

#include <math.h>
#include <limits.h>
#include <stdint.h>

#include "r250.h"

/* set the following if you trust rand(), otherwise the minimal standard
   generator is used
*/
/* #define TRUST_RAND */

/* defines to allow for 16 or 32 bit integers */
#define BITS 31


#if WORD_BIT == 32
#ifndef BITS
#define BITS	32
#endif
#else
#ifndef BITS
#define BITS    16
#endif
#endif

#if BITS == 31
#define MSB          0x40000000L
#define ALL_BITS     0x7fffffffL
#define HALF_RANGE   0x20000000L
#define STEP         7
#endif

#if BITS == 32
#define MSB          0x80000000L
#define ALL_BITS     0xffffffffL
#define HALF_RANGE   0x40000000L
#define STEP         7
#endif

#if BITS == 16
#define MSB         0x8000
#define ALL_BITS    0xffff
#define HALF_RANGE  0x4000
#define STEP        11
#endif

static unsigned int r250_buffer[ 250 ];
static int r250_index;

#ifdef NO_PROTO
void r250_init(sd)
int seed;
#else
void r250_init(int sd)
#endif
{
	int j, k;
	unsigned int mask, msb;

#ifdef TRUST_RAND

#if BITS == 32 || BITS == 31
	srand48( sd );
#else
	srand( sd );
#endif


#else
	set_seed( sd );
#endif

	r250_index = 0;
	for (j = 0; j < 250; j++)      /* fill r250 buffer with BITS-1 bit values */
#ifdef TRUST_RAND
#if BITS == 32 || BITS == 31
		r250_buffer[j] = (unsigned int)lrand48();
#else
		r250_buffer[j] = rand();
#endif
#else
		r250_buffer[j] = randlcg();
#endif


	for (j = 0; j < 250; j++)	/* set some MSBs to 1 */
#ifdef TRUST_RAND
		if ( rand() > HALF_RANGE )
			r250_buffer[j] |= MSB;
#else
		if ( randlcg() > HALF_RANGE )
			r250_buffer[j] |= MSB;
#endif


	msb = MSB;	        /* turn on diagonal bit */
	mask = ALL_BITS;	/* turn off the leftmost bits */

	for (j = 0; j < BITS; j++)
	{
		k = STEP * j + 3;	/* select a word to operate on */
		r250_buffer[k] &= mask; /* turn off bits left of the diagonal */
		r250_buffer[k] |= msb;	/* turn on the diagonal bit */
		mask >>= 1;
		msb  >>= 1;
	}

}

unsigned int r250()		/* returns a random unsigned integer */
{
    register int	j;
    register unsigned int new_rand;

	if ( r250_index >= 147 )
		j = r250_index - 147;	/* wrap pointer around */
	else
		j = r250_index + 103;

	new_rand = r250_buffer[ r250_index ] ^ r250_buffer[ j ];
	r250_buffer[ r250_index ] = new_rand;

	if ( r250_index >= 249 )	/* increment pointer for next time */
		r250_index = 0;
	else
		r250_index++;

	return new_rand;

}


double dr250()		/* returns a random double in range 0..1 */
{
    register int	j;
    register unsigned int new_rand;

	if ( r250_index >= 147 )
		j = r250_index - 147;	/* wrap pointer around */
	else
		j = r250_index + 103;

	new_rand = r250_buffer[ r250_index ] ^ r250_buffer[ j ];
	r250_buffer[ r250_index ] = new_rand;

	if ( r250_index >= 249 )	/* increment pointer for next time */
		r250_index = 0;
	else
		r250_index++;
#if _WIN32||_WIN64
    return (double)new_rand / ALL_BITS;
#else
    uint32_t tmpu32=new_rand;
    double tmpd=tmpu32;
    double result=tmpd/ (2147483647.0*2);//32 not 31 bits
    return result;
#endif

}

/* rndlcg            Linear Congruential Method, the "minimal standard generator"
                     Park & Miller, 1988, Comm of the ACM, 31(10), pp. 1192-1201

*/

static long int quotient  = LONG_MAX / 16807L;
static long int _remainder = LONG_MAX % 16807L;

static long int seed_val = 1L;

long set_seed(long int sd)
{
        return seed_val = sd;
}

long get_seed()
{
        return seed_val;
}


unsigned long int randlcg()       /* returns a random unsigned integer */
{
        if ( seed_val <= quotient )
                seed_val = (seed_val * 16807L) % LONG_MAX;
        else
        {
                long int high_part = seed_val / quotient;
                long int low_part  = seed_val % quotient;

                long int test = 16807L * low_part - _remainder * high_part;

                if ( test > 0 )
                        seed_val = test;
                else
                        seed_val = test + LONG_MAX;

        }

        return seed_val;
}

/* boxmuller.c           Implements the Polar form of the Box-Muller
                         Transformation

                      (c) Copyright 1994, Everett F. Carter Jr.
                          Permission is granted by the author to use
			  this software for any application provided this
			  copyright notice is preserved.

*/


double dr250_box_muller(double m, double s)	/* normal random variate generator */
{				        /* mean m, standard deviation s */
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * dr250() - 1.0;
			x2 = 2.0 * dr250() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( m + y1 * s );
}

