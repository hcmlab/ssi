// SceneTracker.cpp
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

//#include <ssiocv.h>
#include <opencv2\opencv.hpp>

#include <algorithm>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include "SceneTracker.h"
#include "EyeTracker.h"
#include "thread/Lock.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	ssi_char_t *SceneTracker::ssi_log_name = "scenetrack___";
	bool mouseThreadStarted = false;

	//header
	std::vector<cv::Point2f>* _calibrationEyePoints;
	std::vector<cv::Point2f>* _calibrationScenePoints;
	float latestSceneGazePoint[] = { SSI_SCENETRACKER_NO_VALUE, SSI_SCENETRACKER_NO_VALUE };
	std::vector<cv::Point2f> currentFeatures, prevFeatures, tmpFeatures;
	uchar oldErrByte = 0;
	cv::Point2f currentHeadMovePixels(0, 0);
	cv::Point2f currentHeadMoveDegrees(0, 0);
	cv::Point2f totalHeadMove(0, 0);

	SceneTracker::SceneTracker(const ssi_char_t *file)
		: _video_stride(0),
		_video_image(0),
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	SceneTracker::~SceneTracker() {

		if (_video_image) {
			cvReleaseImageHeader(&_video_image);
		}

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void SceneTracker::setVideoFormat(ssi_video_params_t format) {

		_video_format = format;
		_video_stride = ssi_video_stride(format);
		_video_image = cvCreateImageHeader(cvSize(_video_format.widthInPixels, _video_format.heightInPixels), _video_format.depthInBitsPerChannel, _video_format.numOfChannels);
	}

	void SceneTracker::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		SSI_ASSERT(xtra_stream_in_num > 0);
		

		_calibrationStatus = SSI_CALIBRATIONSTATUS_NOT_CALIBRATED;
		_calibrationEyePoints = new std::vector<cv::Point2f>;
		_calibrationScenePoints = new std::vector<cv::Point2f>;

		if (_options.preciseCalibration){
			_calibrationResult = new cv::Mat(6, 2, CV_64F);
		}
		else{
			_calibrationResult = new cv::Mat();
		}

		_sceneFrameRgb = new cv::Mat();
		_sceneFrame = new cv::Mat();
		_prevSceneFrame = new cv::Mat();
		
		_mouseListener = new MouseListener(_options.calibrationWindow, _video_format);
		mouseThreadStarted = true;

	}

	void SceneTracker::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {


		int tStartTime = ssi_time_ms();

		cvSetData(_video_image, stream_in.ptr, _video_stride);

		*_sceneFrameRgb = cv::cvarrToMat(_video_image);

		cvtColor(*_sceneFrameRgb, *_sceneFrame, CV_RGB2GRAY);


		ssi_real_t *pupil = ssi_pcast(ssi_real_t, xtra_stream_in[0].ptr);

		//handle left click
		if (_calibrationStatus != SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED){

			float leftClick[2];
			std::memcpy(leftClick, _mouseListener->getLeftMouseClick(), 2 * sizeof(float));
			_mouseListener->setLeftMouseClickInvalid();

			if (leftClick[0] != SSI_SCENETRACKER_NO_MOUSE_CLICK){

				if (pupil[SSI_EYETRACKER_RESULT_STATUS] < 0){ //no errors
					ssi_print("[st] Error: could not add calibration point, no pupil!\n");
				}
				else{
					_calibrationEyePoints->push_back(cv::Point2f(pupil[SSI_EYETRACKER_RESULT_PUPIL_X], pupil[SSI_EYETRACKER_RESULT_PUPIL_Y]));
					_calibrationScenePoints->push_back(cv::Point2f(leftClick[0], leftClick[1]));

					if (!_options.preciseCalibration && _calibrationEyePoints->size() < SSI_SCENETRACKER_CALIBRATIONPOINTS_HOMOGRAPHY){
						_calibrationStatus = SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_HOMOGRAPHY;
					}
					else if (_options.preciseCalibration && _calibrationEyePoints->size() < SSI_SCENETRACKER_CALIBRATIONPOINTS_POLYNOMIAL){
						_calibrationStatus = SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_POLYNOMIAL;
					}
					else{
						calibrate();
						ssi_print("[st] Calibration finished\n");
					}
				}
			}
		}

		//handle right click
		float rightClick[2];
		std::memcpy(rightClick, _mouseListener->getRightMouseClick(), 2 * sizeof(float));
		_mouseListener->setRightMouseClickInvalid();

		if (rightClick[0] != SSI_SCENETRACKER_NO_MOUSE_CLICK){
			resetCalibration();
			ssi_print("[st] Calibration reset\n");
		}
		
		
		if (_calibrationStatus == SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED || _calibrationStatus == SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED_NO_PUPIL){

			if (pupil[SSI_EYETRACKER_RESULT_STATUS] >= 0){ //no errors
				
				mapToScene(pupil[SSI_EYETRACKER_RESULT_PUPIL_X], pupil[SSI_EYETRACKER_RESULT_PUPIL_Y], latestSceneGazePoint);
				_calibrationStatus = SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED;
			}
			else{
				_calibrationStatus = SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED_NO_PUPIL;
			}

		}

		if (_options.trackHeadMovement){
			headTracking();
		}


		ssi_real_t *dst = ssi_pcast(ssi_real_t, stream_out.ptr);

		*dst++ = (float)_calibrationStatus;

		if (_calibrationStatus == SSI_CALIBRATIONSTATUS_NOT_CALIBRATED){

			*dst++ = SSI_SCENETRACKER_NO_VALUE; //eye
			*dst++ = SSI_SCENETRACKER_NO_VALUE;

			if (_options.trackHeadMovement){
				*dst++ = currentHeadMoveDegrees.x; //head
				*dst++ = currentHeadMoveDegrees.y;
			}
			else{
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING; //head
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING;
			}

			for (int i = SSI_SCENETRACKER_CALIBRATIONRESULTS_START; i < SSI_SCENETRACKER_RESULTS_SIZE; i++){
				*dst++ = SSI_SCENETRACKER_NO_VALUE;
			}

		}
		else if (_calibrationStatus == SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_HOMOGRAPHY || _calibrationStatus == SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_POLYNOMIAL){

			*dst++ = SSI_SCENETRACKER_NO_CALIBRATIONPOINT;
			*dst++ = SSI_SCENETRACKER_NO_CALIBRATIONPOINT;

			if (_options.trackHeadMovement){
				*dst++ = currentHeadMoveDegrees.x;
				*dst++ = currentHeadMoveDegrees.y;
			}
			else{
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING;
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING;
			}

			for (int i = 0; i < _calibrationScenePoints->size(); i++){
				*dst++ = _calibrationScenePoints->at(i).x;
				*dst++ = _calibrationScenePoints->at(i).y;
			}
		}
		else if (_calibrationStatus == SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED || _calibrationStatus == SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED_NO_PUPIL){
			
			if (_video_format.heightInPixels == SSI_SCENETRACKER_STANDARD_HEIGHT){
				*dst++ = latestSceneGazePoint[0];
				*dst++ = latestSceneGazePoint[1];
			}
			else{
				*dst++ = latestSceneGazePoint[0] * (float)SSI_SCENETRACKER_STANDARD_HEIGHT / (float)_video_format.heightInPixels;
				*dst++ = latestSceneGazePoint[1] * (float)SSI_SCENETRACKER_STANDARD_HEIGHT / (float)_video_format.heightInPixels;
			}
			

			if (_options.trackHeadMovement){
				*dst++ = currentHeadMoveDegrees.x;
				*dst++ = currentHeadMoveDegrees.y;
			}
			else{
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING;
				*dst++ = SSI_SCENETRACKER_NO_HEADTRACKING;
			}

			for (int i = SSI_SCENETRACKER_CALIBRATIONRESULTS_START; i < SSI_SCENETRACKER_RESULTS_SIZE; i++){
				*dst++ = SSI_SCENETRACKER_NO_VALUE;
			}

		}

		if (_options.printTimes){
			int totalTime = ssi_time_ms() - tStartTime;
			ssi_print("[scenetracker] time:%3ims\n", totalTime);
		}

	}

	void SceneTracker::calibrate() {

		bool openCV3stillAlpha = true; // set to false when opencv 3 matrix multiplcation works on all platforms

		if (_options.preciseCalibration){
			
			cv::Mat A = cv::Mat(9, 6, CV_64F);
			double row[6];
			double X;
			double Y;
			
			for (int i = 0; i < 9; i++)
			{

				X = _calibrationEyePoints->at(i).x;
				Y = _calibrationEyePoints->at(i).y;

				row[0] = 1;
				row[1] = X;
				row[2] = Y;
				row[3] = X * Y;
				row[4] = X * X;
				row[5] = Y * Y;

				for (int r = 0; r < 6; r++)
				{
					A.at<double>(i, r) = row[r];
				}
			}

			//solve least squares

			int M = A.rows;
			int N = A.cols;

			cv::Mat B = cv::Mat(9, 2, CV_64F);

			for (int i = 0; i < 9; i++)
			{
				B.at<double>(i, 0) = (double) _calibrationScenePoints->at(i).x;
				B.at<double>(i, 1) = (double) _calibrationScenePoints->at(i).y;
			}

			cv::Mat U = cv::Mat(M, M, CV_64F);
			cv::Mat W = cv::Mat(M, N, CV_64F);

			cv::Mat Vt = cv::Mat(N, N, CV_64F);
			cv::Mat V = cv::Mat(N, N, CV_64F);

			cv::Mat c = cv::Mat(M, 2, CV_64F);
			cv::Mat y = cv::Mat(N, 2, CV_64F);


			//http://docs.opencv.org/master/df/df7/classcv_1_1SVD.html
			cv::SVD::compute(A, W, U, Vt, cv::SVD::FULL_UV);
			V = Vt.t();
			
			//transpose and multiply
			cv::Mat Ut = U.t();
			
			if (openCV3stillAlpha){ //opencv 3.1 cant handle matrix multiplication yet, so we have to do this by hand
				c.setTo(0);
				for (int row1 = 0; row1 < Ut.rows; row1++) {
					for (int col2 = 0; col2 < B.cols; col2++) {
						for (int col1 = 0; col1 < Ut.cols; col1++) {
							c.at<double>(row1, col2) += Ut.at<double>(row1, col1) * B.at<double>(col1, col2);
						}
					}
				}
			}
			else{
				c = Ut * B;
			}
			

			for (int i = 0; i < N; i++)
			{
				y.at<double>(i, 0) = c.at<double>(i, 0) / W.at<double>(i, 0); //new SVD
				y.at<double>(i, 1) = c.at<double>(i, 1) / W.at<double>(i, 0); //new SVD
			}

			if (openCV3stillAlpha){
				_calibrationResult->setTo(0);
				for (int row1 = 0; row1 < V.rows; row1++) {
					for (int col2 = 0; col2 < y.cols; col2++) {
						for (int col1 = 0; col1 < V.cols; col1++) {
							(*_calibrationResult).at<double>(row1, col2) += V.at<double>(row1, col1) * y.at<double>(col1, col2);
						}
					}
				}
			}
			else{
				*_calibrationResult = V * y;
			}


		}
		else{
			*_calibrationResult = cv::findHomography(*_calibrationEyePoints, *_calibrationScenePoints, 0, 0);
		}

		_calibrationStatus = SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED;

		
	}

	void SceneTracker::mapToScene(float eyeX, float eyeY, float* sceneGazePoint){

		if (_options.preciseCalibration){

			cv::Mat A = cv::Mat(1, 6, CV_64F);

			A.at<double>(0, 0) = 1;
			A.at<double>(0, 1) = eyeX;
			A.at<double>(0, 2) = eyeY;
			A.at<double>(0, 3) = eyeX * eyeY;
			A.at<double>(0, 4) = eyeX * eyeX;
			A.at<double>(0, 5) = eyeY * eyeY;

			sceneGazePoint[0] = (float) A.dot(_calibrationResult->col(0).t());
			sceneGazePoint[1] = (float) A.dot(_calibrationResult->col(1).t());


		}
		else{

			std::vector<cv::Point2f> pupilArr;
			std::vector<cv::Point2f> sceneArr;
			pupilArr.push_back(cv::Point2f(eyeX, eyeY));

			cv::perspectiveTransform(pupilArr, sceneArr, *_calibrationResult);

			sceneGazePoint[0] = sceneArr[0].x;
			sceneGazePoint[1] = sceneArr[0].y;

		}
	}

	void SceneTracker::resetCalibration() {
		_calibrationEyePoints->clear();
		_calibrationScenePoints->clear();

		_calibrationStatus = SSI_CALIBRATIONSTATUS_NOT_CALIBRATED;
	}

	void SceneTracker::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		if (mouseThreadStarted){
			_mouseListener->disconnect();
			_mouseListener->stop();
		}


		delete _sceneFrameRgb;
		delete _sceneFrame;
		delete _prevSceneFrame;
		delete _calibrationEyePoints;

	}

	void SceneTracker::headTracking(){

		int maxFeatures = 50;
		double qLevel = 0.0001;
		double minDist = 10;
				
		std::vector<uchar> status;
		std::vector<float> err;

		//re-detect GFTT
		if (prevFeatures.size() < (maxFeatures / 2)){

			// Obtain new set of features
			cv::goodFeaturesToTrack(*_sceneFrame, // the image 
				tmpFeatures,   // the output detected features
				maxFeatures,  // the maximum number of features 
				qLevel,     // quality level
				minDist     // min distance between two features
				);

			//add to leftover keypoints from last time
			for (int i = 0; i < tmpFeatures.size(); i++){
				prevFeatures.push_back(tmpFeatures.at(i));
			}

		}
		
		if (_prevSceneFrame->cols > 0 && _prevSceneFrame->rows > 0 && prevFeatures.size() > 0 ){
			
			// Find position of feature in new image
			cv::calcOpticalFlowPyrLK(
				*_prevSceneFrame,
				*_sceneFrame, // 2 consecutive images
				prevFeatures, // input point positions in first im
				currentFeatures, // output point positions in the 2nd
				status,    // tracking success
				err, // tracking error
				cv::Size(21, 21), 3, cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 30, (0.01)), //leave this alone
				0 //flags
				);

			//paint
			
			double maxErr = -999;
			for (int i = 0; i < err.size(); i++){
				
				if (err.at(i) > maxErr){
					maxErr = err.at(i);
				}
			}
			//ssi_print("err min/max: %.2lf, %.2lf\n", minErr, maxErr);
			
			int minSize = MIN(currentFeatures.size(), prevFeatures.size());
			currentHeadMovePixels.x = 0;
			currentHeadMovePixels.y = 0;
			currentHeadMoveDegrees.x = 0;
			currentHeadMoveDegrees.y = 0;

			double errSum = 0;

			for (int i = 0; i < minSize; i++){


				if (_options.showHeadtrackingDebugImage){
					uchar errByte;
					if (maxErr < 0.001){
						errByte = oldErrByte;
					}
					else{
						errByte = (uchar)(err.at(i) * 255 / maxErr);
						oldErrByte = errByte;
					}

					cv::circle(*_sceneFrameRgb, cv::Point(prevFeatures.at(i).x, prevFeatures.at(i).y), 4, CV_RGB(errByte, 255 - errByte, 0), 2);
					cv::line(*_sceneFrameRgb, cv::Point(prevFeatures.at(i).x, prevFeatures.at(i).y), cv::Point(currentFeatures.at(i).x, currentFeatures.at(i).y), CV_RGB(0, 0, 255), 2);
				}
				
				if (err.at(i) > 0.000001){
					currentHeadMovePixels.x += (prevFeatures.at(i).x - currentFeatures.at(i).x) / err.at(i);
					currentHeadMovePixels.y += (prevFeatures.at(i).y - currentFeatures.at(i).y) / err.at(i);
					errSum += 1 / (err.at(i));
				}
			}

			if (errSum > 0.0000001){
				currentHeadMovePixels.x /= (float)errSum;
				currentHeadMovePixels.y /= (float)errSum;
			}
			
			totalHeadMove.x += currentHeadMovePixels.x;
			totalHeadMove.y += currentHeadMovePixels.y;

			currentHeadMoveDegrees.x = currentHeadMovePixels.x * SSI_SCENETRACKER_SCENECAMERA_HFOV / (float)_sceneFrame->cols;
			currentHeadMoveDegrees.y = currentHeadMovePixels.y * SSI_SCENETRACKER_SCENECAMERA_HFOV / (float)_sceneFrame->cols;

			if (_options.showHeadtrackingDebugImage){
				//draw total rotation
				cv::Point2f totalRot(totalHeadMove.x * SSI_SCENETRACKER_SCENECAMERA_HFOV / (float)_sceneFrame->cols, totalHeadMove.y * SSI_SCENETRACKER_SCENECAMERA_HFOV / (float)_sceneFrame->cols);
				char totalRotStr[SSI_MAX_CHAR];
				sprintf(totalRotStr, "TOTAL: (%.0f, %.0f)", currentHeadMoveDegrees.x, currentHeadMoveDegrees.y);
				cv::putText(*_sceneFrameRgb, totalRotStr, cv::Point(20, 20), cv::FONT_HERSHEY_PLAIN, 1, CV_RGB(255, 255, 255), 1, CV_AA);

				float r = 20;
				float totalRotRadX = totalRot.x * CV_PI / 180;
				float totalRotRadY = totalRot.y * CV_PI / 180;
				cv::Point2f circleCenterX = cv::Point2f(r, _sceneFrameRgb->rows - r);
				cv::Point2f circleCenterY = cv::Point2f(4 * r, _sceneFrameRgb->rows - r);
				cv::Point2f circleCenterCurr = cv::Point2f(7 * r, _sceneFrameRgb->rows - r);
				cv::Point2f rotVecX = cv::Point2f(cosf(totalRotRadX), sinf(totalRotRadX));
				cv::Point2f rotVecY = cv::Point2f(cosf(totalRotRadY), sinf(totalRotRadY));
				rotVecX *= r;
				rotVecY *= r;
				cv::Point2f rotLineXEnd = rotVecX + circleCenterX;
				cv::Point2f rotLineYEnd = rotVecY + circleCenterY;

				cv::circle(*_sceneFrameRgb, circleCenterX, r, CV_RGB(0, 255, 0), -1);
				cv::circle(*_sceneFrameRgb, circleCenterY, r, CV_RGB(255, 255, 0), -1);
				cv::line(*_sceneFrameRgb, circleCenterX, rotLineXEnd, CV_RGB(0, 0, 0), 2);
				cv::line(*_sceneFrameRgb, circleCenterY, rotLineYEnd, CV_RGB(0, 0, 0), 2);


				cv::circle(*_sceneFrameRgb, circleCenterCurr, r, CV_RGB(0, 0, 255), -1);
				float chmVecLen = sqrtf(pow(currentHeadMovePixels.x, 2) + pow(currentHeadMovePixels.y, 2));
				cv::Point2f chmCapped = currentHeadMovePixels;
				if (chmVecLen > r){
					chmCapped.x *= (r/chmVecLen);
					chmCapped.y *= (r/chmVecLen);
				}

				cv::circle(*_sceneFrameRgb, circleCenterCurr + chmCapped, 2, CV_RGB(0, 0, 0), 2);
				cv::line(*_sceneFrameRgb, circleCenterCurr, circleCenterCurr + chmCapped, CV_RGB(0, 0, 0), 1);

				cv::imshow("optical flow", *_sceneFrameRgb);
				cv::waitKey(1);
			}
		}

		//prepare for next frame

		if (status.size() == 0){
			prevFeatures = currentFeatures;
		}
		else{
			prevFeatures.clear();
			for (int i = 0; i < status.size(); i++){
				if (status.at(i) == 1) prevFeatures.push_back(currentFeatures.at(i)); //only keep tracked features
			}
			currentFeatures.clear();
		}

		*_prevSceneFrame = _sceneFrame->clone();
		
	}

	float SceneTracker::angleBetween(float p1x, float p1y, float p2x, float p2y)
	{
		cv::Point2f cp(_video_format.widthInPixels, _video_format.heightInPixels);
		cv::Point2f v1(cp.x - p1x, cp.y - p1y);
		cv::Point2f v2(cp.x - p2x, cp.y - p2y);
		float angle = 0;

		float len1 = sqrt(v1.x * v1.x + v1.y * v1.y);
		float len2 = sqrt(v2.x * v2.x + v2.y * v2.y);

		float dot = v1.x * v2.x + v1.y * v2.y;

		float aRad = dot / (len1 * len2);

		if (aRad >= 1.0)
			angle = 0.0;
		else if (aRad <= -1.0)
			angle = CV_PI;
		else
			angle = acos(aRad); // 0..PI


		//check direction
		float a = (p1x - cp.x);
		float b = (p1y - cp.y);
		float c = (p2x - cp.x);
		float d = (p2y - cp.y);

		float atanA = atan2f(a, b);
		float atanB = atan2f(c, d);

		if (atanA - atanB > 0){ //clockwise
			return angle;
		}
		else{
			return -angle;
		}

	}

	//------------- mouse input thread

	char* Windowname = new char[256];
	bool t_prevLmb = false;
	bool t_prevRmb = false;

	MouseListener::MouseListener(ssi_char_t* cw, ssi_video_params_t vp){
		t_listening = true;
		t_leftMouseClick[0] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		t_leftMouseClick[1] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		t_rightMouseClick[0] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		t_rightMouseClick[1] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		ssi_strcpy(_calibrationWindow, cw);
		t_params = vp;
		start();
	}


	void MouseListener::run()
	{
		t_listening = true;

		while (t_listening)
		{
			checkMouseButtonStatus();
			sleep_ms(25);
		}

	}

	void MouseListener::disconnect(){
		t_listening = false;
	}

	float* MouseListener::getLeftMouseClick()
	{
		Lock lock(_mutex);
		return t_leftMouseClick;
	}

	float* MouseListener::getRightMouseClick()
	{
		Lock lock(_mutex);
		return t_rightMouseClick;
	}

	void MouseListener::setLeftMouseClickInvalid()
	{
		Lock lock(_mutex);

		t_leftMouseClick[0] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		t_leftMouseClick[1] = SSI_SCENETRACKER_NO_MOUSE_CLICK;

	}

	void MouseListener::setRightMouseClickInvalid()
	{
		Lock lock(_mutex);

		t_rightMouseClick[0] = SSI_SCENETRACKER_NO_MOUSE_CLICK;
		t_rightMouseClick[1] = SSI_SCENETRACKER_NO_MOUSE_CLICK;

	}

		
	void MouseListener::checkMouseButtonStatus()
	{
		POINT screenpoint;
		HWND  pFoundWindow = NULL;
		GetCursorPos(&screenpoint);
		pFoundWindow = WindowFromPoint(screenpoint);
		GetWindowText(pFoundWindow, Windowname, 256); //Get Title of Window
		std::string strWord(Windowname);

		bool inWindow = false;

		if (std::strcmp(strWord.c_str(), _calibrationWindow) == 0) //See if the window fits our "Calibration" Window
		{
			HCURSOR hCross = LoadCursor(NULL, IDC_CROSS);
			SetClassLong(pFoundWindow, GCLP_HCURSOR, (LONG)hCross);
			inWindow = true;
		}
		else
		{
			HCURSOR hCross = LoadCursor(NULL, IDC_ARROW);
			SetClassLong(pFoundWindow, GCLP_HCURSOR, (LONG)hCross);
		}


		bool lmb = GetKeyState(VK_LBUTTON) & 0x80;
		bool rmb = GetKeyState(VK_RBUTTON) & 0x80;

		if (inWindow)
		{

			std::string strWord(Windowname);
			int width;
			int height;
			int widthfullwindow;
			int heightfullwindow;
			int widthdiff;
			int heightdiff;
			float xp;
			float yp;
			bool inFrame;
			RECT rt;
			RECT rt2;

			GetWindowRect(pFoundWindow, &rt2);
			GetClientRect(pFoundWindow, &rt); // <= Get Window Position
			width = rt.right - rt.left;
			height = rt.bottom - rt.top;
			widthfullwindow = rt2.right - rt2.left;
			heightfullwindow = rt2.bottom - rt2.top;
			widthdiff = (widthfullwindow - width) / 2;
			heightdiff = (heightfullwindow - height) - widthdiff;

			xp = (((screenpoint.x - (rt2.left + (widthdiff - 1))) * (t_params.widthInPixels)) / width);
			yp = (((screenpoint.y - (rt2.top + (heightdiff - 1))) *  (t_params.heightInPixels)) / height);

			inFrame = true;
			if (xp < 1 || yp < 5 || xp > t_params.widthInPixels - 1 || yp > t_params.heightInPixels - 1) //Might be windows 7 specific, depends on window border width
				inFrame = false;

			if (inFrame){
				
				if (lmb && !t_prevLmb){//lmb down
					_mutex.acquire();
					t_leftMouseClick[0] = xp;
					t_leftMouseClick[1] = yp;
					_mutex.release();
				}

				if (rmb && !t_prevRmb){//rmb down
					_mutex.acquire();
					t_rightMouseClick[0] = xp;
					t_rightMouseClick[1] = yp;
					_mutex.release();
				}

				t_prevLmb = lmb;
				t_prevRmb = rmb;
			}

		}

	}
	


}
