// ScenePainter.cpp
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

#include "ScenePainter.h"
#include "SceneTracker.h"
#include "ScenePainterUtils.h"
#include <ssiocv.h>

namespace ssi {

	int heatmapTime = 0;
	int tStartTime = 0;

	cv::Scalar black = CV_RGB(0, 0, 0);
	cv::Scalar white = CV_RGB(255, 255, 255);
	cv::Scalar yellow = CV_RGB(255, 255, 0);
	cv::Scalar red = CV_RGB(255, 0, 0);

	double fontSize;
	double fontOffset;

	ssi_char_t *ScenePainter::ssi_log_name = "scenepaint___";

	ScenePainter::ScenePainter(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	ScenePainter::~ScenePainter() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}

	void ScenePainter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		SSI_ASSERT(xtra_stream_in_num > 0);

		fontSize = 0.0026 * (double)_video_format.heightInPixels;
		fontOffset = 0.033333 * (double)_video_format.heightInPixels;

		if (_options.drawHeatmap){
			ScenePainterUtils::heatmap_init(_video_format.widthInPixels, _video_format.heightInPixels, (int) (_video_format.framesPerSecond + 0.5f));
		}

	}

	void ScenePainter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		tStartTime = ssi_time_ms();

		BYTE *image = ssi_pcast(BYTE, stream_in.ptr); 
		ssi_real_t *sceneResults = ssi_pcast(ssi_real_t, xtra_stream_in[0].ptr);

		cv::Mat img(cvSize(_video_format.widthInPixels, _video_format.heightInPixels), CV_8UC3, (void*)image);

		cv::Point2f headMovePx(sceneResults[3] * (float)_video_format.widthInPixels / SSI_SCENETRACKER_SCENECAMERA_HFOV, sceneResults[4] * (float)_video_format.widthInPixels / SSI_SCENETRACKER_SCENECAMERA_HFOV);
		bool shiftHeatmap = false;


		//draw head movement
		if (sceneResults[3] != SSI_SCENETRACKER_NO_HEADTRACKING && sceneResults[3] != SSI_SCENETRACKER_NO_HEADTRACKING){
			//cv::Point2f headMove(sceneResults[3], sceneResults[4]);

			if (_options.drawHeatmap){
				shiftHeatmap = true;
			}

			float r = 24;
			float cos = 4;

			cv::Point2f circleCenterCurr = cv::Point2f(img.cols - r - cos, img.rows - r - cos);
			cv::Point2f textAnchor(img.cols - cos - 2 * r, img.rows - 2 * cos - 2 * r);

			cv::circle(img, circleCenterCurr, r, CV_RGB(200, 200, 200), -1);
			cv::circle(img, circleCenterCurr, r, CV_RGB(127, 127, 127), 1, CV_AA);
			float chmVecLen = sqrtf(pow(headMovePx.x, 2) + pow(headMovePx.y, 2));
			cv::Point2f chmCapped = headMovePx;
			if (chmVecLen > r){
				chmCapped.x *= (r / chmVecLen);
				chmCapped.y *= (r / chmVecLen);
				chmVecLen = r;
			}

			cv::circle(img, circleCenterCurr + chmCapped, 2, CV_RGB(0, 0, 0), 2, CV_AA);
			cv::line(img, circleCenterCurr, circleCenterCurr + chmCapped, CV_RGB(0, 0, 0), 1, CV_AA);

			cv::putText(img, "HEAD", textAnchor, cv::FONT_HERSHEY_PLAIN, fontSize, black, 2, CV_AA);
			cv::putText(img, "HEAD", textAnchor, cv::FONT_HERSHEY_PLAIN, fontSize, CV_RGB(200, 200, 200), 1, CV_AA);

		}


		if (sceneResults[0] == SSI_CALIBRATIONSTATUS_NOT_CALIBRATED){
			cv::putText(img, "CLICK TO CALIBRATE", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, black, 2, CV_AA);
			cv::putText(img, "CLICK TO CALIBRATE", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, white, 1, CV_AA);
		}
		else if (sceneResults[0] == SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_HOMOGRAPHY || sceneResults[0] == SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_POLYNOMIAL){

			cv::Point2f cp = cv::Point2f();
			int n_calibrationPoints = 0;

			//paint clicked points
			for (int i = SSI_SCENETRACKER_CALIBRATIONRESULTS_START; i < SSI_SCENETRACKER_RESULTS_SIZE; i += 2){
				cp.x = sceneResults[i];
				cp.y = sceneResults[i + 1];


				if (cp.x > 0 && cp.y > 0 && cp.x < _video_format.widthInPixels && cp.y < _video_format.heightInPixels){
					float r = 10;

					cv::circle(img, cp, r, black, 2, CV_AA);
					cv::line(img, cv::Point2f(cp.x + r, cp.y), cv::Point2f(cp.x - r, cp.y), black, 2, CV_AA);
					cv::line(img, cv::Point2f(cp.x, cp.y + r), cv::Point2f(cp.x, cp.y - r), black, 2, CV_AA);

					cv::circle(img, cp, r, yellow, 1, CV_AA);
					cv::line(img, cv::Point2f(cp.x + r, cp.y), cv::Point2f(cp.x - r, cp.y), yellow, 1, CV_AA);
					cv::line(img, cv::Point2f(cp.x, cp.y + r), cv::Point2f(cp.x, cp.y - r), yellow, 1, CV_AA);

					n_calibrationPoints++;
				}
			}

			int n_maxCalibrationPoints = (sceneResults[0] == SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_HOMOGRAPHY) ? SSI_SCENETRACKER_CALIBRATIONPOINTS_HOMOGRAPHY : SSI_SCENETRACKER_CALIBRATIONPOINTS_POLYNOMIAL;

			char calibChar[SSI_MAX_CHAR];
			sprintf(calibChar, "CALIBRATING (GOT %i OF %i POINTS)", n_calibrationPoints, n_maxCalibrationPoints);
			std::string calibString = std::string(calibChar);
			
			cv::putText(img, calibString, cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, black, 2, CV_AA);
			cv::putText(img, calibString, cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, yellow, 1, CV_AA);
			
		}
		else if (sceneResults[0] == SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED_NO_PUPIL){

			cv::putText(img, "NO PUPIL TO MAP", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, black, 2, CV_AA);
			cv::putText(img, "NO PUPIL TO MAP", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, red, 1, CV_AA);

			if (_options.drawHeatmap){
				ssi_tic();
				
				ScenePainterUtils::heatmap_update(-1, -1, &img);
				if (shiftHeatmap){
					ScenePainterUtils::heatmap_shift((int) std::roundf(headMovePx.x /2.0), (int) std::round(headMovePx.y /2.0));
				}
				
				heatmapTime = ssi_toc();
			}
		}
		else{

			cv::Point2f gp = cv::Point2f();

			float scale = (float)_video_format.heightInPixels / (float)SSI_SCENETRACKER_STANDARD_HEIGHT;

			//paint gazepoint
			gp.x = sceneResults[1] * scale;
			gp.y = sceneResults[2] * scale;

			if (_options.drawHeatmap){
				ssi_tic();
				
				ScenePainterUtils::heatmap_update((int)gp.x, (int)gp.y, &img);
				if (shiftHeatmap){
					ScenePainterUtils::heatmap_shift((int)std::roundf(headMovePx.x / 2.0), (int)std::round(headMovePx.y / 2.0));
				}
				
				heatmapTime = ssi_toc();
			}

			if (gp.x >= 0 && gp.y >= 0 && gp.x < _video_format.widthInPixels && gp.y < _video_format.heightInPixels){
				float r = 10;

				cv::circle(img, gp, r, black, 2, CV_AA);
				cv::line(img, cv::Point2f(gp.x + r / 2, gp.y), cv::Point2f(gp.x + r + r / 2, gp.y), black, 2, CV_AA);
				cv::line(img, cv::Point2f(gp.x - r / 2, gp.y), cv::Point2f(gp.x - r - r / 2, gp.y), black, 2, CV_AA);
				cv::line(img, cv::Point2f(gp.x, gp.y + r / 2), cv::Point2f(gp.x, gp.y + r + r / 2), black, 2, CV_AA);
				cv::line(img, cv::Point2f(gp.x, gp.y - r / 2), cv::Point2f(gp.x, gp.y - r - r / 2), black, 2, CV_AA);

				cv::circle(img, gp, r, white, 1, CV_AA);
				cv::line(img, cv::Point2f(gp.x + r / 2, gp.y), cv::Point2f(gp.x + r + r / 2, gp.y), white, 1, CV_AA);
				cv::line(img, cv::Point2f(gp.x - r / 2, gp.y), cv::Point2f(gp.x - r - r / 2, gp.y), white, 1, CV_AA);
				cv::line(img, cv::Point2f(gp.x, gp.y + r / 2), cv::Point2f(gp.x, gp.y + r + r / 2), white, 1, CV_AA);
				cv::line(img, cv::Point2f(gp.x, gp.y - r / 2), cv::Point2f(gp.x, gp.y - r - r / 2), white, 1, CV_AA);

			}
			else{
				cv::putText(img, "GAZE POINT OUT OF RANGE", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, black, 2, CV_AA);
				cv::putText(img, "GAZE POINT OUT OF RANGE", cv::Point2d(0, fontOffset), cv::FONT_HERSHEY_PLAIN, fontSize, yellow, 1, CV_AA);

				bool xOk = (gp.x >= 0 && gp.x < _video_format.widthInPixels);
				bool yOk = (gp.y >= 0 && gp.y < _video_format.heightInPixels);

				float al = 18;
				float os = 3;

				int w = _video_format.widthInPixels;
				int h = _video_format.heightInPixels;

				if (gp.x < 0 && yOk){
					cv::arrowedLine(img, cv::Point(al + os, gp.y), cv::Point(os, gp.y), black, 2, CV_AA, 0, 0.5);
					cv::arrowedLine(img, cv::Point(al + os, gp.y), cv::Point(os, gp.y), white, 1, CV_AA, 0, 0.5);
				}
				else if (gp.x > w && yOk){
					cv::arrowedLine(img, cv::Point(w - os - al, gp.y), cv::Point(w - os, gp.y), black, 2, CV_AA, 0, 0.5);
					cv::arrowedLine(img, cv::Point(w - os - al, gp.y), cv::Point(w - os, gp.y), white, 1, CV_AA, 0, 0.5);
				}
				else if (gp.y < 0 && xOk){
					cv::arrowedLine(img, cv::Point(gp.x, al + os), cv::Point(gp.x, os), black, 2, CV_AA, 0, 0.5);
					cv::arrowedLine(img, cv::Point(gp.x, al + os), cv::Point(gp.x, os), white, 1, CV_AA, 0, 0.5);
				}
				else if (gp.y > h && xOk){
					cv::arrowedLine(img, cv::Point(gp.x, h - os - al), cv::Point(gp.x, h - os), black, 2, CV_AA, 0, 0.5);
					cv::arrowedLine(img, cv::Point(gp.x, h - os - al), cv::Point(gp.x, h - os), white, 1, CV_AA, 0, 0.5);
				}
				else{ //diagonal
					
					al /= std::sqrt(2.0f);

					if (gp.x < 0 && gp.y < 0){
						cv::arrowedLine(img, cv::Point(os + al, os + al), cv::Point(os, os), black, 2, CV_AA, 0, 0.5);
						cv::arrowedLine(img, cv::Point(os + al, os + al), cv::Point(os, os), white, 1, CV_AA, 0, 0.5);
					}
					else if (gp.x > w && gp.y < 0){
						cv::arrowedLine(img, cv::Point(w - os - al, os + al), cv::Point(w - os, os), black, 2, CV_AA, 0, 0.5);
						cv::arrowedLine(img, cv::Point(w - os - al, os + al), cv::Point(w - os, os), white, 1, CV_AA, 0, 0.5);
					}
					else if (gp.x < 0 && gp.y > h){
						cv::arrowedLine(img, cv::Point(os + al, h - os - al), cv::Point(os, h - os), black, 2, CV_AA, 0, 0.5);
						cv::arrowedLine(img, cv::Point(os + al, h - os - al), cv::Point(os, h - os), white, 1, CV_AA, 0, 0.5);
					}
					else if (gp.x > w && gp.y > h){
						cv::arrowedLine(img, cv::Point(w - os - al, h - os - al), cv::Point(w - os, h - os), black, 2, CV_AA, 0, 0.5);
						cv::arrowedLine(img, cv::Point(w - os - al, h - os - al), cv::Point(w - os, h - os), white, 1, CV_AA, 0, 0.5);
					}
				}
			}
		}

		

		if (_options.printTimes){
			int totalTime = ssi_time_ms() - tStartTime;
			ssi_print("[scenepainter] total :%3ims, heatmap:%3ims\n", totalTime, heatmapTime);
		}
				
		memcpy(stream_out.ptr, stream_in.ptr, stream_in.tot);
	}

	void ScenePainter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}

	
}



