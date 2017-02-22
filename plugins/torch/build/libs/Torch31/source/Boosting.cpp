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

#include "Boosting.h"
#include "BoostingMeasurer.h"
#include "Random.h"
#include "NullXFile.h"

namespace Torch {

static void randw(int *selected_examples, real *ex_weights, int n_examples)
{
  real *repartition = (real *)Allocator::sysAlloc(sizeof(real)*(n_examples+1));
  repartition[0] = 0;
  for(int i = 0; i < n_examples; i++)
    repartition[i+1] = repartition[i]+ex_weights[i];

  for(int i = 0; i < n_examples; i++)
  {
    real z = Random::uniform();
    int gauche = 0;
    int droite = n_examples;
    while(gauche+1 != droite)
    {
      int centre = (gauche+droite)/2;
      if(repartition[centre] < z)
        gauche = centre;
      else
        droite = centre;
    }
    selected_examples[i] = gauche;
//    printf("%g < %g < %g\n", repartition[gauche], z, repartition[gauche+1]);
  }
  free(repartition);
}

Boosting::Boosting(WeightedSumMachine* w_machine_, ClassFormat *class_format_) : Trainer(w_machine_)
{
  w_machine = w_machine_;
  class_format = class_format_;

  n_trainers = w_machine->n_trainers;
  weights = w_machine->weights;
}

void Boosting::train(DataSet *data, MeasurerList* measurers)
{
  int n_examples = data->n_examples;
  int *selected_examples = (int *)Allocator::sysAlloc(n_examples*sizeof(int));
  real *ex_weights = (real *)Allocator::sysAlloc(n_examples*sizeof(real));
  for(int t = 0; t < n_examples; t++)
    ex_weights[t] = 1./((real)n_examples);

  NullXFile null_xfile;
  BoostingMeasurer *measurer = new BoostingMeasurer(class_format, &null_xfile);

  measurer->setDataSet(data);
  measurer->setWeights(ex_weights);

  message("Boosting: training...");
  w_machine->n_trainers_trained = 0;

  MeasurerList the_boost_meas;
  the_boost_meas.addNode(measurer);

  // Initialise le boxon.
  for(int i = 0; i < n_trainers; i++)
    weights[i] = 0;

  for(int i = 0; i < n_trainers; i++)
  {
    randw(selected_examples, ex_weights, n_examples);
    data->pushSubset(selected_examples, n_examples);
    w_machine->trainers[i]->machine->reset();
    w_machine->trainers[i]->train(data, w_machine->trainers_measurers ? w_machine->trainers_measurers[i] : NULL);
    data->popSubset();

    // Calcule le nouveau 'beta'...
    measurer->setInputs(w_machine->trainers[i]->machine->outputs);
    w_machine->trainers[i]->test(&the_boost_meas);

    // Ben on vient d'entrainer un truc tu sais...
    w_machine->n_trainers_trained = i+1;

    // Check if all is classified [cas limite 1]
    if(measurer->beta == 0)
    {
      for(int j = 0; j < n_trainers; j++)
        weights[j] = 0;
      weights[i] = 1;
      warning("Boosting: train stopped. All examples are well classified.");

      // On teste quand meme...
      if(measurers)
        test(measurers);

      break;
    }

    // Regarde si c'est la misere [cas limite 2]
    if(measurer->beta >= 1)
    {
      w_machine->n_trainers_trained = i;
      warning("Boosting: train stopped. SSI_Model %d too weak.", i);
      break;
    }

    // Si tout va bien... ////////////////////////

    // Compute new weights
    int *ptr_status = measurer->status;
    real mul_pos = exp( 0.5*log(measurer->beta));
    real mul_neg = exp(-0.5*log(measurer->beta));
    for(int t = 0; t < n_examples; t++)
    {
      if(ptr_status[t] > 0)
        ex_weights[t] *= mul_pos;
      else
        ex_weights[t] *= mul_neg;
    }

    // Normalize les poids des exemples
    real z = 0;
    for(int t = 0; t < n_examples; t++)
      z += ex_weights[t];
    for(int t = 0; t < n_examples; t++)
      ex_weights[t] /= z;

    // Refourgue le bon poids a l'autre naze...
    weights[i] = -0.5*log(measurer->beta);

    // Teste pour voir ce que ca donne...
    if(measurers)
      test(measurers);

    // Fin de si tout va bien... ////////////////
  }

  // Jarte la misere
  free(selected_examples);
  free(ex_weights);
  delete measurer;

  // Vraiment faible mon pote!
  if(w_machine->n_trainers_trained == 0)
    return;

  // Normalize trainers weights...
  real z = 0;
  for(int i = 0; i < n_trainers; i++)
    z += weights[i];
  for(int i = 0; i < n_trainers; i++)
    weights[i] /= z;  
}

Boosting::~Boosting()
{
}

}
