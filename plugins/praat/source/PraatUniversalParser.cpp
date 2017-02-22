// PraatUniversalParser.cpp
// author: Andreas Seiderer
// created: 2013/09/16
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "PraatUniversalParser.h"
#include "base/Factory.h"

#include <sstream>
#include <iostream>
#include <fstream>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

PraatUniversalParser::PraatUniversalParser(ssi_real_t undefined_value)
: _undefined_value(undefined_value) {
}

PraatUniversalParser::~PraatUniversalParser () {
	_parsed_names.clear();
	_parsed_values.clear();
}


//parser for a file that has the format: "key: value\n"
bool PraatUniversalParser::parseValues (std::string input) {

	std::istringstream f(input);
	std::string line;

	int pos;
	std::string value_name;

	_parsed_names.clear();
	_parsed_values.clear();

	std::getline(f, line);
	if (line.find("Error") != std::string::npos)
	{
		ssi_wrn("Error in Praat: %s", line.c_str());
		return false;
	}

	do {			
		if (line.size () > 2 && line.at (line.size()-2) != ':') {

			pos = ssi_cast(int, line.find (":")+2);

			value_name = line.substr (0,pos-2);
			_parsed_names.push_back (value_name);

			if (line.substr (pos, line.size () - 1).compare (0,13,"--undefined--") == 0) {		
				_parsed_values.push_back (getUndefinedValue());		//pushing back NaN
			} else {
				ssi_real_t value = ssi_cast (ssi_real_t, atof (line.substr(pos, line.size()-1).c_str ()));
				_parsed_values.push_back(value);
			}
		}			
	} while (std::getline(f, line));

	return true;
}

}
