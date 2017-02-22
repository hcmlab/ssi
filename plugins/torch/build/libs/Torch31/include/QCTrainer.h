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

#ifndef QC_TRAINER_INC
#define QC_TRAINER_INC

#include "Trainer.h"
#include "QCMachine.h"
#include "QCCache.h"

namespace Torch {


/** Train a #QCMachine#.
    With the conventions of QCMachine.h,
    Q is given by the class QCCache (in #cache#)

    Options:
    \begin{tabular}{lcll}
      "unshrink"      & bool  &  unshrink or not unshrink              & [false] \\
      "max unshrink"  & int   &  maximal number of unshrinking         & [1] \\
      "iter shrink"   & int   &  minimal number of iterations to shrink& [100] \\
      "eps shrink"    & real  &  shrinking accuracy                    & [1E-4 (f)  1E-9 (d)] \\
      "end accuracy"  & real  &  end accuracy                          & [0.01] \\
      "iter message"  & int   &  number of iterations between messages & [1000]
    \end{tabular}


    Note: "iter shrink" must be carefully chosen.
    Read http://www.ai.mit.edu/projects/jmlr/papers/volume1/collobert01a/collobert01a.ps.gz
    for more details.

    @author Ronan Collobert (collober@idiap.ch)
    @see QCCache
    @see QCMachine
*/
class QCTrainer : public Trainer
{
  public:
    // ohhh boy, c'est la zone
    QCMachine *qcmachine;
    QCCache *cache;

    int n_unshrink;
    int n_max_unshrink;

    real *k_xi;
    real *k_xj;
  
    real old_alpha_xi;
    real old_alpha_xj;
    real current_error;

    int *active_var_new;
    int n_active_var_new;

    int n_alpha;                  // le nb de alphas

    bool deja_shrink;
    bool unshrink_mode;

    real *y;
    real *alpha;
    real *grad;

    real eps_shrink;
    real end_eps;
    real bound_eps;

    int n_active_var;
    int *active_var;
    int *not_at_bound_at_iter;
    int iter;
    int n_iter_min_to_shrink;
    int n_iter_message;

    char *status_alpha;
    real *Cup;
    real *Cdown;

    //-----

    ///
    QCTrainer(QCMachine *qcmachine_);

    /** Train it...
        Before calling this function, #grad# in #qcmachine#
        must contain the gradient of QP(alpha) with respect
        to alpha = 0.

        ( = $beta$, with the conventions of QCMachine.h)

        Moreover #alpha# in #qcmachine# has to be zero.
    */
    void train(DataSet *data, MeasurerList *measurers);

    //-----

    void prepareToLaunch();
    void atomiseAll();

    bool bCompute();
    bool selectVariables(int *i, int *j);
    int checkShrinking(real bmin, real bmax);
    void shrink();
    void unShrink();

    void analyticSolve(int xi, int xj);
    void updateStatus(int i);  
    inline bool isNotUp(int i)   {  return(status_alpha[i] != 2); };
    inline bool isNotDown(int i) {  return(status_alpha[i] != 1); };

    virtual ~QCTrainer();
};


}

#endif
