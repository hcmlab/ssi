// FilePath.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/06
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

#ifndef SSI_IOPUT_FILEPATH_H
#define SSI_IOPUT_FILEPATH_H

#include "SSI_Cons.h"

namespace ssi {

class FilePath {

public:

	FilePath (const ssi_char_t *_filepathfull);	
	virtual ~FilePath ();

	const ssi_char_t *getPath () { return _filepath; };
	const ssi_char_t *getPathFull () { return _filepathfull; };
	const ssi_char_t *getDir () { return _dirpath; };	
	const ssi_char_t *getName () { return _filename; };
	const ssi_char_t *getNameFull () { return _filenamefull; };
	const ssi_char_t *getExtension () { return _extension; };
	bool isRelative () { return _is_relative; };

	void print (FILE *file = stdout);

protected:

	void parse ();

	ssi_char_t *_filepath;
	ssi_char_t *_filepathfull;
	ssi_char_t *_filename;
	ssi_char_t *_filenamefull;
	ssi_char_t *_dirpath;
	ssi_char_t *_extension;
	bool _is_relative;
};

}

#endif
