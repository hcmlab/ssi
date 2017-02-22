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
#include "DecoderSingleTest.h"
#include "IOHTK.h"
#include "Sequence.h"
#include "DiskXFile.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"


namespace Torch {


DecoderSingleTest::DecoderSingleTest()
{
    test_filename = NULL ;
    test_id = -1 ;
    expected_words = NULL ;
    n_expected_words = 0 ;
    actual_words = NULL ;
    actual_words_times = NULL ;
    n_actual_words = 0 ;
    data_format = DST_NOFORMAT ;
    decode_time = 0.0 ;
    output_result = false ;
    remove_sent_marks = false ;
    output_ctm = false ;
    frames_per_msec = 0.0 ;
    archive_offset = -1 ;
    output_fd=NULL ;

    n_frames=0 ;
    n_features=0 ;
    n_emission_probs=0 ;
    decoder_input = NULL ;
    phone_models = NULL ;
}


DecoderSingleTest::~DecoderSingleTest()
{
    if ( test_filename != NULL )
        free( test_filename ) ;
    if ( expected_words != NULL )
        free( expected_words ) ;
    if ( actual_words != NULL )
        free( actual_words ) ;
    if ( actual_words_times != NULL )
        free( actual_words_times ) ;
    if ( decoder_input != NULL )
    {
        for ( int i=0 ; i<n_frames ; i++ )
            free( decoder_input[i] ) ;
        free( decoder_input ) ;
    }
}


void DecoderSingleTest::configure( int test_id_ , char *test_filename_ , int n_expected_words_ ,
                             int *expected_words_ , DSTDataFileFormat data_format_ ,
                             PhoneModels *phone_models_ , bool remove_sent_marks_ , 
                             bool output_result_ , FILE *out_fd , bool output_ctm_ ,
                             real frame_ms_step_size )
{
    test_id = test_id_ ;
    archive_offset = -1 ;
    if ( (phone_models = phone_models_) == NULL )
        error("DST::configure - phone_models is NULL\n") ;
    
    // Allocate memory to hold the filename of the test data and copy the string.
    test_filename = (char *)Allocator::sysAlloc( (strlen(test_filename_)+1) * sizeof(char) ) ;
    strcpy( test_filename , test_filename_ ) ;
    
    // Allocate memory to hold the array of word indices that constitute the
    //   expected result of the test and copy the results.
    if ( (n_expected_words_>0) && (expected_words_!=NULL) )
    {
        n_expected_words = n_expected_words_ ;
        expected_words = (int *)Allocator::sysAlloc( n_expected_words * sizeof(int) ) ;
        memcpy( expected_words , expected_words_ , n_expected_words*sizeof(int) ) ;
    }
    else
    {
        n_expected_words = 0 ;
        expected_words = NULL ;
    }

    // If there are existing actual results - delete them
    if ( actual_words != NULL )
    {
        free( actual_words ) ;
        actual_words = NULL ;
    }
    if ( actual_words_times != NULL )
    {
        free( actual_words_times ) ;
        actual_words_times = NULL ;
    }
    n_actual_words = 0 ;

    data_format = data_format_ ;
    frames_per_msec = 1.0 / frame_ms_step_size ;
    output_ctm = output_ctm_ ;
    if ( output_ctm == true )
        remove_sent_marks = false ;
    else
        remove_sent_marks = remove_sent_marks_ ;
    output_result = output_result_ ;
    if ( (output_fd = out_fd) == NULL )
        output_fd = stdout ;
}


void DecoderSingleTest::configure( int test_id_ , long archive_offset_ , int n_expected_words_ ,
                              int *expected_words_ , DSTDataFileFormat data_format_ ,
                              PhoneModels *phone_models_ , bool remove_sent_marks_ , 
                              bool output_result_ , FILE *out_fd , bool output_ctm_ ,
                              real frame_ms_step_size )
{
    test_id = test_id_ ;
    test_filename = NULL ;
    if ( (phone_models = phone_models_) == NULL )
        error("DST::configure(2) - phone_models is NULL\n") ;
    archive_offset = archive_offset_ ;
    
    // Allocate memory to hold the array of word indices that constitute the
    //   expected result of the test and copy the results.
    if ( (n_expected_words_>0) && (expected_words_!=NULL) )
    {
        n_expected_words = n_expected_words_ ;
        expected_words = (int *)Allocator::sysAlloc( n_expected_words * sizeof(int) ) ;
        memcpy( expected_words , expected_words_ , n_expected_words*sizeof(int) ) ;
    }
    else
    {
        n_expected_words = 0 ;
        expected_words = NULL ;
    }

    // If there are existing actual results - delete them
    if ( actual_words != NULL )
    {
        free( actual_words ) ;
        actual_words = NULL ;
    }
    if ( actual_words_times != NULL )
    {
        free( actual_words_times ) ;
        actual_words_times = NULL ;
    }
    n_actual_words = 0 ;

    data_format = data_format_ ;
    frames_per_msec = 1.0 / frame_ms_step_size ;
    output_ctm = output_ctm_ ;
    if ( output_ctm == true )
        remove_sent_marks = false ;
    else
        remove_sent_marks = remove_sent_marks_ ;
    output_result = output_result_ ;
    if ( (output_fd = out_fd) == NULL )
        output_fd = stdout ;
}


void DecoderSingleTest::run( BeamSearchDecoder *decoder , FILE *archive_fd )
{
    clock_t start_time , end_time ;
    int start_index = 0 ;

    // The data file hasn't been loaded yet - load it
    loadDataFile( archive_fd ) ;

    // Now look at the type of data that was in the file and compare it with the
    //   type expected by the phone set.
    if ( ((n_emission_probs == 0) && (phone_models->input_vecs_are_features == false)) ||
         ((n_features == 0) && (phone_models->input_vecs_are_features == true)) )
    {
        // We've got feature vectors (or nothing), but the phone_models is expecting
        //   vectors of emission probabilities (or vice versa).
        error("DecoderSingleTest::run - datafile format does not agree with phone_models\n") ;
    }

    if ( (n_features != phone_models->n_features) && 
         (n_emission_probs != phone_models->n_emission_probs) )
    {
        error("DecoderSingleTest::run - input vector size does not agree with phone_models\n") ;
    }
   
    // If the input vectors are features and we are calculating emission probs
    //   using an MLP, we need to initialise the context window of the MLP.
    if ( (phone_models->input_vecs_are_features == true) && (phone_models->mlp != NULL) )
        start_index = phone_models->mlp->initContextWindow( decoder_input ) ;
    n_frames -= start_index ;
    
    // invoke the decoder
    start_time = clock() ;
    decoder->decode( decoder_input+start_index , n_frames , &n_actual_words , 
                     &actual_words , &actual_words_times ) ;
    end_time = clock() ;
    decode_time = (real)(end_time-start_time) / CLOCKS_PER_SEC ;
    
    // process the decoding result
    if ( remove_sent_marks == true )
        removeSentMarksFromActual( decoder->vocabulary ) ;
    if ( output_result == true )
        outputText( decoder->vocabulary ) ;

    // Free up some memory
    for( int i=0 ; i<(n_frames+start_index) ; i++ )
        free( decoder_input[i] ) ;
    free( decoder_input ) ;
    decoder_input = NULL ;
    n_emission_probs = 0 ;
    n_features = 0 ;
}


void DecoderSingleTest::removeSentMarksFromActual( Vocabulary *vocabulary )
{
    if ( (n_actual_words == 0) || (vocabulary == NULL) )
        return ;
  
    // remove the sentence start word
    if ( actual_words[0] == vocabulary->sent_start_index )
    {
        for ( int j=1 ; j<n_actual_words ; j++ )
        {
            actual_words[j-1] = actual_words[j] ;
            actual_words_times[j-1] = actual_words_times[j] ;
        }
        n_actual_words-- ;
    }
    
    // remove the sentence end word
    if ( actual_words[n_actual_words-1] == vocabulary->sent_end_index )
        n_actual_words-- ;

    // remove any instances of silence
    if ( vocabulary->sil_index >= 0 )
    {
        for ( int j=0 ; j<n_actual_words ; j++ )
        {
            while ( (j<n_actual_words) && (actual_words[j] == vocabulary->sil_index) )
            {
                for ( int k=(j+1) ; k<n_actual_words ; k++ )
                {
                    actual_words[k-1] = actual_words[k] ;
                    actual_words_times[k-1] = actual_words_times[k] ;
                }
                n_actual_words-- ;
            }
        }
    }
}


void DecoderSingleTest::loadDataFile( FILE *archive_fd )
{
    // Make sure that the test_filename and data_format member variables
    //   have been configured.
    if ( (test_filename == NULL) && (archive_fd == NULL) )
        error("DecoderSingleTest::loadDataFile - test_filename/archive_fd not configured\n") ;
    if ( data_format == DST_NOFORMAT )
        error("DecoderSingleTest::loadDataFile - data_format not configured\n") ;

    // Free any existing data and reset the size-related member variables
    if ( decoder_input != NULL )
    {
        for( int i=0 ; i<n_frames ; i++ )
            free( decoder_input[i] ) ;
        free( decoder_input ) ;
        decoder_input = NULL ;
    }
    n_frames=0 ;
    n_features=0 ;
    n_emission_probs=0 ;
    
    switch ( data_format )
    {
    case DST_FEATS_HTK:
    {
        // load the input htk data (ie. feature vectors) into a new IOHtk instance
        IOHTK *htk_data = new IOHTK( test_filename , true ) ;

        // reorganise the IOHtk 1-dimesional data into a 2D array
        n_frames = htk_data->n_total_frames ;
        n_features = htk_data->frame_size ;
        decoder_input = (real **)Allocator::sysAlloc( n_frames * sizeof(real *) ) ;

        // read the HTK data into a sequence
        Sequence *temp_seq = new Sequence( n_frames , n_features ) ;
        htk_data->getSequence( 0 , temp_seq ) ;
        
        // copy the sequence data into the decoder_input array
        for ( int i=0 ; i<n_frames ; i++ )
        {
            decoder_input[i] = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
            memcpy( decoder_input[i] , temp_seq->frames[i] , n_features * sizeof(real) ) ;
        }    

        delete htk_data ;
        delete temp_seq ;
        break ;
    }
    case DST_FEATS_ONLINE_FTRS:
    {
        loadOnlineFtrs( test_filename ) ;
        break ;
    }
    case DST_FEATS_ONLINE_FTRS_ARCHIVE:
    {
        loadOnlineFtrsFromArchive( archive_fd ) ;
        break ;
    }
    case DST_PROBS_LNA8BIT:
    {
        loadLNA8bit( test_filename ) ;
        break ;
    }
    case DST_PROBS_LNA8BIT_ARCHIVE:
    {
        loadLNA8bitFromArchive( archive_fd ) ; 
        break ;
    }
    default:
        error("DecoderSingleTest::loadDataFile - data_format not recognised\n") ;
    }

#ifdef DEBUG
    if ( (n_features==0) && (n_emission_probs==0) )
        error("DecoderSingleTest::loadDataFile - no data loaded\n") ;
#endif
}


void DecoderSingleTest::outputText( Vocabulary *vocab )
{
    real sec_duration , sec_start_time ;
    
    if ( (test_filename == NULL) && (archive_offset<0) )
        return ;
    
    if ( (n_expected_words > 0) && (output_ctm == false) )
    {
        // We have expected words, so output a verbose results containing
        //   the filename, expected result, actual result, segmentation.
        if ( test_filename != NULL )
            fprintf( output_fd , "%s\n" , test_filename ) ;
        else
            fprintf( output_fd , "\n") ;

        fprintf( output_fd , "\tExpected :  ") ;
        for ( int i=0 ; i<n_expected_words ; i++ )
        {
            if ( vocab == NULL )
                fprintf( output_fd , "%d " , expected_words[i] ) ;
            else if ( expected_words[i] < 0 )
                fprintf( output_fd , "<OOV> ") ;
            else
                fprintf( output_fd , "%s " , vocab->getWord(expected_words[i]) ) ;
        }

        fprintf( output_fd , "\n\tActual :    ") ;
        for ( int i=0 ; i<n_actual_words ; i++ )
        {
            if ( vocab == NULL )
                fprintf( output_fd , "%d " , actual_words[i] ) ;
            else
                fprintf( output_fd , "%s " , vocab->getWord(actual_words[i]) ) ;
        }
        fprintf( output_fd , "  [ ") ;
        for ( int i=0 ; i<n_actual_words ; i++ )
            fprintf( output_fd , "%d " , actual_words_times[i]+1 ) ;
        fprintf( output_fd , "(%d) ]\n",n_frames) ;
    }
    else if ( (output_ctm == true) && (n_actual_words>0) )
    {
        if ( actual_words[0] != vocab->sent_start_index )
            error("DST::outputText - did not see sent start symbol in output\n") ;
        if ( actual_words[n_actual_words-1] != vocab->sent_end_index )
            error("DST::outputText - did not see sent end symbol in output\n") ;
            
        for ( int i=1 ; i<(n_actual_words-1) ; i++ )
        {
            sec_duration = ( actual_words_times[i+1] - actual_words_times[i] ) / 
                           frames_per_msec / 1000.0 ;
            sec_start_time = actual_words_times[i] / frames_per_msec / 1000.0 ;
            fprintf( output_fd , "%d A %.3f %.3f %s\n" , test_id , sec_start_time , sec_duration , 
                                                         vocab->getWord(actual_words[i]) ) ;
        }
    }
    else
    {
        // We just want to output the actual result words - nothing more or less
        for ( int i=0 ; i<n_actual_words ; i++ )
            fprintf( output_fd , "%s " , vocab->getWord(actual_words[i]) ) ;
        fprintf( output_fd , "\n") ;
    }
    fflush( output_fd ) ;
}


void DecoderSingleTest::loadLNA8bit( char *lna_filename )
{
    FILE *lna_fd ;
    int buf_size , step_size , i ;
    unsigned char buf[2000] ;
    real sum=0.0 ;

#ifdef DEBUG
    if ( sizeof(unsigned char) != 1 )
        error("DecoderSingleTest::loadLNA8bit - unsigned char not 1 byte\n") ;
    if ( (lna_filename == NULL) || (strcmp(lna_filename,"")==0) )
        error("DecoderSingleTest::loadLNA8bit - lna_filename undefined\n") ;
    if ( phone_models->n_emission_probs <= 0 )
        error("DecoderSingleTest::loadLNA8bit - ph_models->n_emission_probs not set\n") ;
#endif

    n_frames = 0 ;
    decoder_input = NULL ;
    n_emission_probs = phone_models->n_emission_probs ;
    step_size = 1 + (n_emission_probs * sizeof(unsigned char)) ;

    if ( (lna_fd = fopen( lna_filename , "r" )) == NULL )
        error("DecoderSingleTest::loadLNA8bit - error opening LNA file\n") ;
        
    do
    {
        if ( (buf_size=(int)fread( buf , 1 , step_size , lna_fd )) != step_size )
            error("DecoderSingleTest::loadLNA8bit - error reading prob vector\n") ;

        if ( (buf[0] != 0x00) && (buf[0] != 0x80) )
            error("DecoderSingleTest::loadLNA8bit - flag byte error\n") ;
            
        n_frames++ ;
        decoder_input = (real **)Allocator::sysRealloc( decoder_input , n_frames*sizeof(real *) ) ;
        decoder_input[n_frames-1] = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;
        
        // Convert from the 8-bit integer in the file to the equivalent floating point
        //   log probability.
        sum = 0.0 ;
        for ( i=0 ; i<n_emission_probs ; i++ )
        {
            decoder_input[n_frames-1][i] = -((real)buf[i+1] + 0.5) / 24.0 ;
            sum += (real)exp( decoder_input[n_frames-1][i] ) ;
        }

        if ( (sum < 0.97) || (sum > 1.03) )
            error("DecoderSingleTest::loadLNA8bit - sum of probs = %.4f not in [0.97,1.03]\n",sum) ;
    }
    while ( buf[0] != 0x80 ) ;
           
    // We're done.
    n_features = 0 ;
    fclose( lna_fd ) ;
}


void DecoderSingleTest::loadLNA8bitFromArchive( FILE *archive_fd )
{
    int buf_size , step_size , i ;
    unsigned char buf[2000] ;
    real sum=0.0 ;

#ifdef DEBUG
    if ( sizeof(unsigned char) != 1 )
        error("DecoderSingleTest::loadLNA8bitFromArchive - unsigned char not 1 byte\n") ;
    if ( archive_fd == NULL )
        error("DecoderSingleTest::loadLNA8bitFromArchive - archive_fd NULL\n") ;
    if ( phone_models->n_emission_probs <= 0 )
        error("DecoderSingleTest::loadLNA8bitFromArchive - ph_models->n_emission_probs not set\n") ;
    if ( archive_offset < 0 )
        error("DecoderSingleTest::loadLNA8bitFromArchive - archive_offset not setup\n") ;
#endif
        
    n_frames = 0 ;
    decoder_input = NULL ;
    n_emission_probs = phone_models->n_emission_probs ;
    step_size = 1 + (n_emission_probs * sizeof(unsigned char)) ;

    // Go to the correct place in the archive file
    fseek( archive_fd , archive_offset , SEEK_SET ) ;
    
    do
    {
        if ( (buf_size=(int)fread( buf , 1 , step_size , archive_fd )) != step_size )
            error("DecoderSingleTest::loadLNA8bitFromArchive - error reading prob vector\n") ;

        if ( (buf[0] != 0x00) && (buf[0] != 0x80) )
            error("DecoderSingleTest::loadLNA8bitFromArchive - flag byte error\n") ;
            
        n_frames++ ;
        decoder_input = (real **)Allocator::sysRealloc( decoder_input , n_frames*sizeof(real *) ) ;
        decoder_input[n_frames-1] = (real *)Allocator::sysAlloc( n_emission_probs * sizeof(real) ) ;
        
        // Convert from the 8-bit integer in the file to the equivalent floating point
        //   log probability.
        sum = 0.0 ;
        for ( i=0 ; i<n_emission_probs ; i++ )
        {
            decoder_input[n_frames-1][i] = -((real)buf[i+1] + 0.5) / 24.0 ;
            sum += (real)exp( decoder_input[n_frames-1][i] ) ;
        }

        if ( (sum < 0.97) || (sum > 1.03) )
            error("DST::loadLNA8bitFromArchive - sum_probs=%.4f not in [0.97,1.03]\n",sum) ;
    }
    while ( buf[0] != 0x80 ) ;
           
    // We're done.
    n_features = 0 ;
}


void DecoderSingleTest::loadOnlineFtrs( char *online_ftrs_filename )
{
    // Only read the first sentence in the file.
    DiskXFile *online_ftrs_fd ;
    unsigned char buf[2000] , flag ;
    
#ifdef DEBUG
    if ( decoder_input != NULL )
        error("DecoderSingleTest::loadOnlineFtrs - already have decoder input data\n") ;
    if ( (sizeof(unsigned char) != 1) || (sizeof(float) != 4) )
        error("DecoderSingleTest::loadOnlineFtrs - types have unexpected sizes\n") ;
#endif

    n_frames = 0 ;
    decoder_input = NULL ;
    n_features = phone_models->n_features ;

    // Open the file
    online_ftrs_fd = new DiskXFile( online_ftrs_filename , "r" ) ;

    do
    {
        // manually read the flag byte (so that DiskXFile does not reverse).
        online_ftrs_fd->read( &flag , sizeof(unsigned char) , 1 ) ;

        // read the features
        if ( online_ftrs_fd->read( buf , sizeof(float) , n_features ) != n_features )
            error("DecoderSingleTest::loadOnlineFtrs - error reading feature vector\n") ;

        if ( (flag != 0x00) && (flag != 0x80) )
            error("DecoderSingleTest::loadOnlineFtrs - flag byte error\n") ;
            
        n_frames++ ;
        decoder_input = (real **)Allocator::sysRealloc( decoder_input, n_frames*sizeof(real *) ) ;
        decoder_input[n_frames-1] = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
#ifdef USE_DOUBLE
        for ( int i=0 ; i<n_features ; i++ )
            decoder_input[n_frames-1][i] = (real)((float *)buf)[i] ;
#else
        memcpy( decoder_input[n_frames-1] , buf , n_features*sizeof(real) ) ;
#endif
    }
    while ( flag != 0x80 ) ;
           
    // We're done.
    n_emission_probs = 0 ;
    delete online_ftrs_fd ;
}


void DecoderSingleTest::loadOnlineFtrsFromArchive( FILE *archive_fd )
{
    unsigned char buf[2000] , flag ;
    DiskXFile *arch_file ;

#ifdef DEBUG
    if ( (sizeof(unsigned char) != 1) || (sizeof(float) != 4) )
        error("DecoderSingleTest::loadOnlineFtrsFromArchive - types have unexpected sizes\n") ;
    if ( archive_fd == NULL )
        error("DecoderSingleTest::loadOnlineFtrsFromArchive - archive_fd NULL\n") ;
    if ( phone_models->n_features <= 0 )
        error("DecoderSingleTest::loadOnlineFtrsFromArchive - ph_models->n_features not set\n") ;
    if ( archive_offset < 0 )
        error("DecoderSingleTest::loadOnlineFtrsFromArchive - archive_offset not setup\n") ;
#endif
        
    n_frames = 0 ;
    decoder_input = NULL ;
    n_features = phone_models->n_features ;

    // Go to the correct place in the archive file
    arch_file = new DiskXFile( archive_fd ) ;
    arch_file->seek( archive_offset , SEEK_SET ) ;

    // Read until the end of the the sentence.
    do
    {
        // manually read the flag byte (so that DiskXFile does not reverse).
        arch_file->read( &flag , sizeof(unsigned char) , 1 ) ;
        
        if ( arch_file->read( buf , sizeof(float) , n_features ) != n_features )
            error("DecoderSingleTest::loadOnlineFtrsFromArchive - error reading feature vector\n") ;

        if ( (flag != 0x00) && (flag != 0x80) )
            error("DecoderSingleTest::loadOnlineFtrsFromArchive - flag byte error\n") ;
            
        n_frames++ ;
        decoder_input = (real **)Allocator::sysRealloc( decoder_input, n_frames*sizeof(real *) ) ;
        decoder_input[n_frames-1] = (real *)Allocator::sysAlloc( n_features * sizeof(real) ) ;
#ifdef USE_DOUBLE
        for ( int i=0 ; i<n_features ; i++ )
            decoder_input[n_frames-1][i] = (real)((float *)buf)[i] ;
#else
        memcpy( decoder_input[n_frames-1] , buf , n_features*sizeof(real) ) ;
#endif
    }
    while ( flag != 0x80 ) ;

    n_emission_probs = 0 ;
    delete arch_file ;
}


}
