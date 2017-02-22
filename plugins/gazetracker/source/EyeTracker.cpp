// EyeTracker.cpp
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

#include <ssiocv.h>

#include <algorithm>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include "EyeTracker.h"
#include "EyeTrackerUtils.h"

//#include "photo\photo.hpp"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif



namespace ssi {

	cv::Rect g_ROI;
	cv::Rect _lastPupil;
	std::vector<EyeTrackerUtils::Blob> blobs;

	const char* sliderWindowName = "Live Option Adjust";
	const char* debugWindowName = "Eye Tracker Debug";

	const char* pupilThresholdSliderName = "pupil thr";
	const char* pupilThresholdConstantSliderName = "p thr con";
	const char* pupilMinSliderName = "pupil min";
	const char* pupilMaxSliderName = "pupil max";

	const char* glintThresholdSliderName = "gl thres";
	const char* glintSearchSliderName = "gl search";
	const char* glintExcludeSliderName = "gl exclude";

	ssi_char_t *EyeTracker::ssi_log_name = "eyetrack___";

	EyeTracker::EyeTracker(const ssi_char_t *file)
		: _video_stride(0),
		_video_image(0),
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}

	}

	EyeTracker::~EyeTracker() {

		if (_video_image) {
			cvReleaseImageHeader(&_video_image);
		}

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}

	void EyeTracker::setVideoFormat(ssi_video_params_t format) {

		_video_format = format;
		_video_stride = ssi_video_stride(format);
		_video_image = cvCreateImageHeader(cvSize(_video_format.widthInPixels, _video_format.heightInPixels), _video_format.depthInBitsPerChannel, _video_format.numOfChannels);
	}

	void EyeTracker::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		checkOptions();

		_greyscaleFrame = new cv::Mat();

		if (_options.liveAdjustOptions){

			cv::namedWindow(sliderWindowName);
			cv::resizeWindow(sliderWindowName, 640, _options.useHaytham ? 280 : 310);
			cv::moveWindow(sliderWindowName, 645, 480);

			cv::createTrackbar(pupilThresholdSliderName, sliderWindowName, &_options.pupilThreshold, 255);

			if (_options.useHaytham){
				cv::createTrackbar(pupilThresholdConstantSliderName, sliderWindowName, &_options.pupilThresholdConstant, 100);
			}

			cv::createTrackbar(pupilMinSliderName, sliderWindowName, &_options.minPupilWidth, 100);
			cv::createTrackbar(pupilMaxSliderName, sliderWindowName, &_options.maxPupilWidth, 100);
			cv::createTrackbar(glintThresholdSliderName, sliderWindowName, &_options.glintThreshold, 255);

			if (!_options.useHaytham){
				cv::createTrackbar(glintSearchSliderName, sliderWindowName, &_options.glintSearchArea, 100);
				cv::createTrackbar(glintExcludeSliderName, sliderWindowName, &_options.glintExcludeArea, 100);
			}

		}

		if (_options.showDebugImage){
			cv::namedWindow(debugWindowName);
			cv::resizeWindow(debugWindowName, 320, 240);
			cv::moveWindow(debugWindowName, 1280, 0);
		}

		if (_options.useHaytham){
			_lastPupil = cv::Rect(0, 0, 0, 0);
			_pupilFoundInLastFrame = false;
			g_ROIWidth = _video_format.widthInPixels;
			g_ROIHeight = _video_format.heightInPixels;

			_gsfROI = new cv::Mat();
			_gsfPyr = new cv::Mat();
			_glintMask = new cv::Mat();
			_gsfThresh = new cv::Mat();
			_rgbFrame = new cv::Mat();
			blobs = std::vector<EyeTrackerUtils::Blob>();
		}
		else{
			_thresholdedFrame = new cv::Mat();

			//Hardcoded ROI for SimpleEyeTracker
			g_ROIWidth = 500;
			g_ROIHeight = 400;
		}

		g_ROI = cv::Rect(int((_video_format.widthInPixels - g_ROIWidth) / 2),
			int((_video_format.heightInPixels - g_ROIHeight) / 2),
			g_ROIWidth, g_ROIHeight);

	}

	void EyeTracker::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_tic();

		checkOptions();

		cvSetData(_video_image, stream_in.ptr, _video_stride);
		*_rgbFrame = cv::cvarrToMat(_video_image);
		cvtColor(*_rgbFrame, *_greyscaleFrame, CV_RGB2GRAY);

		int res = 0;
		if (_options.useHaytham){
			res = detectPupilHaytham(detectionResults);
		}
		else{
			res = detectPupilSET(_options.pupilThreshold, _options.glintSearchArea, _options.glintThreshold, _options.glintExcludeArea, _options.minPupilWidth, _options.maxPupilWidth, detectionResults);
		}

		if (_options.printTimes){
			ssi_print("[eyetracker] time:%3ims, ", ssi_toc());
		}

		detectionResults[SSI_EYETRACKER_RESULT_STATUS] = (float)res;

		if (_options.showDebugImage){
			cv::imshow(debugWindowName, *_rgbFrame);
		}


		ssi_tic();
		if (_options.showDebugImage || _options.liveAdjustOptions){
			cv::waitKey(1);
			//waitkey is needed to update debug window and trackbars, but can take up to 15ms.
			//disable debug window and trackbars in options for better performance 
		}

		if (_options.printTimes){
			ssi_print("waitKey:%3ims\n", ssi_toc());
		}

		ssi_real_t *dst = ssi_pcast(ssi_real_t, stream_out.ptr);
		if (_video_format.heightInPixels == SSI_EYETRACKER_STANDARD_HEIGHT){

			for (int i = 0; i < SSI_EYETRACKER_RESULTS_SIZE; i++){
				*dst++ = detectionResults[i];
			}
		}
		else{ //scale everything to 480p (keep aspect ratio)

			float scale = (float)SSI_EYETRACKER_STANDARD_HEIGHT / (float)_video_format.heightInPixels;
			for (int i = 0; i < SSI_EYETRACKER_RESULTS_SIZE; i++){
				
				if (i == SSI_EYETRACKER_RESULT_STATUS
					|| i == SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE
					|| i == SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE){
					*dst++ = detectionResults[i];
				}
				else{
					*dst++ = detectionResults[i] * scale;
				}
				
			}
		}
		
	}


	void EyeTracker::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		delete _rgbFrame;
		delete _greyscaleFrame;

		if (_options.useHaytham){
			delete _gsfROI;
			delete _gsfPyr;
			delete _glintMask;
			delete _gsfThresh;
		}
		else{
			delete _thresholdedFrame;
		}
	}


	/*
	Altered detection Algorithm from SimpleEyeTracker
	The original algorithm is from here: http://gazeparser.sourceforge.net/
	Info: http://gazeparser.sourceforge.net/adv/principle.html
	*/
	int EyeTracker::detectPupilSET(int Threshold1, int PurkinjeSearchArea, int PurkinjeThreshold, int PurkinjeExclude, int MinWidth, int MaxWidth, float results[MAX_DETECTION_RESULTS])
	{
		cv::Mat roi;
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Vec4i> hierarchy;
		std::vector<std::vector<cv::Point> >::iterator it;
		std::vector<cv::Point> firstCandidatePoints[MAX_FIRST_CANDIDATES], candidatePoints;
		std::vector<cv::Point> candidatePointsFine;
		std::vector<cv::Point>::iterator itFine;
		cv::RotatedRect firstCandidateRects[MAX_FIRST_CANDIDATES], candidateRect, candidateRectFine;
		int numCandidates = 0;
		int numPurkinjeCandidates = 0;
		int indexPupilPurkinjeCandidate;
		float PurkinjeCandidateCenterX, PurkinjeCandidateCenterY;
		int result = SSI_EYETRACKER_PUPIL_DETECTION_OK;
		bool ignoreGlint = false;
		bool gotGlint = false;

		if (_options.showDebugImage){
			cv::rectangle(*_rgbFrame, g_ROI, CV_RGB(0, 192, 0), 1);
		}

		//Find areas darker than Threshold1
		cv::threshold((*_greyscaleFrame)(g_ROI), *_thresholdedFrame, Threshold1, 127, CV_THRESH_BINARY);
		cv::findContours(*_thresholdedFrame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cv::Point(g_ROI.x, g_ROI.y));

		int thresholdedPixels = 0;

		//tint dark areas blue.
		for (int iy = 0; iy < g_ROI.height; iy++){
			unsigned char* p = (*_thresholdedFrame).ptr<unsigned char>(iy);
			for (int ix = 0; ix < g_ROI.width; ix++)
			{
				if (p[ix] == 0){

					if (_options.showDebugImage){
						_rgbFrame->at<cv::Vec3b>(cv::Point(ix + g_ROI.x, iy + g_ROI.y)).val[0] |= 160;
					}
					thresholdedPixels++;
				}
			}
		}

		int borderExclude = ((g_ROI.width + g_ROI.height) * 2) - 4;

		if (borderExclude >= 0 && thresholdedPixels >= borderExclude)
			thresholdedPixels -= borderExclude;

		float tpPercent = (float)(thresholdedPixels * 100) / (float)(g_ROI.width * g_ROI.height);
		char tpChar[10];
		sprintf(tpChar, "%.3f", tpPercent);
		std::string tpString = std::string("BP: ");
		tpString.append(tpChar);
		tpString.append("%");

		if (tpPercent > THRESHOLDED_PIXELS_TOO_MANY_PROBLEMATIC){

			if (_options.autoPupilThreshold){
				_options.pupilThreshold -= AUTO_PUPIL_THRESHOLD_STEP * 2;

				if (_options.liveAdjustOptions){
					cvSetTrackbarPos(pupilThresholdSliderName, sliderWindowName, _options.pupilThreshold);
				}
			}

			//too many thresholded pixels will take too long to calculate and a pupll cant be found anyways, so stop here
			return SSI_EYETRACKER_E_THRESHOLD_TOO_HIGH;
		}
		else if (tpPercent > THRESHOLDED_PIXELS_TOO_MANY){

			if (_options.autoPupilThreshold){
				_options.pupilThreshold -= AUTO_PUPIL_THRESHOLD_STEP;

				if (_options.liveAdjustOptions){
					cvSetTrackbarPos(pupilThresholdSliderName, sliderWindowName, _options.pupilThreshold);
				}
			}

		}

		if (tpPercent < THRESHOLDED_PIXELS_TOO_FEW){
			if (_options.autoPupilThreshold){
				_options.pupilThreshold += AUTO_PUPIL_THRESHOLD_STEP;

				if (_options.liveAdjustOptions){
					cvSetTrackbarPos(pupilThresholdSliderName, sliderWindowName, _options.pupilThreshold);
				}
			}
		}

		if (_options.showDebugImage){
			cv::putText(*_rgbFrame, tpString, cv::Point2d(0, 32), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
		}

		//Find a pupil candidate.
		for (it = contours.begin(); it != contours.end(); it++){
			if ((int)(*it).size() < 6){
				continue;
			}

			cv::Rect rr;
			rr = cv::boundingRect(*it);
			double minw = (double)MinWidth / 100 * _video_format.widthInPixels;
			double maxw = (double)MaxWidth / 100 * _video_format.widthInPixels;
			if (rr.width < minw || rr.width > maxw || rr.height < minw || rr.height > maxw){
				continue;
			}

			//Fit ellipse
			cv::Mat points(*it);
			cv::RotatedRect r;

			r = cv::fitEllipse(points);

			//Is Center of the ellipse in g_ROI? 
			if (r.center.x <= g_ROI.x || r.center.y <= g_ROI.y || r.center.x >= g_ROI.x + g_ROI.width || r.center.y >= g_ROI.y + g_ROI.height){
				continue;
			}

			//Check the shape of the ellipse
			if (OBLATENESS_LOW > r.size.height / r.size.width || r.size.height / r.size.width > OBLATENESS_HIGH){
				continue;
			}

			//Is PurkinjeSearchArea in CameraImage?
			if (r.center.x - g_ROI.x < PurkinjeSearchArea || r.center.y - g_ROI.y < PurkinjeSearchArea ||
				r.center.x - g_ROI.x > g_ROI.width - PurkinjeSearchArea || r.center.y - g_ROI.y > g_ROI.height - PurkinjeSearchArea){
				ignoreGlint = true;
				//continue;
			}

			//Count dark pixels within the ellipse
			double areac = 0;
			for (int ix = (int)(-r.size.width) / 2; ix < (int)r.size.width / 2; ix++){
				for (int iy = (int)(-r.size.height) / 2; iy < (int)r.size.height / 2; iy++){
					int xp;
					int yp;
					double rad;
					rad = r.angle * CV_PI / 180;
					xp = (int)(ix*cos(rad) - iy*sin(rad) + r.center.x) - g_ROI.x;
					yp = (int)(ix*sin(rad) + iy*cos(rad) + r.center.y) - g_ROI.y;

					if (xp >= g_ROI.width || yp >= g_ROI.height || xp < 0 || yp < 0){
						continue;
					}

					unsigned char* p = (*_thresholdedFrame).ptr<unsigned char>(yp);
					if (p[xp] == 0){
						areac += 1;
					}
				}
			}

			areac /= (r.size.width*r.size.height * CV_PI / 4);

			//Dark area occupies more than 75% of ellipse?
			if (areac < 0.75){
				continue;
			}

			//show all candidates
			if (_options.showDebugImage){
				cv::ellipse(*_rgbFrame, r, CV_RGB(0, 192, 0));
			}

			//This may be a pupil
			if (_options.showDebugImage){
				cv::ellipse(*_rgbFrame, r, CV_RGB(255, 0, 0));
				cv::line(*_rgbFrame, cv::Point2f(r.center.x, r.center.y - 20), cv::Point2f(r.center.x, r.center.y + 20), CV_RGB(255, 0, 0));
				cv::line(*_rgbFrame, cv::Point2f(r.center.x - 20, r.center.y), cv::Point2f(r.center.x + 20, r.center.y), CV_RGB(255, 0, 0));
			}

			firstCandidateRects[numCandidates] = r;
			firstCandidatePoints[numCandidates] = *it;
			numCandidates++;
			if (numCandidates >= MAX_FIRST_CANDIDATES)
				break;
		}

		if (numCandidates >= MAX_FIRST_CANDIDATES){
			//Too many candidates are found.
			return SSI_EYETRACKER_E_TOO_MANY_PUPIL_CANDIDATES;
		}
		else if (numCandidates == 0){
			//No candidate is found.
			return SSI_EYETRACKER_E_NO_PUPIL_CANDIDATE;
		}

		if (!ignoreGlint){

			for (int ic = 0; ic < numCandidates; ic++)
			{
				//Get a region where we search the 1st Purkinje image.
				int x = (int)(firstCandidateRects[ic].center.x) - PurkinjeSearchArea;
				int y = (int)(firstCandidateRects[ic].center.y) - PurkinjeSearchArea;
				int w = PurkinjeSearchArea * 2;
				int h = PurkinjeSearchArea * 2;

				unsigned char* p;
				float cogx, cogy;

				//Find areas brighter than PurkinjeThreshold
				p = _greyscaleFrame->ptr<unsigned char>((int)firstCandidateRects[ic].center.y);
				cv::threshold((*_greyscaleFrame)(cv::Rect(x, y, w, h)), roi, PurkinjeThreshold, 200, CV_THRESH_BINARY);
				cv::findContours(roi, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cv::Point(x, y));

				int npc = 0;
				float dx1, dx2, dy1, dy2;
				cogx = cogy = 0.0;
				for (it = contours.begin(); it != contours.end(); it++){
					if ((int)(*it).size() < 6){
						//Contour of the area is too short.
						continue;
					}

					//Fit ellipse
					cv::Mat points(*it);
					cv::RotatedRect r;
					r = cv::fitEllipse(points);
					dx1 = firstCandidateRects[ic].center.x - cogx;
					dy1 = firstCandidateRects[ic].center.y - cogy;
					dx2 = firstCandidateRects[ic].center.x - r.center.x;
					dy2 = firstCandidateRects[ic].center.y - r.center.y;
					//Find bright area nearest to the pupil center.
					if (dx1*dx1 + dy1*dy1 > dx2*dx2 + dy2*dy2){
						cogx = r.center.x;
						cogy = r.center.y;
					}
					npc++;
				}

				if (npc != 0){
					indexPupilPurkinjeCandidate = ic;
					candidateRect = firstCandidateRects[ic];
					candidatePoints = firstCandidatePoints[ic];
					PurkinjeCandidateCenterX = cogx;
					PurkinjeCandidateCenterY = cogy;
					numPurkinjeCandidates++;

					if (_options.showDebugImage){
						cv::rectangle(*_rgbFrame, cv::Rect(x, y, w, h), CV_RGB(255, 255, 255));
						cv::line(*_rgbFrame, cv::Point2f(cogx, cogy - 20), cv::Point2f(cogx, cogy + 20), CV_RGB(255, 192, 0));
						cv::line(*_rgbFrame, cv::Point2f(cogx - 20, cogy), cv::Point2f(cogx + 20, cogy), CV_RGB(255, 192, 0));
						cv::circle(*_rgbFrame, cv::Point2d(cogx, cogy), PurkinjeExclude, CV_RGB(255, 192, 0));
					}
				}
			}

		}

		if (numPurkinjeCandidates == 0)
		{
			result = SSI_EYETRACKER_W_NO_GLINT_CANDIDATE;
		}
		else if (numPurkinjeCandidates > 1)
		{
			result = SSI_EYETRACKER_W_TOO_MANY_GLINT_CANDIDATES;
		}
		else{
			gotGlint = true;
		}

		if (gotGlint){

			//Re-fit ellipse
			for (itFine = candidatePoints.begin(); itFine != candidatePoints.end(); itFine++){
				if (((*itFine).x - PurkinjeCandidateCenterX)*((*itFine).x - PurkinjeCandidateCenterX) + ((*itFine).y - PurkinjeCandidateCenterY)*((*itFine).y - PurkinjeCandidateCenterY) > PurkinjeExclude*PurkinjeExclude){
					candidatePointsFine.push_back(*itFine);
					cv::circle(*_rgbFrame, *itFine, 1, CV_RGB(255, 255, 255));
				}
			}

			if (candidatePointsFine.size() < 10)
			{
				//Re-fitted ellipse is too small
				result = SSI_EYETRACKER_W_NO_SUITABLE_GLINT_CANDIDATE;
			}

			candidateRectFine = cv::fitEllipse(cv::Mat(candidatePointsFine));

			if (_options.showDebugImage){
				cv::ellipse(*_rgbFrame, candidateRectFine, CV_RGB(0, 255, 64), 2);
			}

			results[SSI_EYETRACKER_RESULT_PUPIL_X] = candidateRectFine.center.x;
			results[SSI_EYETRACKER_RESULT_PUPIL_Y] = candidateRectFine.center.y;
			results[SSI_EYETRACKER_RESULT_PUPIL_WIDTH] = candidateRectFine.size.width;
			results[SSI_EYETRACKER_RESULT_PUPIL_HEIGHT] = candidateRectFine.size.height;
			results[SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE] = candidateRectFine.angle;
			results[SSI_EYETRACKER_RESULT_PUPIL_AVERAGERADIUS] = 0;
			results[SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE] = 0;
			results[SSI_EYETRACKER_RESULT_GLINT_X] = PurkinjeCandidateCenterX;
			results[SSI_EYETRACKER_RESULT_GLINT_Y] = PurkinjeCandidateCenterY;
			results[SSI_EYETRACKER_RESULT_GLINT_RADIUS] = (float)PurkinjeExclude; //from options

		}
		else{
			if (numCandidates >= 1){

				//pick ellipse closest to center

				int bestCandidate = 0;
				float lengthBest = 99999999;

				cv::Point2f roiCenter = cv::Point2f(g_ROI.x + g_ROI.width / 2, g_ROI.y + g_ROI.height / 2);

				for (int i = 0; i < numCandidates; i++){
					cv::Point2f diff = roiCenter - firstCandidateRects[i].center;
					float length = std::abs(cv::sqrt(diff.x*diff.x + diff.y*diff.y));

					if (length < lengthBest){
						lengthBest = length;
						bestCandidate = i;
					}
				}

				if (_options.showDebugImage){
					cv::ellipse(*_rgbFrame, firstCandidateRects[bestCandidate], CV_RGB(0, 255, 128), 2);
				}

				results[SSI_EYETRACKER_RESULT_PUPIL_X] = firstCandidateRects[bestCandidate].center.x;
				results[SSI_EYETRACKER_RESULT_PUPIL_Y] = firstCandidateRects[bestCandidate].center.y;
				results[SSI_EYETRACKER_RESULT_PUPIL_WIDTH] = firstCandidateRects[bestCandidate].size.width;
				results[SSI_EYETRACKER_RESULT_PUPIL_HEIGHT] = firstCandidateRects[bestCandidate].size.height;
				results[SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE] = firstCandidateRects[bestCandidate].angle;
				results[SSI_EYETRACKER_RESULT_PUPIL_AVERAGERADIUS] = 0;
				results[SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE] = 0;
				results[SSI_EYETRACKER_RESULT_GLINT_X] = 0;
				results[SSI_EYETRACKER_RESULT_GLINT_Y] = 0;
				results[SSI_EYETRACKER_RESULT_GLINT_RADIUS] = 0;
			}
		}

		return result;
	}

	/*
	Detection Algorithm from Haytham Gaze Tracker
	The original algorithm is from here: http://sourceforge.net/projects/haytham/
	*/
	int EyeTracker::detectPupilHaytham(float results[MAX_DETECTION_RESULTS]){

		int result;
		blobs.clear();


		// 0) Set ROI around last known pupil position

		adjustROI();

		if (_options.showDebugImage){
			cv::rectangle(*_rgbFrame, g_ROI, CV_RGB(192, 192, 192), 1);
		}

		*_gsfROI = (*_greyscaleFrame)(g_ROI);

		//----- 1) Remove glint

		if (_pupilFoundInLastFrame){
			//only do this if we have a small ROI

			//cv::imshow("roi", *_gsfROI);

			cv::pyrDown(*_gsfROI, *_gsfPyr);

			//Find areas brighter than glintThreshold
			cv::threshold(*_gsfPyr, *_glintMask, _options.glintThreshold, 255, CV_THRESH_BINARY);

			cv::Point anchorForGlintLeft = cv::Point(6, 3);
			cv::dilate(*_glintMask, *_glintMask, cv::Mat(7, 7, CV_8UC1, 255), cv::Point(6, 3), 1);

			//cv::imshow("glintMask", *_glintMask);

			cv::inpaint(*_gsfPyr, *_glintMask, *_gsfPyr, 7, cv::INPAINT_NS);
			cv::pyrUp(*_gsfPyr, *_gsfROI);

			//cv::imshow("inpainted", *_gsfROI);

			if (_options.showDebugImage){
				cv::Mat GlintMaskPyrUp;
				cv::pyrUp(*_glintMask, GlintMaskPyrUp);

				/*cv::Mat rgbTmp;
				cv::cvtColor(*_rgbFrame, rgbTmp, CV_BGR2GRAY);
				cv::cvtColor(rgbTmp, *_rgbFrame, CV_GRAY2BGR);*/

				//show glint in pink
				for (int iy = 0; iy < g_ROI.height; iy++){
					unsigned char* p = (GlintMaskPyrUp).ptr<unsigned char>(iy);
					for (int ix = 0; ix < g_ROI.width; ix++)
					{
						if (p[ix] == 255){
							_rgbFrame->at<cv::Vec3b>(cv::Point(ix + g_ROI.x, iy + g_ROI.y)).val[0] = 255;
							_rgbFrame->at<cv::Vec3b>(cv::Point(ix + g_ROI.x, iy + g_ROI.y)).val[1] = 0;
							_rgbFrame->at<cv::Vec3b>(cv::Point(ix + g_ROI.x, iy + g_ROI.y)).val[2] = 255;
						}
					}
				}
			}
		}


		//----- 2) Gray Erode/Dilate (reduce noise)

		//cv::imshow("before ED", *_gsfROI);

		if (_pupilFoundInLastFrame){
			//only do this if we have a small ROI
			cv::erode(*_gsfROI, *_gsfROI, cv::Mat(), cv::Point(-1, -1), 4);
			cv::dilate(*_gsfROI, *_gsfROI, cv::Mat(), cv::Point(-1, -1), 3);
		}

		//cv::imshow("after ED", *_gsfROI);

		//----- 3) Adaptive Threshold

		int blocksize = _options.pupilThreshold * 2 + 1;
		if (blocksize % 2 == 0){
			blocksize += 1;
		}
		cv::adaptiveThreshold(*_gsfROI, *_gsfThresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, blocksize, _options.pupilThresholdConstant);
		
		//open gaps between blobs that are stuck together
		cv::dilate(*_gsfThresh, *_gsfThresh, cv::Mat(), cv::Point(-1, -1), 2);
		cv::erode(*_gsfThresh, *_gsfThresh, cv::Mat(), cv::Point(-1, -1), 2);

		//cv::imshow("AT", *_gsfThresh);
		
		if (_options.showDebugImage){
			cv::Mat GlintMaskPyrUp;
			cv::pyrUp(*_glintMask, GlintMaskPyrUp);

			/*cv::Mat rgbTmp;
			cv::cvtColor(*_rgbFrame, rgbTmp, CV_BGR2GRAY);
			cv::cvtColor(rgbTmp, *_rgbFrame, CV_GRAY2BGR);*/

			//show adaptive threshold in blue
			for (int iy = 0; iy < g_ROI.height; iy++){
				unsigned char* p = (*_gsfThresh).ptr<unsigned char>(iy);
				for (int ix = 0; ix < g_ROI.width; ix++)
				{
					if (p[ix] == 255){
						_rgbFrame->at<cv::Vec3b>(cv::Point(ix + g_ROI.x, iy + g_ROI.y)).val[0] |= 192;
					}
				}
			}
		}

		//cv::imshow("before ellipse", *_rgbFrame);


		//----- 4) Detect Pupil Blobs (was an Aforge function)

		double minw = (double)_options.minPupilWidth / 100 * (double)_video_format.widthInPixels;
		double maxw = (double)_options.maxPupilWidth / 100 * (double)_video_format.widthInPixels;
		double minArea = (double)CV_PI * (minw / 2) * (minw / 2);
		double maxArea = (double)CV_PI * (maxw / 2) * (maxw / 2);

		EyeTrackerUtils::blobdetector_setMinArea(minArea);
		EyeTrackerUtils::blobdetector_setMaxArea(maxArea);
		EyeTrackerUtils::blobdetector_detect(*_gsfThresh, blobs);

		if (_options.showDebugImage){
			for (int i = 0; i < blobs.size(); i++){
				float kpx = blobs[i].x + g_ROI.x;
				float kpy = blobs[i].y + g_ROI.y;
				cv::ellipse(*_rgbFrame, cv::RotatedRect(cv::Point2f(kpx, kpy), cv::Size2f(blobs[i].width, blobs[i].height), blobs[i].angle), CV_RGB(255, 0, 0), 1);
				cv::line(*_rgbFrame, cv::Point(kpx - blobs[i].width / 4, kpy), cv::Point(kpx + blobs[i].width / 4, kpy), CV_RGB(255, 0, 0));
				cv::line(*_rgbFrame, cv::Point(kpx, kpy - blobs[i].height / 4), cv::Point(kpx, kpy + blobs[i].height / 4), CV_RGB(255, 0, 0));

				char sConvexity[20];
				char sInertia[20];
				char sCircularity[20];
				sprintf(sConvexity, "CO: %i", (int)(blobs[i].convexity * 100));
				sprintf(sInertia, "IN: %i", (int)(blobs[i].inertia * 100));
				sprintf(sCircularity, "CI: %i", (int)(blobs[i].circularity * 100));
				cv::putText(*_rgbFrame, sConvexity, cv::Point(kpx, kpy), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
				cv::putText(*_rgbFrame, sInertia, cv::Point(kpx, kpy + 16), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
				cv::putText(*_rgbFrame, sCircularity, cv::Point(kpx, kpy + 32), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
			}

			//show min/max pupil width
			int h = _video_format.heightInPixels;
			int miw = (int)minw;
			int maw = (int)maxw;
			int offset = 10;

			cv::line(*_rgbFrame, cv::Point(offset + miw, h - offset), cv::Point(offset + maw, h - offset), CV_RGB(255, 255, 255));
			cv::line(*_rgbFrame, cv::Point(offset + miw, h - offset), cv::Point(offset + miw, h - offset * 2), CV_RGB(255, 255, 255));
			cv::line(*_rgbFrame, cv::Point(offset + maw, h - offset), cv::Point(offset + maw, h - offset * 2), CV_RGB(255, 255, 255));
			cv::putText(*_rgbFrame, "MIN", cv::Point(offset + miw, h - offset * 2), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
			cv::putText(*_rgbFrame, "MAX", cv::Point(offset + maw, h - offset * 2), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 255, 255));
		}

		if (blobs.size() == 0){
			_pupilFoundInLastFrame = false;
			result = SSI_EYETRACKER_E_NO_PUPIL_CANDIDATE;
		}
		else{

			//pick blob with best convexity
			int bestCandidate = 0;

			for (int i = 0; i < blobs.size(); i++){
				if (blobs[i].convexity > blobs[bestCandidate].convexity){
					bestCandidate = i;
				}
			}


			if (_options.showDebugImage){
				//paint best candidate
				float kpx = blobs[bestCandidate].x + g_ROI.x;
				float kpy = blobs[bestCandidate].y + g_ROI.y;
				cv::ellipse(*_rgbFrame, cv::RotatedRect(cv::Point2f(kpx, kpy), cv::Size2f(blobs[bestCandidate].width, blobs[bestCandidate].height), blobs[bestCandidate].angle), CV_RGB(0, 255, 0), 2);
				cv::line(*_rgbFrame, cv::Point(kpx - blobs[bestCandidate].width / 4, kpy), cv::Point(kpx + blobs[bestCandidate].width / 4, kpy), CV_RGB(0, 255, 0));
				cv::line(*_rgbFrame, cv::Point(kpx, kpy - blobs[bestCandidate].height / 4), cv::Point(kpx, kpy + blobs[bestCandidate].height / 4), CV_RGB(0, 255, 0));

				//show current pupil width
				int offset = 10;
				int h = _video_format.heightInPixels;
				int cw = blobs[bestCandidate].averageRadius * 2;
				cv::line(*_rgbFrame, cv::Point(offset + cw, h - offset), cv::Point(offset + cw, h - offset * 2), CV_RGB(0, 255, 0));
				cv::putText(*_rgbFrame, "CUR", cv::Point(offset + cw, h - offset * 2), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0));
			}

			//confidence as weighted value between convexity and circularity
			float convexityNormalized = (blobs[bestCandidate].convexity - EyeTrackerUtils::blobdetector_getMinConvexity()) * (1.0f / (1.0f - EyeTrackerUtils::blobdetector_getMinConvexity()));
			float circularityNormalized = (blobs[bestCandidate].circularity - EyeTrackerUtils::blobdetector_getMinCircularity()) * (1.0f / (1.0f - EyeTrackerUtils::blobdetector_getMinCircularity()));
			float confidence = (0.8f * convexityNormalized + 0.2f * circularityNormalized);


			results[SSI_EYETRACKER_RESULT_PUPIL_X] = blobs[bestCandidate].x + g_ROI.x;
			results[SSI_EYETRACKER_RESULT_PUPIL_Y] = blobs[bestCandidate].y + g_ROI.y;
			results[SSI_EYETRACKER_RESULT_PUPIL_WIDTH] = blobs[bestCandidate].width;
			results[SSI_EYETRACKER_RESULT_PUPIL_HEIGHT] = blobs[bestCandidate].height;
			results[SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE] = blobs[bestCandidate].angle;
			results[SSI_EYETRACKER_RESULT_PUPIL_AVERAGERADIUS] = blobs[bestCandidate].averageRadius;
			results[SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE] = confidence;
			results[SSI_EYETRACKER_RESULT_GLINT_X] = 0; //no glint values from this algrithm
			results[SSI_EYETRACKER_RESULT_GLINT_Y] = 0;
			results[SSI_EYETRACKER_RESULT_GLINT_RADIUS] = 0;

			_lastPupil.x = results[SSI_EYETRACKER_RESULT_PUPIL_X];
			_lastPupil.y = results[SSI_EYETRACKER_RESULT_PUPIL_Y];
			_lastPupil.width = blobs[bestCandidate].averageRadius * 2;
			_lastPupil.height = blobs[bestCandidate].averageRadius * 2;

			_pupilFoundInLastFrame = true;

			result = SSI_EYETRACKER_PUPIL_DETECTION_OK;
		}

		return result;
	}

	//Helper for Haytham algorithm
	void EyeTracker::adjustROI(){

		if (!_pupilFoundInLastFrame){

			//no pupil found in last frame, reset ROI
			g_ROI.x = 0;
			g_ROI.y = 0;
			g_ROI.width = _video_format.widthInPixels;
			g_ROI.height = _video_format.heightInPixels;

		}
		else{

			//ROI around last pupil position
			g_ROI = cv::Rect(_lastPupil.x - _lastPupil.width * 2, _lastPupil.y - _lastPupil.height * 2, _lastPupil.width * 4, _lastPupil.height * 4);

			if (g_ROI.width < 1 || g_ROI.height < 1){

				//ssi_print("ROI too small! Resetting...\n");
				g_ROI.x = 0;
				g_ROI.y = 0;
				g_ROI.width = _video_format.widthInPixels;
				g_ROI.height = _video_format.heightInPixels;
			}
			else if ((g_ROI.width > _greyscaleFrame->cols - 1) || (g_ROI.height > _greyscaleFrame->rows - 1)){

				//ssi_print("ROI too big! Resetting...\n");
				g_ROI.x = 0;
				g_ROI.y = 0;
				g_ROI.width = _video_format.widthInPixels;
				g_ROI.height = _video_format.heightInPixels;

			}
			else{ //avoid image borders


				if (g_ROI.x < 0){
					g_ROI.x = 0;
				}

				if (g_ROI.y < 0){
					g_ROI.y = 0;
				}

				if (g_ROI.x + g_ROI.width > (*_greyscaleFrame).cols){
					g_ROI.x = (*_greyscaleFrame).cols - g_ROI.width - 1;
				}

				if (g_ROI.y + g_ROI.height > (*_greyscaleFrame).rows){
					g_ROI.y = (*_greyscaleFrame).rows - g_ROI.height - 1;
				}
			}
		}
	}

	void EyeTracker::checkOptions(){

		if (_options.pupilThreshold < 1){
			ssi_print("Pupil threshold cannot be lower than 1. Setting to 1...\n");
			_options.pupilThreshold = 1;
			cvSetTrackbarPos(pupilThresholdSliderName, sliderWindowName, _options.pupilThreshold);
		}

		if (_options.pupilThreshold > 255){
			ssi_print("Pupil threshold cannot be higher than 255. Setting to 255...\n");
			_options.pupilThreshold = 255;
			cvSetTrackbarPos(pupilThresholdSliderName, sliderWindowName, _options.pupilThreshold);
		}

		if (_options.minPupilWidth < 1){
			ssi_print("Min pupil width cannot be lower than 1. Setting to 1...\n");
			_options.minPupilWidth = 1;
			cvSetTrackbarPos(pupilMinSliderName, sliderWindowName, _options.minPupilWidth);
		}

		if (_options.minPupilWidth > 100){
			ssi_print("Min pupil width cannot be higher than 100. Setting to 100...\n");
			_options.maxPupilWidth = 100;
			cvSetTrackbarPos(pupilMinSliderName, sliderWindowName, _options.minPupilWidth);
		}

		if (_options.maxPupilWidth < 1){
			ssi_print("Max pupil width cannot be lower than 1. Setting to 1...\n");
			_options.maxPupilWidth = 1;
			cvSetTrackbarPos(pupilMaxSliderName, sliderWindowName, _options.maxPupilWidth);
		}

		if (_options.maxPupilWidth > 100){
			ssi_print("Max pupil width cannot be higher than 100. Setting to 100...\n");
			_options.maxPupilWidth = 100;
			cvSetTrackbarPos(pupilMaxSliderName, sliderWindowName, _options.maxPupilWidth);
		}

		if (_options.glintThreshold < 1){
			ssi_print("Glint threshold cannot be lower than 1. Setting to 1...\n");
			_options.glintThreshold = 1;
			cvSetTrackbarPos(glintThresholdSliderName, sliderWindowName, _options.glintThreshold);
		}

		if (_options.glintThreshold > 255){
			ssi_print("Glint threshold cannot be higher than 100. Setting to 100...\n");
			_options.glintThreshold = 255;
			cvSetTrackbarPos(glintThresholdSliderName, sliderWindowName, _options.glintThreshold);
		}

		if (_options.glintSearchArea < 1){
			ssi_print("Glint search area cannot be lower than 1. Setting to 1...\n");
			_options.glintSearchArea = 1;
			cvSetTrackbarPos(glintSearchSliderName, sliderWindowName, _options.glintSearchArea);
		}

		if (_options.glintSearchArea > 100){
			ssi_print("Glint search area cannot be higher than 100. Setting to 100...\n");
			_options.glintSearchArea = 100;
			cvSetTrackbarPos(glintSearchSliderName, sliderWindowName, _options.glintSearchArea);
		}

		if (_options.glintExcludeArea < 1){
			ssi_print("Glint exclude area cannot be lower than 1. Setting to 1...\n");
			_options.glintExcludeArea = 1;
			cvSetTrackbarPos(glintExcludeSliderName, sliderWindowName, _options.glintExcludeArea);
		}

		if (_options.glintExcludeArea > 100){
			ssi_print("Glint exclude area cannot be higher than 100. Setting to 100...\n");
			_options.glintExcludeArea = 100;
			cvSetTrackbarPos(glintExcludeSliderName, sliderWindowName, _options.glintExcludeArea);
		}

	}

}
