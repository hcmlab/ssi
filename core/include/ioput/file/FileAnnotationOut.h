// FileAnnotationOut.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/26
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

#ifndef SSI_IOPUT_FILEANNOTATIONOUT_H
#define SSI_IOPUT_FILEANNOTATIONOUT_H

#include "base/String.h"
#include "ioput/file/File.h"
#include "base/IAnnotation.h"

namespace ssi {

class FileAnnotationOut {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileAnnotationOut ();
	virtual ~FileAnnotationOut ();

	bool open (const ssi_char_t *path,
		ssi_scheme_t &scheme,
		File::TYPE ftype, 
		File::VERSION version = DEFAULT_VERSION);
	bool close ();
	bool write (IAnnotation &data);
	bool write (ssi_label_t &data);
	bool writeMeta(const ssi_char_t *key, const ssi_char_t *value);

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
	File::VERSION _version;
	ssi_scheme_t _scheme;

	std::map<String, String> _meta;

	ssi_size_t _n_labels;
	ssi_char_t _string[SSI_MAX_CHAR];
	bool _console;
	ssi_char_t *_path;
};

}

#endif
