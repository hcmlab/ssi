// PreEmphasis.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/10/16
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

#ifndef SSI_AUDIO_PREEMPHASIS_H
#define SSI_AUDIO_PREEMPHASIS_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class PreEmphasis : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: de (false), k (0.97f) {

			addOption ("de", &de, 1, SSI_BOOL, "De-emphasis instead of pre-emphasis (i.e. y[n] = x[n] + k*x[n-1])");		
			addOption ("k", &k, 1, SSI_REAL, "The pre-emphasis coefficient k in y[n] = x[n] - k*x[n-1].");			
		};

		ssi_real_t k;
		bool de;
	};

public:

	static const ssi_char_t *GetCreateName () { return "PreEmphasis"; };
	static IObject *Create (const ssi_char_t *file) { return new PreEmphasis (file); };
	~PreEmphasis ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component performs pre- and de-emphasis of speech signals using a 1st order difference equation: y(t) = x(t) - k*x(t-1)  (de-emphasis: y(t) = x(t) + k*x(t-1))."; };

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
		if (sample_dimension_in != 1) {
			ssi_err ("dimension > 1 not supported");
		}
		return sample_dimension_in;
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

	PreEmphasis (const ssi_char_t *file = 0);
	PreEmphasis::Options _options;
	ssi_char_t *_file;

	ssi_real_t k;
    int de;
	float hist;
	bool first_call;
};

}

#endif
