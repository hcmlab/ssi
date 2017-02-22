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

#include "ConnectedMachine.h"

// Entering Level 2 code zone.
// Wizard with more than 100000XP allowed only.
// You shoud go away.
// You're still here ?!
// You've been warned.

namespace Torch {

ConnectedMachine::ConnectedMachine() : GradientMachine(0, 0)
{
  n_layers = 0;
  n_machines_on_layer = NULL;
  machines = NULL;

  current_layer = -1;
  current_machine = -1;

  // NOTE: start_alpha hack_a_donf mode. Comme je le rempli moi-meme,
  // je fous pas frame_size ici.
  start_alpha = new(allocator) Sequence;
  current_alpha_offset = 0;

  addLayer();
}

void ConnectedMachine::build()
{
  // Check links
  checkInternalLinks();

  // Outputs... direct connection ?
  if(n_machines_on_layer[n_layers-1] > 1)
    outputs = new(allocator) Sequence(0, n_outputs);
  else
    outputs = machines[n_layers-1][0]->machine->outputs;

  // Beta...
  if(n_machines_on_layer[0] > 1)
    beta = new(allocator) Sequence(0, n_inputs);
  else
    beta = machines[0][0]->machine->beta;

  // Params...
  for(int l = 0; l < n_layers; l++)
  {
    for(int m = 0; m < n_machines_on_layer[l]; m++)
    {
      ConnectedNode *node = machines[l][m];

      if(l > 0)
      {
        if(node->n_input_links == 1)
          node->inputs = node->input_links[0];
        else
          node->inputs = new(allocator) Sequence(0, node->machine->n_inputs);
      }

      node->alpha = new(allocator) Sequence(0, node->machine->n_outputs);
      params->add(node->machine->params);
      der_params->add(node->machine->der_params);
    }
  }
}

void ConnectedMachine::addFCL(GradientMachine *machine)
{
  if(n_machines_on_layer[current_layer])
    addLayer();
  
  addMachine(machine);
  
  if(n_layers > 1)
  {
    for(int i = 0; i < n_machines_on_layer[current_layer-1]; i++)
      connectOn(machines[current_layer-1][i]->machine);
  }
}

void ConnectedMachine::addLayer()
{
  n_outputs = 0;

  if(n_layers > 0)
  {
    if(n_machines_on_layer[n_layers-1] == 0)
      error("ConnectedMachine: one layer without any machine !?!");
  }

  machines = (ConnectedNode ***)allocator->realloc(machines, (n_layers+1)*sizeof(ConnectedNode **));
  n_machines_on_layer = (int *)allocator->realloc(n_machines_on_layer, sizeof(int)*(n_layers+1));
  machines[n_layers] = NULL;
  n_machines_on_layer[n_layers] = 0;
  
  current_layer = n_layers;
  current_machine = -1;

  n_layers++;
}

void ConnectedMachine::addMachine(GradientMachine *machine)
{
  machines[current_layer] = (ConnectedNode **)allocator->realloc(machines[current_layer],
                                                    (n_machines_on_layer[current_layer] + 1)*sizeof(ConnectedNode *));
  machines[current_layer][n_machines_on_layer[current_layer]] = (ConnectedNode *)allocator->alloc(sizeof(ConnectedNode));

  // Initialisations diverses
  ConnectedNode *node = machines[current_layer][n_machines_on_layer[current_layer]];
  node->machine = machine;
  node->input_links = NULL;
  node->n_input_links = 0;
  node->alpha_links = NULL;
  node->n_alpha_links = 0;
  node->alpha_links_offset = NULL;
  node->n_inputs_check = 0;
  node->inputs = NULL;
  node->alpha = NULL;

  //---

  current_machine = n_machines_on_layer[current_layer];
  n_machines_on_layer[current_layer]++;

  if(current_layer == 0)
  {
    if(n_machines_on_layer[0] > 1)
    {
      if(machine->n_inputs != n_inputs)
        error("ConnectedMachine: trying to connect machine of different input size at the first layer");
    }
    else
      n_inputs = machine->n_inputs;
  }

  n_outputs += machine->n_outputs;
  current_alpha_offset = 0;
}

void ConnectedMachine::connectOn(GradientMachine *machine)
{
  if(current_machine < 0)
    error("ConnectedMachine: no machine to connect");

  bool flag = true;
  int l, m = -666;
  for(l = 0; (l < current_layer) && flag; l++)
  {
    for(m = 0; m < n_machines_on_layer[l]; m++)
    {
      if(machines[l][m]->machine == machine)
      {
        flag = false;
        break;
      }
    }
  }

  l--;

  if(flag)
    error("ConnectedMachine: cannot connect your machine");

  ConnectedNode *node = machines[current_layer][current_machine];
  node->input_links = (Sequence **)allocator->realloc(node->input_links, sizeof(Sequence *)*(node->n_input_links+1));
  node->input_links[node->n_input_links] = machine->outputs;
  node->n_inputs_check += machines[l][m]->machine->n_outputs;
  node->n_input_links++;

  node = machines[l][m];
  node->alpha_links = (Sequence **)allocator->realloc(node->alpha_links, sizeof(Sequence *)*(node->n_alpha_links+1));
  node->alpha_links[node->n_alpha_links] = machines[current_layer][current_machine]->machine->beta;
  node->alpha_links_offset = (int *)allocator->realloc(node->alpha_links_offset, sizeof(int)*(node->n_alpha_links+1));
  node->alpha_links_offset[node->n_alpha_links] = current_alpha_offset;
  node->n_alpha_links++;

  current_alpha_offset += machine->n_outputs;

//  printf("[%d %d on %d %d] machine %d outputs. = machine mere: %d outputs. machine fils: %d inputs\n", l, m, current_layer, current_machine, machine->n_outputs, machines[l][m]->n_outputs, machines[current_layer][current_machine]->n_inputs);
}

void ConnectedMachine::checkInternalLinks()
{
  for(int l = 1; l < n_layers; l++)
  {
    for(int m = 0; m < n_machines_on_layer[l]; m++)
    {
      if(machines[l][m]->machine->n_inputs != machines[l][m]->n_inputs_check)
        error("ConnectedMachine: incorrect number of inputs for machine [%d %d] (%d instead of %d)", l, m, machines[l][m]->machine->n_inputs, machines[l][m]->n_inputs_check);
    }
  }
}

void ConnectedMachine::forward(Sequence *inputs)
{
  for(int m = 0; m < n_machines_on_layer[0]; m++)
    machines[0][m]->machine->forward(inputs);

  for(int l = 1; l < n_layers; l++)
  {
    for(int m = 0; m < n_machines_on_layer[l]; m++)
    {
      ConnectedNode *node = machines[l][m];

      // NOTE: check for direct input connection
      // NOTE: node->inputs doit etre alors sur node->input_links[0] (sinon sequence vide)
      if(node->n_input_links == 1)
        node->machine->forward(node->inputs);
      else
      {
        int n_frames_ = node->input_links[0]->n_frames;
        node->inputs->resize(n_frames_);

        int offset_ = 0;
        for(int i = 0; i < node->n_input_links; i++)
        {
          int n_inputs_ = node->input_links[i]->frame_size;
          for(int j = 0; j < n_frames_; j++)
          {
            real *dest_ = node->inputs->frames[j] + offset_;
            real *src_ = node->input_links[i]->frames[j];

            for(int k = 0; k < n_inputs_; k++)
              dest_[k] = src_[k];
          }
          offset_ += n_inputs_;
        }
        node->machine->forward(node->inputs);
      }
    }
  }

  // NOTE: if not direct output connection, updates output.
  if(n_machines_on_layer[n_layers-1] > 1)
  {
    int n_frames_ = machines[n_layers-1][0]->machine->outputs->n_frames;
    outputs->resize(n_frames_);

    int offset_ = 0;
    for(int i = 0; i < n_machines_on_layer[n_layers-1]; i++)
    {
      int n_outputs_ = machines[n_layers-1][i]->machine->n_outputs;
      for(int j = 0; j < n_frames_; j++)
      {
        real *dest_ = outputs->frames[j] + offset_;
        real *src_ = machines[n_layers-1][i]->machine->outputs->frames[j];
        
        for(int k = 0; k < n_outputs_; k++)
          dest_[k] = src_[k];
      }
      offset_ += n_outputs_;
    }
  }
}

void ConnectedMachine::backward(Sequence *inputs, Sequence *alpha)
{
  Sequence *alpha_ = NULL;
  if(n_machines_on_layer[n_layers-1] > 1)
  {
    start_alpha->resize(alpha->n_frames, false);
    start_alpha->frame_size = machines[n_layers-1][0]->machine->n_outputs;
    for(int i = 0; i < alpha->n_frames; i++)
      start_alpha->frames[i] = alpha->frames[i];
    alpha_ = start_alpha;
  }
  else
    alpha_ = alpha;

  if(n_layers > 1)
  {
    for(int m = 0; m < n_machines_on_layer[n_layers-1]; m++)
    {
      ConnectedNode *node = machines[n_layers-1][m];

      // NOTE: on ne tripote donc pas, en aucun cas, le alpha donne par l'utilisateur...
      // NOTE: dans le truc qui suit c'est bien un +=...
      if(m > 0)
      {
        int offset_ = machines[n_layers-1][m-1]->machine->n_outputs;
        alpha_->frame_size = node->machine->n_outputs;
        for(int i = 0; i < alpha_->n_frames; i++)
          alpha_->frames[i] += offset_;
      }

      node->machine->backward(node->inputs, alpha_);
    }
  }
  else
  {
    for(int m = 0; m < n_machines_on_layer[0]; m++)
    {
      ConnectedNode *node = machines[0][m];

      if(m > 0)
      {
        // NOTE: on ne tripote donc pas, en aucun cas, le alpha donne par l'utilisateur...
        // NOTE: dans le truc qui suit c'est bien un +=...
        int offset_ = machines[0][m-1]->machine->n_outputs;
        alpha_->frame_size = node->machine->n_outputs;
        for(int i = 0; i < alpha_->n_frames; i++)
          alpha_->frames[i] += offset_;
      }

      node->machine->backward(inputs, alpha_);
    }
  }

  // NOTE: on pourrait encore optimiser une copie, mais 'sti...
  for(int l = n_layers-2; l >= 0; l--)
  {
    for(int m = 0; m < n_machines_on_layer[l]; m++)
    {
      ConnectedNode *node = machines[l][m];

      // NOTE: n_frames: fournie par alpha_link. Attention offset_alpha.
      // NOTE: taille de alpha fournie par machine->n_outputs;
      // a) Fout la taille de alpha comme il faut et initialise a 0
      int n_frames_ = node->alpha_links[0]->n_frames;
      int size_ = node->machine->n_outputs;
      node->alpha->resize(n_frames_);
      Sequence *alpha_ = node->alpha;

      for(int i = 0; i < n_frames_; i++)
      {
        real *z = alpha_->frames[i];
        for(int j = 0; j < size_; j++)
          z[j] = 0;
      }
      
      // b) Fait la putain de somme des CI_MO_NAK d'alphas...
      for(int i = 0; i < node->n_alpha_links; i++)
      {
        for(int j = 0; j < n_frames_; j++)
        {
          real *src_ = node->alpha_links[i]->frames[j] + node->alpha_links_offset[i];
          real *dest_ = alpha_->frames[j];
          for(int k = 0; k < size_; k++)
            dest_[k] += src_[k];
        }
      }

      // c) backward le boxon
      if(l == 0)
        machines[0][m]->machine->backward(inputs, alpha_);
      else
        machines[l][m]->machine->backward(node->inputs, alpha_);
    }
  }


  if( (n_machines_on_layer[0] > 1) && (!partial_backprop) )
  {
    // a) Fout la taille de beta comme il faut et initialise a 0
    int n_frames_ = machines[0][0]->machine->beta->n_frames;
    beta->resize(n_frames_);

    for(int i = 0; i < n_frames_; i++)
    {
      real *dest_ = beta->frames[i];
      for(int j = 0; j < n_inputs; j++)
        dest_[j] = 0;
    }
    
    // b) Fait la putain de somme des putains de beta
    for(int i = 0; i < n_machines_on_layer[0]; i++)
    {
      for(int j = 0; j < n_frames_; j++)
      {
        real *dest_ = beta->frames[j];
        real *src_ = machines[0][i]->machine->beta->frames[j];
        
        for(int k = 0; k < n_inputs; k++)
          dest_[k] += src_[k];
      }
    }
  }
}

void ConnectedMachine::reset()
{
  for(int i = 0; i < n_layers; i++)
  {
    for(int m = 0; m < n_machines_on_layer[i]; m++)
      machines[i][m]->machine->reset();
  }
}

void ConnectedMachine::iterInitialize()
{
  for(int i = 0; i < n_layers; i++)
  {
    for(int m = 0; m < n_machines_on_layer[i]; m++)
      machines[i][m]->machine->iterInitialize();
  }  
}

void ConnectedMachine::loadXFile(XFile *file)
{
  for(int i = 0; i < n_layers; i++)
  {
    for(int m = 0; m < n_machines_on_layer[i]; m++)
      machines[i][m]->machine->loadXFile(file);
  }
}

void ConnectedMachine::saveXFile(XFile *file)
{
  for(int i = 0; i < n_layers; i++)
  {
    for(int m = 0; m < n_machines_on_layer[i]; m++)
      machines[i][m]->machine->saveXFile(file);
  }
}

void ConnectedMachine::setPartialBackprop(bool flag)
{
  partial_backprop = flag;
  for(int i = 0; i < n_machines_on_layer[0]; i++)
    machines[0][i]->machine->setPartialBackprop(flag);
}

void ConnectedMachine::setDataSet(DataSet *dataset_)
{
  for(int i = 0; i < n_layers; i++)
  {
    for(int m = 0; m < n_machines_on_layer[i]; m++)
      machines[i][m]->machine->setDataSet(dataset_);
  }
}

ConnectedMachine::~ConnectedMachine()
{
}

}

// End of special zone.
