// Openface2Helper.h
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
#pragma once


// Libraries for landmark detection (includes CLNF and CLM modules)
#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"
#include "FaceAnalyser.h"
#include "Visualizer.h"

#include <fstream>
#include <sstream>
#include "base/Factory.h"

// OpenCV includes
#include <opencv2/opencv.hpp>

// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

namespace ssi {

	class Openface2Helper
	{

	public:

		Openface2Helper(std::vector<std::string> argumentsModel, std::vector<std::string> argumentsAU);
		Openface2Helper();
		~Openface2Helper();

		/*			/////For Tracking/////			*/
		bool update_landmark(cv::Mat &captured_image);
		void update_faceanalyser(cv::Mat &captured_image, bool video_input, double fps);
		cv::Vec6d get_pose();
		cv::Vec6d get_pose_corrected();
		std::vector<cv::Point2d> get_detected_landmarks();
		std::vector<cv::Point3f> get_detected_landmarks_3d();
		std::vector<cv::Point2f> get_eye_landmarks();
		std::vector<cv::Point3f> get_eye_landmarks_3d();
		double get_detection_certainty();
		std::vector<cv::Point3f> get_gaze_directions();
		cv::Vec2f get_gaze_angle(cv::Point3f gazeDirection0, cv::Point3f gazeDirection1);
		std::vector<float> calculate_coi(cv::Mat captured_image);
		std::vector<float> calculate_fl(cv::Mat captured_image);
		std::vector<double> get_current_action_unit_reg();
		std::vector<double> get_current_action_unit_classes();

		/*			/////For Drawing/////			*/
		void update_image(cv::Mat img);
		void draw_landmarks(cv::Mat_<float> landmarks, double confidence);
		void draw_pose(cv::Vec6f pose, double confidence);
		void draw_gaze(cv::Point3f gaze_direction0, cv::Point3f gaze_direction1, std::vector<cv::Point2f> eye_landmarks2d, std::vector<cv::Point3f> eye_landmarks3d, double confidence);
		cv::Mat get_image();


		// Some globals for tracking timing information for visualisation
		int frame_count;
		float fx, fy, cx, cy;

		LandmarkDetector::FaceModelParameters *det_parameters;
		// The modules that are being used for tracking
		LandmarkDetector::CLNF *face_model;

		// Creating a face analyser that will be used for AU extraction
		FaceAnalysis::FaceAnalyser *face_analyser;

		// Creating a visualizer that will be used for painting
		Utilities::Visualizer *visualizer;

	};

}
