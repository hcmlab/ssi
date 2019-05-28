// Openface2Painter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>, Björn Bittner <bittner@hcm-lab.de>
// created: 14/6/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "Openface2Painter.h"
#include "Openface2Helper.h"
#include "Openface2.h"


namespace ssi {

	char Openface2Painter::ssi_log_name[] = "__________";

	Openface2Painter::Openface2Painter(const ssi_char_t *file)
		: _file(0),
		_helper(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

	}

	Openface2Painter::~Openface2Painter() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}


	void Openface2Painter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		_helper = new Openface2Helper();

	}

	void Openface2Painter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		cv::Mat captured_image = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));

		_helper->update_image(captured_image);

		float *xtra_in = ssi_pcast(float, xtra_stream_in[0].ptr);
		cv::Mat_<float> landmarks;

		// get input
		BYTE *in = ssi_pcast(BYTE, stream_in.ptr);

		// prepare output
		BYTE *out = ssi_pcast(BYTE, stream_out.ptr);

		if (xtra_in[Openface2::FEATURE::DETECTION_SUCCESS]) {
			for (size_t i = Openface2::FEATURE::FACIAL_LANDMARK_1_X; i <= Openface2::FEATURE::FACIAL_LANDMARK_68_X; i+=2) {
				landmarks.push_back(xtra_in[i]);
			}
			for (size_t i = Openface2::FEATURE::FACIAL_LANDMARK_1_Y; i <= Openface2::FEATURE::FACIAL_LANDMARK_68_Y; i+=2) {
				landmarks.push_back(xtra_in[i]);
			}
			_helper->draw_landmarks(landmarks, xtra_in[Openface2::FEATURE::DETECTION_CERTAINTY]);

			cv::Vec6d pose = cv::Vec6d((double)xtra_in[Openface2::FEATURE::POSE_X],
									   (double)xtra_in[Openface2::FEATURE::POSE_Y],
									   (double)xtra_in[Openface2::FEATURE::POSE_Z],
									   (double)xtra_in[Openface2::FEATURE::POSE_ROT_X],
									   (double)xtra_in[Openface2::FEATURE::POSE_ROT_Y],
									   (double)xtra_in[Openface2::FEATURE::POSE_ROT_Z]);
			_helper->draw_pose(pose, xtra_in[Openface2::FEATURE::DETECTION_CERTAINTY]);

			cv::Point3f left_eye(xtra_in[Openface2::FEATURE::GAZE_LEFT_EYE_X],
								 xtra_in[Openface2::FEATURE::GAZE_LEFT_EYE_Y],
								 xtra_in[Openface2::FEATURE::GAZE_LEFT_EYE_Z]);

			cv::Point3f right_eye(xtra_in[Openface2::FEATURE::GAZE_RIGHT_EYE_X],
								  xtra_in[Openface2::FEATURE::GAZE_RIGHT_EYE_Y],
								  xtra_in[Openface2::FEATURE::GAZE_RIGHT_EYE_Z]);

			std::vector<cv::Point2f> eye_landmarks2d;
			std::vector<cv::Point3f> eye_landmarks3d;

			for (size_t i = Openface2::FEATURE::EYE_LANDMARK_1_X; i <= Openface2::FEATURE::EYE_LANDMARK_56_X; i+=2) {
				eye_landmarks2d.push_back(cv::Point2f(xtra_in[i], xtra_in[i+1]));
			}

			for (size_t i = Openface2::FEATURE::EYE_LANDMARK_3D_1_X; i <= Openface2::FEATURE::EYE_LANDMARK_3D_56_X; i+=3) {
				eye_landmarks3d.push_back(cv::Point3f(xtra_in[i], xtra_in[i+1], xtra_in[i+2]));
			}

			_helper->draw_gaze(left_eye, right_eye, eye_landmarks2d, eye_landmarks3d, xtra_in[Openface2::FEATURE::DETECTION_CERTAINTY]);

			cv::Mat image = _helper->get_image();
			memcpy(out, image.data, image.cols * image.rows * image.channels() * sizeof(ssi_byte_t));
		} else {
			// copy input to output
			for (int j = 0; j < _video_format.heightInPixels; j++) { // Sample Schleife

				for (int i = 0; i < _video_format.widthInPixels*_video_format.numOfChannels; i++) // 2 Dimensionen pro Sample i=0 => x; i=1 => y
				{
					out[i] = in[i];
				}
				out += _stride;
				in += _stride;
			}
		}

	}

	void Openface2Painter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		delete _helper; _helper = 0;
	}


}
