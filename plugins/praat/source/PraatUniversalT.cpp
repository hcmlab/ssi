// PraatUniversalT.cpp
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

#include "PraatUniversalT.h"
#include "PraatUniversalParser.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

char *PraatUniversalT::ssi_log_name = "praatuniv_t";

PraatUniversalT::PraatUniversalT (const ssi_char_t *file)
	: PraatScriptBaseT (file), 
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
}

PraatUniversalT::~PraatUniversalT () {
}

//parser for a file that has the format: "key: value\n"
PraatScriptIParser *PraatUniversalT::getParser () {
	return new PraatUniversalParser (_options.undefined_value);
}

}
