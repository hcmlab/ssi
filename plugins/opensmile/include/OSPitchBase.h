// OSPitchBase.h
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

/**

Provides energy analysis.

*/

#pragma once

#ifndef SSI_OPENSMILE_PITCHBASE_H
#define SSI_OPENSMILE_PITCHBASE_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSPitchBase : public IFilter {

public:

	struct meta_s {
		int nCandidates;
		float voicingCutoff;

		int pos_nCandiates;
		int pos_candidates;
		int pos_voicing;
		int pos_scores;
		int pos_F0C1;
		int pos_voicingC1;
		int pos_F0raw;
		int pos_voicingClip;
	};

	// derived class MUST derive from this class!
	class Options : public OptionList {

	public:

		Options ()
			: baseSr (16000.0), fsSec (-1.0), minPitch (52.0), maxPitch (620.0), nCandidates (3), scores (true), voicing (true), F0C1 (false), voicingC1 (false), F0raw (false), voicingClip (false), voicingCutoff (1.0), octaveCorrection (false) {

			addOption ("baseSr", &baseSr, 1, SSI_DOUBLE, "Samplerate of original wave input in Hz.");		
			addOption ("fsSec", &fsSec, 1, SSI_DOUBLE, "Frame size in seconds (if -1.0 calculated from sample rate of input stream).");
			addOption ("minPitch", &minPitch, 1, SSI_DOUBLE, "Minimum detectable pitch in Hz.");		
			addOption ("maxPitch", &maxPitch, 1, SSI_DOUBLE, "Maximum detectable pitch in Hz.");		
			addOption ("nCandidates", &nCandidates, 1, SSI_INT, "The number of F0 candidates to output [1-20] (0 disables ouput of candidates AND their voicing probs).");		
			addOption ("scores", &scores, 1, SSI_BOOL, "Output of F0 candidates scores, if available.");		
			addOption ("voicing", &voicing, 1, SSI_BOOL, "Output of voicing probability for F0 candidates.");		
			addOption ("F0C1", &F0C1, 1, SSI_BOOL, "Output of raw best F0 candidate without thresholding in unvoiced segments.");		
			addOption ("voicingC1", &voicingC1, 1, SSI_BOOL, "Output of output voicing (pseudo) probability for best candidate.");		
			addOption ("F0raw", &F0raw, 1, SSI_BOOL, "Output of raw F0 (best candidate), > 0 only for voiced segments (using voicingCutoff threshold).");		
			addOption ("voicingClip", &voicingClip, 1, SSI_BOOL, "Output of voicing of raw F0 (best candidate), > 0 only for voiced segments (using voicingCutoff threshold).");		
			addOption ("voicingCutoff", &voicingCutoff, 1, SSI_REAL, "This sets the voicing (pseudo) probability threshold for pitch detection. Frames with voicing probability values above this threshold will be considered as voiced.");		
			addOption ("octaveCorrection", &octaveCorrection, 1, SSI_BOOL, "If this pitch detector algorithm offers algorithm specific low-level octave correction, enable it.");					
		};

		double baseSr, maxPitch, minPitch;
		int nCandidates;
		bool scores, voicing, F0C1, voicingC1, F0raw, voicingClip, octaveCorrection;
		ssi_real_t voicingCutoff;
		double fsSec;
	};

public:

	~OSPitchBase ();

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		
		ssi_size_t n = 0;

		OSPitchBase::Options *options = ssi_pcast (OSPitchBase::Options, getOptions ());
		
		if (options->nCandidates > 0) {
			n += options->nCandidates;
			if (options->voicing) {
				n += options->nCandidates;
			}
			if (options->scores) {
				n += options->nCandidates;
			}
		}
		if (options->F0C1) {
			n++;
		}
		if (options->voicingC1) {
			n++;
		}
		if (options->F0raw) {
			n++;
		}
		if (options->voicingClip) {
			n++;
		}

		return n+1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	const void *getMetaData (ssi_size_t &size) { 

		OSPitchBase::Options *options = ssi_pcast (OSPitchBase::Options, getOptions ());
		_meta.nCandidates = options->nCandidates; 
		_meta.voicingCutoff = options->voicingCutoff; 

		//the position of each result in the output stream
		int n = 0;
		_meta.pos_nCandiates = -1;
		_meta.pos_candidates = -1;
		_meta.pos_voicing = -1;
		_meta.pos_scores = -1;
		_meta.pos_F0C1 = -1;
		_meta.pos_voicingC1 = -1;
		_meta.pos_F0raw = -1;
		_meta.pos_voicingClip = -1;		

		if (options->nCandidates > 0) {

			_meta.pos_nCandiates = n; n += 1;
			_meta.pos_candidates = n; n += options->nCandidates;

			if (options->voicing) {
				_meta.pos_voicing = n;
				n += options->nCandidates;
			}
			if (options->scores) {
				_meta.pos_scores = n;
				n += options->nCandidates;
			}
		}
		if (options->F0C1) {
			_meta.pos_F0C1 = n;
			n++;
		}
		if (options->voicingC1) {
			_meta.pos_voicingC1 = n;
			n++;
		}
		if (options->F0raw) {
			_meta.pos_F0raw = n;
			n++;
		}
		if (options->voicingClip) {
			_meta.pos_voicingClip = n;
			n++;
		}

		size = sizeof (_meta);
		return &_meta;
	};

protected:

	OSPitchBase ();
	meta_s _meta;
	
    virtual int pitchDetect(FLOAT_DMEM * inData, long _N, double _fsSec, double baseT, FLOAT_DMEM *f0cand, FLOAT_DMEM *candVoice, FLOAT_DMEM *candScore, long nCandidates) = 0;
    virtual int addCustomOutputs(FLOAT_DMEM *dstCur, long NdstLeft) = 0;

	void release ();

	const char *inputFieldPartial;
    int nCandidates;
    int scores, voicing;
    int F0C1, voicingC1;
    int F0raw;
    int voicingClip;
    double maxPitch, minPitch;
	FLOAT_DMEM voicingCutoff;
    int octaveCorrection;
	double basePeriod;
	double fsSec;

    FLOAT_DMEM *inData;
    FLOAT_DMEM *f0cand, *candVoice, *candScore;
};

}

#endif
