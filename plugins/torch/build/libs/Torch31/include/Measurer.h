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

#ifndef MEASURER_INC
#define MEASURER_INC

#include "Object.h"
#include "Machine.h"
#include "DataSet.h"
#include "XFile.h"

namespace Torch {


/** Used to measure what you want during training/testing.
    Usually, trainers call measurers.
    The #DataSet# associated to the measurer allow us
    to know when the measurer should be called.
    (if the #DataSet# is the train dataset, the
    measurer will be called during the train phase...)

    Options:
    \begin{tabular}{lcll}
      "binary mode"  &  bool  &  binary mode for output & [false]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class Measurer : public Object
{
  public:
    /// The measurer save measures in this file.
    XFile *file;

    /// The associated #DataSet#.
    DataSet *data;

    /// Is the measurer in binary mode ?
    bool binary_mode;

    //-----

    /** Measurer with the associated #DataSet# #data_#,
        and put results in the file #file_#.
    */
    Measurer(DataSet *data_, XFile *file_);

    /** Measure something for the current example.
        (This example has been selected in #data#
         by the trainer)
    */
    virtual void measureExample();

    /** Measure something after the current iteration.
        (After the call of #measureExample()# for each
        example of #data#)
    */
    virtual void measureIteration();

    /// Measure something at the end of the training/testing phase.
    virtual void measureEnd();

    /// Reset the measurer. (By default, do nothing).
    virtual void reset();

    //-----

    virtual ~Measurer();
};


}

#endif
