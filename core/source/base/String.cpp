// String.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/15
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

ssi_char_t *String::ssi_log_name = "string____";

String::String () {
	_string = new ssi_char_t[1];
	_string[0] = '\0';
}

String::String (const ssi_char_t *string) {
	if (string == 0) {
		_string = new ssi_char_t[1];
		_string[0] = '\0';
	} else {
		_string = ssi_strcpy (string);
	}
}

String::String (const String &string) {
	_string = ssi_strcpy (string._string);	
}

String::~String () {
	delete[] _string;
}

String & String::operator=(const String &other) {
	 if (this != &other) {
		delete[] _string;
		_string = ssi_strcpy (other._string);
	 }
	return *this;
}

String & String::operator=(const ssi_char_t *other) {
	delete[] _string;
	if (other == 0) {
		_string = new ssi_char_t[1];
		_string[0] = '\0';
	} else {
		_string = ssi_strcpy (other);
	}
	return *this;
}

bool String::operator==(const String &other) const {
	return strcmp (_string, other._string) == 0;
}

bool String::operator!=(const String &other) const {
    return !(*this == other);
}

bool String::operator< (const String &other) const {
	return strcmp (_string, other._string) > 0;
}

bool String::operator==(const char *other) const {
	return strcmp (_string, other) == 0;
}

bool String::operator!=(const char *other) const {
    return !(*this == other);
}

bool String::operator< (const char *other) const {
	return strcmp (_string, other) > 0;
}

String & String::operator+=(const String &other) {
	
	ssi_char_t *cat_string = ssi_strcat (_string, other._string);
	delete[] _string;
	_string = cat_string;

	return *this;
}

String & String::operator+=(const char *other) {

	ssi_char_t *cat_string = ssi_strcat (_string, other);
	delete[] _string;
	_string = cat_string;

	return *this;
}

const String String::operator+(const String &other) const {
	return String(*this) += other;
}

const String String::operator+(const char *other) const {
	return String(*this) += other;
}

}
