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
#include "base/String.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

FilePath::FilePath (const ssi_char_t *path, const ssi_char_t *ext)
	: _filepath (0),
	_filepathfull (0),
	_filename (0),
	_filenamefull (0),
	_dirpath (0),
	_extension (0),
	_is_relative (true) {

	if (ext)
	{
		FilePath fp(path);		
		if (strcmp(fp.getExtension(), ext) != 0) 
		{
			_filepathfull = ssi_strcat(path, ext);
		}
		else 
		{
			_filepathfull = ssi_strcpy(path);
		}		
	}
	else
	{
		_filepathfull = ssi_strcpy(path);
	}
	
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
    if ((len >= 2 && _filepathfull[1] == ':') ||_filepathfull[0]=='/') {
		_is_relative = false;
	}
}

ssi_char_t *FilePath::getUnique(bool createDirectory)
{
	if (!_filepathfull || _filepathfull[0] == '\0')
	{
		ssi_wrn("empty file path")
		return 0;
	}		

	ssi_size_t leadingZeros = 0;
	bool hasVariable = ssi_strhas(_filepathfull, "$(num");
	ssi_char_t *variable = 0;
	if (hasVariable)
	{
		if (ssi_strhas(_filepathfull, "$(num)"))
		{
			variable = ssi_strcpy("$(num)");
		}
		else
		{
			int pos = ssi_strfind(_filepathfull, "$(num");
			ssi_size_t from = (ssi_size_t) pos;
			ssi_char_t *chr = _filepathfull + from;
			bool found = false;
			while (*chr != '\0')
			{
				if (*chr == ',')
				{
					found = true;
					break;
				}
				from++;
				chr++;
			}
			if (found)
			{
				found = false;
				ssi_size_t to = from;
				while (*chr != '\0')
				{
					if (*chr == ')')
					{
						found = true;
						break;
					}
					to++;
					chr++;
				}
				from++;
				if (found && to > from)
				{
					ssi_char_t *numstr = ssi_strsub(_filepathfull, from, to);					
					int num = atoi(numstr);
					if (num >= 0)
					{
						leadingZeros = (ssi_size_t)num;
						variable = ssi_strsub(_filepathfull, pos, to + 1);						
					}
					else
					{
						ssi_wrn("could not parse number variable");
						hasVariable = false;
					}
					delete[] numstr;
				}
				else
				{
					ssi_wrn("could not parse number variable");
					hasVariable = false;
				}
			}
		}
	}

	if (!hasVariable && !ssi_exists(_filepathfull))
	{
		if (createDirectory)
		{
			ssi_mkdir_r(_dirpath);
		}
		return ssi_strcpy(_filepathfull);
	}
	
	ssi_char_t number[25];
	ssi_char_t *result = 0;
	ssi_size_t count = 1;	
	do
	{			
		delete[] result;
		ssi_sprint(number, "%0*d", leadingZeros, count++);
		if (hasVariable && variable)
		{
			result = ssi_strrepl(_filepathfull, variable, number);
		}
		else
		{
			result = ssi_strcat(_filepath, number, _extension);
		}
	} while (ssi_exists(result));

	delete[] variable;

	if (createDirectory)
	{
		FilePath fp(result);
		ssi_mkdir_r(fp.getDir());
	}	

	return result;
}

}

