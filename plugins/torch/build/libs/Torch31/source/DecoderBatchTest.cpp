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
#include "DecoderBatchTest.h"
#include "EditDistance.h"
#include "time.h"
#include "string_stuff.h"
#include "DiskXFile.h"


namespace Torch {


DecoderBatchTest::DecoderBatchTest( char *datafiles_filename , DSTDataFileFormat datafiles_format ,
                                    char *expected_results_file , BeamSearchDecoder *decoder_ , 
                                    bool remove_sil , bool output_res , char *out_fname ,
                                    bool output_ctm , real frame_msec_step_size )
{
    clock_t start_time , end_time ;

    if ( decoder_ == NULL )
        error("DBT::DBT - decoder_ is NULL\n") ;
    if ( (datafiles_filename == NULL) || (strcmp(datafiles_filename,"")==0) )
        error("DBT::DBT - datafiles_filename is undefined\n") ;
        
    total_time = 0.0 ;
    start_time = clock() ;
    decoder = decoder_ ;
    vocabulary = decoder->vocabulary ;
    n_tests = 0 ;
    tests = NULL ;
    archive_fd = NULL ;

    if ( (expected_results_file == NULL) || (strcmp(expected_results_file,"")==0) )
        have_expected_results = false ;
    else
        have_expected_results = true ;

    // Open the results file
    if ( (out_fname != NULL) && (strcmp(out_fname,"")!=0) )
    {
        if ( (output_fd=fopen( out_fname , "w" )) == NULL )
            error("DBT::DBT - error opening output file\n") ;
    }
    else
        output_fd = NULL ;

    if ( (datafiles_format == DST_FEATS_PFILE_ARCHIVE) ||
         (datafiles_format == DST_PROBS_LNA8BIT_ARCHIVE) ||
         (datafiles_format == DST_FEATS_ONLINE_FTRS_ARCHIVE) )
    {
        // 'datafiles_filename' is the actual feature or prob archive file for all test
        //   sentences.  'expected_results_file' (if specified) contains the ground truth 
        //   transcriptions in "raw" format - 1 sentence per line with the order of 
        //   sentences matching the order in the archive file.
        configureWithArchiveInput( datafiles_filename , datafiles_format ,
                                   expected_results_file , remove_sil , output_res , output_ctm ,
                                   frame_msec_step_size ) ;
    }
    else
    {
        // We have a separate input file for each test (eg. separate lna file for 
        //   each sentence).  'datafiles_filename' should contain a list of absolute 
        //   pathnames of the input files for all tests (1 abs filename per line). 
        // 'expected_results_file' is the ground truth transcriptions for each test
        //   in HTK MLF format (see below) (filenames must be absolute and must have
        //   a 3 letter extension (the extension gets ignored)).
        configureWithIndividualInputs( datafiles_filename , datafiles_format ,
                                       expected_results_file , remove_sil , output_res ,
                                       output_ctm , frame_msec_step_size ) ;
    }
    end_time = clock() ;
    total_time = (real)(end_time-start_time) / CLOCKS_PER_SEC ;
}


DecoderBatchTest::~DecoderBatchTest()
{
    if ( archive_fd != NULL )
        fclose( archive_fd ) ;

    if ( output_fd != NULL )
        fclose( output_fd ) ;

    if ( tests != NULL )
    {
        for ( int i=0 ; i<n_tests ; i++ )
            delete tests[i] ;
        free( tests ) ;
    }
}


void DecoderBatchTest::printStatistics( int i_cost , int d_cost , int s_cost )
{
    // Calculates the insertions, deletions, substitutions, accuracy
    //   and outputs. Also calculates the total time taken to decode (not including
    //   loading of datafiles).
    real decode_time=0.0 ;
    
    EditDistance totalRes , singleRes ;
    totalRes.setCosts( i_cost , d_cost , s_cost ) ;
    singleRes.setCosts( i_cost , d_cost , s_cost ) ;

    for (int i=0 ; i<n_tests ; i++ )
    {
        decode_time += tests[i]->decode_time ;
        if ( tests[i]->actual_words != NULL )
        {
            singleRes.distance( tests[i]->actual_words , tests[i]->n_actual_words ,
                                tests[i]->expected_words , tests[i]->n_expected_words ) ;
            totalRes.add( &singleRes ) ;
        }
    }

    printf("\nTotal time spent actually decoding = %.2f secs\n",decode_time) ;
    printf("Total time spent configuring and running batch test = %.2f secs\n\n",total_time) ;
    DiskXFile xf(stdout);
    totalRes.print( &xf ) ;
    totalRes.printRatio( &xf ) ;
    printf("\n") ;
}


void DecoderBatchTest::run()
{
    clock_t start_time , end_time ;

    start_time = clock() ;
    for ( int i=0 ; i<n_tests ; i++ )
    {
        tests[i]->run( decoder , archive_fd ) ;
    }
    end_time = clock() ;
    total_time += (real)(end_time-start_time) / CLOCKS_PER_SEC ;

    if ( have_expected_results == true )
        printStatistics(7,7,10) ;   // HTK settings for Ins, Sub, Del calculations
}


void DecoderBatchTest::configureWithIndividualInputs( 
                               char *datafiles_filename , DSTDataFileFormat datafiles_format ,
                               char *expected_results_file , bool remove_sil , bool output_res ,
                               bool output_ctm , real frame_msec_step_size )
{    
    FILE *datafiles_fd=NULL , *results_fd=NULL ;
    char line[1000] , fname[1000] , result_fname[1000] , res_word[1000] , *ptr ;
    int temp_result_list[1000] , n_sentence_words=0 , i=0 , test_index , word_cnt ;
    char **filenames=NULL ;
    bool have_mlf=false ;

    // Open the file containing the names of the data files we want to run tests for.
    datafiles_fd = fopen( datafiles_filename , "r" ) ;
    if ( datafiles_fd == NULL )
        error("DecoderBatchTest::configure - error opening datafiles file") ;

    // Open the file containing the expected results for each test.
    // We assume that the format is as per HTK MLF format.
    // Note that the filename line must be enclosed in "". 
    if ( have_expected_results == true )
    {
        if ( (results_fd = fopen( expected_results_file , "r" )) == NULL )
            error("DecoderBatchTest::configureWII - error opening results file") ;

        // Read the first line of the results file to determine its type
        fgets( line , 1000 , results_fd ) ;
        if ( strstr( line , "MLF" ) )
            have_mlf = true ;
        else
        {
            have_mlf = false ;
            fseek( results_fd , 0 , SEEK_SET ) ;
        }
    }
    
    // Determine the number of filenames present in the datafiles file
    n_tests=0 ;
    while ( fgets( line , 1000 , datafiles_fd ) != NULL )
    {
        if ( (sscanf(line,"%s",fname)==0) || (line[0] == '#') || (line[0] == '\n') ||
             (line[0] == '\r') || (line[0] == ' ') || (line[0] == '\t') )
            continue ;
        n_tests++ ;
        tests = (DecoderSingleTest **)Allocator::sysRealloc( tests ,
                                                   n_tests * sizeof(DecoderSingleTest *) ) ;
        tests[n_tests-1] = NULL ;
        filenames = (char **)Allocator::sysRealloc( filenames , n_tests * sizeof(char *) ) ;
        filenames[n_tests-1] = (char *)Allocator::sysAlloc( (strlen(fname)+1)*sizeof(char) ) ;
        strcpy( filenames[n_tests-1] , fname ) ;
    }

    if ( have_expected_results == true )
    {
        // Read each entry in the expected results file, find its corresponding
        //   filename in the temporary list of filename, create a DecoderSingleTest
        //   instance and add it to the list of tests.
        test_index = 0 ;
        while ( fgets( line , 1000 , results_fd ) != NULL )
        {
            if ( have_mlf == true )
            {
                if ( sscanf(line,"\"%[^\"]",result_fname) != 0 )
                {
                    // remove the extension and path from the filename
                    if ( (ptr=strrchr( result_fname , '/' )) != NULL )
                        memmove( result_fname , ptr+1 , strlen(ptr)+1 ) ;
                    if ( (ptr=strrchr( result_fname , '.' )) != NULL )
                        *ptr = '\0' ;

                    // find the filename in the temporary list of filenames
                    for ( i=0 ; i<n_tests ; i++ )
                    {
                        if ( strstr( filenames[i] , result_fname ) )
                        {
                            // We found the filename in the temporary list of filenames.
                            // Read the expected words.
                            n_sentence_words = 0 ;
                            fgets( line , 1000 , results_fd ) ;
                            while ( line[0] != '.' )
                            {
                                sscanf( line , "%s" , res_word ) ;
                                temp_result_list[n_sentence_words]=vocabulary->getIndex(res_word) ;
                                if ( temp_result_list[n_sentence_words] >= 0 )
                                    n_sentence_words++ ;
                                fgets( line , 1000 , results_fd ) ;
                            }

                            // Now configure the next DecoderSingleTest instance 
                            //   with the details of the test.
                            if ( tests[i] != NULL )
                                error("DecoderSingleTest::configureWII - duplicate exp results\n");
                            tests[i] = new DecoderSingleTest() ;
                            tests[i]->configure( i , filenames[i] , n_sentence_words , 
                                    temp_result_list , datafiles_format , decoder->phone_models , 
                                    remove_sil , output_res , output_fd , output_ctm ,
                                    frame_msec_step_size ) ;
                            break ;
                        }
                    }
                }
            }
            else
            {
                // We have expected results in reference format
                // Extract the words in the sentence
                ptr = strtok( line , " \r\n\t" ) ;
                word_cnt = 0 ;
                while ( ptr != NULL )
                {
                    if ( (temp_result_list[word_cnt] = vocabulary->getIndex( ptr )) < 0 )
                        printf("DBT::cWAI - result word %s not in vocab for test %d\n",ptr,i+1) ;
                    word_cnt++ ;
                    ptr = strtok( NULL , " \r\n\t" ) ;
                }

                // Configure the DecoderSingleTest instance
                if ( test_index >= n_tests )
                    error("DecoderSingleTest::configureWII - test_index out of range\n");
                if ( tests[test_index] != NULL )
                    error("DecoderSingleTest::configureWII - duplicate exp results\n");
                tests[test_index] = new DecoderSingleTest() ;
                tests[test_index]->configure( test_index , filenames[test_index] , word_cnt , 
                                              temp_result_list , datafiles_format , 
                                              decoder->phone_models , remove_sil , output_res , 
                                              output_fd , output_ctm , frame_msec_step_size ) ;
                test_index++ ;
            }
        }
        
        // Check that each element of 'tests' has been configured
        for ( i=0 ; i<n_tests ; i++ )
        {
            if ( tests[i] == NULL )
            {
                error( "DecoderSingleTest::configureWII - exp res not found for file %s\n" ,
                       filenames[i] ) ;
            }

            // this is useful to generate an in-order ref transcription file from an MLF file.
            //for ( int j=0 ; j<tests[i]->n_expected_words ; j++ )
            //    printf("%s ",vocabulary->getWord( tests[i]->expected_words[j] ) ) ;
            //printf("\n");
        }
    }
    else
    {
        for ( i=0 ; i<n_tests ; i++ )
        {
            tests[i] = new DecoderSingleTest() ;
            tests[i]->configure( i , filenames[i] , 0 , NULL , datafiles_format , 
                                 decoder->phone_models , remove_sil , 
                                 output_res , output_fd , output_ctm , frame_msec_step_size ) ;
        }
    }

    // Free the temporary list of filenames
    for ( i=0 ; i<n_tests ; i++ )
        free( filenames[i] ) ;
    free( filenames ) ; 
                    
    if ( have_expected_results == true )
        fclose( results_fd ) ;
    fclose( datafiles_fd ) ;
}


void DecoderBatchTest::configureWithArchiveInput( 
                               char *archive_filename , DSTDataFileFormat archive_format ,
                               char *expected_results_file , bool remove_sil , bool output_res ,
                               bool output_ctm , real frame_msec_step_size )
{
    FILE *results_fd=NULL ;
    long *offsets=NULL ;
    char line[1000] , *ptr ;
    int temp_result_list[1000] , word_cnt ;
    
    if ( (archive_fd = fopen( archive_filename , "r" )) == NULL )
        error("DecoderBatchTest::configureWithArchiveInput - error opening archive file\n") ;

    if ( have_expected_results == true )
    {
        if ( (results_fd = fopen( expected_results_file , "r" )) == NULL )
            error("DecoderBatchTest::configureWithArchiveInput - error opening exp res file\n") ;
    }
        
    if ( archive_format == DST_PROBS_LNA8BIT_ARCHIVE )
    {
        // We need to find out the offset in the file where each new utterance begins
        //   and create a new test for each.
        findLNA8ArchiveUtteranceOffsets( archive_fd , decoder->phone_models->n_emission_probs , 
                                         &offsets , &n_tests ) ;
    }
    else if ( archive_format == DST_FEATS_ONLINE_FTRS_ARCHIVE )
    {
        findOnlineFtrsArchiveUtteranceOffsets( archive_fd , decoder->phone_models->n_features , 
                                               &offsets , &n_tests ) ;
    }
    
    fseek( archive_fd , 0 , SEEK_SET ) ;

    // We now know how many tests (utterances) are in the archive, and the
    //   offset into the archive where the utterance data resides.  We can now
    //   configure each test.
    tests = (DecoderSingleTest **)Allocator::sysAlloc( n_tests*sizeof(DecoderSingleTest *) ) ;
    for ( int i=0 ; i<n_tests ; i++ )
    {
        tests[i] = new DecoderSingleTest() ;

        if ( have_expected_results == true )
        {
            // Read the next line from the expected results file
            if ( fgets( line , 1000 , results_fd ) == NULL )
                error("DBT::configureWithArchiveInput - error reading from results file\n") ;
            //strtoupper( line ) ;

            // Extract the words in the sentence
            ptr = strtok( line , " \r\n\t" ) ;
            word_cnt = 0 ;
            while ( ptr != NULL )
            {
                if ( (temp_result_list[word_cnt] = vocabulary->getIndex( ptr )) < 0 )
                    printf("DBT::cWAI - result word %s not in vocab for test %d\n",ptr,i+1) ;
                word_cnt++ ;
                ptr = strtok( NULL , " \r\n\t" ) ;
            }

            // Configure the DecoderSingleTest instance
            tests[i]->configure( i , offsets[i] , word_cnt , temp_result_list , 
                    archive_format , decoder->phone_models , 
                    remove_sil , output_res , output_fd , output_ctm ,
                    frame_msec_step_size ) ;
        }
        else
        {
            tests[i]->configure( i , offsets[i] , 0 , NULL , archive_format , 
                    decoder->phone_models , remove_sil , output_res , 
                    output_fd , output_ctm , frame_msec_step_size ) ;
        }
    }

    if ( offsets != NULL )
        free( offsets ) ;

    if ( have_expected_results == true )
        fclose(results_fd) ;
}


void DecoderBatchTest::findLNA8ArchiveUtteranceOffsets( FILE *arch_fd , int n_probs , 
                                                        long **offsets , int *n_utts )
{
    int count ;
    long pos=0 ;
    unsigned char buf[1000] ;
    bool got_end_word ;
    
    *n_utts = 0 ;
    *offsets = NULL ;
    got_end_word = true ;

    if ( n_probs <= 0 )
        error("DBT::findLNA8ArchiveUtteranceOffsets - lna vector size unspecified\n") ;

    while ( (count=fread( buf , sizeof(unsigned char) , n_probs+1 , arch_fd )) == (n_probs+1) )
    {
        if ( (got_end_word == true) && (buf[0] == 0x00) )
        {
            (*n_utts)++ ;
            *offsets = (long *)Allocator::sysRealloc( (*offsets) , (*n_utts)*sizeof(long) ) ;
            (*offsets)[(*n_utts)-1] = pos ;
            got_end_word = false ;
        }
        else if ( buf[0] == 0x80 )
        {
            if ( got_end_word == true )
                error("DBT::findLNA8ArchiveUtteranceOffsets - double 0x80\n") ;
            got_end_word = true ;
        }
        else
        {
            if ( buf[0] != 0x00 )
                error("DBT::findLNA8ArchiveUtteranceOffsets - first byte of line was not 0x00\n") ;
        }

        pos += (n_probs+1) ;
#ifdef DEBUG
        if ( pos != ftell( arch_fd ) )
            error("DBT::findLNA8ArchiveUtteranceOffsets - pos does not match ftell\n") ;
#endif
    }
}


void DecoderBatchTest::findOnlineFtrsArchiveUtteranceOffsets( FILE *arch_fd , int n_feats , 
                                                              long **offsets , int *n_utts )
{
    int count , step_size ;
    long pos=0 ;
    unsigned char buf[1000] ;
    bool got_end_word ;
    
    *n_utts = 0 ;
    *offsets = NULL ;
    got_end_word = true ;

    if ( n_feats <= 0 )
        error("DBT::findOnlineFtrsArchiveUtteranceOffsets - n_features unspecified\n") ;

    step_size = 1 + (n_feats * sizeof(float)) ;
    while ( (count=fread( buf , sizeof(unsigned char) , step_size , arch_fd )) == step_size )
    {
        if ( (got_end_word == true) && (buf[0] == 0x00) )
        {
            (*n_utts)++ ;
            *offsets = (long *)Allocator::sysRealloc( (*offsets) , (*n_utts)*sizeof(long) ) ;
            (*offsets)[(*n_utts)-1] = pos ;
            got_end_word = false ;
        }
        else if ( buf[0] == 0x80 )
        {
            if ( got_end_word == true )
                error("DBT::findOnlineFtrsArchiveUtteranceOffsets - double 0x80\n") ;
            got_end_word = true ;
        }
        else
        {
            if ( buf[0] != 0x00 )
                error("DBT::findOnlineFtrsArchiveUtteranceOffsets - first byte was not 0x00\n") ;
        }

        pos += step_size ;
#ifdef DEBUG
        if ( pos != ftell( arch_fd ) )
            error("DBT::findLNA8ArchiveUtteranceOffsets - pos does not match ftell\n") ;
#endif
    }
}

#ifdef DEBUG
void DecoderBatchTest::outputText()
{
    printf("Number of tests = %d\n",n_tests) ;
    for ( int i=0 ; i<n_tests ; i++ )
    {
        printf("\nTest %d\n********\n",i) ;
        tests[i]->outputText() ;
        printf("\n") ;
    }
}
#endif


}
