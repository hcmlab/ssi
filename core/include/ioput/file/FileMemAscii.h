// FileMemAscii.h
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

#pragma once

#ifndef SSI_IOPUT_FILEMEMASCII_H
#define SSI_IOPUT_FILEMEMASCII_H

#include "ioput/file/FileMem.h"

namespace ssi {

#define FILEASCII_MAX_FORMAT	20 

class FileMemAscii : public FileMem {

	friend class FileMem;

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

	FileMemAscii ();
	~FileMemAscii ();

	template<class T>
	static int read_h (ssi_size_t pos, ssi_size_t n_memory, ssi_byte_t *memory, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, void *ptr, ssi_size_t elements_per_line, ssi_size_t count);
	template<class T>
	static int write_h (ssi_size_t pos, ssi_size_t n_memory, ssi_byte_t *memory, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, const void *ptr, ssi_size_t elements_per_line, ssi_size_t count);

	ssi_type_t _type;
	char _format[FILEASCII_MAX_FORMAT+1];
	char *_format_with_delim, *_format_with_newline;
	char *_format_with_delim_read, *_format_with_newline_read;
	char _delim[FILEASCII_MAX_FORMAT], _flags[FILEASCII_MAX_FORMAT];
};

}

#endif
