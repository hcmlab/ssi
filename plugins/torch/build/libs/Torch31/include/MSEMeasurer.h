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

#ifndef MSE_MEASURER_INC
#define MSE_MEASURER_INC

#include "Measurer.h"

namespace Torch {

/** Mean Squared Error measurer.
    Compute the MSE between its inputs,
    and the targets of its associated #DataSet#.

  addBOption("average examples", &average_examples, true, "divided by the number of examples");
  addBOption("average frame size", &average_frame_size, true, "divided by the frame size");
  addBOption("average frames", &average_frames, true, "divided by the number of frames");

    Options:
    \begin{tabular}{lcll}
      "average examples"    &  bool  &  divided by the number of examples  &  [true]\\
      "average frame size"  &  bool  &  divided by the frame size          &  [true]\\
      "average frames"      &  bool  &  divided by the number of frames    &  [true]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class MSEMeasurer : public Measurer
{
  public:
    bool average_examples;
    bool average_frame_size;
    bool average_frames;
    real internal_error;
    Sequence *inputs;

    //-----

    ///
    MSEMeasurer(Sequence *inputs_, DataSet *data_, XFile *file_);

    //-----

    virtual void reset();
    virtual void measureExample();
    virtual void measureIteration();

    virtual ~MSEMeasurer();
};

}

#endif
