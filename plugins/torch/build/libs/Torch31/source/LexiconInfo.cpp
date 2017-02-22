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

#include "LexiconInfo.h"
#include "Allocator.h"

namespace Torch {


LexiconInfo::LexiconInfo( const char *phones_fname , const char *sil_phone , const char *pause_phone ,
                          const char *lex_fname , const char *sent_start_word , const char *sent_end_word ,
                          const char *sil_word )
{
    char line[1000] , ch , curr_word[1000] , *curr_phone ;
    DiskXFile lex_fd( lex_fname , "r" ) ;
    real prior=1.0 ;
    int n_flds , index , vocab_index=0 , temp_phones[100] , temp , ind , total_n_entries ;
    LexiconInfoEntry *curr_entry ;

    n_entries = 0 ;
    entries = NULL ;
    sent_start_index = -1 ;
    sent_end_index = -1 ;
    sil_index = -1 ;
    phone_info = NULL ;
    vocabulary = NULL ;
    vocab_to_lex_map = NULL ;

    // Create the PhoneInfo and Vocabulary objects
    phone_info = new(allocator) PhoneInfo( phones_fname , sil_phone , pause_phone ) ; 
    vocabulary = new(allocator) Vocabulary( lex_fname , sent_start_word , 
                                            sent_end_word , sil_word ) ;

    // Allocate memory for mappings between vocab entries and dictionary entries.
    vocab_to_lex_map = (VocabToLexMapEntry *)allocator->alloc( vocabulary->n_words *
                                                               sizeof(VocabToLexMapEntry) ) ;
    for ( int i=0 ; i<vocabulary->n_words ; i++ )
    {
        vocab_to_lex_map[i].n_pronuns = 0 ;
        vocab_to_lex_map[i].pronuns = NULL ;
    }

    // Do a first pass off the file to determine the total number of pronuns.
    total_n_entries = 0 ;
    while ( lex_fd.gets( line , 1000 ) != NULL )
    {
        if ( (line[0] == '#') || (strtok( line , "(\r\n\t " ) == NULL) )
            continue ;

        total_n_entries++ ;
    }
    
    // Allocate some memory
    entries = (LexiconInfoEntry *)allocator->alloc( total_n_entries * sizeof(LexiconInfoEntry) ) ;
    lex_fd.seek( 0 , SEEK_SET ) ;
   
    // Now re-read the file and fill in the entries.
    n_entries = 0 ;
    while ( lex_fd.gets( line , 1000 ) != NULL )
    {
#ifdef USE_DOUBLE
        if ( (line[0] == '#') || ((n_flds = sscanf( line , "%[^( \t]%c%lf" , 
                                                    curr_word , &ch , &prior)) == 0) )
#else
        if ( (line[0] == '#') || ((n_flds = sscanf( line , "%[^( \t]%c%f" , 
                                                    curr_word , &ch , &prior)) == 0) )
#endif
        {
            continue ;
        }

        if ( n_flds < 3 )
            prior = 1.0 ;
            
        if ( n_entries >= total_n_entries )
            error("LexiconInfo::LexiconInfo - n_entries exceeded expected\n") ;

        // Find the vocab index of the new word
        vocab_index = vocabulary->getIndex( curr_word , vocab_index ) ;
        if ( vocab_index < 0 )
            error("LexiconInfo::LexiconInfo - word %s not found in vocabulary\n",curr_word) ;
        
        // Allocate memory for the new lexicon entry.
        curr_entry = entries + n_entries ;
        initLexInfoEntry( curr_entry ) ;
        
        curr_entry->vocab_index = vocab_index ;
        curr_entry->log_prior = log( prior ) ;
        
        // read in the phones for the new pronunciation
        strtok( line , " \r\n\t" ) ;    // get past the word
        while ( (curr_phone=strtok(NULL," \r\n\t")) != NULL )
        {
            // find the index of the phone's model
            index = phone_info->getIndex( curr_phone ) ;

            // Add it to the list of models we are compiling or report error
            //   if the phone name was not found.
            if ( (index < 0) || (index >= phone_info->n_phones) )
                error("LexiconInfo::LexiconInfo - %s not found in phone list\n" , curr_phone) ;
            else
                temp_phones[(curr_entry->n_phones)++] = index ;
        }

        curr_entry->phones = (int *)allocator->alloc( curr_entry->n_phones*sizeof(int) ) ;
        memcpy( curr_entry->phones , temp_phones , curr_entry->n_phones*sizeof(int) ) ;
        
        if ( curr_entry->n_phones == 0 )
            error("LexiconInfo::LexiconInfo - %s had no phones\n",curr_word) ;
        
        // Update the appropriate vocab_to_lex_map entry
        temp = ++(vocab_to_lex_map[vocab_index].n_pronuns) ;
        vocab_to_lex_map[vocab_index].pronuns = (int *)allocator->realloc( 
                                 vocab_to_lex_map[vocab_index].pronuns , temp * sizeof(int) ) ;
        vocab_to_lex_map[vocab_index].pronuns[temp-1] = n_entries ;
        
        // Check if these are 'special' words
        if ( vocab_index == vocabulary->sent_start_index )
        {
            if ( sent_start_index >= 0 )
                error("LexiconInfo::LexiconInfo - cannot have >1 pronuns of the start word\n") ;
            sent_start_index = n_entries ;
        }
        if ( vocab_index == vocabulary->sent_end_index )
        {
            if ( sent_end_index >= 0 )
                error("LexiconInfo::LexiconInfo - cannot have >1 pronuns of the end word\n") ;
            sent_end_index = n_entries ;
        }
        if ( vocab_index == vocabulary->sil_index )
        {
            if ( sil_index >= 0 )
                error("LexiconInfo::LexiconInfo - cannot have >1 pronuns of the sil word\n") ;
            sil_index = n_entries ;
        }
        
        n_entries++ ;
    }

    if ( n_entries != total_n_entries )
        error("LexiconInfo::LexiconInfo - unexpected n_entries\n") ;
        
    if ( (sent_end_index >= 0) && (sent_start_index == sent_end_index) )
    {
        // Create a separate, identical entry for the sent_end_index
        //   so that there will be a separate model for the sentence end word.
        entries = (LexiconInfoEntry *)allocator->realloc( entries , 
                                          (n_entries+1)*sizeof(LexiconInfoEntry) ) ;
        curr_entry = entries + n_entries ;
        initLexInfoEntry( curr_entry ) ;
        curr_entry->vocab_index = entries[sent_start_index].vocab_index ;
        curr_entry->log_prior = entries[sent_start_index].log_prior ;
        curr_entry->n_phones = entries[sent_start_index].n_phones ;
        curr_entry->phones = (int *)allocator->alloc( curr_entry->n_phones*sizeof(int) ) ;
        memcpy( curr_entry->phones , entries[sent_start_index].phones ,
                curr_entry->n_phones*sizeof(int) ) ;

        // Update the appropriate vocab_to_lex_map entry
        temp = ++(vocab_to_lex_map[curr_entry->vocab_index].n_pronuns) ;
        vocab_to_lex_map[curr_entry->vocab_index].pronuns = (int *)allocator->realloc( 
                  vocab_to_lex_map[curr_entry->vocab_index].pronuns , temp*sizeof(int) ) ;
        vocab_to_lex_map[curr_entry->vocab_index].pronuns[temp-1] = n_entries ;
        
        sent_end_index = n_entries++ ;
    }
    
    if ( (sil_index >= 0) && ((sil_index==sent_start_index) || (sil_index==sent_end_index)) )
    {
        if ( sil_index == sent_end_index )
            ind = sent_end_index ;
        else
            ind = sent_start_index ;
            
        // Create a separate, identical entry for the sil_index
        //   so that there will be a separate model for the silence word.
        entries = (LexiconInfoEntry *)allocator->realloc( entries , 
                                          (n_entries+1)*sizeof(LexiconInfoEntry) ) ;
        curr_entry = entries + n_entries ;
        initLexInfoEntry( curr_entry ) ;
        curr_entry->vocab_index = entries[ind].vocab_index ;
        curr_entry->log_prior = entries[ind].log_prior ;
        curr_entry->n_phones = entries[ind].n_phones ;
        curr_entry->phones = (int *)allocator->alloc( curr_entry->n_phones*sizeof(int) ) ;
        memcpy( curr_entry->phones , entries[ind].phones ,
                curr_entry->n_phones*sizeof(int) ) ;

        // Update the appropriate vocab_to_lex_map entry
        temp = ++(vocab_to_lex_map[curr_entry->vocab_index].n_pronuns) ;
        vocab_to_lex_map[curr_entry->vocab_index].pronuns = (int *)allocator->realloc( 
                  vocab_to_lex_map[curr_entry->vocab_index].pronuns , temp*sizeof(int) ) ;
        vocab_to_lex_map[curr_entry->vocab_index].pronuns[temp-1] = n_entries ;
        
        sil_index = n_entries++ ;
    }
}
   

LexiconInfo::~LexiconInfo()
{
}


void LexiconInfo::initLexInfoEntry( LexiconInfoEntry *entry )
{
    entry->n_phones = 0 ;
    entry->phones = NULL ;
    entry->log_prior = 0.0 ;
    entry->vocab_index = -1 ;
}


#ifdef DEBUG
void LexiconInfo::outputText()
{
    printf("n_entries=%d  start_ind=%d  end_ind=%d  sil_ind=%d\n",n_entries,sent_start_index,
           sent_end_index,sil_index) ;
    for ( int i=0 ; i<n_entries ; i++ )
    {
        printf("%-16s(%.3f)",vocabulary->getWord( entries[i].vocab_index ),entries[i].log_prior) ;
        for ( int j=0 ; j<entries[i].n_phones ; j++ )
            printf(" %s" , phone_info->getPhone( entries[i].phones[j] )) ;
        printf("\n") ;
    }
}
#endif


}
