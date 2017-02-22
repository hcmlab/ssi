// MFCC.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/03
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

#pragma once

#ifndef SSI_SIGNAL_MFCC_H
#define SSI_SIGNAL_MFCC_H

#include "base/IFeature.h"
#include "Spectrogram.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MFCC : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: n_last (13), n_first (0), n_ffts (512) {

			addOption ("n_first", &n_first, 1, SSI_UCHAR, "first cepstral");
			addOption ("n_last", &n_last, 1, SSI_UCHAR, "last cepstral");		
			addOption ("n_ffts", &n_ffts, 1, SSI_UCHAR, "fft size");
		};

		ssi_size_t n_first;
		ssi_size_t n_last;
		ssi_size_t n_ffts;
	};

public:

	static const ssi_char_t *GetCreateName () { return "MFCC"; };
	static IObject *Create (const ssi_char_t *file) { return new MFCC (file); };
	~MFCC ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes mel frequency cepstral coefficients (MFCCs) of input stream."; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

	// performes mfcc analysis
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

	// matrix interface
	void transform (Matrix<ssi_real_t> *matrix_in, 
		Matrix<ssi_real_t> *matrix_out);

protected:

	MFCC (const ssi_char_t *file = 0);
	MFCC::Options _options;
	ssi_char_t *_file;
	
	Matrix<ssi_real_t> *_filterbank;
	Spectrogram *_spectrogram;
	Matrix<ssi_real_t> *_dctmat;
	Matrix<ssi_real_t> *_spect;
};

}

#endif
