// Copyright (C) 2003--2004 Darren Moore (moore@idiap.ch)
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

#ifndef WORDCHAINELEMPOOL_INC
#define WORDCHAINELEMPOOL_INC

#include "general.h"

namespace Torch {


struct WCEStruct
{
    int word ;
    int word_start_frame ;
    struct WCEStruct *prev_elem ;
    int n_connected ;
} ;
typedef struct WCEStruct WordChainElem ;

/** 
    This class is a pool of pre-allocated WordChainElem structures. It's
    purpose is to enable fast access to the WordChainElem instances during
    decoding. If the pool is empty, and something asks for a WordChainElem 
    instance then a block of new instances are allocated before the requested
    instance is returned.
    
    @author Darren Moore (moore@idiap.ch)
*/


class WordChainElemPool
{
public:
    int n_total ;
    int n_free ;
    int n_used ;
    int realloc_amount ;
    int n_allocs ;
    WordChainElem **allocs ;
    WordChainElem **free_elems ;

    /* Constructors / destructor */
    
    /// Creates the pool. 
    /// 'max_size' is the initial size of the pool.
    /// 'realloc_amount_' is the number of additional WordChainElem instances 
    ///   that are to be allocated each time the pool empties.
    /// If 'realloc_amount_' is undefined, then 'max_size' elements are
    ///   allocated each time the pool empties.
    WordChainElemPool( int max_size , int realloc_amount_=-1 ) ;
    virtual ~WordChainElemPool() ;

    /* Methods */

    /// Gets the next free WordChainElem instance from the pool. Allocates
    ///   new instances if the pool is empty.
    WordChainElem *getElem( int word_ , WordChainElem *prev_elem_ , int word_start_frame_=-1) ;

    /// Returns a WordChainElem instance that is no longer required to the pool.
    void returnElem( WordChainElem *elem ) ;
} ;


}


#endif
