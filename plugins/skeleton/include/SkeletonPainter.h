// MicrosoftKinect.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/11/22
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

#ifndef SSI_SENSOR_SKELETONPAINTER_H
#define SSI_SENSOR_SKELETONPAINTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

typedef struct _IplImage IplImage;
typedef struct CvPoint CvPoint;

#define MIN_ALLOWED_VALUE -5000
#define MAX_ALLOWED_VALUE 5000

namespace ssi {

class SkeletonPainter : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: width (320), height (240), fps(0) {
				
			addOption ("width", &width, 1, SSI_SIZE, "image width in pixels");
			addOption ("height", &height, 1, SSI_SIZE, "image height in pixels");
			addOption ("fps", &fps, 1, SSI_TIME, "frames per second of the input stream");
		};		
		
		ssi_size_t width;		
		ssi_size_t height;
		ssi_time_t fps;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SkeletonPainter"; };
	static IObject *Create (const ssi_char_t *file) { return new SkeletonPainter (file); };
	~SkeletonPainter ();
	SkeletonPainter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "visualizes the ssi skeleton on a video stream"; };

	void setVideoFormat (ssi_video_params_t video_format) { _format = video_format; };
	ssi_video_params_t getVideoFormat () { return _format; };

	const void *getMetaData (ssi_size_t &size) { 
		ssi_video_params (_format, _options.width, _options.height, _options.fps, 8, 3);
		size = sizeof (_format);
		return &_format; 
	};
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (SSI_SKELETON_META) != size) {
			ssi_err ("invalid meta size");
			return;
		}				
	};

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { 
		if (sample_dimension_in % (SSI_SKELETON_JOINT::NUM * SSI_SKELETON_JOINT_VALUE::NUM) != 0) {
			ssi_err ("invalid dimension");
		}
		return 1; 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != sizeof (ssi_real_t)) {
			ssi_err ("invalid byte size");
			return 0;
		}
		ssi_video_params (_format, _options.width, _options.height, 0, 8, 3);
		return ssi_video_size (_format); 
	};	
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_IMAGE;
	};

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	SkeletonPainter (const ssi_char_t *file = 0);
	SkeletonPainter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void paintSkeleton (IplImage *image,
		SSI_SKELETON &skel,
		ssi_size_t index);
	void paintBone (IplImage *image, 
		SSI_SKELETON &skel,
		CvPoint *points,  
		SSI_SKELETON_JOINT::List joint0, 
		SSI_SKELETON_JOINT::List joint1, 
		bool ishead=false);

	IplImage *_image;
	ssi_video_params_t _format;
	ssi_size_t _n_skeletons;

	SSI_SKELETON_VALUE_TYPE *_min_value_y, *_max_value_y, *_min_value_x, *_max_value_x; 
};

}

#endif
