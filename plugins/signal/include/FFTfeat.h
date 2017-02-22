// FFTFeat.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/19
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

#ifndef SSI_SIGNAL_FFTFEAT_H
#define SSI_SIGNAL_FFTFEAT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "FilterTools.h"

namespace ssi {

class FFT;

class FFTfeat : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: nfft (512), wintype (WINDOW_TYPE_HAMMING) {

			addOption ("nfft", &nfft, 1, SSI_SIZE, "#fft coefficients");
			addOption ("wintype", &wintype, 1, SSI_INT, "window type (0=rectangle,1=triangle,2=gauss,3=hamming");
		};

		ssi_size_t nfft;
		WINDOW_TYPE wintype;
	};

public:

	static const ssi_char_t *GetCreateName () {return "FFTfeat"; };
	static IObject *Create (const ssi_char_t *file) {return new FFTfeat (file); };
	~FFTfeat ();

	FFTfeat::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Applies fast fourier transformation and outputs coefficients as feature values."; };
	
	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		ssi_size_t rfft = ((_options.nfft >> 1) + 1);
		return sample_dimension_in * rfft;
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

protected:

	FFTfeat (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	void release_fft ();
	void init_fft ();

	ssi_size_t _fft_size;
	ssi_size_t _fft_dim;
	FFT *_fft;
	ssi_real_t **_fft_out;
	ssi_real_t **_fft_in;

	WINDOW_TYPE _win_type;
	ssi_size_t _win_size;
	Matrix<ssi_real_t> *_window;
};

}

#endif
