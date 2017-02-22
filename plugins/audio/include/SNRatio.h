// SNRatio.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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

/**

Provides fast signal-to-noise ratio estimation.
  
Implementation is based on code by Richard Sikora.
http://www.mathworks.com/matlabcentral/fileexchange/9808-audio-noise-gate-for-texas-instruments-c5510-c6713

*/

#pragma once

#ifndef SSI_SIGNAL_SNRATIO_H
#define SSI_SIGNAL_SNRATIO_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class SNRatio : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options () 
			: thresh (0.0f) {

			addOption ("thresh", &thresh, 1, SSI_REAL, "values below thresh are set to zero.");	
			
		};

		ssi_real_t thresh;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SNRatio"; };
	static IObject *Create (const ssi_char_t *file) { return new SNRatio (file); };
	~SNRatio ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes signal-to-noise ratio of input stream."; };
	
	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in > 1) {
			ssi_err ("sample dimension > 1 not supported");
		}
		return 1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_SHORT && sample_type_in != SSI_FLOAT ) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return sample_type_in;
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

	SNRatio (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static int decibels[90];

	static int signal_to_noise_ratio_short (short *input, int N);
        static int covariance_short (int16_t *x, int16_t *y, int N);
	static int variance_short (short *x, int N);
	static int calculate_decibels (int input);

	bool input_is_float;
	ssi_stream_t stream_cast;

	short _thres_s;
	float _thres_f;
};

}

#endif
