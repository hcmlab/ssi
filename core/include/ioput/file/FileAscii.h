// FileAscii.h
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

#pragma once

#ifndef SSI_IOPUT_FILEASCII_H
#define SSI_IOPUT_FILEASCII_H

#include "ioput/file/File.h"

namespace ssi {

#define SSI_FILEASCII_MAX_FORMAT	20 

class FileAscii : public File {

	friend class File;

public:

	// after reading elements_per_line elements a new line is expected instead of a delim
	// if elements_per_line == 0 all elements are expected to be in one line ending with a delim
	ssi_size_t read (void *ptr, ssi_size_t elements_per_line, ssi_size_t count);
	// after reading elements_per_line elements a new line is put instead of a delim
	// if elements_per_line == 0 all elements are put in one line ending with a delim
	ssi_size_t write (const void *ptr, ssi_size_t elements_per_line, ssi_size_t count);

	// reads characters before either newline or EOF occurs
	// if length of line exceeds num-1 an error is thrown
	ssi_size_t readLine (ssi_size_t num, ssi_char_t *string);
	// adds newline character to string and stores it
	ssi_size_t writeLine (const ssi_char_t *string);

	void setType (ssi_type_t type);
	void setFormat (const char *delim, const char *flags);
	
	void show ();

protected:

	FileAscii ();
	~FileAscii ();

	template<class T>
	static ssi_size_t read_h (FILE *file, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, void *ptr, ssi_size_t elements_per_line, ssi_size_t count);
	template<class T>
	static ssi_size_t write_h (FILE *file, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, const void *ptr, ssi_size_t elements_per_line, ssi_size_t count);

	ssi_type_t _type;
	char _format[SSI_FILEASCII_MAX_FORMAT+1];
	char *_format_with_delim, *_format_with_newline;
	char _delim[SSI_FILEASCII_MAX_FORMAT], _flags[SSI_FILEASCII_MAX_FORMAT];
};

}

#endif
