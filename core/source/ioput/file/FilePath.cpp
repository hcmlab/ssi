// FilePath.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/21
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

#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

FilePath::FilePath (const ssi_char_t *filepathfull)
	: _filepath (0),
	_filepathfull (0),
	_filename (0),
	_filenamefull (0),
	_dirpath (0),
	_extension (0),
	_is_relative (true) {

	_filepathfull = ssi_strcpy (filepathfull);
	parse ();
}

FilePath::~FilePath () {

	delete[] _filepath;
	delete[] _filepathfull;
	delete[] _filename;
	delete[] _filenamefull;
	delete[] _dirpath;
	delete[] _extension;
}

void FilePath::print (FILE *file) {

	ssi_fprint (file, "%s\n  path = %s\n  name = %s\n  full = %s\n  dir  = %s\n  ext  = %s\n", _filepathfull, _filepath, _filename, _filenamefull, _dirpath, _extension);
}

void FilePath::parse () {	

	size_t len = strlen (_filepathfull);
	size_t pos = len;
	while (pos > 0 && (_filepathfull[pos - 1] != '\\' && _filepathfull[pos - 1] != '/')) {
		pos--;
	}
	
	_dirpath = new ssi_char_t[pos+1];
	_filenamefull = new ssi_char_t[len-pos+1];
	if (pos > 0) {
		memcpy (_dirpath, _filepathfull, pos * sizeof (ssi_char_t));
	} 
	if (pos != len) {
		memcpy (_filenamefull, _filepathfull+pos, (len-pos) * sizeof (ssi_char_t));
	}
	_dirpath[pos] = '\0';
	_filenamefull[len-pos] = '\0';

	len = strlen (_filenamefull);
	pos = len;
	while (pos > 0 && (_filenamefull[pos - 1] != '.')) {
		pos--;
	}
		
	if (pos > 1) {
		_filename = new ssi_char_t[pos];		
		memcpy (_filename, _filenamefull, (pos-1) * sizeof (ssi_char_t));
		_filename[pos-1] = '\0';
		_extension = new ssi_char_t[len-pos+2];
		memcpy (_extension, _filenamefull+pos-1, (len-pos+1) * sizeof (ssi_char_t));
		_extension[len-pos+1] = '\0';
	} else {
		_filename = ssi_strcpy (_filenamefull);
		_extension = new ssi_char_t[1];
		_extension[0] = '\0';
	}	
	
	_filepath = ssi_strcat (_dirpath, _filename);

	len = strlen (_filepathfull);
	_is_relative = true;
	if (len >= 2 && _filepathfull[1] == ':') {
		_is_relative = false;
	}
}

}

