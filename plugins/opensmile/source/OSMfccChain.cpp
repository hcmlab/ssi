// OSMfccChain.cpp
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

#include "OSMfccChain.h"
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

OSMfccChain::OSMfccChain (const ssi_char_t *file)
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
		ssi_sprint (string, "%s.OSMfcc", file);
		_mfcc = ssi_factory_create (OSMfcc, string, false);
		ssi_sprint (string, "%s.Deltas", file);

		if (USE_DERIVATIVE) {
			_deltas = ssi_factory_create (Derivative, string, false);
		} else {
			_deltas = ssi_factory_create (Deltas, string, false);
		}

		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	} 
	else {
		_fft = ssi_factory_create (OSTransformFFT, 0, false);
		_fftmag = ssi_factory_create (OSFFTmagphase, 0, false);
		_spect = ssi_factory_create (OSSpecScale, 0, false);
		_mfcc = ssi_factory_create (OSMfcc, 0, false);

		if (USE_DERIVATIVE) {
			_deltas = ssi_factory_create (Derivative, 0, false);
			ssi_pcast (Derivative, _deltas)->getOptions ()->set (Derivative::D0TH | Derivative::D1ST);
		} else {
			_deltas = ssi_factory_create (Deltas, 0, false);
		}

	}	

	_filter[0] = _fftmag;
	_filter[1] = _spect;
	_filter[2] = _mfcc;
	_filter[3] = _deltas;

}

OSMfccChain::~OSMfccChain () {

	delete _fft;
	delete _fftmag;
	delete _spect;
	delete _mfcc;
	delete _deltas;
	delete _chain;	

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OSMfccChain::initChain () {

	_chain = ssi_pcast (Chain, Factory::Create (Chain::GetCreateName (), 0, false));
	if (!_chain) {
		ssi_err("could not create chain");
	}

	if (_options.deltas_enable){
		_chain->set ( 4, _filter, 0, 0 );
	} else {
		_chain->set ( 3, _filter, 0, 0 );
	}
}

void OSMfccChain::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (!_chain) {
		initChain ();
	}

	ssi_stream_init (_stream_fft, 
		1, 
		_fft->getSampleDimensionOut (stream_in.dim),
		_fft->getSampleBytesOut (stream_in.byte),
		_fft->getSampleTypeOut (stream_in.type),
		stream_out.sr);

	_fft->transform_enter (stream_in, _stream_fft);
	_chain->transform_enter (_stream_fft, stream_out);
}

void OSMfccChain::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	_fft->transform (info, stream_in, _stream_fft, 0, 0);
	info.frame_num = 1;
	info.delta_num = 0;
	_chain->transform (info, _stream_fft, stream_out);
}

void OSMfccChain::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_fft->transform_flush (stream_in, _stream_fft);
	_chain->transform_flush (stream_in, stream_out);
	ssi_stream_destroy (_stream_fft);
}


}

