// ISensor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/01
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

#ifndef SSI_ISENSOR_H
#define SSI_ISENSOR_H

#include "base/IProvider.h"
#include "base/IRunnable.h"
#include "base/IChannel.h"

namespace ssi {

class ISensor : public SSI_IRunnableObject {

public:

	virtual ssi_size_t getChannelSize () = 0;
	virtual IChannel *getChannel (ssi_size_t index) = 0;
	virtual bool setProvider (const ssi_char_t *name, IProvider *provider) = 0;
	virtual bool connect () = 0;
	virtual bool disconnect () = 0;

	ssi_object_t getType () { return SSI_SENSOR; };
};

class IWaitableSensor : public ISensor, public IWaitable {};

}

#endif
