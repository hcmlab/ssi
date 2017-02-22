// MyObject.cpp
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

#include "MyObject.h"

char MyObject::ssi_log_name[] = "myobject__";

MyObject::MyObject (const ssi_char_t *file)
: _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MyObject::~MyObject () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool MyObject::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {

	case INotify::COMMAND::OPTIONS_CHANGE:
	{
		_options.lock();		
		char *value = 0;
		if (_options.getOptionValueAsString(message, &value)) {
			ssi_msg(SSI_LOG_LEVEL_BASIC, "new value of option '%s' is %s", message, value);
		}
		_options.unlock();
		delete[] value;

		return true;
	}

	case INotify::COMMAND::RESET:
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "reset");

		return true;
	}

	}

	return false;
};
