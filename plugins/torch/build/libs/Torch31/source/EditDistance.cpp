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

#include "EditDistance.h"
#include "XFile.h"

namespace Torch {

EditDistance::EditDistance(bool is_confusion_)
{
  obtained = NULL;
  desired = NULL;
  obt_size = 0;
  des_size = 0;
  insert_cost = 1;
  delete_cost = 1;
  subst_cost = 1;
  is_confusion = is_confusion_;
  confusion = NULL;
  conf_ins = NULL;
  conf_del = NULL;
  n_confusion = 0;
  reset();
}

void EditDistance::setCosts(int i_cost, int d_cost, int s_cost)
{
  insert_cost = i_cost;
  delete_cost = d_cost;
  subst_cost = s_cost;
}

void EditDistance::reset()
{
  accuracy = 0;
  n_insert=0;
  n_delete=0;
  n_subst=0;
  n_seq=0;
  if (confusion) {
    for (int i=0;i<n_confusion;i++) {
      for (int j=0;j<n_confusion;j++)
        confusion[i][j] = 0;
      conf_ins[i] = 0;
      conf_del[i] = 0;
    }
  }
}

void EditDistance::distance(int* obtained_, int obt_size_, int* desired_, int des_size_)
{
  obtained = obtained_;
  obt_size = obt_size_;
  desired = desired_;
  des_size = des_size_;

  if (is_confusion) {
    // count the max number of symbols in desired/obtained
    int the_max = -1;
    for (int i=0;i<obt_size;i++)
      if (the_max < obtained[i])
        the_max = obtained[i];
    for (int i=0;i<des_size;i++)
      if (the_max < desired[i])
        the_max = desired[i];
    the_max++;
    // prepare the confusion matrix itself
    if (n_confusion<the_max) {
      confusion = (int**)allocator->realloc(confusion,the_max*sizeof(int*));
      conf_ins = (int*)allocator->realloc(conf_ins,the_max*sizeof(int));
      conf_del = (int*)allocator->realloc(conf_del,the_max*sizeof(int));
      for (int i=0;i<n_confusion;i++) {
        confusion[i] = (int*)allocator->realloc(confusion[i],the_max*sizeof(int));
      }
      for (int i=n_confusion;i<the_max;i++) {
        confusion[i] = (int*)allocator->alloc(the_max*sizeof(int));
      }
      for (int i=0;i<the_max;i++) {
        for (int j=0;j<the_max;j++) 
          confusion[i][j] = 0;
        conf_ins[i] = 0;
        conf_del[i] = 0;
      }
      n_confusion = the_max;
    }
  }

  n_insert = 0;
  n_delete = 0;
  n_subst = 0;
  int subst;
  Allocator allocator_;
  int **d = (int**)allocator_.alloc((des_size+1)*sizeof(int*));
  int **d_ins = (int**)allocator_.alloc((des_size+1)*sizeof(int*));
  int **d_del = (int**)allocator_.alloc((des_size+1)*sizeof(int*));
  int **d_sub = (int**)allocator_.alloc((des_size+1)*sizeof(int*));
  for (int i=0;i<des_size+1;i++) {
    d[i] = (int*)allocator_.alloc((obt_size+1)*sizeof(int));
    d_ins[i] = (int*)allocator_.alloc((obt_size+1)*sizeof(int));
    d_del[i] = (int*)allocator_.alloc((obt_size+1)*sizeof(int));
    d_sub[i] = (int*)allocator_.alloc((obt_size+1)*sizeof(int));
    for (int j=0;j<obt_size+1;j++) {
      d[i][j] = 0;
      d_ins[i][j] = 0;
      d_del[i][j] = 0;
      d_sub[i][j] = 0;
    }
  }
  int** previous = NULL;
  if (is_confusion) {
    // in this table, 1 = subst, 2 = del, 3 = ins
    previous = (int**)allocator_.alloc((des_size+1)*sizeof(int*));
    for (int i=0;i<des_size+1;i++) {
      previous[i] = (int*)allocator_.alloc((obt_size+1)*sizeof(int));
      for (int j=0;j<obt_size+1;j++)
        previous[i][j] = 0;
    }
  }
  for (int i=0;i<des_size;i++) {
    d[i+1][0] = d[i][0] + delete_cost;
    d_del[i+1][0] = d[i+1][0];
    if (is_confusion)
      previous[i+1][0] = 2;
  }
  for (int i=0;i<obt_size;i++) {
    d[0][i+1] = d[0][i] + insert_cost;
    d_ins[0][i+1] = d[0][i+1];
    if (is_confusion)
      previous[0][i+1] = 3;
  }
  for (int i=0;i<des_size;i++) {
    for (int j=0;j<obt_size;j++) {
      if (desired[i] == obtained[j]) {
        subst = 0;
      } else {
        subst = subst_cost;
      }
      int s_cost = d[i][j]+subst;
      int d_cost = d[i][j+1]+delete_cost;
      int i_cost = d[i+1][j]+insert_cost;
      if (s_cost <= d_cost && s_cost <= i_cost) {
        d[i+1][j+1] = s_cost;
        d_sub[i+1][j+1] = d_sub[i][j]+subst;
        d_del[i+1][j+1] = d_del[i][j];
        d_ins[i+1][j+1] = d_ins[i][j];
        if (is_confusion)
          previous[i+1][j+1] = 1;
      } else if (d_cost <= i_cost && d_cost <= s_cost) {
        d[i+1][j+1] = d_cost;
        d_del[i+1][j+1] = d_del[i][j+1]+delete_cost;
        d_sub[i+1][j+1] = d_sub[i][j+1];
        d_ins[i+1][j+1] = d_ins[i][j+1];
        if (is_confusion)
          previous[i+1][j+1] = 2;
      } else {
        d[i+1][j+1] = i_cost;
        d_ins[i+1][j+1] = d_ins[i+1][j]+insert_cost;
        d_del[i+1][j+1] = d_del[i+1][j];
        d_sub[i+1][j+1] = d_sub[i+1][j];
        if (is_confusion)
          previous[i+1][j+1] = 3;
      }
    }
  }
  n_subst = d_sub[des_size][obt_size] / subst_cost;
  n_delete = d_del[des_size][obt_size] / delete_cost;
  n_insert = d_ins[des_size][obt_size] / insert_cost;
  n_seq = des_size;
  //dist = d[des_size][obt_size];
  accuracy = (n_seq - n_delete - n_subst - n_insert) * 100. / (real)n_seq;

  if (is_confusion) {
    int i=des_size;
    int j=obt_size;
    while (!(i == 0 && j == 0)) {
//printf("i = %d j = %d\n",i,j);
      // what kind of mistake did we do?
      if (previous[i][j] == 1) {
        // substitution
        int s_des = desired[i-1];
        int s_obt = obtained[j-1];
        confusion[s_obt][s_des] ++;
//printf("subst %d %d at (%d,%d)\n",s_des,s_obt,i-1,j-1);
        i--;
        j--;
      } else if (previous[i][j] == 2) {
        // deletion
        int s_des = desired[i-1];
        conf_del[s_des] ++;
//printf("del %d at (%d,%d)\n",s_des,i-1,j-1);
        i--;
      } else if (previous[i][j] == 3) {
        // insertion
        int s_obt = obtained[j-1];
        conf_ins[s_obt] ++;
//printf("ins %d at (%d,%d)\n",s_obt,i-1,j-1);
        j--;
      }
    }
  }
}

void EditDistance::add(EditDistance* d)
{
  n_insert += d->n_insert;
  n_delete += d->n_delete;
  n_subst += d->n_subst;
  n_seq += d->n_seq;
  accuracy = (n_seq - n_delete - n_subst - n_insert) * 100. / (real)n_seq;
  if (is_confusion) {
    // should enlarge the matrix if too small
    if (n_confusion < d->n_confusion) {
      confusion = (int**)allocator->realloc(confusion,d->n_confusion*sizeof(int*));
      conf_ins = (int*)allocator->realloc(conf_ins,d->n_confusion*sizeof(int));
      conf_del = (int*)allocator->realloc(conf_del,d->n_confusion*sizeof(int));
      for (int i=0;i<n_confusion;i++) {
        confusion[i] = (int*)allocator->realloc(confusion[i],d->n_confusion*sizeof(int));
        for (int j=n_confusion;j<d->n_confusion;j++)
          confusion[i][j] = 0;
      }
      for (int i=n_confusion;i<d->n_confusion;i++) {
        confusion[i] = (int*)allocator->alloc(d->n_confusion*sizeof(int));
        for (int j=0;j<d->n_confusion;j++)
          confusion[i][j] = 0;
        conf_ins[i] = 0;
        conf_del[i] = 0;
      }
      n_confusion = d->n_confusion;
    }
    // then add both results
    for (int i=0;i<d->n_confusion;i++) {
      for (int j=0;j<d->n_confusion;j++) {
        confusion[i][j] += d->confusion[i][j];
      }
      conf_ins[i] += d->conf_ins[i];
      conf_del[i] += d->conf_del[i];
    }
  }
}

void EditDistance::print(XFile *f)
{
  if (is_confusion) {
    for (int i=0;i<n_confusion;i++) {
      int tot=0;
      for (int j=0;j<n_confusion;j++) {
        f->printf("%d  ",confusion[i][j]);
        tot += confusion[i][j];
      }
      f->printf(" total %d\n",tot);
    }
    f->printf("total\n");
    for (int j=0;j<n_confusion;j++) {
      int tot=0;
      for (int i=0;i<n_confusion;i++) {
        tot += confusion[i][j];
      }
      f->printf("%d  ",tot);
    }
    f->printf("\ndel\n");
    for (int i=0;i<n_confusion;i++)
      f->printf("%d  ",conf_del[i]);
    f->printf("\nins\n");
    for (int i=0;i<n_confusion;i++)
      f->printf("%d  ",conf_ins[i]);
    f->printf("\n");
  }

  f->printf("total %d insert %d delete %d subst %d N %d\n",
    n_insert+n_delete+n_subst,n_insert,n_delete,n_subst,n_seq);

  f->flush();
}

void EditDistance::printRatio(XFile *f)
{
  f->printf("accuracy %5.2f insert %5.2f delete %5.2f subst %5.2f N %d\n",  
    accuracy,n_insert*100./n_seq,n_delete*100./n_seq,n_subst*100./n_seq,n_seq);
  f->flush();
}

EditDistance::~EditDistance() 
{
}

}

