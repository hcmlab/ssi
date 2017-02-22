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

#ifndef CLASS_NLL_MEASURER_INC
#define CLASS_NLL_MEASURER_INC

#include "Measurer.h"
#include "DataSet.h"
#include "ClassFormat.h"

namespace Torch {

/** This class measures the negative log likelihood. In fact, it supposes
    that for each input frames, frames[i] is the log-probability for class $i$.
    The given #class_format# gives the class format of the targets of the dataset.

    By default, the measurer divides the log-probabilty by the number of examples
    and the number of frames.

    @author Ronan Collobert (collober@idiap.ch)
*/
class ClassNLLMeasurer : public Measurer
{
  public:
    ClassFormat *class_format;
    bool average_examples;
    bool average_frames;
    real internal_error;
    Sequence *inputs;
    
    //-----

    ///
    ClassNLLMeasurer(Sequence *inputs_, DataSet *data_, ClassFormat *class_format_, XFile *file_);

    //-----

    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~ClassNLLMeasurer();
};


}

#endif
