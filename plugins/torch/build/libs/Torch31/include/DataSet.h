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

#ifndef DATA_SET_INC
#define DATA_SET_INC

#include "Object.h"
#include "Sequence.h"
#include "PreProcessing.h"
#include "Stack.h"

struct FrameSubsets
{
    int **subsets;
    int *subsets_sizes;
    int n_subsets;
    int n_selected_frames;
    int *selected_frames;
};

namespace Torch {

/** Provides an interface to manipulate all kind of data.
    A dataset contains two kind of things: inputs sequences
    and targets sequences.

    @author Ronan Collobert (collober@idiap.ch)
*/
class DataSet : public Object
{
  public:
    //--- internal ---
    int **subsets;
    int *n_examples_subsets;
    int n_subsets;
    Stack *pushed_examples;
    //----------------
    
    // True if a subset of the examples is selected.
    bool select_examples;

    /** The indices of all selected examples.
        When #select_examples# is false, it contains
        the indices of all examples.
    */
    int *selected_examples;

    /// Frame size of #inputs#.    
    int n_inputs;

    /// Frame size of #targets#.    
    int n_targets;

    /** Index of the current example.
        Warning: it's the \emph{real} index and not the index
        in the #selected_examples# table.
    */
    int real_current_example_index;

    /// Pointer on the inputs of the current example.
    Sequence *inputs;

    /// Pointer to the targets of the current example.
    Sequence *targets;

    /** Number of examples in the dataset.
        If you're using #select_examples#, it's
        the number of selected examples.
    */
    int n_examples;
    
    /** Real number of examples in the dataset.
        It's the number of examples in memory.
        (= #n_examples# if #select_examples# is false)
    */
    int n_real_examples;

    //-----

    ///
    DataSet();

    /** Method which initializes some fields of the datasets.
        It has to be called only in the constructor of your subclasses.
        Just for developpers of new datasets.
    */
    void init(int n_examples_, int n_inputs_, int n_targets_);

    /** Set #targets# and #inputs# to the targets and inputs
        of the example with the index #selected_examples[t]#.
        Warning: after a #setExample()# the previous selected example
        is \emph{not} supposed to exist... for that, use #pushExample()#.
    */
    void setExample(int t, bool set_inputs=true, bool set_targets=true);

    /** Set #targets# and #inputs# to the targets and inputs
        of the example with the index #t#. If you create a new
        dataset, you \emph{must} update inside #current_example_index#.
        Warning: after a #setExample()# the previous selected example
        is \emph{not} supposed to exist... for that, use #pushExample()#.
    */
    virtual void setRealExample(int t, bool set_inputs=true, bool set_targets=true) = 0;

    /** Set a new subset.
        \begin{itemize}
          \item #subset_# (of size #n_examples_#) is a set
          of indices which define a subset of #data#.
          \item if a #pushSubset()# has been already called,
          the next #pushSubset()# defines a subset of the
          previous subset, and so on...
          \item this function set #select_examples# to #true#
          and set the read indices of the examples in
          #selected_examples#.
        \end{itemize}
    */
    virtual void pushSubset(int *subset_, int n_examples_);

    /** Remove the last subset.
        \begin{itemize}
          \item recomputes "selected_examples".
          \item if it was the last subset, set #select_examples#
          to #false#.
        \end{itemize}
    */
    virtual void popSubset();

    /** Tells that the current example must be kept in memory
        after next calls of #setExample()#. */
    virtual void pushExample() = 0;

    /** Tells that the last pushed example will be now the current
        example, and therefore, will be forgeted after the next
        call of #setExample()#. */
    virtual void popExample() = 0;

    /** Put in #n_input_frames# and #n_target_frames# the number
        of input frames and target frames for example #t#.
        This take subsets in account.
        If one field is #NULL#, it will not be returned.
    */
    virtual void getNumberOfFrames(int t, int *n_input_frames, int *n_target_frames) = 0;

    /// Perform some pre-processing on data.
    virtual void preProcess(PreProcessing *pre_processing) = 0;

    //-----

    virtual ~DataSet();
};

}

#endif
