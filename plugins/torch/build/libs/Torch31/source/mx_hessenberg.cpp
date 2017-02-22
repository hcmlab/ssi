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

#include "mx_hessenberg.h"
#include "mx_householder.h"

namespace Torch {

/*
		File containing routines for determining Hessenberg
	factorisations.
*/

/* Hfactor -- compute Hessenberg factorisation in compact form.
	-- factorisation performed in situ
	-- for details of the compact form see QRfactor.c and matrix2.doc */
void mxHFactor(Mat * mat, Vec * diag, Vec * beta)
{
  int limit = mat->m - 1;

  Vec *tmp = new Vec(mat->m);

  for (int k = 0; k < limit; k++)
  {
    mat->getCol(k, tmp);
    mxHhVec(tmp, k + 1, &beta->ptr[k], tmp, &mat->ptr[k + 1][k]);
    diag->ptr[k] = tmp->ptr[k + 1];
    mxHhTrCols(mat, k + 1, k + 1, tmp, beta->ptr[k]);
    mxHhTrRows(mat, 0, k + 1, tmp, beta->ptr[k]);
  }

  delete tmp;
}

/* makeHQ -- construct the Hessenberg orthogonalising matrix Q;
	-- i.e. Hess M = Q.M.Q'	*/
void mxMakeHQ(Mat * h_mat, Vec * diag, Vec * beta, Mat * q_out)
{
  int limit = h_mat->m - 1;
//    Qout = m_resize(Qout,H->m,H->m);

  Vec *tmp1 = new Vec(h_mat->m);
  Vec *tmp2 = new Vec(h_mat->m);

  for (int i = 0; i < h_mat->m; i++)
  {
    tmp1->zero();
    tmp1->ptr[i] = 1.0;

    /* apply H/h transforms in reverse order */
    for (int j = limit - 1; j >= 0; j--)
    {
      h_mat->getCol(j, tmp2);
      tmp2->ptr[j + 1] = diag->ptr[j];
      mxHhTrVec(tmp2, beta->ptr[j], j + 1, tmp1, tmp1);
    }

    /* insert into Qout */
    q_out->setCol(i, tmp1);
  }
  delete tmp1;
  delete tmp2;
}

/* makeH -- construct actual Hessenberg matrix */
void mxMakeH(Mat * h_mat, Mat * h_out)
{
//    Hout = m_resize(Hout,H->m,H->m);
  h_out->copy(h_mat);

  int limit = h_mat->m;
  for (int i = 1; i < limit; i++)
  {
    for (int j = 0; j < i - 1; j++)
      h_out->ptr[i][j] = 0.0;
  }
}

}

