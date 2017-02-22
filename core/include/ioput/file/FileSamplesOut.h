// FileSamplesOut.h
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

#ifndef SSI_IOPUT_FILESAMPLESOUT_H
#define SSI_IOPUT_FILESAMPLESOUT_H

#include "ioput/file/File.h"
#include "base/ISamples.h"
#include "ioput/file/FileStreamOut.h"

namespace ssi {

class FileSamplesOut {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileSamplesOut ();
	virtual ~FileSamplesOut ();

	bool open (ISamples &data, // samples are not written!		
		const ssi_char_t *path,
		File::TYPE type, 
		File::VERSION version = DEFAULT_VERSION);
	bool close ();
	bool write (ISamples &data);
	bool write (ssi_sample_t &data);
	
	ssi_size_t getClassId(const ssi_char_t *name);
	ssi_size_t getUserId(const ssi_char_t *name);

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

	File *_file_info;
	File *_file_data;
	File::VERSION _version;
	FileStreamOut *_file_streams;

	ssi_size_t _n_samples;
	ssi_size_t _n_classes;
	ssi_char_t **_classes;
	ssi_size_t *_n_per_class;
	ssi_size_t _n_garbage_class;
	ssi_size_t _n_users;
	ssi_char_t **_users;
	ssi_size_t *_n_per_user;
	ssi_size_t _n_streams;	
	ssi_stream_t *_streams;	
	bool _has_missing_data;
	
	ssi_char_t _string[1024];
	bool _console;
	ssi_char_t *_path;
};

}

#endif
