// IChannel.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#ifndef SSI_ICHANNEL_H
#define SSI_ICHANNEL_H

#include "SSI_Cons.h"

/**
 * \brief Interface for a sensor channel.
 *
 * An instance of ISensor usually provides one or more channels which can be connected via an instance of IProvider.
 * 
 */

namespace ssi {

class IChannel {

public:

	virtual ~IChannel () {};

	virtual const ssi_char_t *getName () = 0;
	virtual const ssi_char_t *getInfo () = 0;
	virtual ssi_stream_t getStream () = 0;
	virtual ssi_stream_t* getStreamPtr () {return 0;};
};

}

#endif
