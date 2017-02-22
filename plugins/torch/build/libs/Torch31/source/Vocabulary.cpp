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

#include "Vocabulary.h"
#include "ctype.h"
#include "log_add.h"
#include "DiskXFile.h"


namespace Torch {


Vocabulary::Vocabulary( const char *lex_fname , const char *sent_start_word , const char *sent_end_word , 
                        const char *sil_word )
{
    DiskXFile lex_fd( lex_fname , "r" ) ;
    char line1[1000] , line2[1000] , *line , *prev_line ;
    int total_n_words ;
    
    if ( (lex_fname == NULL) || (strcmp(lex_fname,"")==0) )
        error("Vocabulary::Vocabulary - lexicon filename undefined\n") ;

    n_words = 0 ;
    words = NULL ;
    sent_start_index = -1 ;
    sent_end_index = -1 ;
    sil_index = -1 ;
    
    // Do a first-pass of the file to determine memory requirements.
    // We assume that multiple pronunciations will be grouped together.
    line = line1 ;
    prev_line = line2 ;
    prev_line[0] = '\0' ;
    total_n_words = 0 ;
    while ( lex_fd.gets( line , 1000 ) != NULL )
    {
        if ( (line[0] == '#') || (strtok( line , "(\r\n\t " ) == NULL) )
            continue ;

        // Is this a new word ?
        if ( strcmp( line , prev_line ) != 0 )
            total_n_words++ ;
        
        if ( line == line1 )
        {
            line = line2 ; 
            prev_line = line1 ;
        }
        else
        {
            line = line1 ;
            prev_line = line2 ;
        }
    }

    // Allocate the 'words' array
    words = (char **)allocator->alloc( total_n_words * sizeof(char *) ) ;

    // Return to the start of the file.
    lex_fd.seek( 0 , SEEK_SET ) ;
    
    // Add words to the vocabulary.
    // Do not add duplicates.
    // Maintain alphabetical order.
    while ( lex_fd.gets( line , 1000 ) != NULL )
    {
        if ( (line[0] == '#') || (strtok( line , "(\r\n\t " ) == NULL) )
            continue ;

        // add it to the vocabulary
        addWord( line ) ;
    }
    
    if ( n_words > total_n_words )
        error("Vocabulary::Vocabulary - n_words exceeds expected.\n") ;

    sent_start_index = -1 ;
    if ( (sent_start_word != NULL) && (strcmp(sent_start_word,"") != 0) )
    {
        for ( int i=0 ; i<n_words ; i++ )
        {
            if ( strcmp( words[i] , sent_start_word ) == 0 )
            {
                if ( sent_start_index >= 0 )
                    error("Vocabulary::Vocabulary - duplicate start words\n") ;
                sent_start_index = i ;
            }
        }
    }
    sent_end_index = -1 ;
    if ( (sent_end_word != NULL) && (strcmp(sent_end_word,"") != 0) )
    {
        for ( int i=0 ; i<n_words ; i++ )
        {
            if ( strcmp( words[i] , sent_end_word ) == 0 )
            {
                if ( sent_end_index >= 0 )
                    error("Vocabulary::Vocabulary - duplicate end words\n") ;
                sent_end_index = i ;
            }
        }
    }
    sil_index = -1 ;
    if ( (sil_word != NULL) && (strcmp(sil_word,"") != 0) )
    {
        for ( int i=0 ; i<n_words ; i++ )
        {
            if ( strcmp( words[i] , sil_word ) == 0 )
            {
                if ( sil_index >= 0 )
                    error("Vocabulary::Vocabulary - duplicate end words\n") ;
                sil_index = i ;
            }
        }
    }

    if ( n_words != total_n_words )
        error("Vocabulary::Vocabulary - did not get expected n_words\n") ;
}


Vocabulary::~Vocabulary()
{
}


void Vocabulary::addWord( char *word )
{
    int cmp_result=0 ;

	if ( word[0] == '#' )
	{
		// The string is a comment so don't add to vocabulary
		return ;
	}
   
    if ( n_words > 0 )
        cmp_result = strcmp( words[n_words-1] , word ) ;
        
    if ( (cmp_result < 0) || (n_words == 0) )
    {
        // The new word belongs at the end of the list
        // Allocate memory in the list of words for the new word   
        words[n_words] = (char *)allocator->alloc( (strlen(word)+1) * sizeof(char) ) ;
        strcpy( words[n_words] , word ) ;
        n_words++ ;
        return ;
    }
    else if ( cmp_result > 0 )
    {
        // Find the place in the list of words where we want to insert the new word
        for ( int i=0 ; i<n_words ; i++ )
        {
            cmp_result = strcmp( words[i] , word ) ;
            if ( cmp_result > 0 )
            {
                // Shuffle down all words from i onwards and place the
                //   new word in position i.

                // Allocate memory in the list of words for the new word   
                for ( int j=n_words ; j>i ; j-- )
                    words[j] = words[j-1] ;
                words[i] = (char *)allocator->alloc( (strlen(word)+1) * sizeof(char) ) ;
                strcpy( words[i] , word ) ;
                n_words++ ;
                return ;
            }
            else if ( cmp_result == 0 )
            {
                // the word is already in our vocab - don't duplicate
                return ;
            }
        }
    }
    else
    {
        // The word is already at the end of the list - don't duplicate
        return ;
    }

    // If we make it here there is a problem
    return ;
}


char *Vocabulary::getWord( int index )
{
    if ( (index<0) || (index>=n_words) )
        error("Vocabulary::getWord - index out of range\n") ;
	else
		return words[index] ;
    return NULL ;
}


int Vocabulary::getIndex( char *word , int guess )
{
    // We assume that the list of words is in ascending order so 
    //   that we can do a binary search.
    int min=0 , max=(n_words-1) , curr_pos=0 ;
    int cmp_result=0 ;
   
    // If guess is valid, do a quick check to see if the word is where
    //   the caller expects it to be - either at guess or at guess+1
    if ( (guess >= 0) && (guess<n_words) )
    {
        if ( strcmp(word,words[guess]) == 0 ) 
            return guess ;
        else if ( ((guess+1)<n_words) && (strcmp(word,words[guess+1])==0) )
            return guess+1 ;
    }
        
    while (1)
    {
        curr_pos = min+(max-min)/2 ;
        cmp_result = strcmp( word , words[curr_pos] ) ;
        if ( cmp_result < 0 )
            max = curr_pos-1 ;
        else if ( cmp_result > 0 )
            min = curr_pos+1 ;
        else
            return curr_pos ;
            
        if ( min > max )
            return -1 ;
    }

    return -1 ;
}


#ifdef DEBUG
void Vocabulary::outputText()
{
    printf("** START VOCABULARY - n_words=%d start_index=%d end_index=%d sil_index=%d**\n" , 
                                     n_words , sent_start_index , sent_end_index , sil_index ) ;
    for ( int i=0 ; i<n_words ; i++ )
        printf("%s\n",words[i]) ;
    printf("** END VOCABULARY **\n") ;
}
#endif


}
