// FileReader.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/16
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

#ifndef SSI_IOPUT_FILEREADER_H
#define SSI_IOPUT_FILEREADER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "thread/Event.h"
#include "ioput/file/File.h"
#include "ioput/file/FileTools.h"
#include "ioput/file/FileStreamIn.h"
#include "ioput/option/OptionList.h"

#define SSI_FILEREADER_PROVIDER_NAME "file"

namespace ssi {

class FileReader : public ISensor, public Thread {
	
	class FileChannel : public IChannel {

		friend class FileReader;

		public:

			FileChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_UNDEF, 0);
			}
			~FileChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_FILEREADER_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Type and size of the stream is determined from the file."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: type (File::BINARY), block (1.0), blockInSamples (0), loop (true), offset (0), offsetInSamples (0), cutoff (0), cutoffInSamples (0) {

			path[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "filepath of input file or of a list with input files");			
			addOption ("block", &block, 1, SSI_DOUBLE, "block size in seconds");
			addOption ("blockInSamples", &blockInSamples, 1, SSI_SIZE, "block size in samples (overwrites 'block' if > 0)");
			addOption ("offset", &offset, 1, SSI_DOUBLE, "offset in seconds");
			addOption ("offsetInSamples", &offsetInSamples, 1, SSI_SIZE, "offset in samples (overwrites 'offset' if > 0)");
			addOption ("cutoff", &cutoff, 1, SSI_DOUBLE, "cutoff in seconds (if > 0)");
			addOption ("cutoffInSamples", &cutoffInSamples, 1, SSI_SIZE, "cutoff in samples (overwrites 'cutoff' if > 0)");
			addOption ("loop", &loop, 1, SSI_BOOL, "loop file");
		};

		void setPath (const ssi_char_t *path) {
			ssi_strcpy (this->path, path);
		}

		ssi_char_t path[SSI_MAX_CHAR];		
		bool loop;
		ssi_time_t block;
		ssi_size_t blockInSamples;
		ssi_time_t offset;
		ssi_size_t offsetInSamples;
		ssi_time_t cutoff;
		ssi_size_t cutoffInSamples;
		File::TYPE type;
	};

public:

	static const ssi_char_t *GetCreateName () { return "FileReader"; };
	static IObject *Create (const ssi_char_t *file) { return new FileReader (file); };
	~FileReader ();
	
	FileReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Reads in a SSI signal file and streams it through a single channel."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_file_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	const void *getMetaData (ssi_size_t &size) { 
		size = _n_meta;
		return _meta;
	};

	// waits either for a key (loop == true) or blocks until data is processed
	void wait () {
		if (_options.loop) { getchar (); } else { _event.wait (); }
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	FileReader (const ssi_char_t *file = 0);
	FileReader::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	bool prepare_file ();
	void setProvider (IProvider *provider);

	FileChannel _file_channel;
	IProvider *_provider;

	FileStreamIn _file_stream_in;
	ssi_size_t _n_meta;
	ssi_byte_t *_meta;
	ssi_stream_t _stream;
	
	ssi_size_t _step_counter, _max_steps;
	ssi_size_t _sample_number_total;
	ssi_size_t _sample_number_per_step;
	ssi_size_t _offset_in_bytes, _offset_in_samples;
	ssi_size_t _cutoff_in_samples;
	bool _is_providing;
	
	Timer *_timer;
	Event _event;
	bool _stopped;
};

}

#endif
