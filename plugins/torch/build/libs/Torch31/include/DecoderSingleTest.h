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

#ifndef DECODERSINGLETEST_INC
#define DECODERSINGLETEST_INC

#include "general.h"
#include "BeamSearchDecoder.h"
#include "PhoneModels.h"
#include "Vocabulary.h"


namespace Torch {


typedef enum 
{
    DST_FEATS_HTK=0 ,
    DST_FEATS_ONLINE_FTRS ,
    DST_FEATS_ONLINE_FTRS_ARCHIVE ,
    DST_FEATS_PFILE_ARCHIVE ,  // unimplemented
    DST_PROBS_LNA8BIT ,
    DST_PROBS_LNA8BIT_ARCHIVE ,
    DST_NOFORMAT
} DSTDataFileFormat ;


/** This class is used to recognise a single input data file, post-process the
    recognition result and output the result.  The DecoderBatchTest class
    contains of an array of these objects.

    @author Darren Moore (moore@idiap.ch)
*/

class DecoderSingleTest
{
public:
    char *test_filename ;
    int test_id ;
    int *expected_words ;
    int n_expected_words ;
    int *actual_words ;
    int *actual_words_times ;
    int n_actual_words ;
    DSTDataFileFormat data_format ;
    real decode_time ;
    bool output_result ;
    bool remove_sent_marks ;
    bool output_ctm ;
    real frames_per_msec ;
    long archive_offset ;
    FILE *output_fd ;

    int n_frames ;
    int n_features ;
    int n_emission_probs ;
    real **decoder_input ;
    PhoneModels *phone_models ;

    /* Constructors/destructor */
    DecoderSingleTest() ;
    ~DecoderSingleTest() ;

    /* Methods */

    /// Configures the test.
    /// 'test_id_' is an ID assigned to this particular test.
    /// 'test_filename_' is the absolute pathname of the input data file.
    /// 'n_expected_words_' and 'expected_words_' contain information about 
    ///   the expected result of the test (ie. the correct transcription).
    /// 'data_format_' indicates the type of data contained in the input
    ///   data file.  Only 1 input data format has been implemented so far.
    /// 'phone_models_' is a pointer to the PhoneModels instance that contains
    ///   the phone HMM's. This used to verify that datafiles contain expected
    ///   data (eg. emission probs or features, correct vector sizes)
    /// 'remove_sent_marks_' indicates whether the sentence start and end
    ///   words in the recognition result are to be removed.
    /// 'output_result_' indicates whether the recognition result is to be
    ///   output after the decoding has occurred.
    /// 'out_fd' is the file where output will be written (if NULL then
    ///   stdout assumed).
    /// 'output_ctm_' indicates if the output is to be in CTM format.
    /// 'frame_msec_step_size' is the frame step size in milliseconds and is
    ///   only used for CTM output.
    void configure( int test_id_ , char *test_filename_ , int n_expected_words_ , 
                    int *expected_words_ , DSTDataFileFormat data_format_ , 
                    PhoneModels *phone_models_=NULL , bool remove_sent_marks_=false , 
                    bool output_result_=false , FILE *out_fd=NULL , bool output_ctm_=false , 
                    real frame_msec_step_size=10.0 ) ;

    /// Configures the test. This version is used when we have input data
    ///   in a big archive file.
    /// Parameters are same as other 'configure' method except 'test_filename_'
    ///   has been replaced by 'archive_offset_'
    /// 'archive_offset' is the offset into an archive input file where the
    ///   input data for this test resides. 
    void configure( int test_id_ , long archive_offset_ , int n_expected_words_ , 
                    int *expected_words_ , DSTDataFileFormat data_format_ , 
                    PhoneModels *phone_models_=NULL , bool remove_sent_marks_=false , 
                    bool output_result_=false , FILE *out_fd=NULL , bool output_ctm_=false , 
                    real frame_msec_step_size=10.0 ) ;
                    
    /// Runs the tests using the recogniser pointed to by 'decoder'.
    /// 'archive_fd' is the big file containing the input data. When
    ///   this is not NULL, the archive_offset member variable is used
    ///   to fseek to the correct point in the archive file.
    void run( BeamSearchDecoder *decoder , FILE *archive_fd=NULL ) ;

    /// Outputs the recognition result. If 'vocab' is defined, then the
    ///   recognition result (ie. vocab entry indices) are converted to
    ///   strings and displayed.  If 'vocab' is NULL then the indices are
    ///   output directly.
    void outputText( Vocabulary *vocab=NULL ) ;
    
    /// Internal functions.
    void removeSentMarksFromActual( Vocabulary *vocabulary ) ;
    void loadDataFile( FILE *archive_fd=NULL ) ;
    void loadLNA8bit( char *lna_filename ) ;
    void loadLNA8bitFromArchive( FILE *archive_fd ) ;
    void loadOnlineFtrs( char *online_ftrs_filename ) ;
    void loadOnlineFtrsFromArchive( FILE *archive_fd ) ;
} ;


}


#endif
