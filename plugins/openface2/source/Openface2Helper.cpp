// Openface2Helper.cpp
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

#include "Openface2Helper.h"

namespace ssi
{
	Openface2Helper::Openface2Helper(std::vector<std::string> argumentsModel, std::vector<std::string> argumentsAU) {
		frame_count = 0;

		det_parameters = new LandmarkDetector::FaceModelParameters(argumentsModel);

		// The modules that are being used for tracking
		face_model = new LandmarkDetector::CLNF(det_parameters->model_location);
		if (!face_model->loaded_successfully) {
			ssi_err("ERROR: Could not load the landmark detector");
		}

		// Creating a  face analyser that will be used for AU extraction
		FaceAnalysis::FaceAnalyserParameters face_analysis_params(argumentsAU);

		//face_analyser = new FaceAnalysis::FaceAnalyser(vector<cv::Vec3d>(), 0.7, 112, 112, action_units_location, triangulation_location);
		face_analyser = new FaceAnalysis::FaceAnalyser(face_analysis_params);

		if (!face_model->eye_model) {
			ssi_wrn("WARNING: no eye model found");
		}

		if (face_analyser->GetAUClassNames().size() == 0 && face_analyser->GetAUClassNames().size() == 0) {
			ssi_wrn("WARNING: no Action Unit models found");
		}

		visualizer = NULL;

		bool u;
		vector<string> files, depth_directories, output_video_files, out_dummy;
		//LandmarkDetector::get_video_input_output_params(files, depth_directories, out_dummy, output_video_files, u, arguments);


		// Grab camera parameters, if they are not defined (approximate values will be used)
		fx = 0, fy = 0, cx = 0, cy = 0;

		// Get camera parameters
		// By default try webcam 0
		int device = 0;
		//LandmarkDetector::get_camera_params(device, fx, fy, cx, cy, arguments);
	}

	Openface2Helper::Openface2Helper() {
		det_parameters = NULL;
		face_model = NULL;
		face_analyser = NULL;
		vector<string> arguments;
		visualizer = new Utilities::Visualizer(arguments);
	};

	Openface2Helper::~Openface2Helper()
	{

		delete det_parameters;
		delete face_model;
		delete face_analyser;
		delete visualizer;
	}

	/*			/////For Tracking/////			*/

	/**
	* Needs to be called every frame so Openface can do its magic.
	* Fills the clnf_model with data needed for all the features of Openface.
	*
	* Returns bool true if landmarks are found (a face was detected)
	*/
	bool Openface2Helper::update_landmark(cv::Mat &captured_image)
	{
		std::vector<float> coi = calculate_coi(captured_image); // center of image
		std::vector<float> fl = calculate_fl(captured_image); // focal length

		// If optical centers are not defined just use center of image
		cx = coi[0];
		cy = coi[1];

		// Use a rough guess-timate of focal length
		fx = fl[0];
		fy = fl[1];

		// Reading the images
		cv::Mat_<float> depth_image;
		cv::Mat_<uchar> grayscale_image;

		if (captured_image.channels() == 3) {
			cv::cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
		} else {
			grayscale_image = captured_image.clone();
		}

		// The actual facial landmark detection / tracking
		bool detection_success = LandmarkDetector::DetectLandmarksInVideo(captured_image, *face_model, *det_parameters, grayscale_image);

		return detection_success;
	}

	/**
	* Needs to be called every frame additionally to update_landmark if action units are to be calculated.
	* Fills the Face_Analyser with the new Frame.
	*/
	void Openface2Helper::update_faceanalyser(cv::Mat &captured_image, bool video_input, double fps)
	{
		double time_stamp = 0;
		// Grab the timestamp first
		if (video_input) {
			time_stamp = (double)frame_count * (1.0 / fps);
		} else {
			// if loading images assume 30fps
			time_stamp = (double)frame_count * (1.0 / 30.0);
		}

		face_analyser->AddNextFrame(captured_image, face_model->detected_landmarks, face_model->detection_success, time_stamp, false);

		frame_count++;
	}

	/**
	* Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z),
	* translation is in millimeters with respect to camera centre,
	* rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign.
	*
	* Returns Vec6d the head pose w.r.t. world coordinates assuming orthographic projection
	*/
	cv::Vec6d Openface2Helper::get_pose()
	{
		return LandmarkDetector::GetPose(*face_model, fx, fy, cx, cy);
	}

	/**
	* Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to perspective projection
	* This method returns a corrected pose estimate with respect to a point camera (NOTE not the world coordinates), which is useful to find out if the person is looking at a camera
	* The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	**/

