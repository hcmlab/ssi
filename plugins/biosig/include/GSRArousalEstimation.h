// GSRArousalEstimation.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/15
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

#ifndef SSI_BIOSIG_GSRAROUSALESTIMATION_H
#define SSI_BIOSIG_GSRAROUSALESTIMATION_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class GSRArousalEstimation : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () : winsize_shortterm (5.0), winsize_longterm (60.0) {
			addOption ("winsize_shortterm", &winsize_shortterm, 1, SSI_REAL, "size in seconds of shorttime window");
			addOption ("winsize_longterm", &winsize_longterm, 1, SSI_REAL, "size in seconds of longtime window");
		}
		ssi_time_t winsize_shortterm;
		ssi_time_t winsize_longterm;
	};

public:

	static const ssi_char_t *GetCreateName () { return "GSRArousalEstimation"; };
	static IObject *Create (const ssi_char_t *file) { return new GSRArousalEstimation (file); };
	~GSRArousalEstimation ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Estimates arousal value [0..1] from gsr signal"; };

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

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return 1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	GSRArousalEstimation (const ssi_char_t *file = 0);
	GSRArousalEstimation::Options _options;
	ssi_char_t *_file;

	ITransformer *_detrend;
	ssi_stream_t _detrend_stream;
	ITransformer *_detrend_norm_minmax;
	ssi_stream_t _detrend_norm_minmax_stream;
	ssi_stream_t _detrend_norm_minmax_stream_copy;
	ITransformer *_mvgavg_longterm;
	ssi_stream_t _mvgavg_longterm_stream;
	ITransformer *_butfilt_longterm;
	ssi_stream_t _butfilt_longterm_stream;
	ITransformer *_mvgavg_shortterm;
	ssi_stream_t _mvgavg_shortterm_stream;
	ITransformer *_combination;
	ssi_stream_t _combination_stream;
	ITransformer *_mvgavg_combination;
	ssi_stream_t _mvgavg_combination_stream;

};

}

#endif
