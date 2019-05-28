// Openface2.cpp
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

#include "Openface2.h"
#include "Openface2Helper.h"


namespace ssi {

	char Openface2::ssi_log_name[] = "openface2_";

	Openface2::Openface2(const ssi_char_t *file)
		: _file(0),
		_helper(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

	}

	Openface2::~Openface2() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void Openface2::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		vector<string> argumentsModel;
		argumentsModel.push_back(std::string(_options.modelPath));
		std::vector<std::string> argumentsAU;
		argumentsAU.push_back(std::string(_options.AuPath));

		_helper = new Openface2Helper(argumentsModel, argumentsAU);
	}

	void Openface2::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		// prepare output   
		float *out = ssi_pcast(float, stream_out.ptr);
		for (int i = 0; i < stream_out.num*stream_out.dim; i++)
			out[i] = 0.0;

		/*
		*****************************
		*****	landmark Part	*****
		*****************************
		*/

		//init default output Values
		double detection_certainty = -10;
		cv::Vec6d pose = cv::Vec6d(-1, -1, -1, -1, -1, -1);

		vector<cv::Point2d> detected_landmarks;
		for (size_t i = 0; i < 68; i++)
			detected_landmarks.push_back(cv::Point2d(-1, -1));

		vector<cv::Point3f> detected_landmarks_3d;
		for (size_t i = 0; i < 68; i++)
			detected_landmarks_3d.push_back(cv::Point3f(-1, -1, -1));

		vector<cv::Point2f> eye_landmarks;
		for (size_t i = 0; i < 56; i++)
			eye_landmarks.push_back(cv::Point2f(-1, -1));

		vector<cv::Point3f> eye_landmarks_3d;
		for (size_t i = 0; i < 56; i++)
			eye_landmarks_3d.push_back(cv::Point3f(-1, -1, -1));

		vector<cv::Point3f> gaze;
		gaze.push_back(cv::Point3f(-1, -1, -1));
		gaze.push_back(cv::Point3f(-1, -1, -1));

		cv::Vec2f gaze_angle(-1, -1);

		//convert img to Mat so Openface can do its Magic 
		cv::Mat captured_image; 
		if (_video_format.numOfChannels == 3) {
			captured_image = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
		}
		else if (_video_format.numOfChannels == 4) {
			cv::Mat temp = cv::Mat(_video_format.heightInPixels, _video_format.widthInPixels, CV_8UC(_video_format.numOfChannels), stream_in.ptr, ssi_video_stride(_video_format));
			cv::cvtColor(temp, captured_image, CV_BGRA2RGB);
		}

		//update needs to be called every Frame so the internal Model of Openface is up to date. Returns True if a Face was detected
		bool detection_success = _helper->update_landmark(captured_image);
		
		if (detection_success) {
			detection_certainty = _helper->get_detection_certainty(); // 1 = best

			if (_options.pose)
				pose = _helper->get_pose();
			if (_options.landmarks)
				detected_landmarks = _helper->get_detected_landmarks();
			if (_options.landmarks3d)
				detected_landmarks_3d = _helper->get_detected_landmarks_3d();
			if (_options.eye)
				eye_landmarks = _helper->get_eye_landmarks();
			if (_options.eye3d)
				eye_landmarks_3d = _helper->get_eye_landmarks_3d();
			if (_options.gaze) {
				gaze = _helper->get_gaze_directions(); // helper[0] => gaze_directions_left_eye, helper[1] => gaze_directions_right_eye
				gaze_angle = _helper->get_gaze_angle(gaze[0], gaze[1]);
			}
		}
		
		//Fill output stream with Features regarding DETECTION 
		out[FEATURE::DETECTION_SUCCESS] = (float)detection_success;
		out[FEATURE::DETECTION_CERTAINTY] = (float)detection_certainty;

		//POSE_CAMERA_X is the First Value of all Features regarding POSE so it works as an offset
		for (int i = 0; i < 6; i++) //a Vec6d has only 6 elements
			out[FEATURE::POSE_X + i] = (float)pose[i];
		
		//Fill output stream with Features regarding FACIAL_LANDMARK
		for (int j = 0; j < detected_landmarks.size(); j++) { //FACIAL_LANDMARK_1_X is the First Value of all Features regarding FACIAL_LANDMARK so it works as an offset
			for (int i = 0; i < 2; i++) {//a Point2d has only 2 elements
				if (i == 0)
					out[FEATURE::FACIAL_LANDMARK_1_X + j * 2 + i] = (float)detected_landmarks[j].x;
				else
					out[FEATURE::FACIAL_LANDMARK_1_X + j * 2 + i] = (float)detected_landmarks[j].y;
			}
		}

		//Fill output stream with Features regarding FACIAL_LANDMARK_3D
		for (int j = 0; j < detected_landmarks_3d.size(); j++) { //FACIAL_LANDMARK_3D_1_X is the First Value of all Features regarding FACIAL_LANDMARK_3D so it works as an offset
			for (int i = 0; i < 3; i++) {//a Point3d has only 3 elements
				if (i == 0)
					out[FEATURE::FACIAL_LANDMARK_3D_1_X + j * 3 + i] = (float)detected_landmarks_3d[j].x;
				else if (i == 1)
					out[FEATURE::FACIAL_LANDMARK_3D_1_X + j * 3 + i] = (float)detected_landmarks_3d[j].y;
				else
					out[FEATURE::FACIAL_LANDMARK_3D_1_X + j * 3 + i] = (float)detected_landmarks_3d[j].z;
			}
		}

		//Fill output stream with Features regarding EYE_LANDMARK
		for (int j = 0; j < eye_landmarks.size(); j++) { //EYE_LANDMARK_1_X is the First Value of all Features regarding EYE_LANDMARK so it works as an offset
			for (int i = 0; i < 2; i++) {//a Point2d has only 2 elements
				if (i == 0)
					out[FEATURE::EYE_LANDMARK_1_X + j * 2 + i] = (float)eye_landmarks[j].x;
				else
					out[FEATURE::EYE_LANDMARK_1_X + j * 2 + i] = (float)eye_landmarks[j].y;
			}
		}

		//Fill output stream with Features regarding EYE_LANDMARK_3D
		for (int j = 0; j < eye_landmarks_3d.size(); j++) { //EYE_LANDMARK_3D_1_X is the First Value of all Features regarding EYE_LANDMARK_3D so it works as an offset
			for (int i = 0; i < 3; i++) {//a Point3d has only 3 elements
				if (i == 0)
					out[FEATURE::EYE_LANDMARK_3D_1_X + j * 3 + i] = (float)eye_landmarks_3d[j].x;
				else if (i == 1)
					out[FEATURE::EYE_LANDMARK_3D_1_X + j * 3 + i] = (float)eye_landmarks_3d[j].y;
				else
					out[FEATURE::EYE_LANDMARK_3D_1_X + j * 3 + i] = (float)eye_landmarks_3d[j].z;
			}
		}

		//Fill output stream with Features regarding GAZE_DIRECTION
		for (int j = 0; j < gaze.size(); j++) { //GAZE_LEFT_EYE_X is the First Value of all Features regarding GAZE_DIRECTION so it works as an offset
			for (int i = 0; i < 3; i++) {//a Point3f has only 3 elements
				if (i==0)
					out[FEATURE::GAZE_LEFT_EYE_X + j * 3 + i] = (float)gaze[j].x;
				else if (i==1)
					out[FEATURE::GAZE_LEFT_EYE_X + j * 3 + i] = (float)gaze[j].y;
				else
					out[FEATURE::GAZE_LEFT_EYE_X + j * 3 + i] = (float)gaze[j].z;
			}
		}

		out[FEATURE::GAZE_ANGLE_X] = gaze_angle[0];
		out[FEATURE::GAZE_ANGLE_Y] = gaze_angle[1];

		/*
		*********************************
		*****	FaceanAlyser Part	*****
		*********************************
		*/
		vector<double> au_reg;
		for (int i = FEATURE::AU01_r; i < FEATURE::AU_CLASS_DETECTION_SUCCESS; i++)
			au_reg.push_back(-1.0);

		vector<double> au_classes;
		for (int i = FEATURE::AU01_c; i < FEATURE::NUM; i++)
			au_classes.push_back(-1.0);

		double au_reg_success = 0.0;
		double au_classes_success = 0.0;

		if (_options.actionunits) {
			_helper->update_faceanalyser(captured_image, true, _video_format.framesPerSecond);
			
			vector<double> au_reg_tmp = _helper->get_current_action_unit_reg();
			if (au_reg_tmp.size() > 0) {
				au_reg = au_reg_tmp;
				au_reg_success = 1.0;
			}

			vector<double> au_classes_tmp = _helper->get_current_action_unit_classes();
			if (au_classes_tmp.size() > 0) {
				au_classes = au_classes_tmp;
				au_classes_success = 1.0;
			}

		}
		out[FEATURE::AU_REG_DETECTION_SUCCESS] = au_reg_success;
		//Fill output stream with Features regarding ACTION_UNIT_REG
		for (int j = 0; j < au_reg.size(); j++) { //AU01_r is the First Value of all Features regarding ACTION_UNIT_REG so it works as an offset
			out[FEATURE::AU01_r + j] = (float) au_reg[j];
		}

		out[FEATURE::AU_CLASS_DETECTION_SUCCESS] = au_classes_success;
		//Fill output stream with Features regarding ACTION_UNIT_CLASS
		for (int j = 0; j < au_classes.size(); j++) { //AU04_c is the First Value of all Features regarding ACTION_UNIT_CLASS so it works as an offset
			out[FEATURE::AU01_c + j] = (float) au_classes[j];
		}
		
	}

	void Openface2::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		delete _helper; _helper = 0;
	}


}
