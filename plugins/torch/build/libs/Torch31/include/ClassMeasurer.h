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

#ifndef CLASS_MEASURER_INC
#define CLASS_MEASURER_INC

#include "Measurer.h"
#include "ClassFormat.h"

namespace Torch {

/** Compute the classification error (in %)
    of the #inputs# with respect to the #targets# of #data#.

    The format of the class is given with a #ClassFormat#.
    It can print the confusion matrix if specified.

    @author Ronan Collobert (collober@idiap.ch)
*/
class ClassMeasurer : public Measurer
{
  public:
    real internal_error;
    int **confusion;
    Sequence *inputs;
    ClassFormat *class_format;
    bool calc_confusion_at_each_iter;
    int n_classes;
    int n_examples;

    //-----

    ///
    ClassMeasurer(Sequence *inputs_, DataSet *data_, ClassFormat *class_format_, XFile *file_,
                  bool calc_confusion_at_each_iter_=false);

    //-----

    void printConfusionMatrix();
    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();
    void reset_();

    virtual ~ClassMeasurer();
};

}

#endif
