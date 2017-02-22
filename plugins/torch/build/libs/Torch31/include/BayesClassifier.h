// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
//                and Bison Ravi (francois.belisle@idiap.ch)
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

#ifndef BAYES_CLASSIFIER_INC
#define BAYES_CLASSIFIER_INC


#include "Trainer.h"
#include "BayesClassifierMachine.h"

namespace Torch {

/** A multi class bayes classifier -- maximizes the likelihood of each class
    separately using a trainer for distribution. When testing, the predicted 
    class corresponds to the trainer giving the maximum output, weighted by
    its prior probability.
 
    @author Samy Bengio (bengio@idiap.ch)
    @author Bison Ravi (francois.belisle@idiap.ch)
*/
class BayesClassifier : public Trainer
{
  public:
    
    /// the bayes machine
    BayesClassifierMachine* bayesmachine; 
    
    /// the number of different classes
    int n_classes;
    
    /// all the example indices of each class.
    int** classes; 
    ///the number of examples per class.
    int* classes_n; 

    /// you need to define a BayesClassifierMachine to use this class
    BayesClassifier( BayesClassifierMachine* );
    virtual ~BayesClassifier();

    virtual void train( DataSet *data, MeasurerList *measurers);

};


}

#endif
