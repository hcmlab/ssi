// Relative.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2016/10/17
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

#ifndef SSI_SIGNAL_RELATIVE_H
#define SSI_SIGNAL_RELATIVE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Relative : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () {	
		};
	};

public:

	static const ssi_char_t *GetCreateName () { return "Relative"; };
	static IObject *Create (const ssi_char_t *file) { return new Relative(file); };
	~Relative();

	Relative::Options *getOptions () override { return &_options; };
	const ssi_char_t *getName () override { return GetCreateName (); };
	const ssi_char_t *getInfo () override { return "Makes each value of each dimension relative to the value before."; };

	void transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) override;
	void transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) override;
	void transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0) override;

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) override
	{
		return  sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) override
	{
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) override
	{
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	Relative(const ssi_char_t *file = 0);
	Relative::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	bool first;

	ssi_real_t *lastPrevVal;
};

}

#endif
