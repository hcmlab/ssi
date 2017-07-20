// HeatMapPainter.cpp
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

#include "HeatMapPainter.h"

namespace ssi {

	HeatMapPainter::HeatMapPainter (const ssi_char_t *file) 
		: _file (0),
		hmWorker (0) {

			if (file) {
				if (!OptionList::LoadXML(file, &_options)) {
					OptionList::SaveXML(file, &_options);
				}
				_file = ssi_strcpy (file);
			}
	}

	HeatMapPainter::~HeatMapPainter () {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void HeatMapPainter::transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

			if (xtra_stream_in_num == 0) {
				ssi_err ("no extra stream found");
			}

			if (xtra_stream_in[0].type != SSI_INT && xtra_stream_in[0].type != SSI_REAL) {
				ssi_err ("type of extra stream has to be int or float");
			}

			hmWorker = new HeatmapWorker(
				_video_format.widthInPixels,
				_video_format.heightInPixels,
				_video_format.flipImage,
				_options.influencradius,
				_options.minimudisplaypercentage,
				_options.horizontalregions,
				_options.verticalregions, 
				_options.decreaseovertime, 
				_options.decreasetick, 
				_options.decreasefactor, 
				_options.smoothen, 
				_options.filterrepetitions);
			hmWorker->start();
	}


	void HeatMapPainter::transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

			float *eyes = 0;
			if (xtra_stream_in[0].type != SSI_INT) {
				ssi_size_t n = xtra_stream_in[0].num * xtra_stream_in[0].dim;
				ssi_real_t *ptr = ssi_pcast (ssi_real_t, xtra_stream_in[0].ptr);
				eyes = new float[n];
				for (ssi_size_t i = 0; i < n; i++) {
					eyes[i] = ssi_cast (float, *ptr++);
				}
			} else {
				eyes = ssi_pcast (float, xtra_stream_in[0].ptr);
			}
			BYTE *image = ssi_pcast (BYTE, stream_in.ptr);
			paint (image, xtra_stream_in[0].num, eyes);  
			memcpy (stream_out.ptr, stream_in.ptr, stream_in.tot);
			if (xtra_stream_in[0].type != SSI_INT) {
				delete[] eyes;
			}
	}

	void HeatMapPainter::transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		hmWorker->stop();
		delete hmWorker; hmWorker = 0;
	}

	void HeatMapPainter::paint (BYTE *image,
		ssi_size_t n_eyes,
		float *eyes) {

			float *ptr = eyes;
			for (ssi_size_t i = 0; i < n_eyes; i++) {
				if (*ptr >= 0 && *(ptr+1) >= 0) {
					hmWorker->increeaseGazePointCount(*ptr*_video_format.widthInPixels, *(ptr+1)*_video_format.heightInPixels);
				}
				ptr += 2;
			}
			paint_heatmap(image);
	}


	void HeatMapPainter::paint_heatmap (BYTE *image) {

		Lock lock(hmWorker->_colorGridMutex);

		int red, green, blue;

		for(int i = 0; i < _video_format.widthInPixels; i++) {
			for(int j = 0; j < _video_format.heightInPixels; j++) {
				red = hmWorker->_colorGrid[i][j*3];
				green = hmWorker->_colorGrid[i][j*3+1];
				blue = hmWorker->_colorGrid[i][j*3+2];
			if(red + green + blue < 3*255)
 					paint_point (image, _video_format, i, j ,2, red , green, blue);
			}
		}
	}

	void HeatMapPainter::paint_point (BYTE *image, 
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

			for (int x = max (0, point_x - border); x < min (width, point_x + border); ++x) {
				for (int y = max (0, point_y - border); y < min (height, point_y + border); ++y) {
					BYTE *pixel = image + 3 * x + y * stride;
					pixel[0] = (float)b_value * _options.opacity + (1-_options.opacity) * (int)pixel[0];
					pixel[1] = (float)g_value * _options.opacity + (1-_options.opacity) * (int)pixel[1];
					pixel[2] = (float)r_value * _options.opacity + (1-_options.opacity) * (int)pixel[2];
				}
			}
	}

}
