// FFTFeat.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/19
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

#include "FFTfeat.h"	
#include "FFT.h"

namespace ssi {

ssi_char_t *FFTfeat::ssi_log_name = "fftfeat___";

FFTfeat::FFTfeat (const ssi_char_t *file) 
	: _fft (0),
	_fft_out (0),
	_fft_in (0),
	_window (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

FFTfeat::~FFTfeat() {
}

SSI_INLINE void FFTfeat::init_fft () {

	_fft = new FFT (_fft_size, 1);
	_fft_out = new ssi_real_t *[_fft_dim];
	_fft_in = new ssi_real_t *[_fft_dim];
	for(ssi_size_t i = 0; i < _fft_dim; i++){
		_fft_in[i]= new ssi_real_t[_fft_size];
		_fft_out[i] = new ssi_real_t[_fft->rfft];
	}
}

SSI_INLINE void FFTfeat::release_fft () {

	delete _fft; _fft = 0;
	for(ssi_size_t i = 0; i < _fft_dim; i++){
		delete[] _fft_in[i];
		delete[] _fft_out[i];
	}	
	delete[] _fft_in; _fft_in = 0;
	delete[] _fft_out; _fft_out = 0;
}

void FFTfeat::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]){

	_fft_dim = stream_in.dim;	
	_fft_size = _options.nfft;	
	init_fft ();
	_win_type = _options.wintype;
	_win_size = _fft_size;
	if (_win_type != WINDOW_TYPE_RECTANGLE) {
		_window = FilterTools::Window (_win_size, _win_type, MATRIX_DIMENSION_COL);
	}
}

void FFTfeat::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *ptr_out = ssi_pcast (ssi_real_t ,stream_out.ptr);

	for (ssi_size_t j = 0; j < _fft_size; j++) {
		for (ssi_size_t i = 0; i < _fft_dim; i++) {
			if (j < stream_in.num){
				_fft_in[i][j] = *ptr_in++;
			} else {
				_fft_in[i][j] = 0;
			}			
		}
	}

	for (ssi_size_t i = 0; i < _fft_dim; i++) {
		if (_win_type != WINDOW_TYPE_RECTANGLE) {
			for (ssi_size_t j = 0; j < _fft_size; j++) {		
				_fft_in[i][j] *= _window->data[j];
			}
		}
		_fft->transform (_fft_size,_fft_in[i], _fft_out[i]);
	}

	for (ssi_size_t j = 0; j < _fft->rfft; j++){
		for (ssi_size_t i = 0; i < _fft_dim; i++){
			*ptr_out++ = _fft_out[i][j];
		}
	}
	
}

void FFTfeat::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]){

	release_fft ();
	delete _window; _window = 0;
	_win_size = 0;
}

}
