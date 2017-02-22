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

#ifndef PHONEINFO_INC
#define PHONEINFO_INC

#include "general.h"
#include "Object.h"
#include "DiskXFile.h"

namespace Torch {


/** 
    This class contains the names of the phonemes that make up
    the words in the lexicon. An empty instance can be created,
    and then phones added manually using the addPhone method,
    or a filename can be specified containing phone information.
    The list of phone names is then extracted automatically
    from the file. 
    
    @author Darren Moore (moore@idiap.ch)
*/

class PhoneInfo : public Object
{
public:
    int n_phones ;
    char **phone_names ;
    int sil_index ;
    int pause_index ;

    /// Creates an empty PhoneInfo instance
    PhoneInfo() ;

    /// Reads the phone information from 'phones_fname' file. 
    /// Looks at the first line of the file. 
    /// If it is "PHONE" then the file is assumed to be in Noway phone models format. 
    /// If it is "~o" then the file is assumed to be in HTK model definition format.
    /// Otherwise the file is assumed to contain just a straight list of phone names.
    /// If the sil_name and pause_name params are set, then the indices of the
    ///   silence and pause phonemes in the list are set.
    PhoneInfo( const char *phones_fname , const char *sil_name=NULL , const char *pause_name=NULL ) ;
    
    virtual ~PhoneInfo() ;
    
    /// Adds a phoneme to the end of the list of phonemes.
    /// is_sil and is_pause indicate if the phoneme is one of the special phonemes.
    /// A phoneme can be both the silence phoneme and the pause phoneme.
    void addPhone( char *phone_name , bool is_sil=false , bool is_pause=false ) ;

    /// Returns a pointer to the phoneme name at position "index" in the list.
    char *getPhone( int index ) ;

    /// Does a linear search through the list and returns the index of the
    ///   phoneme name supplied as a parameter.
    /// If the phoneme was not in the list returns -1.
    int getIndex( char *phone_name ) ;

    // Internal functions.
    void readPhonesFromNoway( DiskXFile *phones_fd , const char *sil_name=NULL , const char *pause_name=NULL ) ;
    void readPhonesFromHTK( DiskXFile *phones_fd , const char *sil_name=NULL , const char *pause_name=NULL ) ;

#ifdef DEBUG
    void outputText() ;
#endif
} ;


}

#endif
