// OSPitchShs.cpp
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

#include "OSPitchShs.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSPitchShs::OSPitchShs (const ssi_char_t *file)
	: OSPitchBase (),
	_file (0),
	SS (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OSPitchShs::~OSPitchShs () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSPitchShs::release () {

	delete[] SS; SS = 0;

	OSPitchBase::release ();
}

void OSPitchShs::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	OSPitchBase::transform_enter (stream_in, stream_out, xtra_stream_in_num, xtra_stream_in); 

	greedyPeakAlgo = _options.greedyPeakAlgo;
	compressionFactor = _options.compressionFactor;
	nHarmonics = _options.nHarmonics;

	ssi_size_t nInput = stream_in.dim;

	// following code taken from openSMILE 1.0.1, pitchShs.cpp
	// http://opensmile.sourceforge.net/

	FLOAT_DMEM _fmint, _fmaxt, _fmin;
	if (mdata != NULL) {
		_fmin = mdata[0];
		//_fmax = mdata->fData[1];
		nOctaves = mdata[2];
		nPointsPerOctave = mdata[3];
		_fmint = mdata[4];
		_fmaxt = mdata[5];
		if (nOctaves == 0.0) {
			ssi_err ("cannot read valid 'nOctaves' from input level meta data, please check if the input is a log(2) scale spectrum from a cSpecScale component!");
		}
	}

	// check for octave scaling:
	base = exp( log((double)_fmin)/(double)_fmint );
	if (fabs(base-2.0) < 0.00001) {
		// oct scale ok
		base = 2.0;
	} else {
		// warn: not oct scale, adjust base internally... untested!
		ssi_wrn ("log base is not 2.0 (no octave scale spectrum)! Untested behaviour! (base = %f, _fmin %f, _fmint %f)",base,_fmin,_fmint);
	}

	Fmint = _fmint;
	Fstept = (_fmaxt-_fmint)/(FLOAT_DMEM)(nInput-1);
	
	SS = new FLOAT_DMEM[nInput];

}

int OSPitchShs::pitchDetect (FLOAT_DMEM * _inData, long _N, double _fsSec, double _baseT, FLOAT_DMEM *_f0cand, FLOAT_DMEM *_candVoice, FLOAT_DMEM *_candScore, long _nCandidates)
{
	/* subharmonic summation; shift spectra by octaves and add */

	int nCand = 0;
	long i,j;

	if (nOctaves == 0.0) return -1;

	for (j=0; j < _N; j++) {
		SS[j] = _inData[j];
	}
	FLOAT_DMEM _scale = compressionFactor;

	for (i=2; i < nHarmonics+1; i++) {
		long shift = (long)floor ((double)nPointsPerOctave * smileMath_log2(i));
		for (j=shift; j < _N; j++) {
			SS[j-shift] += _inData[j] * _scale;
		}
		_scale *= compressionFactor;
	}
	for (j=0; j < _N; j++) {
		SS[j] /= (FLOAT_DMEM)nHarmonics;
		if (SS[j] < 0) SS[j] = 0.0;
	}

	// peak candidate picking & computation of SS vector mean
	_candScore[0] = 0.0;
	double ssMean = (double)SS[0];
	for (i=1; i<_N-1; i++) {
		if (greedyPeakAlgo) { // use new (correct?) max. score peak detector


			if ( (SS[i-1] < SS[i]) && (SS[i] > SS[i+1]) ) { // <- peak detection
				//    && ((SS[i] > _candScore[0])||(_candScore[0]==0.0)) ) { // is max. peak or first peak?

				// add candidate at first free spot or behind another higher scored one...
				for (j=0; j<_nCandidates; j++) {
					if (_candScore[j]==0.0 || _candScore[j]<SS[i]) {
						// move remaining candidates downwards..
						int jj;
						for (jj=_nCandidates-1; jj>j; jj--) {
							_candScore[jj] = _candScore[jj-1];
							_f0cand[jj] = _f0cand[jj-1];
						}
						// add this one...
						_f0cand[j] = (FLOAT_DMEM)i;
						_candScore[j] = SS[i];
						if (nCand<_nCandidates) nCand++;
						break; // leave the for loop after adding candidate to array
					}
				}

			}
		} else {

			if ( (SS[i-1] < SS[i]) && (SS[i] > SS[i+1])
				&& ((SS[i] > _candScore[0])||(_candScore[0]==0.0)) ) { // is max. peak or first peak?


					// TODO:!! this algorithm might only add one candidate, if the first one added is the maximum score candidate. This will degarde performance of following viterbi smoothing!
					// CLEAN SOLUTION: find all peaks, then sort by score, and output to "nCandidates"
					// old algo:
					// shift candScores and f0cand (=indicies)
					for (j=_nCandidates-1; j>0; j--) {
						_candScore[j] = _candScore[j-1];
						_f0cand[j] = _f0cand[j-1];
					}
					_f0cand[0] = (FLOAT_DMEM)i;
					_candScore[0] = SS[i];
					if (nCand<_nCandidates) nCand++;
			}

		}
		ssMean += (double)SS[i];
	}
	ssMean = (ssMean+(double)SS[i])/(double)_N;

	// convert peak candidate frequencies and compute voicing prob.
	for (i=0; i<nCand; i++) {
		long j = (long)_f0cand[i];
		// parabolic peak interpolation:
		FLOAT_DMEM f1 = _f0cand[i]*Fstept + Fmint;
		FLOAT_DMEM f2 = (_f0cand[i]+(FLOAT_DMEM)1.0)*Fstept + Fmint;
		FLOAT_DMEM f0 = (_f0cand[i]-(FLOAT_DMEM)1.0)*Fstept + Fmint;
		double sc=0;
		double fx = smileMath_quadFrom3pts((double)f0, (double)SS[j-1], (double)f1, (double)SS[j], (double)f2, (double)SS[j+1], &sc, NULL);
		// convert log(2) frequency scale to lin frequency scale (Hz):
		_f0cand[i] = (FLOAT_DMEM)exp(fx*log(base));
		_candScore[i] = (FLOAT_DMEM)sc;
		if ((sc > 0.0)&&(sc>ssMean)) {
			_candVoice[i] = (FLOAT_DMEM)( 1.0 - ssMean/sc );
		} else {
			_candVoice[i] = 0.0;
		}
	}

	// octave correction of first candidate:
	if (octaveCorrection) {
		/*
		algo: prefer lower candidate, if voicing prob of lower candidate approx. voicing prob of first candidate (or > voicing cutoff)
		and if score of lower candidate > ( 1/((nHarmonics-1)*compressionFactor) )*score of cand[0]
		*/
		for (i=0; i<nCand; i++) {
			if ( (_f0cand[i] < _f0cand[0])&&(_f0cand[i] > 0) && ((_candVoice[i] > voicingCutoff)||(_candVoice[i]>=0.9*voicingCutoff)) && (_candScore[i] > ((1.0/(FLOAT_DMEM)(nHarmonics-1)*compressionFactor))*_candScore[0]) ) {
				// then swap:
				FLOAT_DMEM tmp;
				tmp = _f0cand[0];
				_f0cand[0] = _f0cand[i]; 
				_f0cand[i] = tmp;
				tmp = _candVoice[0];
				_candVoice[0] = _candVoice[i]; 
				_candVoice[i] = tmp;
				tmp = _candScore[0];
				_candScore[0] = _candScore[i]; 
				_candScore[i] = tmp;
			}
		}
	}

	// return actual number of candidates on success (-1 on failure...)
	return nCand;
}


}
