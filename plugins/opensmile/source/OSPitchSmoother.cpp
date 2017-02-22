// OSPitchSmoother.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/27
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

#include "OSPitchSmoother.h"
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

OSPitchSmoother::OSPitchSmoother (const ssi_char_t *file)
	: _file (0),
	f0cand (0),
	candVoice (0),
	candScore (0),
	median0WorkspaceF0cand (0),
	lastFinal (0),
	onsFlag(0), onsFlagO(0), firstFrame(1),
	lastVoice(NULL), lastFinalF0(0), pitchEnv(0.0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

OSPitchSmoother::~OSPitchSmoother () {

	release ();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OSPitchSmoother::release () {
	
	delete[] f0cand; f0cand = 0; candVoice = 0; candScore = 0;
	delete[] median0WorkspaceF0cand; median0WorkspaceF0cand = 0;
	delete[] lastFinal; lastFinal = 0;
}

void OSPitchSmoother::setMetaData (ssi_size_t size, const void *meta) {

	if (size != sizeof (OSPitchBase::meta_s)) {
		ssi_err ("unexpected meta size");
	}

	const OSPitchBase::meta_s *m = ssi_pcast (const OSPitchBase::meta_s, meta);

	nCandidates = totalCands = m->nCandidates;
	voicingCutoff = m->voicingCutoff;
}

void OSPitchSmoother::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	medianFilter0 = _options.medianFilter0;
	postSmoothing = _options.postSmoothing;
	postSmoothingMethod = _options.postSmoothingMethod;
	switch (postSmoothingMethod) {
		case NONE:		
			postSmoothing = 0;
			break;
		case SIMPLE:
			postSmoothing = 1;
			break;
		case MEDIAN:
			if (postSmoothing < 2) postSmoothing = 2; 
			break;
		default:
			ssi_wrn ("unknown post smoothing method '%d', setting to NONE",postSmoothingMethod);
			postSmoothingMethod = NONE;		
	};

	if (postSmoothing > 0) {
		lastFinal = new FLOAT_DMEM[postSmoothing];
		for (int i = 0; i < postSmoothing; i++) {
			lastFinal[i] = 0;
		}
	}	

	octaveCorrection = _options.octaveCorrection ? 1 : 0;	
	F0final = _options.F0final ? 1 : 0;		
	F0finalEnv = _options.F0finalEnv ? 1 : 0;			
	voicingFinalClipped = _options.voicingFinalClipped ? 1 : 0;			
	voicingFinalUnclipped = _options.voicingFinalUnclipped ? 1 : 0;				
	F0raw = _options.F0raw ? 1 : 0;		
	voicingC1 = _options.voicingC1 ? 1 : 0;	
	voicingClip = _options.voicingClip ? 1 : 0;

	f0candI = 1;
	candVoiceI = f0candI + totalCands;
	candScoreI = candVoiceI + totalCands;
   
	f0cand = new FLOAT_DMEM[totalCands*3];
	candVoice = f0cand+totalCands;
	candScore = f0cand+totalCands*2;

	int n = candScoreI;
	if (F0raw) {
		F0rawI = ++n;
	}
	if (voicingC1) {
		voicingC1I = ++n;
	}
	if (voicingClip) {
		voicingClipI = ++n;
	}	

	if (medianFilter0 > 0) {
		median0WorkspaceF0cand = smileUtil_temporalMedianFilterInitSl (totalCands, 2, medianFilter0);
	}

	firstFrame = 1;
}

void OSPitchSmoother::transform (ITransformer::info info,
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

		// following code taken from openSMILE 1.0.1, pitchSmoother.cpp
		// http://opensmile.sourceforge.net/

		long i, j;
		long n=0;

		// collect all input candidates
		int c = 0;
		
		for (j=0; j<nCandidates; j++) {
			candVoice[c] = src[candVoiceI+j];
			candScore[c] = src[candScoreI+j];
			f0cand[c++] = src[f0candI+j];
			//printf("f0candI[%i] %i\n",i,f0candI[i]);
		}


		// median filter:
		if (medianFilter0 > 0) {
			smileUtil_temporalMedianFilterWslave(f0cand, c /*totalCands*/, 2, median0WorkspaceF0cand);
		}

		if (octaveCorrection) {
			// **ocatve correction**:
			// best candidate is always the 0th (see pitchBase), all other candidates are in arbitrary order
			// thus the smoothing here tries to validate the best candidate
			// a) if another candidate (which is lower in f is found, we consider to use it)
			// b) if the other candidates are multiples of half the 0th candidate, we consider to use half the 0th candidate
			// c) we do global optimisation via median filtering (1st step actually)
			// d) we do global optimisation via octave jump costs and evaluating a) and b) at octave jumps + voicing probs
			int cand0ismin = 1;
			FLOAT_DMEM vpMin = 0.0;
			int minC = -1;
			for (i=1; i<c; i++) {
				if ((f0cand[i] > 0.0)&&(f0cand[i] < f0cand[0])) { // a)
					if ((candVoice[i] > 0.9*candVoice[0])&&(candVoice[i] > vpMin)) {
						vpMin = candVoice[i]; minC = i;
					}
					cand0ismin = 0;
				} 
			}
			if (!cand0ismin) {
				if (minC >= 0) { // use the alternate, lower frequency candidate...
					FLOAT_DMEM t = f0cand[0];
					f0cand[0] = f0cand[minC];
					f0cand[minC] = t;
					t = candVoice[0];
					candVoice[0] = candVoice[minC];
					candVoice[minC] = t;
					t = candScore[0];
					candScore[0] = candScore[minC];
					candScore[minC] = t;
				} 
			} else {
				//if (cand0ismin) { // check for b)
				int halfed = 0; j=0;
				while ((!halfed) && j<c-1) {
					for (i=j+1; i<c; i++) {
						if ((f0cand[i] > 0.0)&&(f0cand[j] > 0.0)) {
							FLOAT_DMEM k = fabs(f0cand[i]-f0cand[j])*(FLOAT_DMEM)2.0/f0cand[0];
							k = (FLOAT_DMEM)fabs(k-1.0);
							if (k<0.1) { // b)
								f0cand[0] /= (FLOAT_DMEM)2.0;
								halfed = 1;
								break;
							} 
						}
					}
					j++;
				}
				//} else {
			}
		}

		/* output */
		FLOAT_DMEM voiceC1 = candVoice[0];
		if (F0final||F0finalEnv) {
			FLOAT_DMEM pitch, pitchOut;
			if (candVoice[0] > voicingCutoff) {
				pitch = f0cand[0]; 
			} else {
				pitch = 0.0;
			}

			// post smoothing:
			if (postSmoothing) {
				if (postSmoothingMethod == SIMPLE) {
					if (firstFrame) { 
						firstFrame = 0; 
						for (long nn = 0; nn < Ndst; nn++) {
							dst[nn] = 0;
						}
						continue; 
					} // for proper synchronisation
					voiceC1 = lastVoice; lastVoice = candVoice[0];

					// simple pitch contour smoothing (delay: 1 frame):

					if ((lastFinal[0] == 0.0)&&(pitch>0.0)) onsFlag = 1;
					if ((lastFinal[0] > 0.0)&&(pitch==0.0)&&(onsFlag==0)) onsFlag = -1;
					if ((lastFinal[0] > 0.0)&&(pitch>0.0)) onsFlag = 0;
					if ((lastFinal[0] == 0.0)&&(pitch==0.0)) onsFlag = 0;

					if ((pitch==0.0)&&(onsFlag==1)) { lastFinal[0] = 0.0; }
					else if ((pitch>0.0)&&(onsFlag==-1)) { lastFinal[0] = pitch; }


					int doubling = 0; int halfing = 0;
					if ((lastFinal[0]>0.0)&&(pitch>0.0)) {
						FLOAT_DMEM factor = lastFinal[0]/pitch;
						if (factor > 1.2) halfing = 1; /* old: fabs(factor-2.0)<0.15 */
						else if (factor < 0.8) doubling = 1; /* old : fabs(factor-0.5)<0.05 */
					}

					if ((doubling)&&(onsFlagO==-1)) { lastFinal[0] = pitch; }
					else if ((halfing)&&(onsFlagO==1)) { lastFinal[0] = pitch; }

					if (doubling) onsFlagO = 1;
					if (halfing && (onsFlag==0)) onsFlagO = -1;
					if (!(halfing||doubling)) onsFlagO = 0;




					pitchOut = lastFinal[0]; // dst[n]
					// shift last final...
					for (i=postSmoothing-1; i>0; i--) {
						lastFinal[i] = lastFinal[i-1];
					}
					lastFinal[0] = pitch;

				} else if (postSmoothingMethod == MEDIAN) {
					//if (firstFrame) { firstFrame = 0; return 0; }  // for proper synchronisation

					// shift last final...
					for (i=postSmoothing-1; i>0; i--) {
						lastFinal[i] = lastFinal[i-1];
					}
					lastFinal[0] = pitch;
					pitchOut /*dst[n]*/ = smileMath_median( lastFinal, postSmoothing, NULL );


				} else { // no smoothing...
					pitchOut/*dst[n]*/ = pitch;
				}
			} else { // no smoothing...
				pitchOut/*dst[n]*/ = pitch;
			}
			if (pitchOut > 0.0) lastFinalF0 = pitchOut;
			if (F0final) {
				dst[n++] = pitchOut;
			}
			if (F0finalEnv) {
				//dst[n++] = lastFinalF0;
				if (pitchOut > 0.0) {
					if (pitchEnv == 0.0) pitchEnv = pitchOut;
					else pitchEnv = (FLOAT_DMEM)0.75*pitchEnv + (FLOAT_DMEM)0.25*pitchOut;
				}
				dst[n++] = pitchEnv;
			}
		}

		if (voicingFinalClipped) {
			if (voiceC1 > voicingCutoff) {
				dst[n] = voiceC1;
			} else {
				dst[n] = 0.0;
			}
			n++;
		}
		if (voicingFinalUnclipped) {
			dst[n] = voiceC1;
			n++;
		}

		// copy raw data from input (if enabled) ( TODO: choose best input level!)
		if (voicingC1) { 
			dst[n] = src[voicingC1I]; n++;
		}
		if (F0raw) { 
			//printf("F0rawI[0] %i\n",F0rawI[0]);
			dst[n] = src[F0rawI]; n++; 
		}
		if (voicingClip) { 
			dst[n] = src[voicingClipI]; n++;
		}
		
	}
}

}
