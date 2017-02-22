// ConsumerBase.h
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

#ifndef SSI_FRAME_CONSUMERBASE_H
#define SSI_FRAME_CONSUMERBASE_H

#include "FrameLibCons.h"
#include "base/IConsumer.h"
#include "base/ITransformer.h"

namespace ssi {

class TheFramework;

//! \brief Consumes _data from a buffer.
class ConsumerBase {

friend class Trigger;
friend class EventConsumer;

public: 

	ConsumerBase (int _buffer_id,
		IConsumer *_consumer, 
		ssi_size_t _frame_size,
		ssi_size_t _delta_size = 0,
		ITransformer *transformer = 0,
		int trigger_id = -1);
	ConsumerBase (ssi_size_t _stream_number, 
		int *_buffer_id,
		IConsumer *_consumer, 
		ssi_size_t _frame_size,
		ssi_size_t _delta_size = 0,
		ITransformer **transformer = 0,
		int trigger_id = -1);
	virtual ~ConsumerBase ();
		
	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	void init (int *buffer_id,
		ITransformer **transformer);

	void enter ();
	int consume (IConsumer::info info);
	void flush ();

	bool check_trigger_stream (ssi_stream_t &s);

	IConsumer *_consumer;
	ITransformer **_transformer;

	ssi_size_t _stream_number;
	int *_buffer_id;
	int *_consume_status;
	int _trigger_id;
	int _trigger_status;

	ssi_stream_t *_streams_trans; // for transformed input
	ssi_stream_t *_streams_raw; // for raw input
	ssi_stream_t _stream_trigger; // for trigger input
	ssi_stream_t *_streams; // final stream array

	ssi_size_t _frame_size, _delta_size;
	ssi_time_t _frame_size_in_sec, _delta_size_in_sec;

	TheFramework *_frame;
};

}

#endif
