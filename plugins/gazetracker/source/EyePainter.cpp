// EyePainter.cpp
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

#include "EyePainter.h"
#include "EyeTracker.h"
#include <ssiocv.h>


namespace ssi {

	ssi_char_t *EyePainter::ssi_log_name = "eyepaint___";

EyePainter::EyePainter(const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

EyePainter::~EyePainter() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EyePainter::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	SSI_ASSERT (xtra_stream_in_num > 0);

	
}

void EyePainter::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	BYTE *image = ssi_pcast (BYTE, stream_in.ptr);
	ssi_real_t *detectionResults = ssi_pcast (ssi_real_t, xtra_stream_in[0].ptr);	

	cv::Mat img(cvSize(_video_format.widthInPixels, _video_format.heightInPixels), CV_8UC3, (void*)image);

	double fontSize = 1.25;

	if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_E_TOO_MANY_PUPIL_CANDIDATES){
		cv::putText(img, "TOO MANY PUPIL CANDIDATES", cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(255, 0, 0), 1, CV_AA);
	}
	else if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_E_NO_PUPIL_CANDIDATE){
		cv::putText(img, "NO PUPIL CANDIDATES", cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(255, 0, 0), 1, CV_AA);
	}
	else if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_W_NO_GLINT_CANDIDATE){
		cv::putText(img, "NO GLINT CANDIDATE", cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(255, 255, 0), 1, CV_AA);
	}
	else if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_W_TOO_MANY_GLINT_CANDIDATES){
		cv::putText(img, "TOO MANY GLINT CANDIDATES", cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(255, 255, 0), 1, CV_AA);
	}
	else if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_W_NO_SUITABLE_GLINT_CANDIDATE){
		cv::putText(img, "NO SUITABLE GLINT CANDIDATE", cv::Point2d(0, 16), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(255, 255, 0), 1, CV_AA);
	}
	
	if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] >= 0){ // only OK or warnings

		//draw pupil

		float scale = (float)_video_format.heightInPixels / (float)SSI_EYETRACKER_STANDARD_HEIGHT;

		cv::Point2f pupilCenter = cv::Point2f(
			detectionResults[SSI_EYETRACKER_RESULT_PUPIL_X] * scale, 
			detectionResults[SSI_EYETRACKER_RESULT_PUPIL_Y] * scale);

		cv::Size2f pupilSize = cv::Point2d(
			detectionResults[SSI_EYETRACKER_RESULT_PUPIL_WIDTH] * scale, 
			detectionResults[SSI_EYETRACKER_RESULT_PUPIL_HEIGHT] * scale);

		float pupilAngle = detectionResults[SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE];

		cv::RotatedRect rr = cv::RotatedRect(pupilCenter, pupilSize, pupilAngle);
		

		cv::ellipse(img, rr, CV_RGB(0, 255, 128), 2, CV_AA);

		int lineLength = detectionResults[SSI_EYETRACKER_RESULT_PUPIL_AVERAGERADIUS] * scale / 2;
		if (lineLength <= 0){
			lineLength = (int)(std::min(pupilSize.width, pupilSize.height) / 4);
		}
		
		cv::line(img, cv::Point2f(rr.center.x, rr.center.y - lineLength), cv::Point2f(rr.center.x, rr.center.y + lineLength), CV_RGB(0, 255, 128), 1, CV_AA);
		cv::line(img, cv::Point2f(rr.center.x - lineLength, rr.center.y), cv::Point2f(rr.center.x + lineLength, rr.center.y), CV_RGB(0, 255, 128), 1, CV_AA);


		if (detectionResults[SSI_EYETRACKER_RESULT_STATUS] == SSI_EYETRACKER_PUPIL_DETECTION_OK){
			//draw glint

			cv::Point2f glintCenter = cv::Point2f(detectionResults[SSI_EYETRACKER_RESULT_GLINT_X], detectionResults[SSI_EYETRACKER_RESULT_GLINT_Y]);
			int glintRadius = (int)detectionResults[SSI_EYETRACKER_RESULT_GLINT_RADIUS];

			cv::circle(img, glintCenter, glintRadius, CV_RGB(192, 192, 0), 1, CV_AA);
		}


		if (detectionResults[SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE] > 0){
			char confStr[20];
			sprintf(confStr, "CONFIDENCE: %i%%", (int) ((detectionResults[SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE] * 100.0f) + 0.5f));
			cv::putText(img, confStr, cv::Point2d(0, _video_format.heightInPixels - 8), cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(0, 255, 0), 1, CV_AA);

		}
		


	}

	memcpy (stream_out.ptr, stream_in.ptr, stream_in.tot);
}

void EyePainter::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}




}
