// OSPitchShs.h
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

#pragma once

#ifndef SSI_OPENSMILE_PITCHSHS_H
#define SSI_OPENSMILE_PITCHSHS_H

#include "OSPitchBase.h"

namespace ssi {

class OSPitchShs : public OSPitchBase {

public:

	class OptionsShs : public OSPitchBase::Options {

	public:

		OptionsShs ()
			: nHarmonics (15), compressionFactor (0.85f), greedyPeakAlgo (false) {

			octaveCorrection = false;
			voicingCutoff = 0.7f;

			addOption ("nHarmonics", &nHarmonics, 1, SSI_INT, "Number of harmonics to consider for subharmonic sampling (feasible values: 5-15).");		
			addOption ("compressionFactor", &compressionFactor, 1, SSI_REAL, "The factor for successive compression of sub-harmonics.");		
			addOption ("greedyPeakAlgo", &greedyPeakAlgo, 1, SSI_BOOL, "use new algorithm to return all maximum score candidates regardless of their order. The old algorithm added new candidates only if they were higher scored as the first one. Enabling this seems to require different viterbi parameters for smoothing though, so use with caution! Default behaviour is 'off' so we remain backwards compatibility.");		
		};

		int nHarmonics;
		bool greedyPeakAlgo;
		FLOAT_DMEM compressionFactor;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSPitchShs"; };
	static IObject *Create (const ssi_char_t *file) { return new OSPitchShs (file); };
	~OSPitchShs ();

	OSPitchShs::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes the fundamental frequency via the Sub-Harmonic-Sampling (SHS) method (this is related to the Harmonic Product Spectrum method)."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void setMetaData (ssi_size_t size, const void *meta) {
		memcpy (&mdata, meta, size);
	}

protected:

	OSPitchShs (const ssi_char_t *file = 0);
	OSPitchShs::OptionsShs _options;
	ssi_char_t *_file;

	int pitchDetect(FLOAT_DMEM * inData, long _N, double _fsSec, double baseT, FLOAT_DMEM *f0cand, FLOAT_DMEM *candVoice, FLOAT_DMEM *candScore, long nCandidates);
	int addCustomOutputs(FLOAT_DMEM *dstCur, long NdstLeft) { return 0; };

	void release ();

	int nHarmonics;
    int greedyPeakAlgo;
    FLOAT_DMEM Fmint, Fstept;
    FLOAT_DMEM nOctaves, nPointsPerOctave;
    FLOAT_DMEM *SS;
    FLOAT_DMEM compressionFactor;
    double base;
    
	FLOAT_DMEM *mdata;
};

}

#endif
