// FileSamplesIn.h
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

#ifndef SSI_IOPUT_FILESAMPLESIN_H
#define SSI_IOPUT_FILESAMPLESIN_H

#include "base/ISamples.h"
#include "ioput/file/File.h"
#include "ioput/file/FileStreamIn.h"

namespace ssi {

class SampleList;

class FileSamplesIn : public ISamples {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileSamplesIn ();
	virtual ~FileSamplesIn ();

	bool open (const ssi_char_t *path);	
	bool close ();

	void reset () {
		if (_file_data) {
			_file_data->seek (0);
			_sample_count = 0;
		}
	}
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 	

	ssi_size_t getSize () {
		return _n_samples;
	}
	ssi_size_t getSize (ssi_size_t class_index) {
		if (class_index >= _n_classes) {
			ssi_err ("index '%u' excees #classes '%u'", class_index, _n_classes);
		}
		return _n_per_class[class_index];
	}

	ssi_size_t getClassSize () {
		return _n_classes;	
	}
	const ssi_char_t *getClassName (ssi_size_t class_index) {
		if (class_index == SSI_ISAMPLES_GARBAGE_CLASS_ID) {
			return SSI_ISAMPLES_GARBAGE_CLASS_NAME;
		} else if (class_index >= _n_classes) {
			ssi_err ("index '%u' exceeds #classes '%u'", class_index, _n_classes);
		}
		return _classes[class_index];
	}
	
	ssi_size_t getUserSize () {
		return _n_users;	
	}
	const ssi_char_t *getUserName (ssi_size_t user_index) {
		if (user_index >= _n_users) {
			ssi_err ("index '%u' exceeds #users '%u'", user_index, _n_users);
		}
		return _users[user_index];
	}

	ssi_size_t getStreamSize () {
		return _n_streams;
	}
	ssi_stream_t getStream (ssi_size_t stream_index) {
		if (stream_index >= _n_streams) {
			ssi_err ("index '%u' exceeds #streams '%u'", stream_index, _n_streams);
		}
		return _streams[stream_index];
	}

	bool hasMissingData () {
		return _has_missing_data;
	}
	bool supportsShallowCopy () {
		return false;
	}
	
	File *getInfoFile () {
		return _file_info;
	}
	File *getDataFile() {
		return _file_data;
	}
	ssi_size_t getSampleSize () {
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
	FileStreamIn *_file_streams;
		
	ssi_size_t _n_samples;	
	ssi_size_t _sample_count;
	ssi_size_t _n_streams;
	ssi_stream_t *_streams;
	ssi_sample_t _sample;
	ssi_size_t _n_classes;	
	ssi_size_t *_n_per_class;
	ssi_char_t **_classes;
	ssi_size_t _n_users;
	ssi_size_t *_n_per_user;
	ssi_char_t **_users;
	bool _has_missing_data;

	ssi_char_t _string[1024];
	ssi_char_t *_path;	
};

}

#endif
