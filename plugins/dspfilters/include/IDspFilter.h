// IDspFilter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/26
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

Provides butter low/high/band-pass filter

*/

#pragma once

#ifndef SSI_SIGNAL_IDSPFILTER_H
#define SSI_SIGNAL_IDSPFILTER_H

#include "base/IFilter.h"
#include "DspFiltersTools.h"

namespace ssi {

class IDspFilter : public IFilter {

public:

	virtual ~IDspFilter ();
	virtual Dsp::Filter *createFilter (DspFiltersTools::FilterProperty &prop) = 0;

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
			return SSI_UNDEF;
		}
		return SSI_REAL;
	}

	virtual void response (ssi_time_t sr,
		ssi_size_t n,
		ssi_real_t *impulse,
		bool clear = true);

	virtual void IDspFilter::gain (ssi_time_t sr,
		ssi_size_t n, 
		ssi_real_t *gain);

	virtual const char *info () {
		return DspFiltersTools::GetInfo (_prop);
	}

protected:

	IDspFilter ();
	static ssi_char_t *ssi_log_name;
		
	ssi_size_t _n_channels;
	Dsp::Filter **_filters;
	DspFiltersTools::FilterProperty _prop;	
	ssi_size_t _n_per_channel;
	ssi_real_t **_channels;	
	bool _offset;
	ssi_real_t *_offsets;

	void separate (ssi_size_t n, const ssi_real_t *data);
	void join (ssi_size_t n, ssi_real_t *data);
};

}

#endif
