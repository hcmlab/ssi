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

#ifndef DECODERBATCHTEST_INC
#define DECODERBATCHTEST_INC

#include "general.h"
#include "BeamSearchDecoder.h"
#include "Vocabulary.h"
#include "DecoderSingleTest.h"

namespace Torch {


/** This class is used to decode a set of test files and display statistics and results
    for each file (ie. expected and actual words recognised) and also for the entire 
    test set (ie. insertions, deletions, substitutions, accuracy, total time).

    @author Darren Moore (moore@idiap.ch)
*/



class DecoderBatchTest
{
public:
    BeamSearchDecoder *decoder ;
    Vocabulary *vocabulary ;
    int n_tests ;
    DecoderSingleTest **tests ;
    real total_time ;
    FILE *archive_fd ;
    FILE *output_fd ;
    bool have_expected_results ;
    
    /* Constructors / destructor */
    
    /// Configures the batch test.
    /// 'datafiles_filename' is the file containing a list of the input data files
    ///   that need to be decoded.  Absolute pathnames are required.  The decoding 
    ///   will occur in the order the files are listed in this file.
    /// 'expected_results_file' is the file containing the correct transcriptions
    ///   for all input data files.  The ordering of the files does not have to be
    ///   the same as the order in 'datafiles_filename'.
    /// 'decoder_' is a pointer to the decoder to be used to perform the recognition.
    /// 'remove_sil' indicates whether the silence word is to be removed from the
    ///   recognition results before statistics are calculated.
    /// 'output_res' indicates whether the result of each recognition is to be output
    ///   immediately after it is obtained.
    /// 'preload_data' indicates whether all of the input data in all files is to be
    ///   preloaded into memory before any decoding occurs (takes lots of memory and lots
    ///   of time before any results are obtained if the number of input data files
    ///   is large.
    /// 'pre_calc_emission_probs' indicates whether emission probabilities for
    ///   all input files are to be calculated before the decoder is invoked.  This
    ///   only applies if 'preload_data' is true.
    DecoderBatchTest( char *datafiles_filename , DSTDataFileFormat datafiles_format , 
                      char *expected_results_file , BeamSearchDecoder *decoder_ , 
                      bool remove_sil=false , bool output_res=false , char *out_fname=NULL , 
                      bool output_ctm=false , real frame_msec_step_size=10.0 ) ; 
    ~DecoderBatchTest() ;

    /* Methods */
    void configureWithArchiveInput( char *archive_filename , DSTDataFileFormat archive_format ,
                                    char *expected_results_file , bool remove_sil , 
                                    bool output_res , bool output_ctm , real frame_msec_step_size );
    void configureWithIndividualInputs( char *datafiles_filename , 
                                        DSTDataFileFormat datafiles_format ,
                                        char *expected_results_file , bool remove_sil , 
                                        bool output_res , bool output_ctm ,
                                        real frame_msec_step_size ) ;
    void findLNA8ArchiveUtteranceOffsets( FILE *arch_fd , int n_probs , long int **offsets , 
                                          int *n_utts ) ;
    void findOnlineFtrsArchiveUtteranceOffsets( FILE *arch_fd , int n_feats , long int **offsets , 
                                                int *n_utts ) ;

    /// Intended to be called after the 'run' method has returned.  Processes the
    ///   recognition results for all input data files and compiles insertions, 
    ///   deletions, substitions, accuracy and time statisitcs for the entire batch run.
    void printStatistics( int i_cost , int d_cost , int s_cost ) ;

    /// Runs the batch test according to the options set by the call to 'configure'.
    void run() ;

#ifdef DEBUG
    void outputText() ;
#endif
} ;


}

#endif
