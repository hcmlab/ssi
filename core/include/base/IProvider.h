// IProvider.h
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

#ifndef SSI_IPROVIDER_H
#define SSI_IPROVIDER_H

#include "base/IObject.h"
#include "base/IComponent.h"
#include "base/IChannel.h"
#include "base/ITransformable.h"

/**
 * \brief Interface for a service that provides information. 
 *
 * A provider feeds information into the system. In fact, any source of information 
 * that generates output at a constant sample rate can be connected. In practise 
 * this would usally be a sensor device, such as a microphone or a video camera.
 *
 * Before a Provider can be used to stream data init() must be called once to 
 * tell SSI what kind of data will be provided. Whenever now data is available 
 * the input device can now call provide() to share it with connected entities.
 * 
 */

namespace ssi {

class IProvider : public IObject, public IComponent {

public:

	/**
	 * \brief Called once before runtime to initalize the provider.
	 * 
	 * @param channel	Channel information including sample rate, dimension, etc.
	 */ 
	virtual void init (IChannel *channel) = 0;

	/**
	 * \brief Called everytime data becomes available.
	 * 
	 * @param data Pointer to sample data. Values are stored interleaved.
	 * @param sample_number The number of samples.
	 * @return true if data was processed (e.g. written to buffer), false otherwise
	 */ 
	virtual bool provide (ssi_byte_t *data, 
		ssi_size_t sample_number) = 0;

	ssi_object_t getType () { return SSI_PROVIDER; };
};

}

#endif
