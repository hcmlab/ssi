// SceneTracker.h
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

#pragma once

#ifndef SSI_SCENETRACKER_H
#define SSI_SCENETRACKER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "thread/Mutex.h"

typedef struct _IplImage IplImage;
typedef struct CvRect CvRect;
typedef struct CvMat CvMat;
typedef struct CvSize CvSize;

namespace cv {
	class Mat;
}


namespace ssi {

#define SSI_SCENETRACKER_RESULTS_SIZE 23
#define SSI_SCENETRACKER_CALIBRATIONRESULTS_START 5

#define SSI_SCENETRACKER_STATUS		 0
#define SSI_SCENETRACKER_GAZEPOINT_X 1
#define SSI_SCENETRACKER_GAZEPOINT_Y 2
#define SSI_SCENETRACKER_HEAD_X      3
#define SSI_SCENETRACKER_HEAD_Y      4

#define SSI_SCENETRACKER_CALIBRATIONPOINTS_HOMOGRAPHY 4
#define SSI_SCENETRACKER_CALIBRATIONPOINTS_POLYNOMIAL 9

#define SSI_SCENETRACKER_NO_MOUSE_CLICK -1.0f

#define SSI_CALIBRATIONSTATUS_NOT_CALIBRATED                   -20
#define SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_HOMOGRAPHY -21 // 4 points
#define SSI_CALIBRATIONSTATUS_CURRENTLY_CALIBRATING_POLYNOMIAL -22 // 9 points
#define	SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED			   -23
#define	SSI_CALIBRATIONSTATUS_CALIBRATION_FINISHED_NO_PUPIL    -24

#define SSI_SCENETRACKER_NO_VALUE 0 //for viewui
#define SSI_SCENETRACKER_NO_CALIBRATIONPOINT -1
#define SSI_SCENETRACKER_NO_HEADTRACKING -100000

#define SSI_SCENETRACKER_STANDARD_HEIGHT 480 //for scaling

#define SSI_SCENETRACKER_SCENECAMERA_HFOV 67.5 //Logitech C930e (pupil pro) at 4/3 aspect ratio
//#define SSI_SCENETRACKER_SCENECAMERA_HFOV 90 //Logitech C930e (pupil pro) at 16/9 aspect ratio
//#define SSI_SCENETRACKER_SCENECAMERA_HFOV 60 //Logitech C270


	class MouseListener : public Thread
	{
	public:

		MouseListener(ssi_char_t* cw, ssi_video_params_t vp);

		virtual void run();
		void disconnect();

		float* getLeftMouseClick();
		float* getRightMouseClick();
		void setLeftMouseClickInvalid();
		void setRightMouseClickInvalid();
		void checkMouseButtonStatus();

		bool t_listening;
		float t_leftMouseClick[2];
		float t_rightMouseClick[2];
		ssi_video_params_t t_params;
		ssi_char_t _calibrationWindow[1024];
		Mutex _mutex;
	};




class SceneTracker : public IFilter {

public: 

	class Options : public OptionList {

	public:

		Options()
			: preciseCalibration(false), trackHeadMovement(false), showHeadtrackingDebugImage(false), printTimes(false) {

			ssi_strcpy(calibrationWindow, "Scene");

			addOption("windowName", calibrationWindow, SSI_MAX_CHAR, SSI_CHAR, "Name of the calibration window. Has to be same as the plot window name");
			addOption("preciseCalibration", &preciseCalibration, 1, SSI_BOOL, "Calibrate with 9 instead of 4 points for higher mapping precision (Polynomial instead of Homography)");
			addOption("printTimes", &printTimes, 1, SSI_BOOL, "Prints computing times to the console");
			addOption("trackHeadMovement", &trackHeadMovement, 1, SSI_BOOL, "Tracks head movement with optical flow");
			addOption("showHeadtrackingDebugImage", &showHeadtrackingDebugImage, 1, SSI_BOOL, "Shows a head tracking debug image displaying the optical flow");

		};

		void setCalibrationWindow(const ssi_char_t *name) {
			ssi_strcpy(calibrationWindow, name);
		}
		
		ssi_char_t calibrationWindow[SSI_MAX_CHAR];
		bool preciseCalibration;
		bool trackHeadMovement;
		bool showHeadtrackingDebugImage;
		bool printTimes;
		
	};

public:

	static const ssi_char_t *GetCreateName () { return "SceneTracker"; };
	static IObject *Create(const ssi_char_t *file) { return new SceneTracker(file); };
	~SceneTracker();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Calculates gazepoint on a scene"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in > 1) {
			ssi_err ("#dimension > 1 not supported");
		}		
		
		return SSI_SCENETRACKER_RESULTS_SIZE;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		if (sample_bytes_in != ssi_video_size (_video_format)) {
			ssi_err ("#bytes not compatible");
		}
		return sizeof (ssi_real_t);
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err ("unsupported type");
		}
		return SSI_REAL;
	}

	ssi_video_params_t getFormatIn () { return _video_format; };

	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_video_format, meta, size);
		setVideoFormat (_video_format);
	};

protected:

	SceneTracker(const ssi_char_t *file);
	SceneTracker::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void setVideoFormat (ssi_video_params_t format);

	ssi_video_params_t _video_format;
	ssi_size_t _video_stride;
	IplImage *_video_image;

	void calibrate();
	void resetCalibration();

	void mapToScene(float eyeX, float eyeY, float* sceneGazePoint);
	void headTracking();
	float angleBetween(float p1x, float p1y, float p2x, float p2y);

	cv::Mat* _sceneFrameRgb;
	cv::Mat* _sceneFrame;
	cv::Mat* _prevSceneFrame;

	int _calibrationStatus;

	MouseListener* _mouseListener;
	cv::Mat* _calibrationResult;
	
};

}

#endif
