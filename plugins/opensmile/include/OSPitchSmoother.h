// OSPitchSmoother.h
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

#pragma once

#ifndef SSI_OPENSMILE_PITCHSMOOTHER_H
#define SSI_OPENSMILE_PITCHSMOOTHER_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSPitchSmoother : public IFilter {

public:

	enum POSTSMOOTHING {
		NONE = 0,
		SIMPLE,
		MEDIAN
	};


public:

	struct meta_s {
		int pos_F0final;
		int pos_F0finalEnv;
		int pos_voicingFinalClipped;
		int pos_voicingFinalUnclipped;
		int pos_voicingC1;
		int pos_F0raw;
		int pos_voicingClip;
	};

	class Options : public OptionList {

	public:

		Options ()
			: medianFilter0 (0), postSmoothing (0), postSmoothingMethod (SIMPLE), octaveCorrection (true), F0final (true), F0finalEnv (false), voicingFinalClipped (false), voicingFinalUnclipped (false), F0raw (false), voicingC1 (false), voicingClip (false) {

			addOption ("medianFilter0", &medianFilter0, 1, SSI_INT, "Apply median filtering of candidates as the FIRST processing step; filter length is 'medianFilter0' if > 0.");
			addOption ("postSmoothing", &postSmoothing, 1, SSI_INT, "Apply post processing (median and spike remover) over 'postSmoothing' frames (0=no smoothing or use default set by postSmoothingMethod).");
			addOption ("postSmoothingMethod", &postSmoothingMethod, 1, SSI_INT, "Post processing method to use. One of the following: 0=disable post smoothing, 1=simple post smoothing using only 1 frame delay (will smooth out 1 frame octave spikes), 2='median' will apply a median filter to the output values (length = value of 'postSmoothing').");
			addOption ("octaveCorrection", &octaveCorrection, 1, SSI_BOOL, "Enable intelligent cross candidate octave correction.");
			addOption ("F0final", &F0final, 1, SSI_BOOL, "Enable output of final (corrected and smoothed) F0.");
			addOption ("F0finalEnv", &F0finalEnv, 1, SSI_BOOL, "Enable output of envelope of final smoothed F0 (i.e. there will be no 0 values (except for end and beginning)).");
			addOption ("voicingFinalClipped", &voicingFinalClipped, 1, SSI_BOOL, "Enable output of final smoothed and clipped voicing (pseudo) probability. 'Clipped' means that the voicing probability is set to 0 for unvoiced regions, i.e. where the probability lies below the voicing threshold.");
			addOption ("voicingFinalUnclipped", &voicingFinalUnclipped, 1, SSI_BOOL, "Enable output of final smoothed, raw voicing (pseudo) probability (UNclipped: not set to 0 during unvoiced regions)..");
			addOption ("F0raw", &F0raw, 1, SSI_BOOL, "Enable output of 'F0raw' copied from input.");
			addOption ("voicingC1", &voicingC1, 1, SSI_BOOL, "Enable output of 'voicingC1' copied from input.");
			addOption ("voicingClip", &voicingClip, 1, SSI_BOOL, "Enable output of 'voicingClip' copied from input.");
		};
		
		POSTSMOOTHING postSmoothingMethod;
		int medianFilter0, postSmoothing;
		bool octaveCorrection, F0final, F0finalEnv, voicingFinalClipped, voicingFinalUnclipped, F0raw, voicingC1, voicingClip;	
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSPitchSmoother"; };
	static IObject *Create (const ssi_char_t *file) { return new OSPitchSmoother (file); };
	~OSPitchSmoother ();

	OSPitchSmoother::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component performs temporal pitch smoothing. Input: candidates produced by a pitchBase descendant (e.g. cPitchSHS). The voicing cutoff threshold is inherited from the input component, thus this smoother component does not provide its own threshold option."; };

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
		if (_options.F0final) n++;
		if (_options.F0finalEnv) n++;
		if (_options.voicingFinalClipped) n++;
		if (_options.voicingFinalUnclipped) n++;
		if (_options.F0raw) n++;
		if (_options.voicingC1) n++;
		if (_options.voicingClip) n++;
		return n;
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

		OSPitchSmoother::Options *options = ssi_pcast (OSPitchSmoother::Options, getOptions ());

		//the position of each result in the output stream
		int n = 0;
		_meta.pos_F0final = -1;
		_meta.pos_F0finalEnv = -1;
		_meta.pos_voicingFinalClipped = -1;
		_meta.pos_voicingFinalUnclipped = -1;
		_meta.pos_voicingC1 = -1;
		_meta.pos_F0raw = -1;
		_meta.pos_voicingClip = -1;	
		
		if (_options.F0final) {
			_meta.pos_F0final = n;
			 n++;
		}
		if (_options.F0finalEnv) {
			_meta.pos_F0finalEnv = n;
			n++;
		}
		if (_options.voicingFinalClipped) {
			_meta.pos_voicingFinalClipped = n;
			n++;
		}
		if (_options.voicingFinalUnclipped) {
			_meta.pos_voicingFinalUnclipped = n;
			n++;
		}
		if (_options.F0raw) {
			_meta.pos_F0raw = n;
			n++;
		}
		if (_options.voicingC1) {
			_meta.pos_voicingC1 = n;
			n++;
		}
		if (_options.voicingClip) {
			_meta.pos_voicingClip = n;
			n++;
		}

		size = sizeof (_meta);
		return &_meta;
	};

	void setMetaData (ssi_size_t size, const void *meta);

protected:

	OSPitchSmoother (const ssi_char_t *file = 0);
	OSPitchSmoother::Options _options;
	ssi_char_t *_file;
	meta_s _meta;

	void release ();

	int firstFrame;
    int medianFilter0;
    int postSmoothing;
	POSTSMOOTHING postSmoothingMethod;
    int onsFlag, onsFlagO;
    int octaveCorrection;

    int F0final, F0finalEnv, voicingFinalClipped, voicingFinalUnclipped;
    int F0raw, voicingC1, voicingClip;

    int nInputLevels; // number of input fields called F0cand (= number of input pdas (algos))
    int voicing, scores; // are candVoicing and candScores fields also present? (1/0 flag)
    
    FLOAT_DMEM lastVoice;
    FLOAT_DMEM pitchEnv;

    // per frame data:
    int nCands; // array holding number of candidates for each pda (current)
    // global:
    int totalCands;
    int nCandidates; // array holding max. number of candidates for each pda; array of size nInputLevels;
    FLOAT_DMEM *f0cand, *candVoice, *candScore;

    // index lookup:
    int f0candI, candVoiceI, candScoreI;
    int F0rawI, voicingClipI, voicingC1I;

    FLOAT_DMEM *median0WorkspaceF0cand;
    FLOAT_DMEM *lastFinal;
    FLOAT_DMEM lastFinalF0;

	FLOAT_DMEM voicingCutoff;



};

}

#endif
