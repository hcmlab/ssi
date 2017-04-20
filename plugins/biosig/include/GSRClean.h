// GSRClean.h
// author: Frank Gaibler <gaibler@hcm-lab.de>
// created: 2017/01/17
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
#ifndef _GSRCLEAN_H
#define _GSRCLEAN_H

#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"

using namespace ssi;

class GSRClean : public ssi::ITransformer {

public:
	class Options : public OptionList {
	public:
		Options() : threshold(5.f) {
			addOption("threshold", &threshold, 1, SSI_REAL, "threshold for cleanup");
		}
		ssi_real_t threshold;
	};

	static const ssi_char_t *GetCreateName() { return "ssi_feature_GSRClean"; };
	static IObject *Create(const ssi_char_t *file) { return new GSRClean(file); };
	~GSRClean();
	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "cleanup gsr"; };

	void transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleNumberOut(ssi_size_t sample_number_in) {
		return sample_number_in;
	}

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return sample_dimension_in;
	}

	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		return sizeof(ssi_real_t);
	}

	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		return SSI_REAL;
	}

protected:
	GSRClean(const ssi_char_t *file = 0);
	Options _options;
	static char ssi_log_name[];

private:
	bool init = true;
	ssi_real_t valLast = 0;
	ssi_real_t valLastOut = 0;
};
#endif

