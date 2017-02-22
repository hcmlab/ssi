// MyFilter2.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/01
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

#ifndef _MYFILTER2_H
#define _MYFILTER2_H

#include "ssi.h"

namespace ssi {

#define MAX_TRIGGER_DUR 5.0

class MyFilter2 : public IFilter {

	class Options : public OptionList {

	public:

		Options()
			: speed(0.05f) {

			addOption("speed", &speed, 1, SSI_REAL, "speed threshold");
		};

		ssi_real_t speed;
	};

public:

	static const ssi_char_t *GetCreateName() { return "MyFilter2"; };
	static IObject *Create(const ssi_char_t *file) { return new MyFilter2(file); };
	~MyFilter2();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "speed filter"; };

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

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return 1;
	}
	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		return sizeof(char);
	}
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_CHAR;
	}

protected:

	MyFilter2(const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static char ssi_log_name[];
	ssi_real_t *_hist;
};

}

#endif
