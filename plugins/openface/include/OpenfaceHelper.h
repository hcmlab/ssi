#pragma once


// Libraries for landmark detection (includes CLNF and CLM modules)
#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"
#include "FaceAnalyser.h"

#include <fstream>
#include <sstream>

// OpenCV includes
#include <ssiocv.h>

// Boost includes
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

namespace ssi {

	class OpenfaceHelper
	{

	public:

		OpenfaceHelper(std::vector<std::string> arguments, std::string action_units_location, std::string triangulation_location);
		OpenfaceHelper();
		~OpenfaceHelper();

		/*			/////For Tracking/////			*/
		bool OpenfaceHelper::update_landmark(cv::Mat &captured_image);
		void OpenfaceHelper::update_faceanalyser(cv::Mat &captured_image, bool video_input, double fps);
		cv::Vec6d OpenfaceHelper::get_pose_camera();
		cv::Vec6d OpenfaceHelper::get_pose_world();
		cv::Vec6d OpenfaceHelper::get_corrected_pose_camera();
		cv::Vec6d OpenfaceHelper::get_corrected_pose_world();
		std::vector<cv::Point2d> OpenfaceHelper::get_detected_landmarks();
		double OpenfaceHelper::get_detection_certainty();
		std::vector<cv::Point3f> OpenfaceHelper::get_gaze_directions();
		std::vector<float> OpenfaceHelper::calculate_coi(cv::Mat captured_image);
		std::vector<float> OpenfaceHelper::calculate_fl(cv::Mat captured_image);
		std::vector<cv::Point3f> OpenfaceHelper::get_pupil_position();
		std::vector<double> OpenfaceHelper::get_current_action_unit_reg();
		std::vector<double> OpenfaceHelper::get_current_action_unit_classes();

		/*			/////For Drawing/////			*/
		void OpenfaceHelper::draw_landmarks(cv::Mat& captured_image, vector<cv::Point> landmarks);
		void OpenfaceHelper::draw_box(cv::Mat captured_image, cv::Vec6d pose, cv::Scalar color, int thickness, float fx, float fy, float cx, float cy);
		void OpenfaceHelper::draw_gaze(cv::Mat img, cv::Point3f gazeVecAxisLeft, cv::Point3f gazeVecAxisRight, cv::Point3f pupil_left, cv::Point3f pupil_right, float fx, float fy, float cx, float cy);

		// Some globals for tracking timing information for visualisation
		int frame_count;
		float fx, fy, cx, cy;

		LandmarkDetector::FaceModelParameters *det_parameters;
		// The modules that are being used for tracking
		LandmarkDetector::CLNF *clnf_model;

		// Creating a  face analyser that will be used for AU extraction
		FaceAnalysis::FaceAnalyser *face_analyser;

	};

}