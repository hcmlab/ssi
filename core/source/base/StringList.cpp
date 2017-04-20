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

#include "base/StringList.h"

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
	push_back (String(filename));
}

bool StringList::find(const ssi_char_t *string, bool case_sensitive, ssi_size_t n)
{
	if (empty())
	{
		return false;
	}

	std::vector<String>::iterator iter;

	for (iter = begin(); iter != end(); ++iter)
	{
		if (!ssi_strcmp(iter->str(), string, case_sensitive, n))
		{
			return true;
		}
	}

	return false;
}

bool StringList::remove(const ssi_char_t *string, bool case_sensitive, ssi_size_t n)
{
	if (empty())
	{
		return false;
	}

	std::vector<String>::iterator iter;

	for (iter = begin(); iter != end(); ++iter)
	{
		if (ssi_strcmp(iter->str(), string, case_sensitive, n))
		{
			erase(iter);
			return true;
		}
	}

	return false;
}

bool StringList::parse(const ssi_char_t *string, ssi_char_t delim)
{
	if (!string || string[0] == '\0')
	{
		return false;
	}

	ssi_size_t n = ssi_split_string_count(string, delim, false);
	if (n > 0)
	{
		ssi_char_t **tokens = new ssi_char_t *[n];
		if (ssi_split_string(n, tokens, string, delim, false))
		{
			for (ssi_size_t i = 0; i < n; i++)
			{
				push_back(tokens[i]);
				delete[] tokens[i];
			}
			delete[] tokens;
		}		
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

const ssi_char_t *StringList::get(ssi_size_t index)
{
	if (index <= size())
	{
		return at(index).str();
	}

	ssi_wrn("index exceeds #strings (%u >= %u)", index, (ssi_size_t) size());

	return 0;
}

void StringList::print (FILE *file) {

	for (ssi_size_t i = 0; i < size (); i++) {
		fprintf (file, "%s\n", get (i));
	}
}

}
