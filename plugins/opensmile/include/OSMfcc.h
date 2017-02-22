// OSMfcc.h
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

Computes Mel-Frequency-Cepstral Coefficients (MFCC) from Mel-Spectrum

*/

#pragma once

#ifndef SSI_OPENSMILE_MFCC_H
#define SSI_OPENSMILE_MFCC_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSMfcc : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: first (1), last (12), melFloor (0.00000001f), cepLifter (22.0f), htk (false) {

			addOption ("first", &first, 1, SSI_SIZE, "The first MFCC to compute.");		
			addOption ("last", &last, 1, SSI_SIZE, "The last MFCC to compute.");		
			addOption ("floor", &melFloor, 1, SSI_REAL, "The minimum value allowed for melspectra when taking the log spectrum (this parameter will be forced to 1.0 when htkcompatible=1).");		
			addOption ("lift", &cepLifter, 1, SSI_REAL, "Parameter for cepstral 'liftering', set this to 0.0 to disable cepstral liftering.");		
			addOption ("htk", &htk, 1, SSI_BOOL, "Appends the 0-th coefficient at the end instead of placing it as the first element of the output vector.");
		};

		ssi_size_t first, last;
		ssi_real_t melFloor, cepLifter;
		bool htk;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSMfcc"; };
	static IObject *Create (const ssi_char_t *file) { return new OSMfcc (file); };
	~OSMfcc ();

	OSMfcc::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes Mel-frequency cepstral coefficients (MFCC) from a critical band spectrum (see 'cMelspec'). An I-DCT of type-II is used from transformation from the spectral to the cepstral domain. Liftering of cepstral coefficients is supported. HTK compatible values can be computed."; };

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
		return _options.last - _options.first + 1;
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

protected:

	OSMfcc (const ssi_char_t *file = 0);
	OSMfcc::Options _options;
	ssi_char_t *_file;

	int htkcompatible;
    int firstMfcc, lastMfcc, nMfcc;
 
	int initTables (long blocksize);
	void releaseTables ();

	FLOAT_DMEM *_costable;
    FLOAT_DMEM *_sintable;
    FLOAT_DMEM melfloor;
    FLOAT_DMEM cepLifter;
};

}

#endif
