// PythonFilter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
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

#ifndef SSI_PYTHON_FILTER_H
#define SSI_PYTHON_FILTER_H

#include "base/IFilter.h"
#include "PythonOptions.h"

namespace ssi {

class PythonHelper;

class PythonFilter : public IFilter {

public:

	class Options : public PythonOptions {
	};

public:

	static const ssi_char_t *GetCreateName () { return "PythonFilter"; };
	static IObject *Create (const ssi_char_t *file) { return new PythonFilter (file); };
	~PythonFilter ();

	PythonFilter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Python filter wrapper."; };

	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in);

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress();
	void send_enter();
	void send_flush();

	void listen_enter();
	bool update(ssi_event_t &e);
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

protected:

	PythonFilter (const ssi_char_t *file = 0);
	PythonFilter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	void initHelper();
	PythonHelper *_helper;
};

}

#endif
