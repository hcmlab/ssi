// Limits.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/06
// Copyright (C) 2007-13 University of Augsburg, Lab for Human Centered Multimedia
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

#ifndef SSI_SIGNAL_LIMITS_H
#define SSI_SIGNAL_LIMITS_H

#include "base/IFilter.h"
#include "signal/SignalCons.h"
#include "ioput/option/OptionList.h"
#include "thread/Lock.h"

namespace ssi {

class Limits : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: minval (0), maxval (1) {

			addOption ("min", &minval, 1, SSI_REAL, "minimum value (lower range)", false);		
			addOption ("max", &maxval, 1, SSI_REAL, "maximum value (upper range)", false);
		};

		ssi_real_t minval;
		ssi_real_t maxval;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Limits"; };
	static IObject *Create (const ssi_char_t *file) { return new Limits (file); };
	~Limits ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Limits lower and upper range of a signal to the given values."; };

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

	Limits (const ssi_char_t *file = 0);
	Limits::Options _options;
	ssi_char_t *_file;

	ssi_real_t _min, _max;
	ssi_size_t _min_id, _max_id;
};

}

#endif
