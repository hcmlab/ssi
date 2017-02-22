// FileAscii.cpp
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

#include "ioput/file/FileAscii.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

FileAscii::FileAscii () 
	: _format_with_delim (0),
	_format_with_newline (0),
	_type (SSI_CHAR) {
	
	setFormat (0, 0); 
};

void FileAscii::setType (ssi_type_t type) {

	if (_type == type) {
		return;
	}

	_type = type;
	setFormat (_delim, _flags); 
}

void FileAscii::setFormat (const char *delim,
	const char *flags) {

	ssi_strcpy (_delim, delim ? delim : SSI_FILE_DEFAULT_DELIM);
	ssi_strcpy (_flags, flags ? flags : SSI_FILE_DEFAULT_FLAGS);

	delete[] _format_with_delim;
	delete[] _format_with_newline;

	switch (_type) {
		case SSI_CHAR:
			sprintf (_format, "%%%sc", _flags);
			break;
		case SSI_UCHAR:
			sprintf (_format, "%%%sc", _flags);
			break;
		case SSI_SHORT:
			sprintf (_format, "%%%shd", _flags);
			break;
		case SSI_USHORT:
			sprintf (_format, "%%%shu", _flags);
			break;
		case SSI_INT:
			sprintf (_format, "%%%sd", _flags);
			break;
		case SSI_UINT:
			sprintf (_format, "%%%su", _flags);
			break;
		case SSI_LONG:
			sprintf (_format, "%%%sld", _flags);
			break;
		case SSI_ULONG:
			sprintf (_format, "%%%slu", _flags);
			break;
		case SSI_FLOAT:
			sprintf (_format, "%%%sf", _flags);
			break;
		case SSI_DOUBLE:
			sprintf (_format, "%%%sLf", _flags);
			break;
		default:
			ssi_err ("unsupported sample type");
	}
	_format_with_delim = ssi_strcat (_format, _delim);
	_format_with_newline = ssi_strcat (_format, "\n");	
}

FileAscii::~FileAscii () {

	delete[] _format_with_delim;
	delete[] _format_with_newline;
}

ssi_size_t FileAscii::read (void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	ssi_size_t result = 0;

	switch (_type) {
		case SSI_CHAR:
			result = FileAscii::read_h<char> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UCHAR:
			result = FileAscii::read_h<unsigned char> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_SHORT:
            result = FileAscii::read_h<int16_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_USHORT:
            result = FileAscii::read_h<uint16_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_INT:
            result = FileAscii::read_h<int32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UINT:
            result = FileAscii::read_h<uint32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_LONG:
            result = FileAscii::read_h<int32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_ULONG:
            result = FileAscii::read_h<uint32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_FLOAT:
			result = FileAscii::read_h<float> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_DOUBLE:
			result = FileAscii::read_h<double> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		default:
			ssi_err ("unsupported sample type");
	}

	if (!result) {
		ssi_wrn ("fscanf() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
	}

	return result;
}

template<class T>
SSI_INLINE ssi_size_t FileAscii::read_h (FILE *file, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, void *ptr, ssi_size_t size, ssi_size_t count) {

	int res = 0;
	int rescount = 0;
	const T *ptrT = ssi_pcast (const T, ptr);
	for (ssi_size_t i = 0; i < count; ++i) {
		if (size > 0 && !((i+1) % size)) {
			res = fscanf (file, format_with_newline, ptrT++);
			rescount += res;
			if (res != 1) {
				return 0;
			}
		} else {
			res = fscanf (file, format_with_delim, ptrT++); 
			rescount += res;
			if (res != 1) {
				return 0;
			}
		}
	}

	return rescount;
}

ssi_size_t FileAscii::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return false;
	}
	
	ssi_size_t result = 0;

	switch (_type) {
		case SSI_CHAR:
			result = FileAscii::write_h<char> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UCHAR:
			result = FileAscii::write_h<unsigned char> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_SHORT:
            result = FileAscii::write_h<int16_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_USHORT:
            result = FileAscii::write_h<uint16_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_INT:
            result = FileAscii::write_h<int32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UINT:
            result = FileAscii::write_h<uint32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_LONG:
            result = FileAscii::write_h<int32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_ULONG:
            result = FileAscii::write_h<uint32_t> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_FLOAT:
			result = FileAscii::write_h<float> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_DOUBLE:
			result = FileAscii::write_h<double> (_file, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		default:
			ssi_err ("unsupported sample type");
	}

	if (!result) {
		ssi_wrn ("fprintf() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
	}

	return result;
}

template<class T>
SSI_INLINE ssi_size_t FileAscii::write_h (FILE *file, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, const void *ptr, ssi_size_t size, ssi_size_t count) {

	int res = 0;
	const T *ptrT = ssi_pcast (const T, ptr);
	for (ssi_size_t i = 0; i < count; ++i) {
		if (size > 0 && !((i+1) % size)) {
			res = fprintf (file, format_with_newline, *ptrT++); 
			if (res < 0) {
				return false;
			}
		} else {
			res = fprintf (file, format_with_delim, *ptrT++);
			if (res < 0) {
				return false;
			}
		}
	}

	return (ssi_size_t) res;
}

ssi_size_t FileAscii::writeLine (const ssi_char_t *string) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	int res = fprintf (_file, "%s\n", string);
	if (res < 0) {
		ssi_wrn ("fprintf() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	return (ssi_size_t)res;
}

ssi_size_t FileAscii::readLine (ssi_size_t num, ssi_char_t *string) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	int res = 0;
	char *string_ptr = string;
	char c;
	c = getc (_file);
	while (c != '\n' && c != EOF) {
		if (c != '\r') {
			if (--num == 0) {
				*string_ptr = '\0';
				ssi_wrn ("input string to short (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
				return 0;
			}
			*string_ptr++ = c;
		}
		c = getc (_file);
		res++;
	}
	*string_ptr = '\0';

	if (ferror (_file)) {
		ssi_wrn ("readLine() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	return res;
}


void FileAscii::show () {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return;
	}

	ssi_size_t store_pos = tell ();
	char c;

	seek (0, File::BEGIN);
	c = fgetc (_file);
	while (c != EOF) {
		putc (c, stdout);
		c = fgetc (_file);
	}

	seek (store_pos, File::BEGIN);
}

}

