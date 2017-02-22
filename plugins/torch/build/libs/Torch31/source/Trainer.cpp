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

#include "Trainer.h"
#include "Random.h"

namespace Torch {

IMPLEMENT_NEW_LIST(MeasurerList, Measurer)

Trainer::Trainer(Machine *machine_)
{
  machine = machine_;
}

void Trainer::test(MeasurerList *measurers)
{
  DataSet **datas;
  Measurer ***mes;
  int *n_mes;
  int n_datas;

  print("# Trainer: testing [");

  Allocator *allocator_ = extractMeasurers(measurers, NULL, &datas, &mes, &n_mes, &n_datas);

  ////
  int n_ex = 0;
  for(int andrea = 0; andrea < n_datas; andrea++)
    n_ex += datas[andrea]->n_examples;
  real n_ex_mod = ( (n_ex == 0) ? 0. : 10.1/((real)n_ex));
  real ex_curr = 0;
  real n_dots = 0;
  ////

  for(int andrea = 0; andrea < n_datas; andrea++)
  {
    DataSet *dataset = datas[andrea];

    for(int i = 0; i < n_mes[andrea]; i++)
      mes[andrea][i]->reset();

    for(int t = 0; t < dataset->n_examples; t++)
    {
      dataset->setExample(t);
      machine->forward(dataset->inputs);
    
      for(int i = 0; i < n_mes[andrea]; i++)
        mes[andrea][i]->measureExample();

      if(++ex_curr * n_ex_mod >= (n_dots+1))
      {
        if(n_ex < 10)
          print("_");
        else
          print(".");
        n_dots++;
      }
    }
  
    for(int i = 0; i < n_mes[andrea]; i++)
      mes[andrea][i]->measureIteration();

    for(int i = 0; i < n_mes[andrea]; i++)
      mes[andrea][i]->measureEnd();

  }
  
  print("]\n");
  delete allocator_;      
}

// ExtractMeasurers, ou la magie du quatre etoiles...
Allocator *Trainer::extractMeasurers(MeasurerList *measurers, DataSet *train, DataSet ***datas, Measurer ****meas, int **n_meas, int *n_datas)
{
  DataSet **datas_ = NULL;
  Measurer ***meas_ = NULL;
  int *n_meas_ = NULL;
  int n_datas_ = 0;

  Allocator *allocator_ = new Allocator;

  // 0) Coup bas ? Eh... l'aut...
  if(!measurers)
  {
    if(train)
    {
      datas_ = (DataSet **)allocator_->alloc(sizeof(DataSet *));
      datas_[n_datas_++] = train;
      n_meas_ = (int *)allocator_->alloc(sizeof(int));
      n_meas_[0] = 0;
    }

    *datas = datas_;
    *meas = meas_;
    *n_meas = n_meas_;
    *n_datas = n_datas_;

    return allocator_;
  }

  // 1) Find all differents datas [-- bourrin -- et en + : +1 au cas ou tous != et tous != de train...]...
  datas_ = (DataSet **)allocator_->alloc(sizeof(DataSet *)*(measurers->n_nodes+1));
  n_datas_ = 0;
  if(train)
    datas_[n_datas_++] = train;

  for(int i = 0; i < measurers->n_nodes; i++)
  {
    DataSet *potential_new_dataset = measurers->nodes[i]->data;
    bool already_registered = false;
    for(int j = 0; j < n_datas_; j++)
    {
      if(potential_new_dataset == datas_[j])
      {
        already_registered = true;
        break;
      }
    }

    if(!already_registered)
      datas_[n_datas_++] = potential_new_dataset;
  }

  // 2) Allocations a la con avec l'allocator qu'on va refiler a l'aut'naze
  if(n_datas_ > 0)
  {
    n_meas_ = (int *)allocator_->alloc(sizeof(int)*n_datas_);
    meas_ = (Measurer ***)allocator_->alloc(sizeof(Measurer **)*n_datas_);
  }

  // 3) For each dataset...
  for(int i = 0; i < n_datas_; i++)
  {
    DataSet *counted_data = datas_[i];

    // 2a) Count associated measurers...
    n_meas_[i] = 0;
    for(int j = 0; j < measurers->n_nodes; j++)
    {
      if(measurers->nodes[j]->data == counted_data)
        n_meas_[i]++;
    }

    // 2b) Y'a qqch ??? [pour le train...]
    if(!n_meas_[i])
      continue;

    // 2c) Allocate memory
    meas_[i] = (Measurer **)allocator_->alloc(sizeof(Measurer *)*n_meas_[i]);
    
    // 2d) Rempli le bordel a donf [processeurs deterministes seulement]
    int index = 0;
    for(int j = 0; j < measurers->n_nodes; j++)
    {
      if(measurers->nodes[j]->data == counted_data)
        meas_[i][index++] = measurers->nodes[j];
    }    
  }

  // 4) Envoie la sauce. Restez pas plante la, y'a plus rien a voir, bordel.
  *datas = datas_;
  *meas = meas_;
  *n_meas = n_meas_;
  *n_datas = n_datas_;

  return allocator_;
}

void Trainer::loadXFile(XFile *file)
{
  machine->loadXFile(file);
}

void Trainer::saveXFile(XFile *file)
{
  machine->saveXFile(file);
}

Trainer::~Trainer()
{
}

}
