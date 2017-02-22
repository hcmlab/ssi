// MyFeature2.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/05
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

#include "MyFeature2.h"

namespace ssi {

char MyFeature2::ssi_log_name[] = "myfeature2";

MyFeature2::MyFeature2(const ssi_char_t *file) {
}

MyFeature2::~MyFeature2() {
}

void MyFeature2::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void MyFeature2::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	ssi_real_t value = 0;

	for (ssi_size_t i = 0; i < stream_in.dim; i++) {
		value = *ptr_in++;
		ptr_out[i * 2] = value;
		ptr_out[i * 2 + 1] = value;
	}

	for (ssi_size_t i = 1; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			value = *ptr_in++;
			if (value < ptr_out[j * 2]) {
				ptr_out[j * 2] = value;
			}
			else if (value > ptr_out[j * 2 + 1]) {
				ptr_out[j * 2 + 1] = value;
			}
		}
	}
}

void MyFeature2::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}

}
