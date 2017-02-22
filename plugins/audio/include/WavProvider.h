// WavProvider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/01/12
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

#ifndef SSI_WAV_WAVPROVIDER_H
#define SSI_WAV_WAVPROVIDER_H

#include "base/IProvider.h"
#include "ioput/option/OptionList.h"
#if _WIN32|_WIN64
#include "WavWriter.h"
#endif

namespace ssi {

class WavProvider : public IProvider {

public:

	class Options : public OptionList {

	public:

		Options () {

			path[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path  (empty for stdout)");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "WavProvider"; };
	static IObject *Create (const ssi_char_t *file) { return new WavProvider (file); };
	~WavProvider ();
	
	WavProvider::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Provides an audio stream to a wav file"; };

	void init (IChannel *channel);
	bool provide (ssi_byte_t *data, 
		ssi_size_t sample_number);

	ssi_time_t getSampleRate () { return _stream.sr; }
	ssi_size_t getSampleDimension () { return _stream.dim; }
	ssi_size_t getSampleBytes () { return _stream.byte; }
	ssi_type_t getSampleType () { return _stream.type; }

	const WAVEFORMATEX &getFormat () { return _format; }

private:

	WavProvider (const ssi_char_t *file = 0);
	WavProvider::Options _options;
	ssi_char_t *_file;

	ssi_stream_t _stream_t;
        #if _WIN32|_WIN64
	WavWriter *_writer;
        IConsumer::info _info;
        #endif
	ssi_stream_t _stream;
	WAVEFORMATEX _format;
	bool _call_flush;

	static const ssi_char_t *ssi_log_name;

};

}


#endif
