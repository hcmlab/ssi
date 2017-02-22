// MvgMedian.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/12
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_SIGNAL_MVGMEDIAN_H
#define SSI_SIGNAL_MVGMEDIAN_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MvgMedianHelper;

class MvgMedian : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () 
			: win (1.0), winInSamples (0) {

			addOption ("win", &win, 1, SSI_TIME, "size of moving/sliding window in seconds");	
			addOption ("winInSamples", &winInSamples, 1, SSI_SIZE, "size of moving/sliding window in samples (overrides win)");	
		};

		ssi_size_t winInSamples;
		ssi_time_t win;
	};

public:

	static const ssi_char_t *GetCreateName () { return "MvgMedian"; };
	static IObject *Create (const ssi_char_t *file) { return new MvgMedian (file); };
	~MvgMedian ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes median of the input stream for a chosen window."; };

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
		return sizeof (ssi_real_t);
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	MvgMedian (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	bool _first_call;
	MvgMedianHelper **_median;
	ssi_size_t _nwin;
	ssi_size_t _ndim;
};

}

#endif
