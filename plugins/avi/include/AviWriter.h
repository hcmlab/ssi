// AviWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/13
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

#ifndef SSI_AVI_AVIWRITER_H
#define SSI_AVI_AVIWRITER_H

#include "base/IConsumer.h"
#include "AviTools.h"

namespace ssi {

class AviWriter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () {

			path[0] = '\0';
			fourcc[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
			addOption ("fourcc", fourcc, 5, SSI_CHAR, "fourcc code (leave empty for no compression)");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}
		void setFourcc (const ssi_char_t *fourcc) {
			if (!fourcc) {
				this->fourcc[0] = '\0';
			}
			memcpy (this->fourcc, fourcc, 4);
			this->fourcc[4] = '\0';
		}

		ssi_char_t path[SSI_MAX_CHAR];
		ssi_char_t fourcc[5];
	};

public:

	static const ssi_char_t *GetCreateName () { return "AviWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new AviWriter (file); };
	~AviWriter ();

	AviWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Stores a video (and an optional audio) stream to an avi file."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_fail (ssi_time_t fail_time, 
		ssi_time_t fail_duration,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void setVideoFormat (ssi_video_params_t	video_format) { _video_format = video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
		}
		memcpy (&_video_format, meta, size);
	}
	ssi_video_params_t getVideoFormat () { return _video_format; };
	WAVEFORMATEX getAudioFormat () { return *_audio_format; };

protected:

	AviWriter (const ssi_char_t *file = 0);
	AviWriter::Options _options;
	ssi_char_t *_file;

	HAVI avi;
	const WAVEFORMATEX *_audio_format;
	ssi_video_params_t	_video_format;

	BITMAPINFO bi;
	void *bits;
	HBITMAP hbm;
	HGDIOBJ holdb;
	HPEN hp;
	HGDIOBJ holdp;
	HDC hdc;

	char errbuf [SSI_MAX_CHAR];
	
	const static ssi_char_t *ssi_log_name;
};

}


#endif
