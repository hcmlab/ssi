// StringList.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/04
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

#include "ioput/file/StringList.h"

#if __gnu_linux__
#include <string.h>
#define _stricmp strcmp
#endif
namespace ssi {

StringList::StringList () {
}

StringList::~StringList () {
	clear ();
}

void StringList::add (const ssi_char_t *filename) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new entry: %s\n", filename);

	ssi_char_t *new_filename = ssi_strcpy (filename);
	_strings.push_back (new_filename);
}

const ssi_char_t *StringList::get (ssi_size_t index) const {

	if (index >= size ()) {
		ssi_err ("index (%u) out of boundary (%u)", index, size ());
	}

	return _strings[index];
}

bool StringList::remove(const ssi_char_t *fileNameToRemove)
{
	if(_strings.empty())
		return false;

	std::vector<ssi_char_t*>::iterator searchIter;

	for(searchIter = _strings.begin(); searchIter != _strings.end(); ++searchIter)
	{
		if(!_stricmp(*searchIter, fileNameToRemove))
		{
			_strings.erase(searchIter);
			return true;
		}
	}

	return false;
}

ssi_size_t StringList::size () const {
	return ssi_cast (ssi_size_t , _strings.size ());
}

void StringList::clear () {
	for (ssi_size_t i = 0; i < size (); i++) {
		delete[] _strings[i];
	}
	_strings.clear ();
}

void StringList::print (FILE *file) {

	for (ssi_size_t i = 0; i < size (); i++) {
		fprintf (file, "%s\n", get (i));
	}
}

}
