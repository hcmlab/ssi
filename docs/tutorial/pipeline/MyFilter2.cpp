// MyFilter2.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/02
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

#include "MyFilter2.h"

namespace ssi {

char MyFilter2::ssi_log_name[] = "myfilter2_";

MyFilter2::MyFilter2(const ssi_char_t *file)
	: _hist(0),
	_file(0) {

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}
}

MyFilter2::~MyFilter2() {

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

void MyFilter2::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void MyFilter2::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	char *ptr_out = ssi_pcast(char, stream_out.ptr);

	if (!_hist) {
		_hist = new ssi_real_t[stream_in.dim];
		for (ssi_size_t i = 0; i < stream_in.dim; i++) {
			_hist[i] = ptr_in[i];
		}
	}

	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		bool result = false;
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			result = result || (abs(_hist[j] - *ptr_in) > _options.speed);
			_hist[j] = *ptr_in++;
		}
		*ptr_out++ = result ? 1 : 0;
	}
}

void MyFilter2::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _hist; _hist = 0;
	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}

}
