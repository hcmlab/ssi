// EyePainter.h
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

#ifndef SSI_EYEPAINTER_H
#define SSI_EYEPAINTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

	class EyePainter : public IFilter {

public:

	class Options : public OptionList {
	public:

		Options()
			: isFlipped(true) {

			addOption("isFlipped", &isFlipped, 1, SSI_BOOL, "set this to true if the painted detection points are upside down");
		};

		bool isFlipped;
	};

public:

	static const ssi_char_t *GetCreateName () { return "EyePainter"; };
	static IObject *Create (const ssi_char_t *file) { return new EyePainter (file); };
	~EyePainter();
	EyePainter::Options *getOptions() { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Paints a detected pupil"; };

	void setVideoFormat (ssi_video_params_t video_format) { _video_format = video_format; };
	ssi_video_params_t getVideoFormat () { return _video_format; };

	const void *getMetaData (ssi_size_t &size) { size = sizeof (_video_format); return &_video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_video_format, meta, size);
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
		return sample_dimension_in; 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != ssi_video_size (_video_format)) {
			ssi_err ("invalid byte size");
		}
		return ssi_video_size (_video_format); 
	};	
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_IMAGE;
	};

protected:

	EyePainter(const ssi_char_t *file = 0);
	EyePainter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_video_params_t _video_format;

};

}

#endif
