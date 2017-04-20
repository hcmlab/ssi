// FileStreamOut.h
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

#ifndef SSI_IOPUT_FILESTREAMOUT_H
#define SSI_IOPUT_FILESTREAMOUT_H

#include "ioput/file/File.h"

namespace ssi {

class FileStreamOut {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileStreamOut ();
	virtual ~FileStreamOut ();

	bool open (ssi_stream_t &data, // data is not written!		
		const ssi_char_t *path,
		File::TYPE type, 
		File::VERSION version = DEFAULT_VERSION);
	bool open (ssi_stream_t &data, // data is not written!		
		const ssi_char_t *path,
		ssi_size_t n_meta, 
		const void *meta,		
		ssi_size_t n_keys,
		ssi_char_t **keys,		
		ssi_char_t **values,
		File::TYPE type, 
		File::VERSION version = DEFAULT_VERSION);
	bool close ();
	bool write (ssi_stream_t &data, 
		bool continued);

	void setDelim(const ssi_char_t *delim);	
	
	File *getInfoFile () {
		return _file_info;
	}
	File *getDataFile() {
		return _file_data;
	}

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	bool _is_open;
	File *_file_info;
	File *_file_data;
	ssi_stream_t _stream;
	ssi_size_t _sample_count;
	ssi_time_t _last_time;
	int64_t _last_byte;
	ssi_char_t _string[1024];
	bool _console;
	ssi_char_t *_path;
	ssi_char_t *_delim;
};

}

#endif
