// Copyright (C) 2003--2004 Zbigniew Leyk (zbigniew.leyk@anu.edu.au)
//                and David E. Stewart (david.stewart@anu.edu.au)
//                and Ronan Collobert (collober@idiap.ch)
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

#include "mx_low_level.h"

namespace Torch {

/* __ip__ -- inner product */
real mxIp__(real * dp1, real * dp2, int len)
{
#ifdef VUNROLL
  int len4;
  real sum1, sum2, sum3;
#endif
  real sum;

  sum = 0.0;
#ifdef VUNROLL
  sum1 = sum2 = sum3 = 0.0;

  len4 = (len >> 2);
  len = len % 4;

  for (int i = 0; i < len4; i++)
  {
    int z = i << 2;
    sum  += dp1[z] * dp2[z];
    sum1 += dp1[z + 1] * dp2[z + 1];
    sum2 += dp1[z + 2] * dp2[z + 2];
    sum3 += dp1[z + 3] * dp2[z + 3];
  }
  sum += sum1 + sum2 + sum3;
  dp1 += (len4 << 2);
  dp2 += (len4 << 2);
#endif

  for (int i = 0; i < len; i++)
    sum += dp1[i] * dp2[i];

  return sum;
}

/* __mltadd__ -- scalar multiply and add c.f. v_mltadd() */
void mxRealMulAdd__(real * dp1, real * dp2, real s, int len)
{
#ifdef VUNROLL
  int len4;

  len4 = len / 4;
  len = len % 4;
  for (int i = 0; i < len4; i++)
  {
    dp1[4 * i] += s * dp2[4 * i];
    dp1[4 * i + 1] += s * dp2[4 * i + 1];
    dp1[4 * i + 2] += s * dp2[4 * i + 2];
    dp1[4 * i + 3] += s * dp2[4 * i + 3];
  }
  dp1 += 4 * len4;
  dp2 += 4 * len4;
#endif

  for (int i = 0; i < len; i++)
    dp1[i] += s * dp2[i];
}

/* __smlt__ scalar multiply array c.f. sv_mlt() */
void mxRealMul__(real * dp, real s, real * out, int len)
{
  for (int i = 0; i < len; i++)
    out[i] = s * dp[i];
}

/* __add__ -- add arrays c.f. v_add() */
void mxAdd__(real * dp1, real * dp2, real * out, int len)
{
  for (int i = 0; i < len; i++)
    out[i] = dp1[i] + dp2[i];
}

/* __sub__ -- subtract arrays c.f. v_sub() */
void mxSub__(real * dp1, real * dp2, real * out, int len)
{
  for (int i = 0; i < len; i++)
    out[i] = dp1[i] - dp2[i];
}

/* __zero__ -- zeros an array of floating point numbers */
void mxZero__(real * dp, int len)
{
  /* else, need to zero the array entry by entry */
  for (int i = 0; i < len; i++)
    dp[i] = 0.0;
}

}

