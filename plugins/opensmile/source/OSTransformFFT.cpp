// OSTransformFFT.cpp
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

#include "OSTransformFFT.h"
#include "OSWindow.h"
#include "base/Factory.h"
#include <fftXg.h>  // fft4g include

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {

OSTransformFFT::OSTransformFFT (const ssi_char_t *file)
	: _file (0),
	_file_win (0),
	_window (0),
	ip (0),
    w (0),
	_n_src_win (0),
	_src_win (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
		_file_win = ssi_strcat (_file, ".win");
	}

	_window = ssi_factory_create (OSWindow, _file_win, false);
}

OSTransformFFT::~OSTransformFFT () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
		OptionList::SaveXML (_file_win, _window->getOptions ());
		delete[] _file_win;
	}

	delete _window;
}

void OSTransformFFT::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	Ndst = _options.nfft;
		
	ip = new int[Ndst+2];
	for (long i = 0; i < Ndst+2; i++) {
		ip[i] = 0;
	}
	w = new ssi_real_t[Ndst+2];
	for (long i = 0; i < Ndst+2; i++) {
		w[i] = 0;
	}
}

void OSTransformFFT::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	long Nsrc = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	if(Nsrc > Ndst)
		ssi_wrn("FFT frame size is smaller than actual frame size. Dropping %d samples.", Nsrc - Ndst);

	if (_n_src_win < Nsrc) {
		delete[] _src_win;
		_n_src_win = Nsrc;
		_src_win = new ssi_real_t[_n_src_win];
	}
	memcpy (_src_win, src, sizeof (ssi_real_t) * _n_src_win);

	// apply window function
	_window->apply (_n_src_win, 1, _src_win);

	// zero padding
	for (long i = 0; i < min (Ndst, Nsrc); i++) {
		dst[i] = _src_win[i];
	}
	for (long i = Nsrc; i < Ndst; i++) {
		dst[i] = 0;
	}

	rdft (Ndst, 1, dst, ip, w);	
}

void OSTransformFFT::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete[] ip; ip = 0;
	delete[] w; w = 0;
	delete[] _src_win; _src_win = 0; 
	_n_src_win = 0;
}

}

