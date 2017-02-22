// FileMessage.h
// author: Florian Lingenfelsr <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/06/12
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

#ifndef SSI_FILE_FILEMESSAGE_H
#define SSI_FILE_FILEMESSAGE_H

#include "base/IMessage.h"
#include "thread/Lock.h"
#include <stdarg.h> 
namespace ssi {

class FileMessage :  public IMessage {

public:

	FileMessage (const char *logpath = 0) {
		if (logpath) {
			_is_file = true;
			_file = fopen (logpath, "w");
		} else {
			_is_file = false;
			_file = stdout;
		}
	}

	~FileMessage () {
		if (_is_file) {
			fclose (_file);
		}
	}

	void print (const char* text, ...) {
		Lock lock(_mutex);
		{
			va_list args;
			va_start(args, text);
			vfprintf (_file, text, args);
			va_end(args);
			fflush (_file);
		}
	}

	void err (const char* logname, const char* file, int line, const char* text, ...){
	
		Lock lock(_mutex);
		{
			fprintf (_file, "[%s] # !ERROR! # ", logname);
			va_list args;
			va_start(args, text);
			vfprintf (_file, text, args);
			va_end(args);
			fprintf (_file, "\nlocation: %s (%d)\n", file, line);
			fflush (_file);
		}
	};

	void wrn (const char* logname, const char* file, int line, const char* text, ...) {

		Lock lock(_mutex);
		{
			fprintf (_file, "[%s] # !WARNING! # ", logname);
			va_list args;
			va_start(args, text);
			vfprintf (_file, text, args);
			va_end(args);
			fprintf (_file, "\nlocation: %s (%d)\n", file, line);
			fflush (_file);
		}
	
	};

	void msg (const char* logname, const char* text, ...) {

		Lock lock(_mutex);
		{
			fprintf (_file, "[%s] ", logname);
			va_list args;
			va_start(args, text);
			vfprintf (_file, text, args);
			va_end(args);
			fprintf (_file, "\n");
			fflush (_file);
		}
	
	};

protected:

	Mutex _mutex;
	ssi_char_t _string [SSI_MAX_CHAR];

	bool _is_file;
	FILE *_file;

};

}

#endif
