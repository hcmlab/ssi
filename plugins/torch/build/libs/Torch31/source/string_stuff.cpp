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

#include "string_stuff.h"
#include "Allocator.h"
#include "string.h"
#include "ctype.h"

namespace Torch {


char *myfgets( FILE *fd )
{
    int step_size=1000 , max_size ;
    char *ptr , *buf ;
    
#ifdef DEBUG
    if ( fd==NULL )
        error("myfgets - invalid input parameter\n") ;
#endif

    // initially allocate 1000 bytes
    max_size = step_size ;
    buf = (char *)Allocator::sysAlloc( max_size * sizeof(char) ) ;
    ptr = buf ;
        
    if ( fgets( buf , step_size , fd ) == NULL )
    {
        free( buf ) ;
        return NULL ;
    }
    
    while ( ((int)strlen( ptr ) >= (step_size-1)) && (ptr[step_size-2] != '\n') )
    {
        // The buffer was not big enough to read the whole line.
        // We now have max_size-1 chars + the NULL char
        // Reallocate and keep reading (overwrite the NULL).
        max_size += (step_size-1) ;
        buf = (char *)Allocator::sysRealloc( buf , max_size*sizeof(char) ) ;
        ptr = buf + max_size - step_size ;
        if ( fgets( ptr , step_size , fd ) == NULL )
            error("myfgets - unexpected EOF\n") ;
    }

    return buf ;
}


void strtoupper( char *str )
{
    if ( str == NULL )
        return ;

    for ( int i=0 ; i<(int)strlen(str) ; i++ )
        str[i] = toupper( str[i] ) ;
}


}
