// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#ifndef EDIT_DISTANCE_MEASURER_INC
#define EDIT_DISTANCE_MEASURER_INC

#include "Measurer.h"
#include "EditDistance.h"
#include "DataSet.h"

namespace Torch {

/** This class can be used to measure and print an #EditDistance# object.
    It is used for instance in speech recognition experiments.

    @author Samy Bengio (bengio@idiap.ch)
*/
class EditDistanceMeasurer : public Measurer
{
  public:
    /// the #EditDistance# to measure
    EditDistance *edit;

    /// an accumulator of #EditDistance# objects, to compute statistics
    EditDistance *sum;

    /// do we print the edit distance for each example?
    bool print_ex;

    ///
    EditDistanceMeasurer(EditDistance *edit_, DataSet *data_, XFile *file_,bool is_confusion_=false);

    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~EditDistanceMeasurer();
};


}

#endif
