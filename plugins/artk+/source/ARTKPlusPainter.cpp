// ARTKPlusPainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/29
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

#include "ARTKPlusPainter.h"

namespace ssi {

ARTKPlusPainter::ARTKPlusPainter (const ssi_char_t *file) 
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}
ARTKPlusPainter::~ARTKPlusPainter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void ARTKPlusPainter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	SSI_ASSERT (xtra_stream_in_num > 0);
}

void ARTKPlusPainter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	ARTKPlusTools::marker_s *ss = ssi_pcast (ARTKPlusTools::marker_s, xtra_stream_in[0].ptr);
	BYTE *image = ssi_pcast (BYTE, stream_in.ptr);
	for (ssi_size_t i = 0; i < xtra_stream_in[0].dim; i++) {
		paint (image, ss[i]);  
	}
	memcpy (stream_out.ptr, stream_in.ptr, stream_in.tot);
}

void ARTKPlusPainter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void ARTKPlusPainter::paint (BYTE *image,
	ARTKPlusTools::marker_s &s) {

	if ( !s.visible ) {
		return;
	}

	int cx = ssi_cast (int, s.scaled ? s.center.x * _video_format.widthInPixels : s.center.x);
	int cy = ssi_cast (int, s.scaled ? s.center.y * _video_format.heightInPixels : s.center.y);

	paint_point (image, 
		_video_format, 
		cx, 
		cy, 
		2, 
		255, 
		0, 
		0);

	for (int i = 0; i < 4; i++) {

		int vx = ssi_cast (int, s.scaled ? s.vertex[i].x * _video_format.widthInPixels : s.vertex[i].x);
		int vy = ssi_cast (int, s.scaled ? s.vertex[i].y * _video_format.heightInPixels : s.vertex[i].y);

		paint_point (image, 
			_video_format,
			vx, 
			vy, 
			2, 
			0, 
			0, 
			255);
	}
}

void ARTKPlusPainter::paint_point (BYTE *image, 
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
