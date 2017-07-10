// SignalTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#ifndef SSI_SIGNAL_SIGNALTOOLS_H
#define SSI_SIGNAL_SIGNALTOOLS_H

#include "base/ITransformer.h"
#include "base/IConsumer.h"
#include "signal/SignalCons.h"
#include "thread/Lock.h"

namespace ssi {

class SignalTools {

public:
	
	// old function:
	// delta corrensponds to a right context
	// the center of the first window is (frame + delta)/2	
	static void Transform (ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,		
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,
		bool call_enter = true,
		bool call_flush = true,
		bool copy_input = true);
	static void Transform(ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		bool call_enter = true,
		bool call_flush = true,
		bool copy_input = true);

	// refined function:
	// allow it to set a left and a right context
	// center of the first window is 0
	static void Transform(ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,
		ssi_size_t frame,
		ssi_size_t context_left,
		ssi_size_t context_right,
		bool call_enter = true,
		bool call_flush = true);
	static void Transform(ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,
		const ssi_char_t *frame,
		const ssi_char_t *context_left,
		const ssi_char_t *context_right,
		bool call_enter = true,
		bool call_flush = true);

	static void Transform_Xtra (ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,
		bool call_enter = true,
		bool call_flush = true,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	static void Transform_Xtra(ssi_stream_t &from,
		ssi_stream_t &to,
		ITransformer &transformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		bool call_enter = true,
		bool call_flush = true,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	static void Consume (ssi_stream_t &from,		
		IConsumer &consumer,		
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,
		bool call_enter = true,
		bool call_flush = true);
	static void Consume(ssi_stream_t &from,
		IConsumer &consumer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		bool call_enter = true,
		bool call_flush = true);

	// generates series
	static void Series (ssi_stream_t &series,		
		ssi_time_t duration, 		
		ssi_real_t offset = 0);

	// applies sine to series
	static void Sine (ssi_stream_t &series,
		ssi_time_t *frequency, 
		ssi_real_t *amplitude);

	// applies cosine to series
	static void Cosine (ssi_stream_t &series, 
		ssi_time_t *frequency, 
		ssi_real_t *amplitude);

	// fills with random values
	static void Random(ssi_stream_t &stream);

	// sum series along dimension
	static void Sum (ssi_stream_t &series);
};

}

#endif
