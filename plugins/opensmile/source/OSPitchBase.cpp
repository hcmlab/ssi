// OSPitchBase.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/22
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#include "OSPitchBase.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSPitchBase::OSPitchBase ()
	: inData (0),
    f0cand (0), 
	candVoice (0),
	candScore (0) {
}

OSPitchBase::~OSPitchBase () {

	release ();
}

void OSPitchBase::release () {
	
	delete[] candScore; candScore = 0;
	delete[] candVoice; candVoice = 0;
	delete[] f0cand; f0cand = 0;
}

void OSPitchBase::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	release ();

	OSPitchBase::Options *options = ssi_pcast (OSPitchBase::Options, getOptions ());
	
	maxPitch = options->maxPitch;
	minPitch = options->minPitch;
	nCandidates = options->nCandidates;
	if (nCandidates < 1) nCandidates = 1;
	if (nCandidates > 20) nCandidates = 20; 
	scores = options->scores ? 1 : 0;
	voicing = options->voicing ? 1 : 0;
	F0C1 = options->F0C1 ? 1 : 0;
	voicingC1 = options->voicingC1 ? 1 : 0;
	F0raw = options->F0raw ? 1 : 0;
	voicingClip = options->voicingClip ? 1 : 0;
	octaveCorrection = options->octaveCorrection ? 1 : 0;
	voicingCutoff = options->voicingCutoff;
	basePeriod = 1.0 / options->baseSr;
	fsSec = options->fsSec;
	if (fsSec == -1.0) {
		fsSec = 1.0/stream_in.sr;
	}

	candScore = new FLOAT_DMEM[nCandidates];
	candVoice = new FLOAT_DMEM[nCandidates];
	f0cand = new FLOAT_DMEM[nCandidates];

}

void OSPitchBase::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	long Nsrc = stream_in.dim;
	long Ndst = stream_out.dim;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	for (ssi_size_t k = 0; k < num; k++) { 

		ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr) + k * Nsrc;
		ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr) + k * Ndst;

		// following code taken from openSMILE 1.0.1, pitchBase.cpp
		// http://opensmile.sourceforge.net/


		// we assume we have fft magnitude as input...
		//double _N = (double)(Nsrc);  
		//double F0fft = 1.0/fsSec;
		//double Tsamp = fsSec/_N;
		int nCand=0; 
		long i, j;

		inData = src;
		long nInput = Nsrc;

		// zero candidates
		for (i=0; i<nCandidates; i++) {
			f0cand[i] = 0.0;
			candVoice[i] = 0.0;
			candScore[i] = 0.0;
		}
		// main algorithm to be implemented by derived component
		nCand = pitchDetect(inData,nInput,fsSec,basePeriod,f0cand,candVoice,candScore,nCandidates);

		// post process candidates wrt. minPitch / maxPitch
		// => remove out of range candidates
		if ( nCand > 0 ) {
			for (i=0; (i<nCandidates)&&(nCand>0); i++) {
				if ( (f0cand[i] > maxPitch)||(f0cand[i] < minPitch) ) {
					FLOAT_DMEM origF = f0cand[i];
					for (j=i+1; j<nCandidates; j++) {
						f0cand[j-1] = f0cand[j];
						candVoice[j-1] = candVoice[j];
						candScore[j-1] = candScore[j];
					}
					f0cand[j-1]=0;
					candVoice[j-1]=0;
					candScore[j-1]=0;
					if (origF > 0.0) { nCand--; i--; }
				}
			}
		}

		// if candidates remain, add them to output
		if ( nCand >= 0 ) {

			long n=0;
			if (nCandidates>0) {
				*(dst++) = (FLOAT_DMEM)nCand;

				// TODO: move max score candidate to 0th (if !octaveCorrection)
				long maxI = 0;
				if (!octaveCorrection) {
					FLOAT_DMEM max = candScore[0]; 
					for (i=1; i<nCandidates; i++) { // find max score (may be equal to voicing prob.)
						if (candScore[i] > max) { max = candScore[i]; maxI = i; }
					}
				}
				// swap
				if (maxI > 0) {
					FLOAT_DMEM tmp;
					tmp = f0cand[0];
					f0cand[0] = f0cand[maxI];
					f0cand[maxI] = tmp;
					tmp = candVoice[0];
					candVoice[0] = candVoice[maxI];
					candVoice[maxI] = tmp;
					tmp = candScore[0];
					candScore[0] = candScore[maxI];
					candScore[maxI] = tmp;
				}

				// add raw candidates to output
				for (i=0; i<nCandidates; i++) {
					*(dst++) = f0cand[i];
				}
				n += nCandidates;
				if (voicing) {
					for (i=0; i<nCandidates; i++) {
						*(dst++) = candVoice[i];
					}
					n += nCandidates;
				}
				if (scores) {
					for (i=0; i<nCandidates; i++) {
						*(dst++) = candScore[i];
					}
					n += nCandidates;
				}

				// now pick best candidate (if octaveCorrection is enabled, always use 0th candidate!)
				if (F0C1||voicingC1||F0raw||voicingClip) {
					long maxI = 0;
					/*if (!octaveCorrection) {
					FLOAT_DMEM max = candScore[0]; 
					for (i=1; i<nCandidates; i++) { // find max score (may be equal to voicing prob.)
					if (candScore[i] > max) { max = candScore[i]; maxI = i; }
					}
					}*/

					//FLOAT_DMEM _f0raw =  ;
					if (F0C1) { *(dst++) = f0cand[maxI]; n++; }
					if (voicingC1) { *(dst++) = candVoice[maxI]; n++; }
					// and apply voicingCutoff
					if (F0raw) {
						if (candVoice[maxI] <= voicingCutoff) *(dst++) = 0.0;
						else *(dst++) = f0cand[maxI]; 
						n++;
					}
					if (voicingClip) {
						if (candVoice[maxI] <= voicingCutoff) *(dst++) = 0.0;
						else *(dst++) = candVoice[maxI]; 
						n++;
					}
				}

			}

		} else { 
			
			for (long i = 0; i < Ndst; i++) {
				*dst++ = 0;
			}
		}

	}
}

}
