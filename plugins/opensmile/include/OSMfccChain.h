// OSMfccChain.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

#ifndef SSI_OPENSMILE_MFCCCHAIN_H
#define SSI_OPENSMILE_MFCCCHAIN_H

#include "base/IFeature.h"
#include "OSTools.h"
#include "OSTransformFFT.h"
#include "OSFFTmagphase.h"
#include "OSSpecScale.h"
#include "OSMfcc.h"
#include "Deltas.h"
#include "ioput/option/OptionList.h"
#include "frame/include/Chain.h"

namespace ssi {

static const bool USE_DERIVATIVE = true;

class OSMfccChain : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: deltas_enable(0){
	
				addOption ("deltas_enable", &deltas_enable, 1, SSI_SIZE, "get deltas of MFCC's");
				 
		};

		ssi_size_t deltas_enable;
		
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSMfccChain"; };
	static IObject *Create (const ssi_char_t *file) { return new OSMfccChain (file); };
	~OSMfccChain ();

	OSMfccChain::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component chains together OSTransformFFT, OSFFTmagphase, OSSpecScale, OSPitchShs, and OSPitchSmoother."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {

		if (sample_dimension_in != 1) {
			ssi_err ("dimension > 1 not supported");
		}
	
		if (!_chain) {
			initChain ();
		}

		ssi_size_t result = _fft->getSampleDimensionOut (sample_dimension_in);
		result = _chain->getSampleDimensionOut (result);

		return result;
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

	virtual OSTransformFFT *getOSTransformFFT () {
		return _fft;
	}
	virtual OSFFTmagphase *getOSFFTmagphase () {
		return _fftmag;
	}
	virtual OSSpecScale *getOSSpecScale () {
		return _spect;
	}
	virtual OSMfcc *getOSMfcc () {
		return _mfcc;
	}

protected:

	OSMfccChain (const ssi_char_t *file = 0);
	OSMfccChain::Options _options;
	ssi_char_t *_file;

	void initChain ();

	OSTransformFFT *_fft;
	ssi_stream_t _stream_fft;

	OSFFTmagphase *_fftmag;
	OSSpecScale *_spect;
	OSMfcc *_mfcc;
	IFilter *_deltas;
	IFilter *_filter[4];
	Chain *_chain;
};

}

#endif
