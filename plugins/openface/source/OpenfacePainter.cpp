// OpenfacePainter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#include "OpenfacePainter.h"
#include "OpenfaceHelper.h"
#include "Openface.h"


namespace ssi {

	char OpenfacePainter::ssi_log_name[] = "__________";

	OpenfacePainter::OpenfacePainter(const ssi_char_t *file)
		: _file(0),
		_helper(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}

	}

	OpenfacePainter::~OpenfacePainter() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}


	void OpenfacePainter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		_helper = new OpenfaceHelper();
	}

	void OpenfacePainter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		cv::Mat captured_image = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
		
		float *xtra_in = ssi_pcast(float, xtra_stream_in[0].ptr); 
		std::vector<cv::Point> landmarks; 
		
		double visualisation_boundary = 0.2;

		if (xtra_in[Openface::FEATURE::DETECTION_SUCCESS] && xtra_in[Openface::FEATURE::DETECTION_CERTAINTY] < visualisation_boundary)
		{
			for (size_t i = Openface::FEATURE::FACIAL_LANDMARK_1_X; i < Openface::FEATURE::FACIAL_LANDMARK_68_Y; i+=2)
			{
				cv::Point tmp = cv::Point(xtra_in[i], xtra_in[i+1]);
				landmarks.push_back(tmp);
			}
			// If optical centers are not defined just use center of image
			std::vector<float> coi = _helper->calculate_coi(captured_image); // center of image
			// Use a rough guess-timate of focal length
			std::vector<float> fl = _helper->calculate_fl(captured_image); // focal length

			cv::Vec6d pose = cv::Vec6d((double)xtra_in[Openface::FEATURE::CORRECTED_POSE_WORLD_X], 
				(double)xtra_in[Openface::FEATURE::CORRECTED_POSE_WORLD_Y], 
				(double)xtra_in[Openface::FEATURE::CORRECTED_POSE_WORLD_Z],
				(double)xtra_in[Openface::FEATURE::CORRECTED_POSE_CAMERA_ROT_X],
				(double)xtra_in[Openface::FEATURE::CORRECTED_POSE_CAMERA_ROT_Y], 
				(double)xtra_in[Openface::FEATURE::CORRECTED_POSE_CAMERA_ROT_Z]);

			
			double vis_certainty = (double)xtra_in[Openface::FEATURE::DETECTION_CERTAINTY];
			if (vis_certainty > 1)
				vis_certainty = 1;
			if (vis_certainty < -1)
				vis_certainty = -1;

			vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);

			// A rough heuristic for box around the face width
			int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);


			_helper->draw_box(captured_image, pose, cv::Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fl[0], fl[1], coi[0], coi[1]);
			_helper->draw_landmarks(captured_image, landmarks);
			_helper->draw_gaze(captured_image,
				cv::Point3f(xtra_in[Openface::FEATURE::GAZE_LEFT_EYE_X], xtra_in[Openface::FEATURE::GAZE_LEFT_EYE_Y], xtra_in[Openface::FEATURE::GAZE_LEFT_EYE_Z]),
				cv::Point3f(xtra_in[Openface::FEATURE::GAZE_RIGHT_EYE_X], xtra_in[Openface::FEATURE::GAZE_RIGHT_EYE_Y], xtra_in[Openface::FEATURE::GAZE_LEFT_EYE_Z]),
				cv::Point3f(xtra_in[Openface::FEATURE::PUPIL_LEFT_EYE_X], xtra_in[Openface::FEATURE::PUPIL_LEFT_EYE_Y], xtra_in[Openface::FEATURE::PUPIL_LEFT_EYE_Z]),
				cv::Point3f(xtra_in[Openface::FEATURE::PUPIL_RIGHT_EYE_X], xtra_in[Openface::FEATURE::PUPIL_RIGHT_EYE_Y], xtra_in[Openface::FEATURE::PUPIL_RIGHT_EYE_Z]),
				fl[0],
				fl[1],
				coi[0],
				coi[1]
				);
		}


		// get input
		BYTE *in = ssi_pcast(BYTE, stream_in.ptr);

		// prepare output   
		BYTE *out = ssi_pcast(BYTE, stream_out.ptr);

		for (int j = 0; j < _video_format.heightInPixels; j++) { // Sample Schleife

			for (int i = 0; i < _video_format.widthInPixels*_video_format.numOfChannels; i++) // 2 Dimensionen pro Sample i=0 => x; i=1 => y
			{
				out[i] = in[i];
			}
			out += _stride;
			in += _stride;
		}
	}

	void OpenfacePainter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		delete _helper; _helper = 0;
	}


}
