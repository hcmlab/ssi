// OSPlpChain.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

#include "OSPlpChain.h"
#include "base/Factory.h"
#include "../../signal/include/Derivative.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OSPlpChain::OSPlpChain (const ssi_char_t *file)
	: _chain (0),
	_file (0) {

	ssi_char_t string[SSI_MAX_CHAR];

	if (file) {

		ssi_sprint (string, "%s.OSTransformFFT", file);
		_fft = ssi_factory_create (OSTransformFFT, string, false);
		ssi_sprint (string, "%s.OSFFTmagphase", file);
		_fftmag = ssi_factory_create (OSFFTmagphase, string, false);
		ssi_sprint (string, "%s.OSSpecScale", file);
		_spect = ssi_factory_create (OSSpecScale, string, false);
		ssi_sprint (string, "%s.OSPlp", file);
		_plp = ssi_factory_create (OSPlp, string, false);
		ssi_sprint (string, "%s.Deltas", file);

		if (USE_DERIVATIVE) {
			_deltas = ssi_factory_create (Derivative, string, false);
		} else {
			_deltas = ssi_factory_create (Deltas, string, false);
		}

		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);

	} else {

		_fft = ssi_factory_create (OSTransformFFT, 0, false);
		_fftmag = ssi_factory_create (OSFFTmagphase, 0, false);
		_spect = ssi_factory_create (OSSpecScale, 0, false);
		_plp = ssi_factory_create (OSPlp, 0, false);

		if (USE_DERIVATIVE) {
			_deltas = ssi_factory_create (Derivative, 0, false);
		} else {
			_deltas = ssi_factory_create (Deltas, 0, false);
		}

	}

	_filter[0] = _fftmag;
	_filter[1] = _spect;
	_filter[2] = _plp;
	_filter[3] = _deltas;
}

OSPlpChain::~OSPlpChain () {

	delete _fft;
	delete _fftmag;
	delete _spect;
	delete _plp;
	delete _deltas;
	delete _chain;	

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void OSPlpChain::initChain () {

	_chain = ssi_pcast(Chain, Factory::Create(Chain::GetCreateName(), 0, false));
	if (!_chain) {
		ssi_err ("could not create chain");
	}

	if(_options.deltas_enable){		
		_chain->set ( 4, _filter, 0, 0 );
	}
	else {
		_chain->set ( 3, _filter, 0, 0 );
	}
}

void OSPlpChain::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (!_chain) {
		initChain ();
	}

	ssi_stream_init (_stream_fft_in,
		0,
		stream_in.dim,
		stream_in.byte,
		stream_in.type,
		stream_in.sr);

	ssi_stream_init (_stream_fft_out, 
		0, 
		_fft->getSampleDimensionOut (stream_in.dim),
		_fft->getSampleBytesOut (stream_in.byte),
		_fft->getSampleTypeOut (stream_in.type),
		stream_out.sr);

	_fft->transform_enter (stream_in, _stream_fft_out);
	_chain->transform_enter (_stream_fft_out, stream_out);

	frame = _options.frame;
	delta = _options.delta;

	_first_call = true;
}

void OSPlpChain::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	//ssi_stream_adjust (_stream_fft_in, stream_in.num + delta);
	ssi_stream_adjust (_stream_fft_in, stream_in.num);
	
	if (_first_call) {
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, _stream_fft_in.ptr);
		for (ssi_size_t i = 0; i < _stream_fft_in.num * _stream_fft_in.dim; i++) {
			*ptr++ = 0;
		}
		_first_call = false;
	}

	ssi_size_t n_shifts = getSampleNumberOut (info.frame_num);
	ssi_stream_adjust (_stream_fft_out, n_shifts);
	
	//memcpy (_stream_fft_in.ptr + delta * _stream_fft_in.dim * _stream_fft_in.byte, stream_in.ptr, stream_in.num * _stream_fft_in.dim * _stream_fft_in.byte);
	//ssi_stream_t from = _stream_fft_in;
	ssi_stream_t from = stream_in;
	from.num = frame + delta;
	ssi_stream_t to = _stream_fft_out;
	to.num = 1;
	
	ssi_size_t byte_shift_from = from.byte * from.dim * frame;
	ssi_size_t byte_shift_to = to.byte * to.dim * to.num;				
	
	ITransformer::info tinfo;
	tinfo.delta_num = delta;
	tinfo.frame_num = frame;
	tinfo.time = info.time;

	for (ssi_size_t i = 0; i < n_shifts; i++) {			
		_fft->transform (tinfo, from, to, 0, 0);					
		tinfo.time += frame / stream_in.sr;

		from.ptr += byte_shift_from;
		to.ptr += byte_shift_to;		
	}
	
	_chain->transform (info, _stream_fft_out, stream_out);

	//Getting Deltas

	//int n = stream_out.num;
	//int dim = stream_out.dim;

	//ssi_byte_t* data = stream_out.ptr;

	//for(int i=0; i<n; i++)

	//memcpy (_stream_fft_in.ptr, stream_in.ptr + (stream_in.num - delta) * _stream_fft_in.dim * _stream_fft_in.byte, delta * _stream_fft_in.dim * _stream_fft_in.byte);
}

void OSPlpChain::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_fft->transform_flush (stream_in, _stream_fft_out);
	_chain->transform_flush (stream_in, stream_out);

	ssi_stream_destroy (_stream_fft_in);
	ssi_stream_destroy (_stream_fft_out);
}


}

