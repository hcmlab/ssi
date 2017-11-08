// FileMemAscii.cpp
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

#include "ioput/file/FileMemAscii.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
#if __gnu_linux__
#define _snprintf snprintf
#endif
namespace ssi {

FileMemAscii::FileMemAscii ()
	: _format_with_delim (0),
	_format_with_newline (0),
	_format_with_delim_read (0),
	_format_with_newline_read (0),
	_type (SSI_CHAR) {

	ssi_strcpy (_delim, " ");
	ssi_strcpy (_flags, "");
	setFormat (_delim, _flags);
};

void FileMemAscii::setType (ssi_type_t type) {

	if (_type == type) {
		return;
	}

	_type = type;
	setFormat (_delim, _flags);
}

void FileMemAscii::setFormat (const char *delim,
	const char *flags) {

	ssi_strcpy (_delim, delim);
	ssi_strcpy (_flags, flags);

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
			return;
	}

	delete[] _format_with_delim;
	delete[] _format_with_newline;
	delete[] _format_with_delim_read;
	delete[] _format_with_newline_read;

	_format_with_delim = ssi_strcat (_format, _delim);
	_format_with_newline = ssi_strcat (_format, "\n");
	_format_with_delim_read = ssi_strcat (_format_with_delim, "%n");
	_format_with_newline_read = ssi_strcat (_format_with_newline, "%n");
}

FileMemAscii::~FileMemAscii () {

	delete[] _format_with_delim;
	delete[] _format_with_newline;
	delete[] _format_with_delim_read;
	delete[] _format_with_newline_read;
}

ssi_size_t FileMemAscii::read (void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_memory) {
		ssi_wrn ("memory not open");
		return 0;
	}

	if (!ready ()) {
		ssi_wrn ("skip read() since end of memory has been reached");
		return 0;
	}

	int result = 0;

	switch (_type) {
		case SSI_CHAR:
			result = FileMemAscii::read_h<char> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_UCHAR:
			result = FileMemAscii::read_h<unsigned char> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_SHORT:
            result = FileMemAscii::read_h<int16_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_USHORT:
            result = FileMemAscii::read_h<uint16_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_INT:
            result = FileMemAscii::read_h<int32_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_UINT:
            result = FileMemAscii::read_h<uint32_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_LONG:
            result = FileMemAscii::read_h<int32_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_ULONG:
            result = FileMemAscii::read_h<uint32_t> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_FLOAT:
			result = FileMemAscii::read_h<float> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		case SSI_DOUBLE:
			result = FileMemAscii::read_h<double> (_pos, _n_memory, _memory, _format_with_delim_read, _format_with_newline_read, ptr, size, count);
			break;
		default:
			ssi_wrn ("unsupported sample type");
			return 0;
	}

	return move (result);
}

template<class T>
SSI_INLINE int FileMemAscii::read_h (ssi_size_t pos, ssi_size_t n_memory, ssi_byte_t *memory, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, void *ptr, ssi_size_t size, ssi_size_t count) {

	int res = 0;
	int n = 0;

	const T *ptrT = ssi_pcast (const T, ptr);
	for (ssi_size_t i = 0; i < count; ++i) {
		if (size > 0 && !((i+1) % size)) {
			res = sscanf (memory + pos, format_with_newline, ptrT++, &n);
			if (res != 1) {
				ssi_wrn ("skip read() since end of memory has been reached");
				return pos;
			}
		} else {
			res = sscanf (memory + pos, format_with_delim, ptrT++, &n);
			if (res != 1) {
				ssi_wrn ("skip read() since end of memory has been reached");
				return pos;
			}
		}
		pos += n;
	}

	return pos;
}

ssi_size_t FileMemAscii::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_memory) {
		ssi_wrn ("memory not open");
		return 0;
	}

	if (!ready ()) {
		ssi_wrn ("skip write() since end of memory has been reached");
		return 0;
	}

	int result = 0;

	switch (_type) {
		case SSI_CHAR:
			result = FileMemAscii::write_h<char> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UCHAR:
			result = FileMemAscii::write_h<unsigned char> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_SHORT:
            result = FileMemAscii::write_h<int16_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_USHORT:
            result = FileMemAscii::write_h<uint16_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_INT:
            result = FileMemAscii::write_h<int32_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_UINT:
            result = FileMemAscii::write_h<uint32_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_LONG:
            result = FileMemAscii::write_h<int32_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_ULONG:
            result = FileMemAscii::write_h<uint32_t> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_FLOAT:
			result = FileMemAscii::write_h<float> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		case SSI_DOUBLE:
			result = FileMemAscii::write_h<double> (_pos, _n_memory, _memory, _format_with_delim, _format_with_newline, ptr, size, count);
			break;
		default:
			ssi_wrn ("unsupported sample type");
			return 0;
	}

	return move (result);
}

template<class T>
SSI_INLINE int FileMemAscii::write_h (ssi_size_t pos, ssi_size_t n_memory, ssi_byte_t *memory, const ssi_char_t *format_with_delim, const ssi_char_t *format_with_newline, const void *ptr, ssi_size_t size, ssi_size_t count) {

	int res = 0;

	const T *ptrT = ssi_pcast (const T, ptr);
	for (ssi_size_t i = 0; i < count; ++i) {
		if (size > 0 && !((i+1) % size)) {
			res = _snprintf (memory+pos, n_memory-pos-1, format_with_newline, *ptrT++);
			if (res < 0) {
				ssi_wrn ("skip write() since end of memory has been reached");
				return pos;
			}
		} else {
			res = _snprintf (memory+pos, n_memory-pos-1, format_with_delim, *ptrT++);
			if (res < 0) {
				ssi_wrn ("skip write() since end of memory has been reached");
				return pos;
			}
		}
		pos += res;
	}

	return pos;
}

ssi_size_t FileMemAscii::writeLine (const ssi_char_t *string) {

	if (!_memory) {
		ssi_wrn ("memory not open");
		return 0;
	}

	if (!ready ()) {
		ssi_wrn ("skip writeLine() since end of memory has been reached");
		return 0;
	}

	int res = _snprintf (_memory+_pos, _n_memory-_pos-1, "%s\n", string);
	if (res < 0) {
		ssi_wrn ("skip writeLine() since end of memory has been reached");
		return 0;
	}

	return move (_pos + res);
}

ssi_size_t FileMemAscii::readLine (ssi_size_t num, ssi_char_t *string) {

	if (!_memory) {
		ssi_wrn ("memory not open");
		return 0;
	}

	if (!ready ()) {
		ssi_wrn ("skip readLine() since end of memory has been reached");
		return 0;
	}

	char *string_ptr = string;
	char c;
	c = _memory[_pos++];
	while (c != '\n' && _pos < _n_memory) {
		if (c != '\r') {
			if (--num == 0) {
				*string_ptr = '\0';
				ssi_wrn ("input string to short");
				return 0;
			}
			*string_ptr++ = c;
		}
		c = _memory[_pos++];
	}
	*string_ptr = '\0';

	return num;
}


void FileMemAscii::show () {

	if (!_memory) {
		ssi_wrn ("memory not open");
		return;
	}

	ssi_char_t str[20];
	ssi_val2str(_type, _memory + _pos, 20, str);
	ssi_print ("%s\n", str);
}

}

