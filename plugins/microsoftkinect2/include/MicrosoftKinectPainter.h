// MicrosoftKinect.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/10/18
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

#ifndef SSI_SENSOR_MICROSOFTKINECTPAINTER_H
#define SSI_SENSOR_MICROSOFTKINECTPAINTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "MicrosoftKinect.h"

namespace ssi {

class MicrosoftKinectPainter : public IFilter {

public:
		class Options : public OptionList {

		public:

			Options ()
				: scaled (true), projected (true), showskeleton(true), showface(true) {
				
				addOption ("projected", &projected, 1, SSI_BOOL, "screen coordinates are projected");
				addOption ("scaled", &scaled, 1, SSI_BOOL, "screen coordinates are scaled (width and height) to [0..1]");
				addOption ("showSkeleton", &showskeleton, 1, SSI_BOOL, "paints the skeleton if selected");
				addOption ("showFace", &showface, 1, SSI_BOOL, "paints the face if selected");
			};		
		
			bool scaled;		
			bool projected;
			bool showskeleton;
		    bool showface;

		};

public:

	static const ssi_char_t *GetCreateName () { return "MicrosoftKinectPainter"; };
	static IObject *Create (const ssi_char_t *file) { return new MicrosoftKinectPainter (file); };
	~MicrosoftKinectPainter ();
	MicrosoftKinectPainter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "visualizes skeleton and face tracking of the microsoft kinect on a video stream"; };

	void setVideoFormat (ssi_video_params_t video_format) { _video_format = video_format; };
	ssi_video_params_t getVideoFormat () { return _video_format; };

	const void *getMetaData (ssi_size_t &size) { 
		size = sizeof (_video_format); 
		return &_video_format;
	};
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			return;
		}
		memcpy (&_video_format, meta, size);
		_israwdepth = false;
		const ssi_video_params_t video_format = *ssi_pcast (const ssi_video_params_t, meta);
		if (video_format.numOfChannels == 1 && video_format.depthInBitsPerChannel == 16) {
			_israwdepth = true;
			_video_format.depthInBitsPerChannel = 8;			
			_raw_format = video_format;
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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { return sample_dimension_in; };
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != (_israwdepth ? ssi_video_size (_raw_format) : ssi_video_size (_video_format))) {
			ssi_err ("invalid byte size");
			return 0;
		}
		return ssi_video_size (_video_format); 
	};	
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_IMAGE;
	};

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	MicrosoftKinectPainter (const ssi_char_t *file = 0);
	MicrosoftKinectPainter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	int _skeleton_index;
	ssi_size_t _n_skeletons;
	int _facepoints_index;
	ssi_size_t _n_faces;
	bool _old_skeleton;
	
	ssi_video_params_t _video_format;
	bool _israwdepth;
	ssi_video_params_t _raw_format;	
};

}

#endif
