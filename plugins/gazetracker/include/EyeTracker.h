// EyeTracker.h
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

#ifndef SSI_EYETRACKER_H
#define SSI_EYETRACKER_H

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

//for SET
#define MAX_DETECTION_RESULTS 10
#define MAX_FIRST_CANDIDATES 5
#define OBLATENESS_LOW  0.5
#define OBLATENESS_HIGH 2.50

//for auto pupil threshold (SET)
#define THRESHOLDED_PIXELS_TOO_MANY_PROBLEMATIC 5.0
#define THRESHOLDED_PIXELS_TOO_MANY 2.5
#define THRESHOLDED_PIXELS_TOO_FEW 0.75
#define AUTO_PUPIL_THRESHOLD_STEP 2

//Status codes
#define	SSI_EYETRACKER_W_NO_GLINT_CANDIDATE				1
#define SSI_EYETRACKER_W_TOO_MANY_GLINT_CANDIDATES		2
#define SSI_EYETRACKER_W_NO_SUITABLE_GLINT_CANDIDATE	3
#define SSI_EYETRACKER_PUPIL_DETECTION_OK				0
#define SSI_EYETRACKER_E_NO_PUPIL_CANDIDATE				-10
#define SSI_EYETRACKER_E_TOO_MANY_PUPIL_CANDIDATES		-11
#define SSI_EYETRACKER_E_THRESHOLD_TOO_HIGH				-15

//for detectionResults
#define SSI_EYETRACKER_RESULT_STATUS				0
#define SSI_EYETRACKER_RESULT_PUPIL_X				1
#define SSI_EYETRACKER_RESULT_PUPIL_Y				2
#define SSI_EYETRACKER_RESULT_PUPIL_WIDTH			3
#define SSI_EYETRACKER_RESULT_PUPIL_HEIGHT			4
#define SSI_EYETRACKER_RESULT_PUPIL_ELLIPSE_ANGLE   5
#define SSI_EYETRACKER_RESULT_PUPIL_AVERAGERADIUS	6
#define SSI_EYETRACKER_RESULT_PUPIL_CONFIDENCE		7
#define SSI_EYETRACKER_RESULT_GLINT_X				8
#define SSI_EYETRACKER_RESULT_GLINT_Y				9
#define SSI_EYETRACKER_RESULT_GLINT_RADIUS			10
#define SSI_EYETRACKER_RESULTS_SIZE					11

#define SSI_EYETRACKER_NO_VALUE 0 //for viewui

#define SSI_EYETRACKER_STANDARD_HEIGHT 480 //for scaling


class EyeTracker : public IFilter {

public: 

	class Options : public OptionList {

	public:

		Options()
			: useHaytham(true), pupilThreshold(175), pupilThresholdConstant(55), minPupilWidth(4), maxPupilWidth(25), glintThreshold(230), glintSearchArea(80), glintExcludeArea(20), liveAdjustOptions(true), showDebugImage(true), autoPupilThreshold(true), printTimes(false) {

			addOption("useHaytham", &useHaytham, 1, SSI_BOOL, "Uses the Haytham eye tracking algorthm instead of SimpleEyeTracker");

			//Info about STE parameters: http://gazeparser.sourceforge.net/rec/setup.html#adjusting-camera
			addOption("pupilThreshold", &pupilThreshold, 1, SSI_INT, "Brightness threshold for pupil detection [0..255]");
			addOption("minPupilWidth", &minPupilWidth, 1, SSI_INT, "Minimum width for an ellipse to count as a pupil (percentage of image width) [0..100]");
			addOption("maxPupilWidth", &maxPupilWidth, 1, SSI_INT, "Maximum width for an ellipse to count as a pupil (percentage of image width) [0..100]");
			addOption("glintThreshold", &glintThreshold, 1, SSI_INT, "Brightness threshold for glint detection (from IR LED) [0..255]");

			addOption("pupilThresholdConstant", &pupilThresholdConstant, 1, SSI_INT, "Only for Haytham. Adaptive threshold constant for pupil detection [0..100]. (http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html)");

			addOption("glintSearchArea", &glintSearchArea, 1, SSI_INT, "Only for STE algorithm. Size of search area around the pupil candidate for glint detection (from IR LED) [0..100]");
			addOption("glintExcludeArea", &glintExcludeArea, 1, SSI_INT, "Only for STE algorithm. Size of exclude area for removing detected glint (from IR LED) [0..100]");
			addOption("autoPupilThreshold", &autoPupilThreshold, 1, SSI_BOOL, "Only for STE algorithm. Automatically adjusts pupil threshold (overrides pupilThreshold option)");

			addOption("liveAdjustOptions", &liveAdjustOptions, 1, SSI_BOOL, "Opens a separate window with sliders to adjust options while the pipeline is running");
			addOption("showDebugImage", &showDebugImage, 1, SSI_BOOL, "Shows an image with thesholded pixels and other debug information");
			addOption("printTimes", &printTimes, 1, SSI_BOOL, "Prints computing times to the console");
			
		};
		
		bool useHaytham;

		int pupilThreshold;
		int minPupilWidth;
		int maxPupilWidth;

		int pupilThresholdConstant;

		int glintThreshold;
		int glintSearchArea;
		int glintExcludeArea;

		bool liveAdjustOptions;
		bool showDebugImage;
		bool autoPupilThreshold;
		bool printTimes;
		
	};

public:

	static const ssi_char_t *GetCreateName () { return "EyeTracker"; };
	static IObject *Create (const ssi_char_t *file) { return new EyeTracker(file); };
	~EyeTracker();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Detects a pupil"; };

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
		
		return SSI_EYETRACKER_RESULTS_SIZE;
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
			return;
		}
		memcpy (&_video_format, meta, size);
		setVideoFormat (_video_format);
	};

protected:

	EyeTracker (const ssi_char_t *file);
	EyeTracker::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void setVideoFormat (ssi_video_params_t format);

	ssi_video_params_t _video_format;
	ssi_size_t _video_stride;
	IplImage *_video_image;

	cv::Mat* _rgbFrame;
	cv::Mat* _greyscaleFrame;
	
	int g_ROIWidth;
	int g_ROIHeight;

	float detectionResults[SSI_EYETRACKER_RESULTS_SIZE];
	bool _pupilFoundInLastFrame;

	//for STE
	cv::Mat* _thresholdedFrame;

	//for haytham
	cv::Mat *_gsfROI;
	cv::Mat *_gsfPyr;
	cv::Mat *_glintMask;
	cv::Mat *_gsfThresh;
	

	

	int EyeTracker::detectPupilSET(int Threshold1, int PurkinjeSearchArea, int PurkinjeThreshold, int PurkinjeExclude, int MinWidth, int MaxWidth, float results[MAX_DETECTION_RESULTS]);
	int EyeTracker::detectPupilHaytham(float results[MAX_DETECTION_RESULTS]);
	
	void adjustROI();
	void checkOptions();

};

}

#endif
