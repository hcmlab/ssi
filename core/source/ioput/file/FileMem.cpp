// FileMem.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/21
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

#include "ioput/file/FileMem.h"
#include "ioput/file/FileMemAscii.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {
	
ssi_char_t *FileMem::TYPE_NAMES[2] = {"BINARY", "ASCII"};

int FileMem::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileMem::ssi_log_name[] = "filemem___";

FileMem::FileMem () 
	: _shallow (true),
	_n_memory (0),
	_memory (0),
	_pos (0) {
}

FileMem *FileMem::Create (TYPE type,
	ssi_size_t n_memory,
	ssi_byte_t *memory,
	bool shallow) {

	FileMem *file = 0;

	switch (type) {
		case FileMem::ASCII: {
			file = new FileMemAscii ();
			break;
		}
		case FileMem::BINARY: {
			ssi_err ("sorry, not yet implemented");
			break;
		}
		default:
			ssi_err ("type not supported");
	}

	file->_type = type;	
	file->set (n_memory, memory, shallow);

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "created memory (%s, shallow=%s)", TYPE_NAMES[file->_type], file->_shallow ? "true" : "false");

	return file;
}

FileMem::~FileMem () {

	clear ();

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "destroyed memory ('%s', shallow=%s)", TYPE_NAMES[_type], _shallow ? "true" : "false");
}

void FileMem::clear () {
	
	if (!_shallow) {
		delete[] _memory;
	}
	_n_memory = 0;
	_pos = 0;
	_memory = 0;	
}

void FileMem::make (ssi_size_t n_memory) {

	clear ();
	_shallow = false,

	_n_memory = n_memory;
	_memory = new ssi_byte_t[_n_memory];

	if (_type == FileMem::ASCII) {
		_memory[0] = '\0';
	}
}

void FileMem::set (ssi_size_t n_memory, 
	ssi_byte_t *memory, 
	bool shallow) {

	clear ();

	_shallow = shallow;

	if (!_shallow && n_memory > 0) {
		_n_memory = n_memory;
		_memory = new ssi_byte_t[_n_memory];
		memcpy (_memory, memory, _n_memory);
	} else {
		_n_memory = n_memory;
		_memory = memory;
	}

}

ssi_size_t FileMem::move (ssi_size_t newpos) {
	ssi_size_t oldpos = _pos;
	if (newpos > _n_memory - (_type == FileMem::ASCII ? 1 : 0)) {
		ssi_wrn ("position out of memory (%u > %u)", newpos, _n_memory - (_type == FileMem::ASCII ? 1 : 0));
		return 0;
	}	
	_pos = newpos;
	return newpos-oldpos;
}

bool FileMem::seek (int32_t offset, FileMem::ORIGIN origin) {

	if (!_memory) {
		ssi_wrn ("memory is closed");
		return false;
	}

	ssi_size_t res;

	switch (origin) {
		case FileMem::BEGIN:
			res = move (offset);
			break;
		case FileMem::CURRENT:
			res = move (_pos + offset);			
			break;
		case FileMem::END:
			res = move (_n_memory + offset);			
			break;
	}

	return res != 0;
}

ssi_size_t FileMem::tell () {

	if (!_memory) {
		ssi_wrn ("memory is closed");
		return 0;
	}

	return _pos;
}

bool FileMem::ready () {

	if (!_memory) {
		ssi_wrn ("memory is closed");
		return false;
	}

	if (_pos >= _n_memory - (_type == FileMem::ASCII ? 1 : 0)) {
		return false;
	}

	return true;
}


}
