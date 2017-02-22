// GSRArtifactFilter.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/05/08
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

#ifndef SSI_FILTER_GSRARTIFACTFILTER_H
#define SSI_FILTER_GSRARTIFACTFILTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class GSRArtifactFilter : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () : winsize (0.2), replacement (0.2), lower_range (0.0), upper_range (1.0), thres (4.0) {
			addOption ("winsize", &winsize, 1, SSI_REAL, "...");
			addOption ("replacement", &replacement, 1, SSI_REAL, "...");
			addOption ("lower_range", &lower_range, 1, SSI_REAL, "...");
			addOption ("upper_range", &upper_range, 1, SSI_REAL, "...");
			addOption ("thres", &thres, 1, SSI_REAL, "...");
		}
		ssi_time_t winsize;
		ssi_time_t replacement;
		ssi_real_t lower_range;
		ssi_real_t upper_range;
		ssi_real_t thres;

	};

public:

	static const ssi_char_t *GetCreateName () { return "GSRArtifactFilter"; };
	static IObject *Create (const ssi_char_t *file) { return new GSRArtifactFilter (file); };
	~GSRArtifactFilter ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Filters artifacts from GSR signal."; };

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
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	};
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	};

	void mark_samples(ITransformer::info &info, ssi_size_t n_sample, ssi_real_t* sample_buffer);
	void interpolate(ITransformer::info &info, ssi_real_t* sample_buffer);

protected:

	GSRArtifactFilter (const ssi_char_t *file = 0);
	GSRArtifactFilter::Options _options;
	ssi_char_t *_file;

	ssi_size_t _samples_to_interpolate;
	ssi_real_t _last_valid_value;

	ssi_real_t _offset;

	bool interpolating;
	bool firstrun;
	
};

}

#endif
