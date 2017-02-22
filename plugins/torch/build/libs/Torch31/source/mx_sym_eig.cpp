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

/*
	File containing routines for symmetric eigenvalue problems
*/

#include "mx_sym_eig.h"

namespace Torch {


#define	SQRT2	1.4142135623730949
#define	sgn(x)	( (x) >= 0 ? 1 : -1 )

/* trieig -- finds eigenvalues of symmetric tridiagonal matrices
	-- matrix represented by a pair of vectors a (diag entries)
		and b (sub- & super-diag entries)
	-- eigenvalues in a on return */
void mxTriEig(Vec * a, Vec * b, Mat * mat_q)
{
  int i_min, i_max;
  real b_sqr, bk, ak1, bk1, ak2, bk2, z;
  real c, c2, cs, s, s2, d, mu;

  int n = a->n;
  real *a_ptr = a->ptr;
  real *b_ptr = b->ptr;

  i_min = 0;
  while (i_min < n)		/* outer while loop */
  {
    /* find i_max to suit;
       submatrix i_min..i_max should be irreducible */
    i_max = n - 1;
    for (int i = i_min; i < n - 1; i++)
    {
      if (b_ptr[i] == 0.0)
      {
	i_max = i;
	break;
      }
    }

    if (i_max <= i_min)
    {
      i_min = i_max + 1;
      continue;			/* outer while loop */
    }

    /* repeatedly perform QR method until matrix splits */
    bool split = false;
    while (!split)		/* inner while loop */
    {
      /* find Wilkinson shift */
      d = (a_ptr[i_max - 1] - a_ptr[i_max]) / 2;
      b_sqr = b_ptr[i_max - 1] * b_ptr[i_max - 1];
      mu = a_ptr[i_max] - b_sqr / (d + sgn(d) * sqrt(d * d + b_sqr));

      /* initial Givens' rotation */
      mx_givens(a_ptr[i_min] - mu, b_ptr[i_min], &c, &s);
      s = -s;
      if (fabs(c) < SQRT2)
      {
	c2 = c * c;
	s2 = 1 - c2;
      }
      else
      {
	s2 = s * s;
	c2 = 1 - s2;
      }
      cs = c * s;
      ak1 =
	  c2 * a_ptr[i_min] + s2 * a_ptr[i_min + 1] -
	  2 * cs * b_ptr[i_min];
      bk1 =
	  cs * (a_ptr[i_min] - a_ptr[i_min + 1]) + (c2 -
						    s2) * b_ptr[i_min];
      ak2 =
	  s2 * a_ptr[i_min] + c2 * a_ptr[i_min + 1] +
	  2 * cs * b_ptr[i_min];
      bk2 = (i_min < i_max - 1) ? c * b_ptr[i_min + 1] : 0.0;
      z = (i_min < i_max - 1) ? -s * b_ptr[i_min + 1] : 0.0;
      a_ptr[i_min] = ak1;
      a_ptr[i_min + 1] = ak2;
      b_ptr[i_min] = bk1;
      if (i_min < i_max - 1)
	b_ptr[i_min + 1] = bk2;
      if (mat_q)
	mx_rot_cols(mat_q, i_min, i_min + 1, c, -s, mat_q);

      for (int i = i_min + 1; i < i_max; i++)
      {
	/* get Givens' rotation for sub-block -- k == i-1 */
	mx_givens(b_ptr[i - 1], z, &c, &s);
	s = -s;

	/* perform Givens' rotation on sub-block */
	if (fabs(c) < SQRT2)
	{
	  c2 = c * c;
	  s2 = 1 - c2;
	}
	else
	{
	  s2 = s * s;
	  c2 = 1 - s2;
	}
	cs = c * s;
	bk = c * b_ptr[i - 1] - s * z;
	ak1 = c2 * a_ptr[i] + s2 * a_ptr[i + 1] - 2 * cs * b_ptr[i];
	bk1 = cs * (a_ptr[i] - a_ptr[i + 1]) + (c2 - s2) * b_ptr[i];
	ak2 = s2 * a_ptr[i] + c2 * a_ptr[i + 1] + 2 * cs * b_ptr[i];
	bk2 = (i + 1 < i_max) ? c * b_ptr[i + 1] : 0.0;
	z = (i + 1 < i_max) ? -s * b_ptr[i + 1] : 0.0;
	a_ptr[i] = ak1;
	a_ptr[i + 1] = ak2;
	b_ptr[i] = bk1;
	if (i < i_max - 1)
	  b_ptr[i + 1] = bk2;
	if (i > i_min)
	  b_ptr[i - 1] = bk;
	if (mat_q)
	  mx_rot_cols(mat_q, i, i + 1, c, -s, mat_q);
      }

      /* test to see if matrix should be split */
      for (int i = i_min; i < i_max; i++)
      {
	if (fabs(b_ptr[i]) <
	    REAL_EPSILON * (fabs(a_ptr[i]) + fabs(a_ptr[i + 1])))
	{
	  b_ptr[i] = 0.0;
	  split = true;
	}
      }
    }
  }
}

/* symmeig -- computes eigenvalues of a dense symmetric matrix
	-- mat_a **must** be symmetric on entry
	-- eigenvalues stored in out
	-- mat_q contains orthogonal matrix of eigenvectors
	-- returns vector of eigenvalues
  -- je pense: if mat_q is NULL, eigenvectors won't be computed
*/
void mxSymEig(Mat * mat_a, Mat * mat_q, Vec * out)
{
  Mat *tmp = new Mat(mat_a->m, mat_a->n);
  tmp->copy(mat_a);

  Vec *b = new Vec(mat_a->m - 1);
  Vec *diag = new Vec(mat_a->m);
  Vec *beta = new Vec(mat_a->m);

  mxHFactor(tmp, diag, beta);
  if (mat_q)
    mxMakeHQ(tmp, diag, beta, mat_q);

  int i;
  for (i = 0; i < mat_a->m - 1; i++)
  {
    out->ptr[i] = tmp->ptr[i][i];
    b->ptr[i] = tmp->ptr[i][i + 1];
  }
  out->ptr[i] = tmp->ptr[i][i];

  mxTriEig(out, b, mat_q);

  delete beta;
  delete diag;
  delete b;
  delete tmp;
}

}

