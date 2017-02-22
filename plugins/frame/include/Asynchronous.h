// Asynchronous.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/11/18
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

/**

Provides down-sampling.

*/

#pragma once

#ifndef SSI_SIGNAL_ASYNCHRONOUS_H
#define SSI_SIGNAL_ASYNCHRONOUS_H

#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"
#include "thread/Thread.h"
#include "thread/Event.h"
#include "thread/Lock.h"

namespace ssi {

class Asynchronous : public ITransformer, public Thread {

public:

	class Options : public OptionList {

	public:

		Options ()
			: factor (1) {

			addOption ("factor", &factor, 1, SSI_SIZE, "downsample factor");		
		};

		ssi_size_t factor;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Asynchronous"; };
	static IObject *Create (const ssi_char_t *file) { return new Asynchronous (file); };
	~Asynchronous ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Turns a non real-time capable transformer into an asynchronous transformer that always receives data from the head of the buffer."; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return _transformer->getSampleDimensionOut (sample_dimension_in);
	}
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) {
		return _transformer->getSampleNumberOut (sample_number_in);
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return _transformer->getSampleBytesOut (sample_bytes_in);
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		return _transformer->getSampleTypeOut (sample_type_in);
	}

	virtual void setTransformer (ITransformer *transformer) {
		_transformer = transformer;
	}

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	const void *getMetaData (ssi_size_t &size) { 
		size = _meta_size;
		return _meta_data; 
	};

	void setMetaData (ssi_size_t size, const void *meta) {
		
		ssi_size_t meta_size = size;
		const void *meta_data = meta;				

		_transformer->setMetaData (meta_size, meta_data);
		meta_data = _transformer->getMetaData (meta_size);		
		
		if (meta_size > 0) {
			_meta_size = meta_size;
			_meta_data = new ssi_byte_t[_meta_size];
			memcpy (_meta_data, meta_data, _meta_size);			
		}
		
	};

protected:

	Asynchronous (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	ITransformer *_transformer;

	void run ();

	bool _first_call;
	Event _run;
	Mutex _mutex;
	bool _ready;
	bool _stop;
	ssi_stream_t _stream_in;
	ITransformer::info _info;
	ssi_stream_t _stream_out;
	ssi_stream_t _stream_out_tmp;
	ssi_size_t _xtra_stream_in_num;
	ssi_stream_t *_xtra_stream_in;

	ssi_size_t _meta_size;
	ssi_byte_t *_meta_data;

};

}

#endif
