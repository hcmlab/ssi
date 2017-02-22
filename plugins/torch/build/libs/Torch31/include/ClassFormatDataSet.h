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

#ifndef CLASS_FORMAT_DATA_SET_INC
#define CLASS_FORMAT_DATA_SET_INC

#include "DataSet.h"

namespace Torch {

/** Given a DataSet, convert (on-the-fly) targets using a conversion table.

    The targets of the given DataSet should be 0,1,2... for the class 0, class 1,
    class 2, and so on. Note that is must START AT 0.

    After a setExample(), the inputs of this DataSet will be the same as
    the provided one. The targets will be #class_labels->frames[i]#, where #i#
    is contained in the targets of the provided DataSet.

    @author Ronan Collobert (collober@idiap.ch)
 */
class ClassFormatDataSet : public DataSet
{
  public:
    /// The provided DataSet.
    DataSet *data;

    /// The class label translation table.
    Sequence *class_labels;

    /// Here you provide the translation table in #class_labels_#.
    ClassFormatDataSet(DataSet *data_, Sequence *class_labels_);

    /** We assume here that you want the one-hot encoding format.
        The number of classes is given by #n_classes#, if positive.
        Otherwise, the number of classes is guessed by taking the
        maximum value of the targets of the provided DataSet.
    */
    ClassFormatDataSet(DataSet *data_, int n_classes=-1);
    
    virtual void getNumberOfFrames(int t_, int *n_input_frames_, int *n_target_frames_);
    virtual void setRealExample(int t, bool set_inputs=true, bool set_targets=true);
    virtual void preProcess(PreProcessing *pre_processing);
    virtual void pushExample();
    virtual void popExample();

    //-----

    virtual ~ClassFormatDataSet();
};

}

#endif
