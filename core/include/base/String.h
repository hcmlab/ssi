// String.h
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

#pragma once

#ifndef SSI_BASE_STRING_H
#define SSI_BASE_STRING_H

#include "SSI_Cons.h"

namespace ssi {

class String {

public:

	String ();
	String (const ssi_char_t *string);
	~String ();
	String (const String &string);
	String & operator=(const String &other);
	String & operator=(const ssi_char_t *string);
	bool operator==(const String &other) const;
	bool operator==(const char *string) const;
	bool operator!=(const String &other) const;
	bool operator!=(const char *string) const;
	bool operator<(const String &other) const;
	bool operator<(const char *string) const;
	String & operator+=(const String &other);
	String & operator+=(const char *string);
	const String operator+(const String &other) const;
	const String operator+(const char *other) const;

	ssi_char_t *str () const { return _string; };

protected:

	static ssi_char_t *ssi_log_name;

	ssi_char_t *_string;
};

}

#endif
