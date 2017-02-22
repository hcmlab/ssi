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

#ifndef TRAINER_INC
#define TRAINER_INC

#include "Object.h"
#include "Machine.h"
#include "DataSet.h"
#include "List.h"
#include "Measurer.h"

namespace Torch {

DEFINE_NEW_LIST(MeasurerList, Measurer);

/** Trainer.
  
    A trainer takes a #Machine# and is able to train this machine on a given dataset
    with the #train()# method.
    For each machine, it should exist a trainer which knows how to train this machine.
    Testing the machine is possible with the #test()# method.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Trainer : public Object
{
  public:
    Machine *machine;

    //-----

    ///
    Trainer(Machine *machine_);

    //-----

    /** Train the machine.
        The Trainer has to call the measurers
        when it want.
    */
    virtual void train(DataSet *data_, MeasurerList *measurers) = 0;

    /** Test the machine.
        This method call all the measurers,
        for all the examples of their associated
        dataset.
        It's already written...
    */
    virtual void test(MeasurerList *measurers);

    /** Make a table of measurers from a #List#.
    
        Given a #List# of #measurers#,
        and, if you want, a #train# #DataSet# (else NULL)
        \begin{itemize}
          \item Returns all datasets associated to the measurers in #datas#.
                For i != j, (*datas)[i] != (*datas)[j].
                Moreover, if #train# != NULL, (*datas)[0] = #train#.
          \item Returns the list of measurers associated to (*datas)[i] in (*meas)[i].
          \item Returns the number of measureurs associated to (*datas)[i] in (*n_meas)[i].
          \item Returns in *n_datas the number of datasets in *datas.
        \end{itemize}
    
    Returns an allocator to all the memory allocated by the function.
    You have to delete this allocator by yourself.
    */
    static Allocator *extractMeasurers(MeasurerList *measurers, DataSet *train, DataSet ***datas, Measurer ****meas, int **n_meas, int *n_datas);

    /// By default, just load the machine
    virtual void loadXFile(XFile *file);

    /// By default, just save the machine
    virtual void saveXFile(XFile *file);

    //-----

    virtual ~Trainer();
};

}

#endif
