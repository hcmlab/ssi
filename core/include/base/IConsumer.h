// IConsumer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/26
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

#ifndef SSI_ICONSUMER_H
#define SSI_ICONSUMER_H

#include "base/IObject.h"
#include "base/IComponent.h"

/**
 * \brief Interface for a service that consumes information. 
 *
 * A Consumer opens the system to the outside world. SSI contains generic consumers for 
 * storing information to disk, visualize it in a graph, or sending it to an external 
 * application through a socket connection. Consumers can also serve as classifiers, which
 * map continuous observations to discrete categories. Because a consumer reads information 
 * from one or more buffer without writing information back to the system, it is not 
 * restricted to work with a constant sample rate (see Trigger).
 *
 */

namespace ssi {

class ITrigger;

class IConsumer : public IObject, public IComponent {

public:

	/**
	 * The consume status gives information if the consume was invoked by a trigger and
	 * if the triggered segment is completed or being continued.
	 *
	 * NO_TRIGGER: no trigger is used
	 * COMPLETED: the segment is completed
	 * CONTINUED: the segment is not completed yet
	 */
	enum STATUS {
		NO_TRIGGER = 0,
		COMPLETED,
		CONTINUED	
	};

	/**
	* The info struct contains additional information on the streams.
	*/
	struct info {
		ssi_time_t time;
		ssi_time_t dur;
		STATUS status;
		ssi_event_t *event;
	};

	/**
	 * \brief Called before the consumer thread is started.
	 * 
	 * @param stream_in_num The number of input streams.
	 * @param stream_in[] An array with the input streams.
	 */
	virtual void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {};

	/**
	 * \brief Called when new data becomes available.
	 * 
	 * @param consume_info A struct with additional information on the streams.
	 * @param stream_in_num The number of input streams.
	 * @param stream_in[] An array with the input streams.
	 */
	virtual void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) = 0;

	/**
	 * \brief Called after the consumer is stopped.
	 * 
	 * @param stream_in_num The number of input streams.
	 * @param stream_in[] An array with the input streams.
	 */
	virtual void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {};

	/**
	 * \brief Called if consume() failed because requested data was not available.
	 *
	 * @param fail_tie Beginning of loss in seconds
	 * @param fail_tie Duration of loss in seconds
	 * @param stream_in_num The number of input streams.
	 * @param stream_in[] An array with the input streams.
	 */
	virtual void consume_fail (ssi_time_t fail_time, 
		ssi_time_t fail_duration,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {};

	ssi_object_t getType () { return SSI_CONSUMER; };
};

}

#endif
