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

#ifndef EDIT_DISTANCE_INC
#define EDIT_DISTANCE_INC

#include "general.h"
#include "Object.h"

namespace Torch {

/** This class can be used to compute the "edit distance" between
    two sequences. It computes the number of insertions, deletions and
    substitutions. The overall distance is the sum of these numbers
    weighted by their cost (which are intergers equal to 1 by default).

    @author Samy Bengio (bengio@idiap.ch)
*/
class EditDistance : public Object
{
  public:
    /// the total edit distance between two sequences
    real accuracy;
    /// the number of insertions (weighted by their cost)
    int n_insert;
    /// the number of deletions (weighted by their cost)
    int n_delete;
    /// the number of substitutions (weighted by their cost)
    int n_subst;
    /// the number of sequences measured (used to normalize #dist# by #n_seq#)
    int n_seq;
    /// the cost of one insertion
    int insert_cost;
    /// the cost of one deletion
    int delete_cost;
    /// the cost of one substitution
    int subst_cost;

    /// the obtained sequence
    int* obtained;
    /// the size of the obtained sequence
    int obt_size;

    /// the desired sequence
    int* desired;
    /// the size of the desired sequence
    int des_size;

    /// the confusion matrix itself
    int** confusion;
    /// the number of symbols
    int n_confusion;
    /// insertions and deletions
    int* conf_ins;
    int* conf_del;
    /// do we want a confusion matrix?
    bool is_confusion;

    ///
    EditDistance(bool is_confusion_=false);

    /// sets the different costs
    virtual void setCosts(int i_cost, int d_cost, int s_cost);

    /// computes the edit distance between #obtained# and #desired#.
    virtual void distance(int* obtained, int obt_size, int* desired, int des_size);

    /// accumulates the distances of the current object and the given object
    virtual void add(EditDistance* d);

    /// prints the edit distance and optionally the sequences
    virtual void print(XFile *f);

    /** prints the edit distance ratio (divided by the number of sequnces) 
        and optionally the obtained and desired sequences
    */
    virtual void printRatio(XFile *f);

    virtual void reset();
    ~EditDistance();
};


}

#endif
