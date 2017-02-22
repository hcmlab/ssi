// MyObject.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/09/17
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

#ifndef _MYOBJECT_H
#define _MYOBJECT_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
using namespace ssi;

class MyObject : public IObject {

public:

	class Options : public OptionList {

	public:

		Options () 
			: toggle (false) {

			string[0] = '\0';
			addOption ("toogle", &toggle, 1, SSI_BOOL, "i'm a toggle");		
			addOption ("string", string, SSI_MAX_CHAR, SSI_CHAR, "i'm a string");
		}

		void setString (const ssi_char_t *string) {
			this->string[0] = '\0';
			if (string) {
				ssi_strcpy (this->string, string);
			}
		}

		bool toggle;
		ssi_char_t string[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "myobject"; };
	static IObject *Create (const ssi_char_t *file) { return new MyObject (file); };
	~MyObject ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "just a sample object"; };

	bool notify(INotify::COMMAND::List command, ssi_char_t *message = 0);

	virtual void print ();

protected:

	MyObject (const ssi_char_t *file = 0);
	ssi_char_t *_file;
	Options _options;
	static char ssi_log_name[];
};

#endif
