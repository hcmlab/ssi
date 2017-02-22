#include "OpenfaceHelper.h"

namespace ssi
{
	OpenfaceHelper::OpenfaceHelper(std::vector<std::string> arguments, std::string action_units_location, std::string triangulation_location)
	{
		frame_count = 0;


		det_parameters = new LandmarkDetector::FaceModelParameters(arguments);
		det_parameters->track_gaze = true;
		det_parameters->quiet_mode = true;

		// The modules that are being used for tracking
		clnf_model = new LandmarkDetector::CLNF(det_parameters->model_location);

		// Creating a  face analyser that will be used for AU extraction
		face_analyser = new FaceAnalysis::FaceAnalyser(vector<cv::Vec3d>(), 0.7, 112, 112, action_units_location, triangulation_location);

		bool u;
		vector<string> files, depth_directories, output_video_files, out_dummy;
		LandmarkDetector::get_video_input_output_params(files, depth_directories, out_dummy, output_video_files, u, arguments);


		// Grab camera parameters, if they are not defined (approximate values will be used)
		fx = 0, fy = 0, cx = 0, cy = 0;

		// Get camera parameters
		// By default try webcam 0
		int device = 0;
		LandmarkDetector::get_camera_params(device, fx, fy, cx, cy, arguments);
	}

	OpenfaceHelper::OpenfaceHelper() {
		det_parameters = NULL;
		clnf_model = NULL;
		face_analyser = NULL;
	};

	OpenfaceHelper::~OpenfaceHelper() 
	{

		delete det_parameters;
		delete clnf_model;
		delete face_analyser;
	}

	/*			/////For Tracking/////			*/

	/**
	* Needs to be called every frame so Openface can do its magic.
	* Fills the clnf_model with data needed for all the features of Openface.
	*
	* Returns bool true if landmarks are found (a face was detected)
	*/
	bool OpenfaceHelper::update_landmark(cv::Mat &captured_image)
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

		if (captured_image.channels() == 3)
		{
			cv::cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
		}
		else
		{
			grayscale_image = captured_image.clone();
		}

