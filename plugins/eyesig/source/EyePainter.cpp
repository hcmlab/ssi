// EyePainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#include "EyePainter.h"

namespace ssi {

EyePainter::EyePainter (const ssi_char_t *file) 
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

EyePainter::~EyePainter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EyePainter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (xtra_stream_in_num == 0) {
		ssi_err ("no extra stream found");
	}

	if (xtra_stream_in[0].type != SSI_INT && xtra_stream_in[0].type != SSI_REAL) {
		ssi_err ("type of extra stream has to be int or float");
	}
}

void EyePainter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	int *eyes = 0;
	if (xtra_stream_in[0].type != SSI_INT) {
		ssi_size_t n = xtra_stream_in[0].num * xtra_stream_in[0].dim;
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, xtra_stream_in[0].ptr);
		eyes = new int[n];
		for (ssi_size_t i = 0; i < n; i++) {
			eyes[i] = ssi_cast (int, *ptr++);
		}
	} else {
		eyes = ssi_pcast (int, xtra_stream_in[0].ptr);
	}
	BYTE *image = ssi_pcast (BYTE, stream_in.ptr);
	paint (image, xtra_stream_in[0].num, eyes);  
	memcpy (stream_out.ptr, stream_in.ptr, stream_in.tot);
	if (xtra_stream_in[0].type != SSI_INT) {
		delete[] eyes;
	}
}

void EyePainter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void EyePainter::paint (BYTE *image,
	ssi_size_t n_eyes,
	int *eyes) {

	int *ptr = eyes;
	for (ssi_size_t i = 0; i < n_eyes; i++) {
		if (*ptr >= 0 && *(ptr+1) >= 0) {
			paint_point (image, 
				_video_format, 
				*ptr, 
				*(ptr+1), 
				10, 
				0, 
				255, 
				0);
		}
		ptr += 2;
	}
}

void EyePainter::paint_point (BYTE *image, 
	ssi_video_params_t &params, 
	int point_x, 
	int point_y, 
	int border, 
	int r_value, 
	int g_value, 
	int b_value) {

	int stride = ssi_video_stride (params);
	int width = params.widthInPixels;
	int height = params.heightInPixels;
	point_y = params.flipImage ? point_y : height - point_y;

	for (int x = max (0, point_x - border); x < min (width, point_x + border); ++x) {
		for (int y = max (0, point_y - border); y < min (height, point_y + border); ++y) {
			BYTE *pixel = image + 3 * x + y * stride;
			pixel[0] = b_value;
			pixel[1] = g_value;
			pixel[2] = r_value;
		}
	}
}

}
