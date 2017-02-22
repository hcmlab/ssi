// Asynchronous.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/03/12
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

#include "Asynchronous.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Asynchronous::Asynchronous (const ssi_char_t *file)
	: _file (0),
	_transformer (0),
	_run (true,true),
	_ready (false),
	_stop (false),
	_xtra_stream_in_num (0),
	_xtra_stream_in (0),
	_meta_size (0),
	_meta_data (0),
	_first_call (false) {

	setName ("asyncworker");

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Asynchronous::~Asynchronous () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	
	delete[] _meta_data;			
	_meta_data = 0;
	_meta_size = 0;
}

void Asynchronous::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_stream_in = stream_in;
	_stream_out = _stream_out_tmp = stream_out;
	_xtra_stream_in_num = xtra_stream_in_num;
	if (_xtra_stream_in_num > 0) {
		_xtra_stream_in = new ssi_stream_t[_xtra_stream_in_num];
		for (ssi_size_t i = 0; i < _xtra_stream_in_num; i++) {
			_xtra_stream_in[i] = xtra_stream_in[i];
		}
	}

	_transformer->transform_enter (stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);

	_first_call = true;
	_ready = true;
	_stop = false;

	start ();
}

void Asynchronous::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_first_call) {
		ssi_stream_adjust (_stream_out, stream_out.num);
		ssi_stream_zero (_stream_out);			
		_first_call = false;
	}

	{
		Lock lock (_mutex);
		if (_ready) {		
			_ready = false;
			ssi_stream_adjust (_stream_in, stream_in.num);
			memcpy (_stream_in.ptr, stream_in.ptr, stream_in.tot);
			_info = info;
			for (ssi_size_t i = 0; i < _xtra_stream_in_num; i++) {
				ssi_stream_adjust (_xtra_stream_in[i], xtra_stream_in[i].num);
				memcpy (_xtra_stream_in[i].ptr, xtra_stream_in[i].ptr, xtra_stream_in[i].num);
			}
			ssi_stream_adjust (_stream_out_tmp, stream_out.num);
			_run.release ();
		}
		memcpy (stream_out.ptr, _stream_out.ptr, stream_out.tot);
	}	
}

void Asynchronous::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	{
		Lock lock (_mutex);
		_stop = true;
	}
	_run.release ();
	stop ();

	_transformer->transform_flush (stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);

	ssi_stream_destroy (_stream_in);	
	for (ssi_size_t i = 0; i < _xtra_stream_in_num; i++) {
		ssi_stream_destroy (xtra_stream_in[i]);
	}
	delete[] xtra_stream_in;
	ssi_stream_destroy (_stream_out);
	ssi_stream_destroy (_stream_out_tmp);
	xtra_stream_in = 0;
}

void Asynchronous::run () {

	{
		Lock lock (_mutex);
		if (_stop) {
			return;
		}
	}
	_run.wait ();
	{
		Lock lock (_mutex);
		if (_stop) {
			return;
		}
	}

	_transformer->transform (_info, _stream_in, _stream_out_tmp, _xtra_stream_in_num, _xtra_stream_in);

	{
		Lock lock (_mutex);
		_ready = true;
		ssi_stream_adjust (_stream_out, _stream_out_tmp.num);
		memcpy (_stream_out.ptr, _stream_out_tmp.ptr, _stream_out.tot);
	}
}

}
