// GazePointShifter.cpp
// author: Daniel Schork
// created: 2015
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

#include "GazePointShifter.h"
#include "SceneTracker.h"
#include <ssiocv.h>
#include "opencv2/highgui/highgui.hpp"

namespace ssi {

	
	ssi_char_t *GazePointShifter::ssi_log_name = "gpshift___";

	GazePointShifter::GazePointShifter(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	GazePointShifter::~GazePointShifter() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}


	const char* GPSsliderWindowName = "Gaze Point Shift";

	const char* shiftXSliderName = "l <---> r";
	const char* shiftYSliderName = "u <---> d";
	
	int _sx_range = 600;
	int _sy_range = 600;

	int _shiftX = _sx_range /2;
	int _shiftY = _sy_range /2;

	cv::Mat shift_img;
	cv::Scalar gray(100, 100, 100);
	cv::Scalar green(0, 200, 0);

	bool _lmb_down = false;

	void ClickCallback(int event, int x, int y, int flags, void* userdata)
	{
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			_lmb_down = true;
			_shiftX = x;
			_shiftY = y;
		}
		else if (event == cv::EVENT_LBUTTONUP)
		{
			_lmb_down = false;
		}
		else if (event == cv::EVENT_RBUTTONDOWN)
		{
			_shiftX = _sx_range / 2;
			_shiftY = _sy_range / 2;
		}
		else if (event == cv::EVENT_MOUSEMOVE)
		{
			if (_lmb_down) {
				_shiftX = x;
				_shiftY = y;
			}

		}
	}

	void GazePointShifter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		SSI_ASSERT(xtra_stream_in_num > 0);

		if (_options.sliders) {

			cv::namedWindow(GPSsliderWindowName);
			cv::resizeWindow(GPSsliderWindowName, 800, 100);
			cv::moveWindow(GPSsliderWindowName, 645, 480);

			cv::createTrackbar(shiftXSliderName, GPSsliderWindowName, &_shiftX, _sy_range);
			cv::createTrackbar(shiftYSliderName, GPSsliderWindowName, &_shiftY, _sx_range);
		}
		else {

			cv::namedWindow(GPSsliderWindowName, 1);
			cv::moveWindow(GPSsliderWindowName, 1280, 0);

			cv::setMouseCallback(GPSsliderWindowName, ClickCallback, NULL);

			shift_img = cv::Mat::zeros(_sx_range, _sy_range, CV_8UC3);
			cv::line(shift_img, cv::Point(_sx_range / 2, 0), cv::Point(_sx_range / 2, _sy_range - 1), gray, 1, CV_AA);
			cv::line(shift_img, cv::Point(0, _sy_range / 2), cv::Point(_sx_range - 1, _sy_range / 2), gray, 1, CV_AA);
			cv::putText(shift_img, "RIGHT CLICK RESETS", cv::Point2d(0, 48), cv::FONT_HERSHEY_PLAIN, 1.0, gray);
			
		}

	}

	void GazePointShifter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_real_t *in = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *out = ssi_pcast(ssi_real_t, stream_out.ptr);

		
		for (int i = 0; i < SSI_SCENETRACKER_RESULTS_SIZE; i++) {
			
			if (i == SSI_SCENETRACKER_GAZEPOINT_X) {
				out[i] = in[i] + ((float)_shiftX - (float)_sx_range / 2.0f) * _options.scale;
			}
			else if (i == SSI_SCENETRACKER_GAZEPOINT_Y) {
				out[i] = in[i] + ((float)_shiftY - (float)_sx_range / 2.0f) * _options.scale;
			}
			else {
				out[i] = in[i];
			}
		}

		if (!_options.sliders) {

			cv::Mat shift_img_tmp = shift_img.clone();
			cv::circle(shift_img_tmp, cv::Point(_shiftX, _shiftY), 6, green, 2, CV_AA);


			int sx = (int)(((float)_shiftX - (float)_sx_range / 2.0f) * _options.scale);
			int sy = (int)(((float)_shiftY - (float)_sy_range / 2.0f) * _options.scale);

			std::string str_shiftX = "SHIFT X: " + std::to_string(sx);
			std::string str_shiftY = "SHIFT Y: " + std::to_string(sy);

			cv::putText(shift_img_tmp, str_shiftX.c_str(), cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, 1.0, green);
			cv::putText(shift_img_tmp, str_shiftY.c_str(), cv::Point2d(0, 32), cv::FONT_HERSHEY_PLAIN, 1.0, green);

			cv::imshow(GPSsliderWindowName, shift_img_tmp);
		}

		cv::waitKey(1);
		
	}

	void GazePointShifter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}
	
}