	cv::Vec6d Openface2Helper::get_pose_corrected()
	{
		return LandmarkDetector::GetPoseWRTCamera(*face_model, fx, fy, cx, cy);
	}


	/**
	* Landmarks are facial landmarks numbered from 1 to 68.
	* For a detailed Layout see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Returns vector<cv::Point2d> vector filled with all the facial landmarks as Point2d. Facial landmark location in pixels (x y)
	*/
	vector<cv::Point2d> Openface2Helper::get_detected_landmarks()
	{
		int n = face_model->detected_landmarks.rows / 2;
		vector<cv::Point2d> landmarks;
		int idx = face_model->patch_experts.GetViewIdx(face_model->params_global, 0);

		for (int i = 0; i < n; ++i)
		{
			if (face_model->patch_experts.visibilities[0][idx].at<int>(i))
			{
				cv::Point2d featurePoint(face_model->detected_landmarks.at<float>(i), face_model->detected_landmarks.at<float>(i + n));

				landmarks.push_back(featurePoint);
			}
			else
			{
				cv::Point2d featurePoint(-1, -1);
				landmarks.push_back(featurePoint);
			}
		}

		return landmarks;
	}

	/**
	* Landmarks are facial landmarks numbered from 1 to 68.
	* For a detailed Layout see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Returns vector<cv::Point3f> vector filled with all the facial landmarks as Point3f. Facial landmark location in mm (x y z)
	*/
	vector<cv::Point3f> Openface2Helper::get_detected_landmarks_3d()
	{
		cv::Mat_<float> landmarks3d = face_model->GetShape(fx, fy, cx, cy);

		vector<cv::Point3f> tmp;

		for (int i = 0; i < landmarks3d.cols; ++i) {
			cv::Point3f featurePoint(landmarks3d.at<float>(0, i), landmarks3d.at<float>(1, i), landmarks3d.at<float>(2, i));
			tmp.push_back(featurePoint);
		}

		return tmp;
	}

	/**
	* Landmarks are eye landmarks numbered from 1 to 56.
	* For a detailed Layout see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Returns vector<cv::Point2d> vector filled with all the eye landmarks as Point2d. Eye landmark location in pixels (x y)
	*/
	vector<cv::Point2f> Openface2Helper::get_eye_landmarks()
	{
		return LandmarkDetector::CalculateAllEyeLandmarks(*face_model);
	}

	/**
	* Landmarks are eye landmarks numbered from 1 to 56.
	* For a detailed Layout see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Returns vector<cv::Point3f> vector filled with all the eye landmarks as Point3f. Eye landmark location in mm (x y z)
	*/
	vector<cv::Point3f> Openface2Helper::get_eye_landmarks_3d()
	{
		return LandmarkDetector::Calculate3DEyeLandmarks(*face_model, fx,  fy,  cx,  cy);
	}

	/**
	* The actual output of the regressor(-1 is perfect detection 1 is worst detection)
	*
	* Returns double value of the regressor
	*/
	double Openface2Helper::get_detection_certainty() {
		return face_model->detection_certainty;
	}

	/**
	* Eye gaze direction vector in world coordinates for both eyes (normalized).
	* Vector in format gaze_x, gaze_y, gaze_z
	*
	* Return vector<cv::Point3f> vector containing 2x Point3f representing gaze direction vector. vec[0] => gaze_directions_left_eye, vec[1] => gaze_directions_right_eye
	*/
	vector<cv::Point3f> Openface2Helper::get_gaze_directions() {

		cv::Point3f gazeDirection0(0, 0, 0);
		cv::Point3f gazeDirection1(0, 0, 0);
		//cv::Vec2d gazeAngle(0, 0);
		vector<cv::Point3f> gazeDirections;

		if (face_model->detection_success && face_model->eye_model)
		{
			GazeAnalysis::EstimateGaze(*face_model, gazeDirection0, fx, fy, cx, cy, true); //left eye
			GazeAnalysis::EstimateGaze(*face_model, gazeDirection1, fx, fy, cx, cy, false); // right eye
			//gazeAngle = GazeAnalysis::GetGazeAngle(gazeDirection0, gazeDirection1);
		}

		gazeDirections.push_back(gazeDirection0);
		gazeDirections.push_back(gazeDirection1);
		//gazeDirections.push_back(gazeAngle);
		return gazeDirections;
	}

	/**
	* Eye gaze angle vector in world coordinates for both eyes.
	* Vector in format angle_x, angle_y
	*
	* Return cv::Vec2f
	*/
	cv::Vec2f Openface2Helper::get_gaze_angle(cv::Point3f gazeDirection0, cv::Point3f gazeDirection1) {
		return GazeAnalysis::GetGazeAngle(gazeDirection0, gazeDirection1);
	}

