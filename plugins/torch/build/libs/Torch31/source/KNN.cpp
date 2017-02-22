// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#include "KNN.h"

namespace Torch {

KNN::KNN(int n_outputs_,int K_)
{
  // works only for sequences of 1 frame (static data!)
  data = NULL;
  distances = NULL;
  indices = NULL;
  setK(K_);
  n_outputs = n_outputs_;
  outputs = new(allocator) Sequence(1,n_outputs);
  n_real_examples = 0;
  real_examples = NULL;
}

void KNN::setDataSet(DataSet *dataset_)
{
  data = dataset_;
  n_real_examples = data->n_examples;
  real_examples = (int*)allocator->realloc(real_examples,n_real_examples*sizeof(int));
  for (int i=0;i<data->n_examples;i++) {
    real_examples[i] = data->selected_examples[i];
  }
}

void KNN::setK(int K_)
{
  K = K_;
  distances = (real *)allocator->realloc(distances,K*sizeof(real));
  indices = (int *)allocator->realloc(indices,K*sizeof(int));
}

real KNN::distance(real* v1, real* v2, int n)
{
  real dist = 0.;
  for(int j=0;j<n;j++) {
    real diff = *v1++ - *v2++;
     dist += diff*diff;
  }
  return dist;
}

void KNN::forward(Sequence* inputs)
{
  // keep current example in order to restore at the end
  data->pushExample();

  // verify that n_examples > K
  int old_K = K;
  if (n_real_examples < K)
    K = n_real_examples;

  // initialization of distances to big values;
  for (int i=0;i<K;i++) {
    distances[i] = INF;
    indices[i] = -1;
  }

  // compute the K nearest neighbords

  // for each vector in data
  int* i_ptr = real_examples;
  for (int i=0;i<n_real_examples;i++) {
    data->setRealExample(*i_ptr++);
    // calculate euclidean distance between example and current vector
    real dist = distance(inputs->frames[0],data->inputs->frames[0],data->n_inputs);

/*
    real dist = 0;
    Sequence* cur = data->inputs;
    real *x = inputs->frames[0];
    real *datax = cur->frames[0];
    for(int j=0;j<data->n_inputs;j++) {
      real diff = *x++ - *datax++;
       dist += diff*diff;
    }
*/
    // eventually add current vector to K nearest neighbors
    if (dist < distances[K-1]) {
      // find insertion point
      real* bptr = distances;
      real* eptr = distances + K - 1;
      real* mptr = bptr + (eptr - bptr) / 2;
      do {
        if (dist < *mptr)
          eptr = mptr;
        else
          bptr = mptr + 1;
        mptr = bptr + (eptr - bptr) / 2;
      } while (mptr < eptr);
      // insert the point by shifting all subsequent distances
      eptr = distances + K - 1;
      bptr = eptr - 1;

      int* eptr_idx = indices + K - 1;
      int* bptr_idx = eptr_idx - 1;

      while (eptr > mptr) {
        *eptr-- = *bptr--;      /*   distances   */
        *eptr_idx-- = *bptr_idx--;      /*   indices   */
      }
      *mptr = dist;
      indices[mptr - distances] = data->real_current_example_index;
    }
  }

  // give an answer as the mean of the answers of the KNNs
  // initialize outputs to null
  real* out = outputs->frames[0];
  for (int j=0;j<n_outputs;j++)
    *out++ = 0;
  for (int i=0;i<K;i++) {
    out = outputs->frames[0];
    data->setRealExample(indices[i]);
    real *targ = data->targets->frames[0];
    for (int j=0;j<n_outputs;j++)
      *out++ += *targ++;
  }
  out = outputs->frames[0];
  for (int j=0;j<n_outputs;j++)
    *out++ /= (real)K;

  // in case K was modified
  K = old_K;
  // restore current_example
  data->popExample();
}

KNN::~KNN()
{
}

}

