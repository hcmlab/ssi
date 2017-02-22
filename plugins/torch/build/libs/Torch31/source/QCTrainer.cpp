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

#include "QCTrainer.h"

namespace Torch {

QCTrainer::QCTrainer(QCMachine *qcmachine_) : Trainer(qcmachine_)
{
  qcmachine = qcmachine_;

  active_var = NULL;
  active_var_new = NULL;
  not_at_bound_at_iter = NULL;
  status_alpha = NULL;

  //------

#ifdef USE_DOUBLE
  addROption("eps shrink", &eps_shrink, 1E-9, "shrinking accuracy");
#else
  addROption("eps shrink", &eps_shrink, 1E-4, "shrinking accuracy");
#endif

  addBOption("unshrink", &unshrink_mode, false, "unshrink or not unshrink");
  addIOption("max unshrink", &n_max_unshrink, 1, "maximal number of unshrinking");
  addIOption("iter shrink", &n_iter_min_to_shrink, 100, "minimal number of iterations to shrink");
  addROption("end accuracy", &end_eps, 0.01, "end accuracy");
  addIOption("iter message", &n_iter_message, 1000, "number of iterations between messages");
}

void QCTrainer::prepareToLaunch()
{
  n_alpha = qcmachine->n_alpha;
  grad = qcmachine->grad;
  Cup = qcmachine->Cup;
  Cdown = qcmachine->Cdown;
  alpha = qcmachine->alpha;
  y = qcmachine->y;
  bound_eps = qcmachine->bound_eps;
  cache = qcmachine->cache;

  cache->allocate();
  deja_shrink = false;
  n_active_var = n_alpha;
  active_var = (int *)allocator->alloc(sizeof(int)*n_alpha);
  active_var_new = (int *)allocator->alloc(sizeof(int)*n_alpha);
  not_at_bound_at_iter = (int *)allocator->alloc(sizeof(int)*n_alpha);
  status_alpha = (char *)allocator->alloc(sizeof(char)*n_alpha);

  for(int i = 0; i < n_alpha; i++)
  {
    active_var[i] = i;
    updateStatus(i);
//    status_alpha[i] = 1;
    not_at_bound_at_iter[i] = 0;
  }
}

void QCTrainer::atomiseAll()
{
  cache->destroy();
  allocator->free(active_var);
  allocator->free(active_var_new);
  allocator->free(not_at_bound_at_iter);
  allocator->free(status_alpha);

  active_var = NULL;
  active_var_new = NULL;
  not_at_bound_at_iter = NULL;
  status_alpha = NULL;
}

bool QCTrainer::selectVariables(int *i, int *j)
{
  real gmax_i = -INF;
  real gmin_j =  INF;
  int i_ = -1;
  int j_ = -1;

  for(int it = 0; it < n_active_var; it++)   
  {
    int t = active_var[it];

    if(y[t] > 0)
    {
      if(isNotDown(t))
      {
        if(grad[t] > gmax_i)
        {
          gmax_i = grad[t];
          i_ = t;
        }
      }

      if(isNotUp(t))
      {
        if(grad[t] < gmin_j)
        {
          gmin_j = grad[t];
          j_ = t;
        }
      }            
    }
    else
    {
      if(isNotUp(t))
      {
        if(-grad[t] > gmax_i)
        {
          gmax_i = -grad[t];
          i_ = t;
        }
      }
      
      if(isNotDown(t))
      {
        if(-grad[t] < gmin_j)
        {
          gmin_j = -grad[t];
          j_ = t;
        }
      }            
    }
  }

  current_error =  gmax_i - gmin_j;

  if(current_error < end_eps)
    return(true);
  
  if( (i_ == -1) || (j_ == -1) )
    return(true);

  *i = i_;
  *j = j_;

  return(false);
}

// Renvoie le nb de var susceptibles d'etre shrinkee
int QCTrainer::checkShrinking(real bmin, real bmax)
{
  real bb = (bmin+bmax)/2.;

  n_active_var_new = 0;
  for(int it = 0; it < n_active_var; it++)
  {
    int t = active_var[it];
    bool garde = true;

    if(isNotDown(t) && isNotUp(t))
      not_at_bound_at_iter[t] = iter;
    else
    {
      if(isNotUp(t)) // Donc elle est en bas.
      {
        if(grad[t] + y[t]*bb < eps_shrink)
          not_at_bound_at_iter[t] = iter;
        else
        {
          if( (iter - not_at_bound_at_iter[t]) > n_iter_min_to_shrink)
            garde = false;
        }
      }
      else
      {
        if(grad[t] + y[t]*bb > -eps_shrink)
          not_at_bound_at_iter[t] = iter;
        else
        {
          if( (iter - not_at_bound_at_iter[t]) > n_iter_min_to_shrink)
            garde = false;
        }
      }      
    }

    if(garde)
      active_var_new[n_active_var_new++] = t;
  }

  return(n_active_var-n_active_var_new);
}

void QCTrainer::shrink()
{
  n_active_var = n_active_var_new;
  int *ptr_sav = active_var;
  active_var = active_var_new;
  active_var_new = ptr_sav;
  deja_shrink = true;

  if(!unshrink_mode)
    cache->setActiveVariables(active_var, n_active_var);
}

void QCTrainer::unShrink()
{
  for(int i = 0; i < n_alpha; i++)
    active_var[i] = i;

  n_active_var = n_alpha;
  deja_shrink = false;

  if(++n_unshrink == n_max_unshrink)
  {
    unshrink_mode = false;
    n_iter_min_to_shrink = 666666666;
    warning("QCTrainer: shrinking and unshrinking desactived...");
  }
}

void QCTrainer::train(DataSet *data, MeasurerList *measurers)
{
  qcmachine->setDataSet(data);
  prepareToLaunch();

  int xi, xj;
  int n_to_shrink = 0;
  n_unshrink = 0;

  message("QCTrainer: training...");

  iter = 0;
  while(1)
  {
    if(selectVariables(&xi, &xj))
    {
      if(unshrink_mode)
      {
        message("QCTrainer: unshrink...");
        unShrink();
        if(selectVariables(&xi, &xj))
        {
          message("QCTrainer: ...finished");
          break;
        }
        else
          message("QCTrainer: ...restart");
      }
      else
        break;
    }

    if(iter >= n_iter_min_to_shrink)
      n_to_shrink = checkShrinking(-y[xi]*grad[xi], -y[xj]*grad[xj]);

    k_xi = cache->adressCache(xi);
    k_xj = cache->adressCache(xj);

    old_alpha_xi = alpha[xi];
    old_alpha_xj = alpha[xj];

    analyticSolve(xi, xj);

    real delta_alpha_xi = alpha[xi] - old_alpha_xi;
    real delta_alpha_xj = alpha[xj] - old_alpha_xj;

    if(deja_shrink && !unshrink_mode)
    {
      for(int t = 0; t < n_active_var; t++)
      {
        int it = active_var[t];
        grad[it] += k_xi[it]*delta_alpha_xi + k_xj[it]*delta_alpha_xj;
      }
    }
    else
    {
      for(int t = 0; t < n_alpha; t++)
        grad[t] += k_xi[t]*delta_alpha_xi + k_xj[t]*delta_alpha_xj;
    }

    iter++;
    if(! (iter % n_iter_message) )
    {
      // Pour ne pas effrayer le neophite.
      if(current_error < 0)
        current_error = 0;
      print("  + Iteration %d\n", iter);
      print("   --> Current error    = %g\n", current_error);
      print("   --> Active variables = %d\n", n_active_var);
    }

    /////////////// Shhhhhrinnnk

    if(!(iter % n_iter_min_to_shrink))
    {
      if( (n_to_shrink > n_active_var/10) && (n_active_var-n_to_shrink > 100) )
        shrink();
    }
  }

  // Pour ne pas effrayer le neophite.
  if(current_error < 0)
    current_error = 0;
  print("  + Iteration %d\n", iter);
  print("   --> Current error    = %g\n", current_error);
  print("   --> Active variables = %d\n", n_active_var);

  qcmachine->checkSupportVectors();
  atomiseAll();
}

void QCTrainer::updateStatus(int i)
{
  if(alpha[i] < Cup[i] - bound_eps)
    status_alpha[i] = 1;
  else
    status_alpha[i] = 0;

  if(alpha[i] > Cdown[i] + bound_eps)
    status_alpha[i] |= 2;
}

void QCTrainer::analyticSolve(int xi, int xj)
{
  real ww, H, L;

  real s = y[xi]*y[xj];
  if(s < 0)
  {
    ww = old_alpha_xi - old_alpha_xj;
    L = ((Cdown[xj]+ww >   Cdown[xi]) ? Cdown[xj]+ww :  Cdown[xi]);
    H = ((Cup[xj]+ww   >     Cup[xi]) ? Cup[xi]      : Cup[xj]+ww);
  }
  else
  {
    ww = old_alpha_xi + old_alpha_xj;
    L = ((ww-Cup[xj]   >    Cdown[xi]) ? ww-Cup[xj] :     Cdown[xi]);
    H = ((ww-Cdown[xj] >      Cup[xi]) ?    Cup[xi] :  ww-Cdown[xj]);
  }

  real eta = k_xi[xi] - 2.*s*k_xi[xj] + k_xj[xj];
  if(eta > 0)
  {
    real alph = old_alpha_xi + (s*grad[xj] - grad[xi])/eta;
	
    if(alph > H)
      alph = H;
    else
    {
      if(alph < L)
        alph = L;
    }
    
    alpha[xi] = alph;
    alpha[xj] -= s*(alpha[xi]-old_alpha_xi);
  }
  else
  {
    print(".");
    real alph = grad[xi] - s*grad[xj];
    if(alph > 0)
    {
      alpha[xi] = L;
      alpha[xj] -= s*(alpha[xi]-old_alpha_xi);
    }
    else
    {
      alpha[xi] = H;
      alpha[xj] -= s*(alpha[xi]-old_alpha_xi);
    }
  }

  updateStatus(xi);
  updateStatus(xj);
}

QCTrainer::~QCTrainer()
{
}

}
