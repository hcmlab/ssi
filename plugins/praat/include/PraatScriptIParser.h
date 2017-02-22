// PraatScriptBase.h
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

#ifndef SSI_PRAAT_SCRIPTIPARSER_H
#define SSI_PRAAT_SCRIPTIPARSER_H

#include "SSI_Cons.h"

namespace ssi {

class PraatScriptIParser {

public:

	virtual ~PraatScriptIParser () {};

	virtual ssi_size_t getNumberOfValues () = 0;
	virtual ssi_real_t getValue (ssi_size_t index) = 0;
	virtual ssi_real_t getUndefinedValue() = 0;
	virtual bool parseValues (std::string praatOutput) = 0;
	virtual const ssi_char_t *getValueName (ssi_size_t index) = 0;


};

}

#endif
