// Noise.h
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

#pragma once

#ifndef SSI_SIGNAL_NOISE_H
#define SSI_SIGNAL_NOISE_H

#include "base/IFilter.h"
#include "signal/SignalTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Noise : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: ampl (1.0), mean (0.0), stdv (1.0), norm (true), cutoff (0), width (0) {

			addOption ("ampl", &ampl, 1, SSI_REAL, "noise amplitude");		
			addOption ("mean", &mean, 1, SSI_DOUBLE, "noise mean");		
			addOption ("stdv", &stdv, 1, SSI_DOUBLE, "noise standard deviation");		
			addOption ("norm", &norm, 1, SSI_BOOL, "frequency values are normalized in interval [0..1], where 1 is the nyquist frequency (=half the sample rate)");
			addOption ("cutoff", &cutoff, 1, SSI_DOUBLE, "if > 0: cutoff frequency either in hz or normalized (see -norm)");
			addOption ("width", &width, 1, SSI_DOUBLE, "if > 0: band width either in hz or normalized (see -norm)");		
		};

		ssi_real_t ampl;
		double mean;
		double stdv;
		bool norm;
		double cutoff;
		double width;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Noise"; };
	static IObject *Create (const ssi_char_t *file) { return new Noise (file); };
	~Noise ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Adds normally distributed noise in certain frequency range to input stream."; };

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

	Noise (const ssi_char_t *file = 0);
	Noise::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
};

}

#endif
