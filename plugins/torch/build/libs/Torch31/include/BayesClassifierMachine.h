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

#ifndef BAYES_CLASSIFIER_MACHINE_INC
#define BAYES_CLASSIFIER_MACHINE_INC

#include "Machine.h"
#include "EMTrainer.h"
#include "ClassFormat.h"

namespace Torch {

/** BayesClassifierMachine is the machine used by the #BayesClassifier#
    trainer to perform a Bayes Classification using different distributions.
    The output corresponds to the class that is the most probable
    (using prior AND posterior information).

    @author Samy Bengio (bengio@idiap.ch)
    @author Bison Ravi (francois.belisle@idiap.ch)
 */
class BayesClassifierMachine : public Machine
{
  public:
    
    /// the number of classes corresponds to the number of #Trainer#
    int n_trainers; 

    /// the number of outputs in this machine
    int n_outputs; 

    /// the actual trainers (EMTrainer since we are training distributions).
    EMTrainer** trainers; 

    /** the log_prior probabilities of each class. default: log_priors are
        taken as the log of the proportions in the training set.
    */
    real* log_priors; 
    
		/// it contains the log posterior probability plus the log prior of the class.
		Sequence* log_probabilities;

    /// used to know if log_priors where given or allocated
    bool allocated_log_priors;

    /// the class format of the output
    ClassFormat* class_format;

    /// the measurers for each individual trainer
    MeasurerList** trainers_measurers; 

    /** creates a machine for BayesClassifier trainers, given a vector of
        trainers (one per class), an associate measurer for each trainer,
        a class_format that explains how the classes are coded, and an eventual
        vector (of size #n_trainers_#) containing the log of the class priors.
    */
    BayesClassifierMachine( EMTrainer**, int n_trainers_, MeasurerList** trainers_measurers_ , ClassFormat* class_format_, real* log_priors_=NULL);
    virtual ~BayesClassifierMachine();

    /** definition of virtual functions of #Machine# */
    virtual void forward(Sequence *inputs);
    virtual void reset();
    virtual void loadXFile( XFile* );
    virtual void saveXFile( XFile* );

};


}

#endif
