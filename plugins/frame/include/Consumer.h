// Consumer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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

#ifndef SSI_FRAME_CONSUMER_H
#define SSI_FRAME_CONSUMER_H

#include "ConsumerBase.h"
#include "base/IConsumer.h"
#include "base/ITransformer.h"

namespace ssi {

//! \brief Consumes _data from a buffer.
class Consumer : public ConsumerBase, public Thread {

friend class TheFramework;

public:

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	};

protected: 

	virtual ~Consumer ();
	
	Consumer (int buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,
		ITransformer *transformer = 0,
		int trigger_id = -1);
	Consumer (ssi_size_t stream_number, 
		int *buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,
		ITransformer **transformer = 0,
		int trigger_id = -1);

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	void enter ();
	void run ();
	void flush ();

	TheFramework *_frame;
	IConsumer::info _consume_info;
};

}

#endif