	/** 
	* Calculates the center of the image
	*
	* Return std::vector<float> vec[0] = cx, vec[1] = cy
	*/
	std::vector<float> Openface2Helper::calculate_coi(cv::Mat captured_image) {
		std::vector<float> tmp;

		tmp.push_back(captured_image.cols / 2.0f); // cx
		tmp.push_back(captured_image.rows / 2.0f); // cy

		return tmp;
	}

	/**
	* Calculates focal length. Currently only uses a rough guess-timate of focal length
	*
	* Return std::vector<float> vec[0] = fx, vec[1] = fy
	*/
	std::vector<float> Openface2Helper::calculate_fl(cv::Mat captured_image) {
		std::vector<float> tmp;

		float fx = 500 * (captured_image.cols / 640.0);
		float fy = 500 * (captured_image.rows / 480.0);
		fx = (fx + fy) / 2.0;
		fy = fx;

		tmp.push_back(fx);
		tmp.push_back(fy);

		return tmp;
	}

	/**
	* Gets the registered action units from the face_analyser.
	* For more details on action units see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	* !!!! NOTE : According to https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format detected intensity ranges from 0 to 5, but we get negative values as well!
	*
	* Return std::vector<double> vector contains detected intensity (see above Note for ranges) of all action units. Vector size = 0 if no action unit was detected
	*/
	std::vector<double> Openface2Helper::get_current_action_unit_reg() {
		auto aus_reg = face_analyser->GetCurrentAUsReg();

		vector<string> au_reg_names = face_analyser->GetAURegNames();
		std::sort(au_reg_names.begin(), au_reg_names.end());

		std::vector<double> retVec;

		// write out ar the correct index
		for (string au_name : au_reg_names)
		{
			for (auto au_reg : aus_reg)
			{
				if (au_name.compare(au_reg.first) == 0)
				{
					retVec.push_back(au_reg.second);
					break;
				}
			}
		}

		return retVec;
	}

	/**
	* Gets the presence of certain action units from the face_analyser.
	* For more details on action units see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Return std::vector<double> vector contains detected classes (0 absent, 1 present) of action units. Vector size = 0 if no action unit was detected
	*/
	std::vector<double> Openface2Helper::get_current_action_unit_classes() {
		auto aus_class = face_analyser->GetCurrentAUsClass();

		vector<string> au_class_names = face_analyser->GetAUClassNames();
		std::sort(au_class_names.begin(), au_class_names.end());

		std::vector<double> retVec;

		// write out ar the correct index
		for (string au_name : au_class_names)
		{
			for (auto au_class : aus_class)
			{
				if (au_name.compare(au_class.first) == 0)
				{
					retVec.push_back(au_class.second);
					break;
				}
			}
		}
		return retVec;
	}

	/*			/////For Drawing/////			*/

	void Openface2Helper::update_image(cv::Mat img) {
		std::vector<float> coi = calculate_coi(img); // center of image
		std::vector<float> fl = calculate_fl(img); // focal length
		visualizer->SetImage(img, fl[0], fl[1], coi[0], coi[1]);
	}

	/**
	* Draws the facial landmarks.
	*
	* Return void since facial landmarks are drawn on captured_image directly
	*/
	void Openface2Helper::draw_landmarks(cv::Mat_<float> landmarks, double confidence) {
		cv::Mat_<int> visibilities; // empty visibilities to draw all landmarks
		visualizer->SetObservationLandmarks(landmarks, confidence, visibilities);
	}

	/**
	* Draws the head pose.
	*
	* Return void since head pose is drawn on captured_image directly
	*/
	void Openface2Helper::draw_pose(cv::Vec6f pose, double confidence) {
		visualizer->SetObservationPose(pose, confidence);
	}

	/**
	* Draws two vectors indicating gaze direction.
	*
	* Param gaze_direction0 normalized gaze vector left
	* Param gaze_direction1 normalized gaze vector right
	* Param eye_landmarks2d eye landmarks 2d
	* Param eye_landmarks3d eye landmarks 3d
	* Param float confidence
	*
	* Return void since the vectors are drawn on img directly
	*/
	void Openface2Helper::draw_gaze(cv::Point3f gaze_direction0, cv::Point3f gaze_direction1, std::vector<cv::Point2f> eye_landmarks2d, std::vector<cv::Point3f> eye_landmarks3d, double confidence) {
		visualizer->SetObservationGaze(gaze_direction0, gaze_direction1, eye_landmarks2d, eye_landmarks3d, confidence);
	}

	cv::Mat Openface2Helper::get_image() {
		cv::Mat image = visualizer->GetVisImage();
		return image;
	}

}