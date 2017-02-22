// FileBinary.cpp
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

#include "ioput/file/FileBinary.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_size_t FileBinary::read (void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t old_pos = tell();

	size_t res = fread (ptr, size, count, _file);
	if (res != count) {
		ssi_wrn ("fread() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t new_pos = tell();

	return new_pos - old_pos;
}

ssi_size_t FileBinary::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t old_pos = tell();

	ssi_size_t res = ssi_cast (ssi_size_t, fwrite (ptr, size, count, _file));
	if (res != count) {
		ssi_wrn ("fwrite() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t new_pos = tell();

	return new_pos - old_pos;
}

ssi_size_t FileBinary::writeLine (const ssi_char_t *string) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_write_mode) {
		ssi_wrn ("file not in write mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	ssi_size_t result_1 = 0;
	ssi_size_t result_2 = 0;
	ssi_size_t len = ssi_cast (ssi_size_t, strlen (string));

	result_1 = write (&len, sizeof (ssi_size_t), 1);
	if (!result_1) 
	{
		ssi_wrn ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	result_2 = write (string, sizeof (ssi_char_t), len);
	if (!result_2)
	{
		ssi_wrn ("write() failed (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	return result_1 + result_2;
}

ssi_size_t FileBinary::readLine (ssi_size_t num, ssi_char_t *string) {

	if (!_is_open) {
		ssi_wrn ("file not open (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	if (!_read_mode) {
		ssi_wrn ("file not in read mode (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}
	
	ssi_size_t result_1 = 0;
	ssi_size_t result_2 = 0;
	ssi_size_t len = 0;

	result_1 = read (&len, sizeof (ssi_size_t), 1);
	if (!result_1)
	{
		return 0;
	}
	if (num <= len) 
	{
		ssi_wrn ("input string too short (path=%s, mode=%d, type=%d, shared=false)", _path, _mode, _type);
		return 0;
	}

	result_2 = read (string, ssi_cast (ssi_size_t, sizeof (ssi_char_t)), len);
	if (!result_2)
	{
		return 0;
	}
	string[len] = '\0';

	return result_1 + result_2;
}


}

