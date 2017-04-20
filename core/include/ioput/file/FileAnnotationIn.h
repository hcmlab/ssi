// FileAnnotationIn.h
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

#ifndef SSI_IOPUT_FILEANNOTATIONIN_H
#define SSI_IOPUT_FILEANNOTATIONIN_H

#include "base/IAnnotation.h"
#include "ioput/file/File.h"
#include "base/String.h"

namespace ssi {

class SampleList;

class FileAnnotationIn {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileAnnotationIn ();
	virtual ~FileAnnotationIn ();

	bool open (const ssi_char_t *path);	
	bool close ();

	const ssi_scheme_t *getScheme ();
	const ssi_label_t *get(ssi_size_t index);
	const ssi_label_t *next ();
	void reset();

	ssi_size_t getMetaSize();	
	const ssi_char_t *getMeta(const ssi_char_t *key);
	const ssi_char_t *getMetaKey(ssi_size_t index);

	ssi_size_t getSize();	
	File *getInfoFile();
	File *getDataFile();	
	File::VERSION getVersion();

	static void SetLogLevel(int level);

protected:

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	File *_file_info;
	File *_file_data;
	File::VERSION _version;	
	ssi_size_t _n_labels;		
	ssi_size_t _label_count;
	ssi_scheme_t *_scheme;
	ssi_label_t _label;
	std::map<String, String> _meta;

	ssi_char_t _string[1024];
	ssi_char_t *_path;	
};

}

#endif
