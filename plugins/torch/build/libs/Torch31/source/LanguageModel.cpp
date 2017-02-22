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
#include "LanguageModel.h"
#include "string_stuff.h"
#include "DiskXFile.h"


namespace Torch {


LanguageModel::LanguageModel( int order_ , Vocabulary *vocabulary_ , 
                              char *lm_fname , real lm_scaling_factor_ )
{
    FILE *lm_fd ;
    char buf[4] ;
    
    if ( vocabulary_ == NULL )
        error("LanguageModel::LanguageModel - no vocabulary defined\n") ;
    if ( order_ <= 0 )
        error("LanguageModel::LanguageModel - LM order must be > 0\n") ;
    if ( (lm_fname == NULL) || (strcmp(lm_fname,"")==0) )
        error("LanguageModel::LanguageModel - no LM filename specified\n") ;
        
    order = order_ ;
    vocabulary = vocabulary_ ;
    n_words = vocabulary->n_words ;
    lm_scaling_factor = lm_scaling_factor_ ;
    ngram = new LMNGram( order , vocabulary ) ;

    lm_has_start_word = false ;
    lm_has_end_word = false ;

    // Open the LM file
    if ( (lm_fd = fopen( lm_fname , "r" )) == NULL )
        error("LanguageModel::LanguageModel - error opening LM file\n") ;

    // Read the first 4 bytes to see if the file is a Noway binary file
    if ( (int)fread( buf , sizeof(char) , 4 , lm_fd ) != 4 )
        error("LanguageModel::LanguageModel - error reading first 4 bytes\n") ;
    
    if ( (strcmp( buf , "NG3" ) == 0) || (strcmp( buf , "TR2" ) == 0) )
    {
        // The file is in Noway binary format.
        readNowayBin( lm_fd ) ;
    }
    else
    {
        // Assume that the file is in ARPA format.
        fseek( lm_fd , 0 , SEEK_SET ) ;
        readARPA( lm_fd ) ;
    }

    fclose( lm_fd ) ;
 
#ifdef DEBUG
    //outputText() ;
#endif
}


LanguageModel::~LanguageModel()
{
    if ( ngram != NULL )
        delete ngram ;
}


real LanguageModel::calcLMProb( DecodingHypothesis *prev_word_end_hyp , int next_word )
{
    int words[30] , n_wrds ;
    WordChainElem *temp_elem ;
    real prob ;

#ifdef DEBUG
    if ( next_word < 0 )
        error("LanguageModel::calcLMProb(2) - next_word < 0\n") ;
#endif

    // We have a word end hypothesis and a next word.
    // We want to calculate the LM probability for this next word.
    // eg. We have a word end for some w2 and a next word w3.
    //     We want to calculate P(w3|w1,w2)
    
    // If the next word is silence or a sentence marker, don't do a LM lookup.
    if ( (next_word == vocabulary->sent_start_index) && (lm_has_start_word == false) )
        return 0.0 ;
    if ( (next_word == vocabulary->sent_end_index) && (lm_has_end_word == false) )
        return 0.0 ;
    if ( next_word == vocabulary->sil_index )
        return 0.0 ;
    
    // Construct a list of the previous words.
    n_wrds = 0 ;
    if ( prev_word_end_hyp != NULL )
    {    
        temp_elem = prev_word_end_hyp->word_level_info ;
#ifdef DEBUG
        if ( prev_word_end_hyp->word_level_info == NULL )
            error("LanguageModel::calcLMProb(2) - word_level_info is NULL\n") ;
#endif

        while ( temp_elem != NULL )
        {
            if ( (temp_elem->word == vocabulary->sil_index) ||
                 ((lm_has_start_word==false) && (temp_elem->word==vocabulary->sent_start_index)) )
            {
                // skip these words for the purpose of LM lookups
                temp_elem = temp_elem->prev_elem ;
                continue ;
            }

            words[n_wrds++] = temp_elem->word ;
            temp_elem = temp_elem->prev_elem ;
            if ( n_wrds >= (order-1) )
                break ;
        }
    }
    
    words[n_wrds++] = next_word ;
    
    // Find the n-gram probability
    prob = ngram->getLogProbBackoff( n_wrds , words ) ;

    // Scale the n-gram probability using the LM scaling factor.
    // Note that we are multiplying the log LM prob by the scaling factor
    prob *= lm_scaling_factor ;

    return ( prob ) ;
}


real LanguageModel::calcLMProb( DecodingHypothesis *word_end_hyp )
{
    int words[30] , n_wrds , temp , j ;
    WordChainElem *temp_elem ;
    real prob ;
    
    if ( word_end_hyp == NULL )
        return LOG_ZERO ;
        
    // We have a word end hypothesis.  We want to tune this using our language model.
    // eg. we have a word end for some w3 and we want to tune this using P(w3|w1,w2)
        
    // Construct a list of the current word and previous words.
    n_wrds = 0 ;
    temp_elem = word_end_hyp->word_level_info ;
#ifdef DEBUG
    if ( word_end_hyp->word_level_info == NULL )
        error("LanguageModel::calcLMProb - word_level_info is NULL\n") ;
#endif
    // If the most recent word is a sentence marker and the LM does not
    //   have entries for the sentence markers, don't do a LM lookup.
    if ( (temp_elem->word == vocabulary->sent_start_index) && (lm_has_start_word == false) )
        return 0.0 ;
    if ( (temp_elem->word == vocabulary->sent_end_index) && (lm_has_end_word == false) )
        return 0.0 ;
    if ( temp_elem->word == vocabulary->sil_index )
        return 0.0 ;

    while ( temp_elem != NULL )
    {
        if ( (temp_elem->word == vocabulary->sil_index) ||
            ((lm_has_start_word==false) && (temp_elem->word==vocabulary->sent_start_index)) )
        {
            // skip these words for the purpose of LM lookups
            temp_elem = temp_elem->prev_elem ;
            continue ;
        }
        words[n_wrds++] = temp_elem->word ;
        temp_elem = temp_elem->prev_elem ;
        if ( n_wrds >= order )
            break ;
    }

    // The method in LMNGram requires a different ordering of words
    temp = words[0] ;
    for ( j=1 ; j<n_wrds ; j++ )
        words[j-1] = words[j] ;
    words[n_wrds-1] = temp ;
        
    // Find the n-gram probability
    prob = ngram->getLogProbBackoff( n_wrds , words ) ;

    // Scale the n-gram probability using the LM scaling factor.
    // Note that we are multiplying the log LM prob by the scaling factor
    //   IS THIS OK ?
    prob *= lm_scaling_factor ;

    // Tune the word end hypothesis score using the n-gram probability and return
    return ( prob ) ;
}


void LanguageModel::readARPA( FILE *arpa_fd )
{
    int n_exp_entries[30] ; // n_exp_entries[0] is the expected number of unigram entries
                            // n_exp_entries[0] is the expected number of bigram entries
    int n_act_entries[30] ; // the actual number of entries read from the file.
    int words[30] ;    // holds the predecessor words for a given word.
    real curr_prob=0.0 , curr_bow=0.0 ;
    char *curr_word=NULL ;
    int curr_index ;
    real ln_10 = (real)log(10.0) ;
    int tempn=0 , tempn_entries=0 , max_n_in_file=0 ;
    char line[1000] ;
    bool got_begin_data_mark=false , expecting_end=false , got_end=false , error_flag ;
    int curr_gram_data=0 ;
    
    if ( arpa_fd == NULL )
        error("LanguageModel::readARPA - arpa_fd is NULL\n") ;
        
    // discard lines until we get the "beginning of data mark".
    while ( fgets( line , 1000 , arpa_fd ) != NULL )
    {
        // if the new line is empty, get the next line
        if ( (line[0]==' ') || (line[0]=='\r') || (line[0]=='\n') || (line[0]=='\t') ||
             (line[0]=='#') )
            continue ;
            
        if ( line[0] == '\\' )
        {
            strtoupper( line ) ;
            if ( strstr( line , "\\END\\" ) != NULL )
            {
                if ( curr_gram_data < order )
                {
                    // we haven't encountered the n-grams we expected
                    error("LanguageModel::readARPA - not enough data in file\n") ;
                }
            
                // we've reached the end of the ARPA file - we're done
                got_end = true ;
                break ;
            }
            else if ( expecting_end == true )
            {
                // we're expecting the end marker and didn't get it - get the next line
                continue ;
            }
            else if ( strstr( line , "\\DATA\\" ) != NULL )
            {
                if ( got_begin_data_mark == true )
                {
                    // we have already seen the beginning of data marker - error !
                    error("LanguageModel::readARPA - duplicate beginning of data marker\n") ;
                }
                got_begin_data_mark = true ;
            }
            else if ( strstr( line , "-GRAMS:" ) != NULL )
            {
                if ( got_begin_data_mark == true )
                {
                    // find out whether we are at the start of the 1-gram, 
                    //   2-gram, 3-gram, etc data.
                    if ( (curr_gram_data+1) != ( line[1]-0x30 ) )
                        error("LanguageModel::readARPA - N-Gram N out of order\n") ;

                    curr_gram_data = line[1]-0x30 ;
                    n_act_entries[curr_gram_data-1] = 0 ;
                    if ( curr_gram_data > order )
                    {
                        // the order has exceeded the order of our LM - we're done 
                        //   reading probabilties.
                        expecting_end = true ;
                    }
                }
            }
            else
            {
                // we got something else that started with a '\' - error !!
                error("LanguageModel::readARPA - unrecognised marker\n%s\n",line) ;
            }
        }
        else
        {
            if ( (got_begin_data_mark == false) || (expecting_end == true) )
                continue ;

            if ( strstr( line , "<UNK>") != NULL )
            {
                n_exp_entries[curr_gram_data-1]-- ;
                continue ;
            }

            if ( curr_gram_data == 0 )
            {
                // we are just below the \data\ - therefore expecting ngram x=y lines
                sscanf( line , "%*s %d=%d" , &tempn , &tempn_entries ) ;
                if ( tempn != (max_n_in_file+1) )
                    error("LanguageModel::readARPA - ngram n=y -> unexpected n\n") ;
                max_n_in_file = tempn ;
                if ( tempn <= order )
                    n_exp_entries[tempn-1] = tempn_entries ;
            }    
            else if ( (curr_gram_data > 0) && (curr_gram_data < max_n_in_file) )
            {
                // The line should contain (curr_gram_data+2) fields.
                // eg. for 2-gram entry -> p wd_1 wd_2 bo_wt_2
            
                // Read the probability from the first field (in log10 format) and convert
                //   to ln format.
#ifdef USE_DOUBLE
                if ( sscanf( line , "%lf" , &curr_prob ) != 1 )
#else
                if ( sscanf( line , "%f" , &curr_prob ) != 1 )
#endif
                    error("LanguageModel::readARPA - error reading prob\n") ;
                if ( curr_prob < -90.0 )
                    curr_prob = LOG_ZERO/2 ;
                else
                    curr_prob *= ln_10 ;
                
                // get past the prob field so we can read the words
                strtok( line , " \n\r\t" ) ;
                                
                // read wd_1 , ... , wd_n (ie. all predecessor words of wd_n)
                error_flag = false ;
                for ( int i=0 ; i<curr_gram_data ; i++ )
                {
                    // Extract the next word from the line
                    curr_word = strtok( NULL , " \n\r\t" ) ;

                    // determine the index of the word in the vocabulary
                    curr_index = vocabulary->getIndex( curr_word ) ;
                    if ( curr_index < 0 ) 
                    {
                        // The word is not in our vocab - don't add the entry to our LM
                        error_flag = true ;
                        n_exp_entries[curr_gram_data-1]-- ;
                        break ;
                        //error("LanguageModel::readARPA - %s in ARPA file not in vocab\n" ,
                        //                           curr_word ) ;
                    }
                    else
                    {
                        if ( curr_index == vocabulary->sent_start_index )
                            lm_has_start_word = true ;
                        if ( curr_index == vocabulary->sent_end_index )
                            lm_has_end_word = true ;
                    }
                    
                    // Place the word index into the array of predecessor words in
                    //   oldest-word-first order.
                    words[i] = curr_index ;
                }

                if ( error_flag == true )
                    continue ;

                // Extract the back off weight from the last field in the line and
                //   convert from log10 to ln.
#ifdef USE_DOUBLE
                if ( sscanf( strtok( NULL , " \n\r\t" ) , "%lf" , &curr_bow ) != 1 )
#else
                if ( sscanf( strtok( NULL , " \n\r\t" ) , "%f" , &curr_bow ) != 1 )
#endif
                    error("LanguageModel::readARPA - back off weight not found\n") ;
                if ( curr_bow < -90.0 )
                    curr_bow = 0.0 ;
                else
                    curr_bow *= ln_10 ;
                
                // add the entry to the curr_gram_data-gram for the new word
                ngram->addEntry( curr_gram_data, words, curr_prob, curr_bow ) ;
                n_act_entries[curr_gram_data-1]++ ;
            }
            else if ( curr_gram_data == max_n_in_file )
            {
                // The line should contain (curr_gram_data+1) fields because
                //   backoff weights are only required for N-grams that form a prefix of
                //   longer N-grams in the model file (ie. not this one - the longest).
                // eg. for 4-gram entry -> p wd_1 wd_2 wd_3 wd_4
                //    (where 4-gram probabilities are the maximum in the file.
            
                // read the probability from the first field
#ifdef USE_DOUBLE
                sscanf( line , "%lf" , &curr_prob ) ;
#else
                sscanf( line , "%f" , &curr_prob ) ;
#endif
                if ( curr_prob < -90.0 )
                    curr_prob = LOG_ZERO/2 ;
                else
                    curr_prob *= ln_10 ;
                
                // get past the prob field so we can read the words
                strtok( line , " \n\r\t" ) ;
                                
                // read wd_1 , ... , wd_n and insert indices in 'words' array.
                error_flag = false ;
                for ( int i=0 ; i<curr_gram_data ; i++ )
                {
                    // Extract the next word from the line
                    curr_word = strtok( NULL , " \n\r\t" ) ;

                    // determine the index of the word in the vocabulary
                    curr_index = vocabulary->getIndex( curr_word ) ;
                    if ( curr_index < 0 ) 
                    {
                        // The word is not in our vocab - don't add the entry to our LM
                        error_flag = true ;
                        n_exp_entries[curr_gram_data-1]-- ;
                        break ;
                        //error("LanguageModel::readARPA - %s in ARPA file not in vocab\n" ,
                        //                           curr_word ) ;
                    }
                    else
                    {
                        if ( curr_index == vocabulary->sent_start_index )
                            lm_has_start_word = true ;
                        if ( curr_index == vocabulary->sent_end_index )
                            lm_has_end_word = true ;
                    }
                    
                    // Place the word index into the array of predecessor words in
                    //   oldest-word-first order.
                    words[i] = curr_index ;
                }
                    
                if ( error_flag == true )
                    continue ;

                // add the entry to the curr_gram_data-gram for the new word
                n_act_entries[curr_gram_data-1]++ ;
                ngram->addEntry( curr_gram_data , words , curr_prob ) ;
            }
        }
    }   
    
    // make sure that we got the /end/ marker
    if ( got_end == false )
        error("LanguageModel::readARPA - EOF but no end marker\n") ;
        
    // Issue warnings if the number of expected entries for each n-gram did
    //   not match the actual number read from the file
    for ( int i=0 ; i<order ; i++ )
    {
        if ( n_exp_entries[i] != n_act_entries[i] )
        {
            warning("LanguageModel::readARPA - warning - %d-gram entry count mismatch\n" , i+1 ) ;
            warning("\t%d expected != %d actual\n",n_exp_entries[i],n_act_entries[i]) ;
        }
    }
}


void LanguageModel::readNowayBin( FILE *nw_fd )
{
    // This reads a Noway Binary format language model file.
    // Only the TR2 and NG3 types are supported (ie. trigrams).
    char buf[1000] , *bptr , c ;
    int n_xgrams[3] ;
    unsigned short wd_index , log10_prob , n_bigrams , n_trigrams ;
    short log10_backoff ;
    int *vocab_index_map ;
    int total_bigrams=0 , total_trigrams=0 , words[3] ;
    real prob , backoff ;
    real neg_ln_10 = -log(10.0) , ln_10 = log(10.0) ;
    bool uni_invocab=true , bi_invocab=true , tri_invocab=true ;
    DiskXFile *lm_file ;
    
#ifdef DEBUG
    if ( (sizeof(unsigned short) != 2) || (sizeof(int) != 4) )
        error("LanguageModel::readNowayBin - unexpected type sizes\n") ;
#endif

    if ( nw_fd == NULL )
        error("LanguageModel::readNowayBin - nw_fd is NULL\n") ;
    if ( order > 3 )
        error("LanguageModel::readNowayBin - order is greater than 3\n") ;

    lm_file = new DiskXFile( nw_fd ) ;
    
    // Assume that the first 4-bytes have already been read, 
    //   and have been verified to contain TR2 or NG3.
    // Read the number of unigrams, bigrams and trigrams.
    if ( lm_file->read( n_xgrams , sizeof(int) , 3 ) != 3 )
        error("LanguageModel::readNowayBin - error reading number of ngrams\n") ;

    // Allocate memory to hold the mapping between our vocab indices and
    //   the indices in the file.
    vocab_index_map = (int *)Allocator::sysAlloc( n_xgrams[0] * sizeof(int) ) ;

    // Now read in the "vocabulary"
    for ( int i=0 ; i<n_xgrams[0] ; i++ )
    {
        // Read the word string and make sure it is uppercase
        bptr = buf ;
        lm_file->read( &c , 1 , 1 ) ;
        while ( c > 0 )
        {
            *bptr = (unsigned char)c ;
            bptr++ ;
            lm_file->read( &c , 1 , 1 ) ;
        }
        *bptr = '\0' ;
        //strtoupper( buf ) ;
        
        // Read the word index from the file
        if ( lm_file->read( &wd_index , sizeof(unsigned short) , 1 ) != 1 )
            error("LanguageModel::readNowayBin - error reading word index\n") ;

        if ( wd_index >= n_xgrams[0] )
            error("LanguageModel::readNowayBin - word index exceeds num unigrams\n") ;
            
        // Find the index of the word in our vocabulary and store the mapping.
        vocab_index_map[wd_index] = vocabulary->getIndex( buf ) ;

        if ( (vocab_index_map[wd_index] >= 0) && 
             (vocab_index_map[wd_index] == vocabulary->sent_start_index) )
             lm_has_start_word = true ;
        if ( (vocab_index_map[wd_index] >= 0) && 
             (vocab_index_map[wd_index] == vocabulary->sent_end_index) )
             lm_has_end_word = true ;
    }
    
    // Now we read the unigram, bigram and trigram entries.
    for ( int i=0 ; i<n_xgrams[0] ; i++ )
    {
        // Read the word index from file
        if ( lm_file->read( &wd_index , sizeof(unsigned short) , 1 ) != 1 )
            error("LanguageModel::readNowayBin - error reading unigram word index\n") ;
        if ( vocab_index_map[wd_index] < 0 )
            uni_invocab = false ;
        
        // Read the probability ( -(log10(prob)*8192) format ) and convert to ln(prob).
        if ( lm_file->read( &log10_prob , sizeof(unsigned short) , 1 ) != 1 )
            error("LanguageModel::readNowayBin - error reading unigram prob\n") ;
        prob = ((real)log10_prob / 8192.0) * neg_ln_10 ;
        
        // Read the backoff weight
        if ( lm_file->read( &log10_backoff , sizeof(short) , 1 ) != 1 )
            error("LanguageModel::readNowayBin - error reading unigram backoff\n") ;
        backoff = ((real)log10_backoff / 8192.0) * ln_10 ;

#ifdef DEBUG
        if ( prob > 0.0 )
            error("LanguageModel::readNowayBin - prob > 0.0\n") ;
#endif

        // Read the number of bigrams associated with this word
        if ( lm_file->read( &n_bigrams , sizeof(unsigned short) , 1 ) != 1 )
            error("LanguageModel::readNowayBin - error reading number of bigrams\n") ;
        total_bigrams += n_bigrams ;

        if ( uni_invocab == true )
        {
            // Add the new entry to the 1-gram
            words[0] = vocab_index_map[wd_index] ;
            ngram->addEntry( 1 , words , prob , backoff ) ;
        }
        
        // Now read all the bigrams that have the current word as the predecessor
        for ( int j=0 ; j<n_bigrams ; j++ )
        {
            // Read the word index from file
            if ( lm_file->read( &wd_index , sizeof(unsigned short) , 1 ) != 1 )
                error("LanguageModel::readNowayBin - error reading bigram word index\n") ;
            if ( vocab_index_map[wd_index] < 0 )
                bi_invocab = false ;
            
            // Read the probability ( -(log10(prob)*8192) format ) and convert to ln(prob).
            if ( lm_file->read( &log10_prob , sizeof(unsigned short) , 1 ) != 1 )
                error("LanguageModel::readNowayBin - error reading bigram prob\n") ;
            prob = ((real)log10_prob / 8192.0) * neg_ln_10 ;

            // Read the backoff weight
            if ( lm_file->read( &log10_backoff , sizeof(short) , 1 ) != 1 )
                error("LanguageModel::readNowayBin - error reading bigram backoff\n") ;
            backoff = ((real)log10_backoff / 8192.0) * ln_10 ;
#ifdef DEBUG
            if ( prob>0.0 )
                error("LanguageModel::readNowayBin - bigram prob > 0.0\n") ;
#endif

            // Read the number of trigrams associated with this word
            if ( lm_file->read( &n_trigrams , sizeof(unsigned short) , 1 ) != 1 )
                error("LanguageModel::readNowayBin - error reading number of trigrams\n") ;
            total_trigrams += n_trigrams ;

            // Add the new entry to the 2-gram
            if ( (uni_invocab == true) && (bi_invocab == true) )
            {
                words[1] = vocab_index_map[wd_index] ;
                if ( order >= 2 )
                    ngram->addEntry( 2 , words , prob , backoff ) ;
            }
        
            // Now read all the trigrams that have the current bigram as the predecessor
            for ( int k=0 ; k<n_trigrams ; k++ )
            {
                // Read the word index from file
                if ( lm_file->read( &wd_index , sizeof(unsigned short) , 1 ) != 1 )
                    error("LanguageModel::readNowayBin - error reading trigram word index\n") ;
                if ( vocab_index_map[wd_index] < 0 )
                    tri_invocab = false ;
                    
                // Read the probability ( -(log10(prob)*8192) format ) and convert to ln(prob).
                if ( lm_file->read( &log10_prob , sizeof(unsigned short) , 1 ) != 1 )
                    error("LanguageModel::readNowayBin - error reading trigram prob\n") ;
                prob = ((real)log10_prob / 8192.0) * neg_ln_10 ;

#ifdef DEBUG
                if ( prob > 0.0 )
                    error("LanguageModel::readNowayBin - bigram prob or backoff > 0.0\n") ;
#endif

                // Add the new entry to the 3-gram
                if ( (uni_invocab == true) && (bi_invocab == true) && (tri_invocab == true) )
                {
                    words[2] = vocab_index_map[wd_index] ;
                    if ( order >= 3 )
                        ngram->addEntry( 3 , words , prob ) ;
                }
                tri_invocab = true ;
            }
            bi_invocab = true ;
        }
        uni_invocab = true ;
    }

    if ( (total_bigrams != n_xgrams[1]) || (total_trigrams != n_xgrams[2]) )
        error("LanguageModel::readNowayBin - did not read expected number of bi & trigrams\n") ;

    free( vocab_index_map ) ;
    delete lm_file ;
}


#ifdef DEBUG
void LanguageModel::outputText()
{
    ngram->outputText() ;
}
#endif


}
