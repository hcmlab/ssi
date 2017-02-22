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

#include "StochasticGradient.h"
#include "Random.h"

namespace Torch {

StochasticGradient::StochasticGradient(GradientMachine *machine_, Criterion *criterion_) : Trainer(machine_)
{
  // Boaf
  criterion = criterion_;

  addROption("end accuracy", &end_accuracy, 0.0001, "end accuracy");
  addROption("learning rate", &learning_rate, 0.01, "learning rate");
  addROption("learning rate decay", &learning_rate_decay, 0, "learning rate decay");
  addIOption("max iter", &max_iter, -1, "maximum number of iterations");
  addBOption("shuffle", &do_shuffle, true, "shuffle the dataset");
}

void StochasticGradient::train(DataSet *data, MeasurerList *measurers)
{
  int iter = 0;
  real err = 0;
  real prev_err = INF;
  real current_learning_rate = learning_rate;
  int n_train = data->n_examples;
  int *shuffle = (int *)Allocator::sysAlloc(n_train*sizeof(int));
  
  DataSet **datas;
  Measurer ***meas;
  int *n_meas;
  int n_datas;

  message("StochasticGradient: training");

  machine->setDataSet(data);
  criterion->setDataSet(data);

  if(measurers)
  {
    for(int i = 0; i < measurers->n_nodes; i++)
      measurers->nodes[i]->reset();
  }
  criterion->reset();

  Allocator *allocator_ = extractMeasurers(measurers, data, &datas, &meas, &n_meas, &n_datas);

  if(do_shuffle)
    Random::getShuffledIndices(shuffle, n_train);
  else
  {
    for(int i = 0; i < n_train; i++)
      shuffle[i] = i;
  }

  while(1)
  {
    ((GradientMachine *)machine)->iterInitialize();
    criterion->iterInitialize();
    err = 0;
    for(int t = 0; t < n_train; t++)
    {
      Parameters *der_params = ((GradientMachine *)machine)->der_params;
      if(der_params)
      {
        for(int i = 0; i < der_params->n_data; i++)
          memset(der_params->data[i], 0, sizeof(real)*der_params->size[i]);
      }
      data->setExample(shuffle[t]);
      machine->forward(data->inputs);
      criterion->forward(machine->outputs);
      criterion->backward(machine->outputs, NULL);
      ((GradientMachine *)machine)->backward(data->inputs, criterion->beta);
      
      for(int i = 0; i < n_meas[0]; i++)
        meas[0][i]->measureExample();
      
      Parameters *params = ((GradientMachine *)machine)->params;
      if(params)
      {
        for(int i = 0; i < params->n_data; i++)
        {
          real *ptr_params = params->data[i];
          real *ptr_der_params = der_params->data[i];
          
          for(int j = 0; j < params->size[i]; j++)
            ptr_params[j] -= current_learning_rate * ptr_der_params[j];
        }
      }
      // Note que peut-etre faudrait foutre
      // un "accumul_erreur" dans la classe Criterion
      // des fois que ca soit pas une somme...
      // Mais bon, a priori ca vient d'une integrale,
      // donc me gonflez pas.
      // PREVENIR ICI L'UTILISATEUR DE L'UTILITE
      // DE L'OUTPUT DANS UN CRITERION
      err += criterion->outputs->frames[0][0];
    }

    for(int i = 0; i < n_meas[0]; i++)
      meas[0][i]->measureIteration();

    // le data 0 est le train dans tous les cas...
    for(int julie = 1; julie < n_datas; julie++)
    {
      DataSet *dataset = datas[julie];

      for(int t = 0; t < dataset->n_examples; t++)
      {
        dataset->setExample(t);
        machine->forward(dataset->inputs);

        for(int i = 0; i < n_meas[julie]; i++)
          meas[julie][i]->measureExample();
      }

      for(int i = 0; i < n_meas[julie]; i++)
        meas[julie][i]->measureIteration();
    }

    print(".");
    err /= (real)(n_train);
    if(fabs(prev_err - err) < end_accuracy)
    {
      print("\n");
      break;
    }
    prev_err = err;

    iter++;      
    current_learning_rate = learning_rate/(1.+((real)(iter))*learning_rate_decay);
    if( (iter >= max_iter) && (max_iter > 0) )
    {
      print("\n");
      warning("StochasticGradient: you have reached the maximum number of iterations");
      break;
    }
  }
  free(shuffle);

  for(int julie = 0; julie < n_datas; julie++)
  {
    for(int i = 0; i < n_meas[julie]; i++)
      meas[julie][i]->measureEnd();
  }

  delete allocator_;
}

StochasticGradient::~StochasticGradient()
{
}

}
