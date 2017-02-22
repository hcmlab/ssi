// Transformer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/26
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

#ifndef SSI_FRAME_TRANSFORMER_H
#define SSI_FRAME_TRANSFORMER_H

#include "FrameLibCons.h"
#include "base/ITransformer.h"
#include "base/ITransformable.h"

namespace ssi {

class TheFramework;

//! \brief Connects two buffers.
//
class Transformer : public Thread, public ITransformable {

friend class TheFramework;

public: 

	int getBufferId () {
		return _buffer_id_out;
	};
	ssi_time_t getSampleRate () {
		return _stream_out.sr;
	};
	ssi_size_t getSampleDimension () {
		return _stream_out.dim;
	}
	ssi_size_t getSampleBytes () {
		return _stream_out.byte;
	}
	ssi_type_t getSampleType () {
		return _stream_out.type;
	}

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	};
	
protected:

	Transformer (int _buffer_id_in, 
		ITransformer *transformer,
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,		
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		int trigger_id = -0);
	Transformer (int buffer_id_in, 
		int xtra_buffer_num,
		int *_xtra_buffer_ids,
		ITransformer *transformer,
		ssi_size_t frame_size,
		ssi_size_t delta_size = 0,		
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		int trigger_id = -1);
	virtual ~Transformer ();

	static char *ssi_log_name;
	static int ssi_log_level;

	void enter ();
	void run ();
	void flush ();

	void init (ssi_size_t _frame_size,
		ssi_size_t _delta_size,
		const ssi_char_t *buffer_size);

	bool check_trigger_stream(ssi_stream_t &s);
	int transform ();

	ITransformer *_transformer;
	ssi_stream_t _stream_in, _stream_out;
	ssi_stream_t _stream_trigger; // for trigger input
	ssi_size_t _sample_number_in, _sample_number_out, _sample_number_frame, _sample_number_delta;
	ssi_time_t _delta_size, _frame_size;
	ssi_lsize_t _read_pos;
	int _buffer_id_in, _buffer_id_out;
	int _trigger_id;

	ssi_size_t _xtra_stream_num;
	int *_xtra_stream_ids;
	ssi_stream_t *_xtra_streams;

	TheFramework *_frame;
};

}

#endif