		// The actual facial landmark detection / tracking
		bool detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, depth_image, *clnf_model, *det_parameters);
		
		return detection_success;
	}

	/**
	* Needs to be called every frame additionally to update_landmark if action units are to be calculated.
	* Fills the Face_Analyser with the new Frame.
	*/
	void OpenfaceHelper::update_faceanalyser(cv::Mat &captured_image, bool video_input, double fps)
	{
		double time_stamp = 0;
		// Grab the timestamp first
		if (video_input)
		{
			time_stamp = (double)frame_count * (1.0 / fps);
		}
		else
		{
			// if loading images assume 30fps
			time_stamp = (double)frame_count * (1.0 / 30.0);
		}

		face_analyser->AddNextFrame(captured_image, *clnf_model, time_stamp, false, false);
		frame_count++;
	}

	/**
	* Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z), 
	* translation is in millimeters with respect to camera centre, 
	* rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign. 
	* The rotation can be either in world or camera coordinates.
	*
	* Returns Vec6d the head pose with respect to camera assuming orthographic projection
	*/
	cv::Vec6d OpenfaceHelper::get_pose_camera()
	{
		return LandmarkDetector::GetPoseCamera(*clnf_model, fx, fy, cx, cy);
	}

	/**
	* Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z),
	* translation is in millimeters with respect to camera centre,
	* rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign.
	* The rotation can be either in world or camera coordinates.
	*
	* Returns Vec6d the head pose w.r.t. world coordinates assuming orthographic projection
	*/
	cv::Vec6d OpenfaceHelper::get_pose_world()
	{
		return LandmarkDetector::GetPoseWorld(*clnf_model, fx, fy, cx, cy);
	}

	/**
	* Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z),
	* translation is in millimeters with respect to camera centre,
	* rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign.
	* The rotation can be either in world or camera coordinates.
	*
	* Returns Vec6d the head pose w.r.t. camera with a perspective camera correction
	*/
	cv::Vec6d OpenfaceHelper::get_corrected_pose_camera()
	{
		return LandmarkDetector::GetCorrectedPoseCamera(*clnf_model, fx, fy, cx, cy);
	}

	/**
	* Head pose is stored in the following format (X, Y, Z, rot_x, roty_y, rot_z),
	* translation is in millimeters with respect to camera centre,
	* rotation is in radians around X,Y,Z axes with the convention R = Rx * Ry * Rz, left-handed positive sign.
	* The rotation can be either in world or camera coordinates.
	*
	* Returns Vec6d the head pose w.r.t. world coordinates with a perspective camera correction
	*/
	cv::Vec6d OpenfaceHelper::get_corrected_pose_world()
	{
		return LandmarkDetector::GetCorrectedPoseWorld(*clnf_model, fx, fy, cx, cy);
	}

	/**
	* Landmarks are facial landmarks numberd from 1 to 68.
	* For a detailed Layout see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Returns vector<cv::Point2d> vector filled with all the facial landmarks as Point2d. Facial landmark location in pixels (x y)
	*/
	vector<cv::Point2d> OpenfaceHelper::get_detected_landmarks()
	{
		int n = clnf_model->detected_landmarks.rows / 2;
		vector<cv::Point2d> landmarks;
		int idx = clnf_model->patch_experts.GetViewIdx(clnf_model->params_global, 0);

		for (int i = 0; i < n; ++i)
		{
			if (clnf_model->patch_experts.visibilities[0][idx].at<int>(i))
			{
				cv::Point2d featurePoint(clnf_model->detected_landmarks.at<double>(i), clnf_model->detected_landmarks.at<double>(i + n));

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
	* The actual output of the regressor(-1 is perfect detection 1 is worst detection)
	*
	* Returns double value of the regressor
	*/
	double OpenfaceHelper::get_detection_certainty()
	{
		return clnf_model->detection_certainty;
	}

	/**
	* Eye gaze direction vector in world coordinates for both eyes (normalized).
	* Vector in format gaze_x, gaze_y, gaze_z
	*
	* Return vector<cv::Point3f> vector containing 2x Point3f representing gace direction vector. vec[0] => gaze_directions_left_eye, vec[1] => gaze_directions_right_eye
	*/
	vector<cv::Point3f> OpenfaceHelper::get_gaze_directions()
	{
		cv::Point3f gazeDirection0(0, 0, -1);
		cv::Point3f gazeDirection1(0, 0, -1);
		vector<cv::Point3f> gazeDirections;

		if (clnf_model->detection_success && clnf_model->eye_model)
		{
			FaceAnalysis::EstimateGaze(*clnf_model, gazeDirection0, fx, fy, cx, cy, true); //left_eye
			FaceAnalysis::EstimateGaze(*clnf_model, gazeDirection1, fx, fy, cx, cy, false); //right_eye
		}

		gazeDirections.push_back(gazeDirection0);
		gazeDirections.push_back(gazeDirection1);
		return gazeDirections;
	}

	/** 
	* Calculates the center of the image
	*
	* Return std::vector<float> vec[0] = cx, vec[1] = cy
	*/
	std::vector<float> OpenfaceHelper::calculate_coi(cv::Mat captured_image)
	{
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
	std::vector<float> OpenfaceHelper::calculate_fl(cv::Mat captured_image)
	{
		std::vector<float> tmp;

		float fx = 500 * (captured_image.cols / 640.0);
		float fy = 500 * (captured_image.rows / 480.0);
		fx = (fx + fy) / 2.0;
		fy = fx;

		tmp.push_back(fx);
		tmp.push_back(fy);

		return tmp;
	}

	//helper for get_pupil_position()
	cv::Point3f GetPupilPosition(cv::Mat_<double> eyeLdmks3d)
	{
		eyeLdmks3d = eyeLdmks3d.t();

		cv::Mat_<double> irisLdmks3d = eyeLdmks3d.rowRange(0, 8);

		cv::Point3f p(mean(irisLdmks3d.col(0))[0], mean(irisLdmks3d.col(1))[0], mean(irisLdmks3d.col(2))[0]);
		return p;
	}

	/**
	* Calculates the pupil position.
	*
	* Return std::vector<cv::Point3f> vector contains 2x Point3f representing Pupilposition x,y,z. vec[0] = pupil_left, vec[1] = pupil_right
	*/
	std::vector<cv::Point3f> OpenfaceHelper::get_pupil_position()
	{
		vector<cv::Point3f> tmp;
		int part_left = -1;
		int part_right = -1;
		for (size_t i = 0; i < clnf_model->hierarchical_models.size(); ++i)
		{
			if (clnf_model->hierarchical_model_names[i].compare("left_eye_28") == 0)
			{
				part_left = i;
			}
			if (clnf_model->hierarchical_model_names[i].compare("right_eye_28") == 0)
			{
				part_right = i;
			}
		}

		cv::Mat eyeLdmks3d_left = clnf_model->hierarchical_models[part_left].GetShape(fx, fy, cx, cy);
		cv::Point3f pupil_left = GetPupilPosition(eyeLdmks3d_left);
		tmp.push_back(pupil_left);

		cv::Mat eyeLdmks3d_right = clnf_model->hierarchical_models[part_right].GetShape(fx, fy, cx, cy);
		cv::Point3f pupil_right = GetPupilPosition(eyeLdmks3d_right);
		tmp.push_back(pupil_right);

		return tmp;
	}

	/**
	* Gets the registered action units from the face_analyser.
	* For more details on action units see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	* !!!! NOTE : According to https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format detected intensity ranges from 0 to 5, but we get negetive values as well!
	*
	* Return std::vector<double> vector contains detected intensity (see above Note for ranges) of all action units. Vector size = 0 if no action unit was detected
	*/
	std::vector<double> OpenfaceHelper::get_current_action_unit_reg() {
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
	* Gets the presens of certain action units from the face_analyser.
	* For more details on action units see https://github.com/TadasBaltrusaitis/OpenFace/wiki/Output-Format
	*
	* Return std::vector<double> vector contains detected classes (0 absent, 1 present) of action units. Vector size = 0 if no action unit was detected
	*/
	std::vector<double> OpenfaceHelper::get_current_action_unit_classes(){
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

	/**
	* Draws the facial landmarks.
	*
	* Return void since facial landmarks are drawn on captured_image directly
	*/
	void OpenfaceHelper::draw_landmarks(cv::Mat& captured_image, vector<cv::Point> landmarks)
	{
		LandmarkDetector::DrawLandmarks(captured_image, landmarks);
	}

	/**
	* Draws a box indicating head direction.
	*
	* Param captured_image the image to draw on
	* Param pose Head pose
	* Param color color of the box lines
	* Param thickness thickness of the box lines
	* Param float fx, float fy focal length
	* Param float cx, float cy center of image
	*
	* Return void since the box is drawn on captured_image directly
	*/
	void OpenfaceHelper::draw_box(cv::Mat captured_image, cv::Vec6d pose, cv::Scalar color, int thickness, float fx, float fy, float cx, float cy)
	{
		LandmarkDetector::DrawBox(captured_image, pose, color, thickness, fx, fy, cx, cy);
	}

	/**
	* Draws two vectores indicating gaze direction.
	*
	* Param img the image to draw on
	* Param gazeVecAxisLeft,gazeVecAxisRight normilized gaze vector
	* Param pupil_left,pupil_right pupil pos
	* Param thickness thickness of the box lines
	* Param float fx, float fy focal length
	* Param float cx, float cy center of image
	*
	* Return void since the vectores are drawn on img directly
	*/
	void OpenfaceHelper::draw_gaze(cv::Mat img, cv::Point3f gazeVecAxisLeft, cv::Point3f gazeVecAxisRight, cv::Point3f pupil_left, cv::Point3f pupil_right, float fx, float fy, float cx, float cy)
	{
		vector<cv::Point3d> points_left;
		points_left.push_back(cv::Point3d(pupil_left));
		points_left.push_back(cv::Point3d(pupil_left + gazeVecAxisLeft*50.0));

		vector<cv::Point3d> points_right;
		points_right.push_back(cv::Point3d(pupil_right));
		points_right.push_back(cv::Point3d(pupil_right + gazeVecAxisRight*50.0));

		cv::Mat_<double> proj_points;
		cv::Mat_<double> mesh_0 = (cv::Mat_<double>(2, 3) << points_left[0].x, points_left[0].y, points_left[0].z, points_left[1].x, points_left[1].y, points_left[1].z);
		LandmarkDetector::Project(proj_points, mesh_0, fx, fy, cx, cy);
		line(img, cv::Point(proj_points.at<double>(0, 0), proj_points.at<double>(0, 1)), cv::Point(proj_points.at<double>(1, 0), proj_points.at<double>(1, 1)), cv::Scalar(110, 220, 0), 2, 8);

		cv::Mat_<double> mesh_1 = (cv::Mat_<double>(2, 3) << points_right[0].x, points_right[0].y, points_right[0].z, points_right[1].x, points_right[1].y, points_right[1].z);
		LandmarkDetector::Project(proj_points, mesh_1, fx, fy, cx, cy);
		line(img, cv::Point(proj_points.at<double>(0, 0), proj_points.at<double>(0, 1)), cv::Point(proj_points.at<double>(1, 0), proj_points.at<double>(1, 1)), cv::Scalar(110, 220, 0), 2, 8);
	}



}