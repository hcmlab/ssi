// MyFeature.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
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

#include "MyFeature.h"

namespace ssi {

char MyFeature::ssi_log_name[] = "myfeature_";

MyFeature::MyFeature(const ssi_char_t *file) {
}

MyFeature::~MyFeature() {
}

void MyFeature::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void MyFeature::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);

	for (ssi_size_t i = 0; i < stream_in.dim; i++) {
		ptr_out[i] = 0;
	}
	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			ptr_out[j] += *ptr_in++;
		}
	}
	for (ssi_size_t i = 0; i < stream_in.dim; i++) {
		ptr_out[i] /= stream_in.num;
	}
}

void MyFeature::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}

}
