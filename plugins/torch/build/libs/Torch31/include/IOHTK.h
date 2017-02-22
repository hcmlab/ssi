// Copyright (C) 2003--2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
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

#ifndef IO_HTK_INC
#define IO_HTK_INC

#include "IOSequence.h"
#include "DiskXFile.h"

namespace Torch {

// HTK File Header 
struct HTKHeader {              
    long  n_samples;
    long  sample_period;
    short sample_size;
    short sample_kind;
};

#define BASEMASK  077         /* Mask to remove qualifiers */
#define HASENERGY  0100       /* _E log energy included */
#define HASNULLE   0200       /* _N absolute energy suppressed */
#define HASDELTA   0400       /* _D delta coef appended */
#define HASACCS   01000       /* _A acceleration coefs appended */
#define HASCOMPX  02000       /* _C is compressed */
#define HASZEROM  04000       /* _Z zero meaned */
#define HASCRCC  010000       /* _K has CRC check */
#define HASZEROC 020000       /* _0 0'th Cepstra included */
#define HASVQ    040000       /* _V has VQ index attached */

enum _BaseParmKind{
  WAVEFORM,            /* Raw speech waveform (handled by HWave) */
  LPC,LPREFC,LPCEPSTRA,LPDELCEP,   /* LP-based Coefficients */
  IREFC,                           /* Ref Coef in 16 bit form */
  MFCC,                            /* Mel-Freq Cepstra */
  FBANK,                           /* Log Filter Bank */
  MELSPEC,                         /* Mel-Freq Spectrum (Linear) */
  USER,                            /* Arbitrary user specified data */
  DISCRETE,                        /* Discrete VQ symbols (shorts) */
  ANON
};

typedef short ParmKind;          /* BaseParmKind + Qualifiers */

/** Handles the standard binary sequence format in HTK.

   @see IOBin
   @author Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
*/
class IOHTK : public IOSequence
{
  private:
    DiskXFile *file;
    int current_frame_index;
		void readHeader(XFile* file);

  public:
    bool one_file_is_one_sequence;
    int n_total_frames;
    char *filename;
    int max_load;
    bool is_sequential;

		/// Contain the htk header informations
		HTKHeader* header;

    /** Reads the sequence contained in #filename#.
        If #one_file_is_one_sequence# is false, #getSequence()# will return one sequence
        with one frame at each call. (If calling #getSequence(t, foo)#,
        it will put in the sequence #foo# the frame corresponding to the line #t# of the file).
        Note also that if #one_file_is_one_sequence# is false, the access to the IO must be
        sequential when calling #getSequence()# if #is_sequential# is true. (Sequential mode
        is faster).
        If #max_load_# is positive, it loads only the first #max_load_# frames,
        if #one_file_is_one_sequence# is false.
        The file will be opened when reading the first sequence, and closed when reading the
        last one if #is_sequential# is true. Otherwise, the file will be opened and closed
        each time you call #getSequence()#.
     */
    IOHTK(const char *filename_, bool one_file_is_one_sequence_=false, int max_load_=-1, bool is_sequential_=false);

    /// Saves #sequence# in #file# using the HTK format.
    static void saveSequence(XFile *file, Sequence *sequence, HTKHeader* header_);

    virtual void getSequence(int t, Sequence *sequence);
    virtual int getNumberOfFrames(int t);
    virtual int getTotalNumberOfFrames();

    virtual ~IOHTK();
		
		/// HTK source code 
    char* parmKind2Str(ParmKind kind, char *buf);
    ParmKind str2ParmKind(char *str);
    ParmKind baseParmKind(ParmKind kind);
    bool hasEnergy(ParmKind kind);
    bool hasDelta(ParmKind kind) ;
    bool hasAccs(ParmKind kind)  ;
    bool hasNulle(ParmKind kind) ;
    bool hasCompx(ParmKind kind) ;
    bool hasCrcc(ParmKind kind)  ;
    bool hasZerom(ParmKind kind) ;
    bool hasZeroc(ParmKind kind) ;
    bool hasVQ(ParmKind kind)    ;      
};

}

#endif
