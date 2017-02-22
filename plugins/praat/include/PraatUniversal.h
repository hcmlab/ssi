// PraatUniversal.h
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

#ifndef SSI_CONSUMER_PRAATUNIVERSAL_H
#define SSI_CONSUMER_PRAATUNIVERSAL_H

#include "PraatScriptBase.h"

namespace ssi {

class PraatUniversal : public PraatScriptBase {

public:

	static const ssi_char_t *GetCreateName () { return "PraatUniversal"; };
	static IObject *Create (const ssi_char_t *file) { return new PraatUniversal (file); };
	~PraatUniversal ();
	
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "praat universal consumer"; };

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	PraatUniversal (const ssi_char_t *file = 0);

	static char *ssi_log_name;
	int ssi_log_level;

	PraatScriptIParser *getParser ();
};

}

#endif
