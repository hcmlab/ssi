// FileStreamIn.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/27
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_IOPUT_FILESTREAMIN_H
#define SSI_IOPUT_FILESTREAMIN_H

#include "ioput/file/File.h"

namespace ssi {

class FileStreamIn {

public:

	static File::VERSION DEFAULT_VERSION;
	static ssi_size_t NEXT_CHUNK;
	static ssi_size_t READ_ERROR;

public:

	FileStreamIn ();
	virtual ~FileStreamIn ();

	bool open (ssi_stream_t &data,
		const ssi_char_t *path);
	bool open (ssi_stream_t &data,
		const ssi_char_t *path,
		ssi_size_t &n_meta,
		void **meta);
	bool close ();
	ssi_size_t read (ssi_stream_t &data); // tries to read data.num samples (must be pre-allocated!), ignores chunks and returns number of read samples on access!
	ssi_size_t read (ssi_stream_t &data, ssi_size_t chunk_id); // tries to read next chunk (re-allocates if necessary) and returns number of read samples
	void reset () {
		_next_chunk = 0;
	}
	
	File *getInfoFile () {
		return _file_info;
	}
	File *getDataFile() {
		return _file_data;
	}
	ssi_size_t getChunkSize () {
		return _n_chunks;
	}
	ssi_size_t getTotalSampleSize () {
		return _n_samples;
	}
	File::VERSION getVersion () {
		return _version;
	}

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	File *_file_info;
	File *_file_data;
	File::VERSION _version;
	ssi_stream_t _stream;
	ssi_size_t _n_chunks;
	ssi_size_t _next_chunk;
	ssi_size_t _n_samples;
	ssi_size_t *_samples;
	ssi_time_t *_time;
	ssi_size_t *_bytes;
	ssi_char_t _string[1024];
	ssi_char_t *_path;
};

}

#endif
