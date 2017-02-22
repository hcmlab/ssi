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

#include "Allocator.h"
#include "WordChainElemPool.h"

namespace Torch {


WordChainElemPool::WordChainElemPool( int max_size , int realloc_amount_ )
{
    n_allocs = 0 ;
    allocs = (WordChainElem **)Allocator::sysAlloc( (n_allocs+1) * sizeof(WordChainElem *) ) ;
    allocs[n_allocs] = (WordChainElem *)Allocator::sysAlloc( max_size * sizeof(WordChainElem) ) ;
    free_elems = (WordChainElem **)Allocator::sysAlloc( max_size * sizeof(WordChainElem *) ) ;
    for ( int i=0 ; i<max_size ; i++ )
        free_elems[i] = allocs[n_allocs] + i ;
    n_total = max_size ;
    n_free = n_total ;
    n_used = 0 ;
    n_allocs++ ;

    if ( realloc_amount_ <= 0 )
        realloc_amount = max_size ;
    else
        realloc_amount = realloc_amount_ ;
}


WordChainElemPool::~WordChainElemPool()
{
    if ( allocs != NULL )
    {
        for ( int i=0 ; i<n_allocs ; i++ )
            free( allocs[i] ) ;
        free( allocs ) ;
    }

    if ( free_elems != NULL )
        free( free_elems ) ;
}


WordChainElem *WordChainElemPool::getElem( int word_ , WordChainElem *prev_elem_ , 
                                           int word_start_frame_ )
{
    WordChainElem *temp ;
    if ( n_free == 0 )
    {
        free_elems = (WordChainElem **)Allocator::sysRealloc( free_elems , 
                                          (n_total+realloc_amount) * sizeof(WordChainElem *) ) ;
        allocs = (WordChainElem **)Allocator::sysRealloc( allocs , 
                                          (n_allocs+1) * sizeof(WordChainElem *) ) ;
        allocs[n_allocs] = (WordChainElem *)Allocator::sysAlloc( 
                                          realloc_amount * sizeof(WordChainElem) ) ;
        for ( int i=0 ; i<realloc_amount ; i++ )
            free_elems[i] = allocs[n_allocs] + i ;
        n_total += realloc_amount ;
        n_free += realloc_amount ;
        n_allocs++ ;
    }

    n_free-- ;
    n_used++ ;

    temp = free_elems[n_free] ;
    temp->word = word_ ;
    temp->word_start_frame = word_start_frame_ ;
    temp->prev_elem = prev_elem_ ;
    if ( temp->prev_elem != NULL )
        temp->prev_elem->n_connected++ ;
    temp->n_connected = 0 ;
    return temp ;
}


void WordChainElemPool::returnElem( WordChainElem *elem )
{
    // unlink the returned element from the previous element in the chain
    if ( elem->prev_elem != NULL )
    {
        if ( --(elem->prev_elem->n_connected) <= 0 )
            returnElem( elem->prev_elem ) ;
        elem->prev_elem = NULL ;
    }

    free_elems[n_free] = elem ;
    n_free++ ;
    n_used-- ;
}


}
