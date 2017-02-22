// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef RANDOM_INC
#define RANDOM_INC

#include "general.h"

namespace Torch {

/** Random class which contains several static random methods.
    
    These methods are based on a uniform random generator,
    named "Mersenne Twister", available at:
    http://www.math.keio.ac.jp/matumoto/emt.html.
    Copyright Makoto Matsumoto and Takuji Nishimura.
    (Have a look inside the implementation file for details).

    The random generator can be initialized with the manualSeed() method.
    Otherwise, it will be automatically initialized with a seed based
    on the current computer clock.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Random
{
  public:
    // The seed used to initialize the random generator.
    static unsigned long the_initial_seed;

    // Internal variables for the Mersenne Twister generator
    static const int n;
    static const int m;
    static unsigned long state[]; /* the array for the state vector  */
    static int left;
    static int initf;
    static unsigned long *next;

    // Internal variables for the normal distribution generator
    static real normal_x;
    static real normal_y;
    static real normal_rho;
    static bool normal_is_valid;

    // Internal method for the Mersenne Twister generator
    static void nextState();

    /// Initializes the random number generator with the computer clock.
    static void seed();

    /// Initializes the random number generator with the given long "the_seed_".
    static void manualSeed(unsigned long the_seed_);

    /// Returns the starting seed used.
    static unsigned long getInitialSeed();

    /// Generates a uniform 32 bits integer.
    static unsigned long random();

    /// Generates a uniform random number on [0,1[.
    static real uniform();

    /// Returns in #indices# #n_indices# shuffled. (between 0 and #n_indices-1#).
    static void getShuffledIndices(int *indices, int n_indices);

    /// Shuffles tabular, which contains #n_elems# of size #size_elem#.
    static void shuffle(void *tabular, int size_elem, int n_elems);

    /// Generates a uniform random number on [a,b[ (b>a).
    static real boundedUniform(real a, real b);

    /** Generates a random number from a normal distribution.
        (With mean #mean# and standard deviation #stdv >= 0#).
    */
    static real normal(real mean=0, real stdv=1);

    /** Generates a random number from an exponential distribution.
        The density is $p(x) = lambda * exp(-lambda * x)$, where
        lambda is a positive number.
    */
    static real exponential(real lambda);

    /** Returns a random number from a Cauchy distribution.
        The Cauchy density is $p(x) = sigma/(pi*(sigma^2 + (x-median)^2))$
    */
    static real cauchy(real median=0, real sigma=1);

    /** Generates a random number from a log-normal distribution.
        (#mean > 0# is the mean of the log-normal distribution
        and #stdv# is its standard deviation).
    */
    static real logNormal(real mean, real stdv);

    /** Generates a random number from a geometric distribution.
        It returns an integer #i#, where $p(i) = (1-p) * p^(i-1)$.
        p must satisfy $0 < p < 1$.
    */
    static int geometric(real p);

    /// Returns true with probability $p$ and false with probability $1-p$ (p > 0).
    static bool bernouilli(real p=0.5);
};

}

#endif
