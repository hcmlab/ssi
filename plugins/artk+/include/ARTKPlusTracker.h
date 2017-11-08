// ARTKPlusTracker.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/29
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

#ifndef SSI_ARTKPLUS_TRACKER_H
#define SSI_ARTKPLUS_TRACKER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "ARTKPlusTools.h"

namespace ARToolKitPlus {
	class TrackerMultiMarker;
}

namespace ssi {

class ARTKPlusTracker : public IFilter {

public:

	const static int MAX_MARKER = 20;

	class Options : public OptionList {

	public:

		Options ()
			: n_marker (MAX_MARKER), flip (false), scale (false) {

			camcfg[0] = '\0';
			markcfg[0] = '\0';
			addOption ("camcfg", camcfg, SSI_MAX_CHAR, SSI_CHAR, "file path of camera config file");
			addOption ("markcfg", markcfg, SSI_MAX_CHAR, SSI_CHAR, "file path of marker config file");
			addOption ("n_marker", &n_marker, 1, SSI_SIZE, "number of detected markers");
			addOption ("flip", &flip, 1, SSI_BOOL, "search flipped markers");
			addOption ("scale", &scale, 1, SSI_BOOL, "scale marker position to interval [0..1]");
		};

		void setConfigPaths (const ssi_char_t *camcfg, const ssi_char_t *markcfg) {
			this->camcfg[0] = '\0';
			this->markcfg[0] = '\0';
			if (camcfg) { ssi_strcpy (this->camcfg, camcfg); }
			if (markcfg) { ssi_strcpy (this->markcfg, markcfg); }
		}

		ssi_char_t camcfg[SSI_MAX_CHAR];
		ssi_char_t markcfg[SSI_MAX_CHAR];
		bool flip;
		ssi_size_t n_marker;
		bool scale;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "ARTKPlusTracker"; };
	static IObject *Create (const ssi_char_t *file) { return new ARTKPlusTracker (file); };
	~ARTKPlusTracker ();
	ARTKPlusTracker::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "detects marker in a video stream"; };

	const void *getMetaData (ssi_size_t &size) { size = sizeof (_video_format); return &_video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
			return;
		}
		memcpy (&_video_format, meta, size);
	};

	void setVideoFormat (ssi_video_params_t video_format) { _video_format = video_format; };
	ssi_video_params_t getVideoFormat () { return _video_format; };

	void transform_enter(
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush(
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { return _options.n_marker; };
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != ssi_video_size (_video_format)) {
			ssi_err ("invalid byte size");
			return 0;
		}
		return sizeof (ARTKPlusTools::marker_s); 
	};	
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_IMAGE) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_STRUCT;
	}

protected:

	ARTKPlusTracker (const ssi_char_t *file = 0);
	ARTKPlusTracker::Options _options;
	ssi_char_t *_file;

	ssi_video_params_t _video_format;
	ARToolKitPlus::TrackerMultiMarker *_tracker;
	BYTE *_flip_buffer;

};

}

#endif
