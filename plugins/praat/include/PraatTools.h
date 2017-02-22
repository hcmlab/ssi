// PraatTools.h
// author: Johannes Wagner
// created: 2014/06/05
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

#ifndef SSI_PRAATTOOLS_H
#define SSI_PRAATTOOLS_H

#include "SSI_Cons.h"

namespace ssi {

class PraatTools {

public:

	static std::string RunPraat (ssi_stream_t stream, const ssi_char_t *exe, const ssi_char_t *script, const ssi_char_t *script_args = 0, const ssi_char_t *tmpfile = "~.wav");
	static std::string RunCommand (const ssi_char_t *command);
};

}

#endif
