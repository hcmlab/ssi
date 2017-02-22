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

#ifndef SEQUENCE_INC
#define SEQUENCE_INC

#include "Object.h"
#include "List.h"

namespace Torch {

/** Sequence definition.
    A sequence is a set of frames (that is, a vector) which have the same size.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Sequence : public Object
{
  private:
    /* Resize the array of frames to #n_frames_#.
       If the array hasn't be allocated by the class, allocate a new one
       and copy previous frame pointers. Else, just do a realloc.
       Note that #n_frames_# must be the new #n_real_frames#...
    */
    void reallocFramesArray(int n_frames_);

  public:
    /// Real number of frames
    int n_real_frames;

    /** Array of usable frame pointers.
        The actual size of this array is given by #n_real_frames#.
        And the usable size is given by #n_frames#. */
    real **frames;

    /// Number of visible frames
    int n_frames;

    /// Frame size
    int frame_size;

    /// Create an empty sequence
    Sequence();

    /** Create a sequences of #n_frames_# frames with size #frame_size_#.
        The frames are given by the #frames_# array.
        Nothing (except pointers!) will be copied.
    */
    Sequence(real **frames_, int n_frames_, int frame_size_);

    /** Create a sequence with #n_frames_# \emph{standard} frames
        of size #frame_size#.
    */
    Sequence(int n_frames_, int frame_size_);

    /** Resize the sequence to #n_frames_#. Note that if #n_frames_# is lower
        than the previous one, the frames won't be deallocated, and can be retrieved
        by another resize...
    */
    void resize(int n_frames_, bool allocate_new_frames=true);

    /** Add a frame at the end of the Sequence.
        If #do_copy# is true, copy the sequence.
        Else, just copy the pointer.
    */
    void addFrame(real *frame, bool do_copy=false);
    
    /** Add a sequence at the end of the Sequence.
        If #do_copy# is true, copy the frame contents.
        Else, just copy the frame pointers.
     */
    void add(Sequence *sequence, bool do_copy=false);

    /** Copy the given sequence.
        The given sequence don't need to have the same structure.
        But it must have the same total length.
    */
    void copy(Sequence *from);

    /** Copy a real vector in the full sequence.
        The sequence \emph{must} have the good size!
    */
    void copyFrom(real *vec);

    /** Copy the full sequence in a real vector.
        The sequence \emph{must} have the good size!
    */
    void copyTo(real *vec);

    /// Get the space needed to allocate one Sequence
    virtual int getSequenceSpace();

    /// Get the space needed to allocate frames contained in the sequence.
    virtual int getFramesSpace();

    /** Clone the sequence.
        If #allocator_# is non-null, call it for all memory allocation and object initialization.
        Else, the returned sequence will be destroyed when the original sequence will be destroyed.
        If #sequence_memory# is non-null, puts the class memory-space in it; it must contain
        the space given by #getSequenceSpace()#.
        If #frames_memory# is non-null, use the given memory for frames allocation; it must contain
        the space given by #getFramesSpace()#.
    */
    virtual Sequence *clone(Allocator *allocator_=NULL, void *sequence_memory=NULL, void *frames_memory=NULL);

    /// Save the \emph{usable} frames. (#n_frames# available in #frames#).
    virtual void saveXFile(XFile *file);

    /// Load the \emph{usable} frames. (#n_frames# available in #frames#).
    virtual void loadXFile(XFile *file);

    virtual ~Sequence();
};

DEFINE_NEW_LIST(SequenceList, Sequence);

}

#endif
