// File.cpp
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

#include "ioput/file/File.h"
#include "ioput/file/FileAscii.h"
#include "ioput/file/FileBinary.h"
#include "ioput/file/FileBinaryLZ4.h"
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
	
ssi_char_t *File::MODE_NAMES[6] = {"READ", "WRITE", "APPEND", "READ+", "WRITE+", "APPEND+"};
ssi_char_t *File::TYPE_NAMES[3] = {"BINARY", "ASCII", "BIN_LZ4"};
ssi_char_t *File::VERSION_NAMES[4] = {"V0", "V1", "V2", "V2EX"};

int File::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t File::ssi_log_name[] = "file______";

File::VERSION  File::DEFAULT_VERSION = File::V2;
ssi_char_t File::FILE_ID[] = "SSI";

File::File () 
	: _read_mode (false),
	_write_mode (false),
	_path (0),
	_file (0),
	_close_file (false),
	_is_open (false) {
}

File *File::CreateAndOpen (TYPE type,
	MODE mode,
	const char *path) {

	File *file = Create (type, mode, path);
	if (!file->isOpen ()) {
		file->open ();
	}
	return file;
}

File *File::Create (TYPE type,
	MODE mode,
	const char *path,
	FILE *fileptr) {

	File *file = 0;

	switch (type) {
		case File::ASCII: {
			file = new FileAscii ();
			break;
		}
		case File::BINARY: {
			file = new FileBinary ();
			break;
		}
		case File::BIN_LZ4: {
			file = new FileBinaryLZ4 ();
			break;
		}
		default:
			ssi_err ("type not supported");
	}

	file->_type = type;
	file->_mode = mode;
	if (path == 0 || path[0] == '\0') {
		file->_path = ssi_strcpy ("");		
		file->_file = fileptr ? fileptr : ssiout;	
		file->_is_open = true;	
		file->_close_file = false;		
	} else {
		file->_path = ssi_strcpy (path);
		file->_file = fileptr;	
		file->_is_open = fileptr != 0;	
		file->_close_file = fileptr == 0;		
	}
	file->init ();

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "created ('%s', %s, %s, shared=%s)", file->_path, MODE_NAMES[file->_mode], TYPE_NAMES[file->_type], file->_is_open ? "true" : "false");
	return file;
}

File::~File () {

	if (_close_file && _is_open && _file) {
		close ();
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "destroyed ('%s', %s, %s, shared=false)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
	delete[] _path;
}

void File::init () {

	switch (_mode) {
		case File::READ: {
			_read_mode = true;
			break;
		}
		case File::WRITE: {
			_write_mode = true;
			break;
		}
		case File::APPEND: {
			_write_mode = true;
			break;
		}
		case File::READPLUS: {
			_read_mode = _write_mode = true;
			break;
		}
		case File::WRITEPLUS: {
			_read_mode = _write_mode = true;
			break;
		}
		case File::APPENDPLUS: {
			_read_mode = _write_mode = true;
			break;
		}
	}
}

bool File::open () {

	if (_is_open) {
		ssi_wrn ("file already open ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (_file) {
		ssi_wrn ("file pointer not empty ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}	

	if (!_close_file) {
		ssi_wrn ("no permission to open file ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	FilePath fp(_path);
	if (!ssi_exists_dir(fp.getDir()))
	{
		if (!ssi_mkdir_r(fp.getDir()))
		{			
			ssi_wrn("could not create directory '%s'", fp.getDir());
			ssi_PrintLastError();
			return false;
		}
	}

	switch (_type) {

		case File::ASCII: {
			switch (_mode) {
				case File::READ: {
					_file = ssi_fopen(_path, "r");
					break;
				}
				case File::WRITE: {
					_file = ssi_fopen(_path, "w");
					break;
				}
				case File::APPEND: {
					_file = ssi_fopen (_path, "a");
					break;
				}
				case File::READPLUS: {
					_file = ssi_fopen (_path, "r+");
					break;
				}
				case File::WRITEPLUS: {
					_file = ssi_fopen (_path, "w+");
					break;
				}
				case File::APPENDPLUS: {
					_file = ssi_fopen (_path, "a+");
					break;
				}
			}
			break;
		}

		case File::BINARY: {
			switch (_mode) {
				case File::READ: {
					_file = ssi_fopen (_path, "rb");
					break;
				}
				case File::WRITE: {
					_file = ssi_fopen (_path, "wb");
					break;
				}
				case File::APPEND: {
					_file = ssi_fopen (_path, "ab");
					break;
				}
				case File::READPLUS: {
					_file = ssi_fopen (_path, "r+b");
					break;
				}
				case File::WRITEPLUS: {
					_file = ssi_fopen (_path, "w+b");
					break;
				}
				case File::APPENDPLUS: {
					_file = ssi_fopen (_path, "a+b");
					break;
				}
			}
			break;
		}
	};

	if (!_file) {
		ssi_wrn ("fopen() failed (\"%s\")", _path);
		return false;
	}
	
	_is_open = true;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "opened ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
	return true;
}

bool File::close () {

	if (!_is_open) {
		ssi_wrn ("file already closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (!_file) {
		ssi_wrn ("file pointer empty ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return true;
	}

	if (!_close_file) {
		ssi_wrn ("no permission to close file ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	if (fclose (_file)) {
		ssi_wrn ("fclose() failed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	_is_open = false;
	_file = 0;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
	return true;
}


bool File::seek (int32_t offset, File::ORIGIN origin) {

	if (!_is_open) {
		ssi_wrn ("file is closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	int res;

	switch (origin) {
		case File::BEGIN:
			res = fseek (_file, offset, SEEK_SET);
			break;
		case File::CURRENT:
			res = fseek (_file, offset, SEEK_CUR);
			break;
		case File::END:
			res = fseek (_file, offset, SEEK_END);
			break;
	}

	return res == 0;
}

ssi_size_t File::tell () {

	if (!_is_open) {
		ssi_wrn ("file is closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return 0;
	}

	return ftell (_file);
}

bool File::ready () {

	if (!_is_open) {
		ssi_wrn ("file is closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	if (fgetc (_file) == EOF) {
		return false;
	} else {
		fseek (_file, -1, SEEK_CUR);
		return true;
	}
}

bool File::flush () {

	if (!_is_open) {
		ssi_wrn ("file is closed ('%s', %s, %s)", _path, MODE_NAMES[_mode], TYPE_NAMES[_type]);
		return false;
	}

	return fflush (_file) == 0;
}

}
