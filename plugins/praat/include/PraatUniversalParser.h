// speechAnalyzer.h
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
// version 3 of the License, or any laterversion.
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

#pragma once

#ifndef SSI_PRAAT_UNIVERSALPARSER_H
#define SSI_PRAAT_UNIVERSALPARSER_H

#include "PraatScriptIParser.h"

namespace ssi {

class PraatUniversalParser : public PraatScriptIParser {

public:

	PraatUniversalParser(ssi_real_t undefined_value);
	~PraatUniversalParser ();

	ssi_size_t getNumberOfValues () {
		SSI_ASSERT (_parsed_names.size() == _parsed_values.size());
		return ssi_cast (ssi_size_t, _parsed_names.size()); 
	}
	ssi_real_t getValue (ssi_size_t index) {
		return _parsed_values.at(index);
	}
	bool parseValues (std::string praatOutput);
	const ssi_char_t *getValueName (ssi_size_t index) {
		return _parsed_names.at(index).c_str();
	}
	ssi_real_t getUndefinedValue() {
		return _undefined_value;
	}

protected:

	std::vector<std::string> _parsed_names;
	std::vector<ssi_real_t> _parsed_values;

	ssi_real_t _undefined_value;
};

}

#endif
