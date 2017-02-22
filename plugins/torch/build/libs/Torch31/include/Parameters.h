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

#ifndef PARAMETERS_INC
#define PARAMETERS_INC

#include "Object.h"

namespace Torch {

/** Parameters definition.
    Parameters are a bench of real arrays.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Parameters : public Object
{
  public:
    /** Array of pointers to array of parameters.
        The size of this array is given by #n_data#.
        The size of #data[i]# is given by #size[i]#
    */
    real **data;

    /// Size of #data#
    int n_data;

    /// #size[i]# is the size of #data[i]#
    int *size;

    /// Total number of parameters
    int n_params;
    
    /// No parameters ?
    Parameters();

    /// Create one entry in #data# with the given size.
    Parameters(int n_params_);

    /** Add an entry in #data#.
        If #do_copy# is true, copy the parameters.
        Else, just copy the pointer.
    */
    void addParameters(real *params, int n_params_, bool do_copy=false);
    
    /** Add all entries given by #params# in #data#.
        If #do_copy# is true, copy the parameters, else just copy the pointers.
     */
    void add(Parameters *params, bool do_copy=false);

    /** Copy the given parameters.
        The given parameters don't need to have the same structure.
        But it must have the same total length.
    */
    void copy(Parameters *from);

    /** Copy a real vector in the parameters.
        The parameters \emph{must} have the good size!
    */
    void copyFrom(real *vec);

    /** Copy the full parameters in a real vector.
        The parameters \emph{must} have the good size!
    */
    void copyTo(real *vec);

    /// Save all the parameters.
    virtual void saveXFile(XFile *file);

    /// Load all the parameters.
    virtual void loadXFile(XFile *file);

    virtual ~Parameters();
};

}

#endif
