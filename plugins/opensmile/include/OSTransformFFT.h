// OSTransformFFT.h
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

/**

Fast fourier transform using fft4g library
output: complex values of fft or real signal values (for iFFT)

*/

#pragma once

#ifndef SSI_OPENSMILE_TRANSFORMFFT_H
#define SSI_OPENSMILE_TRANSFORMFFT_H

#include "base/IFeature.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSWindow;

class OSTransformFFT : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: nfft (1024) {

			addOption ("nfft", &nfft, 1, SSI_SIZE, "Frame size on which to apply FFT. Must be a power of 2 and should be LARGER than frame size + delta.");
		};

		ssi_size_t nfft;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSTransformFFT"; };
	static IObject *Create (const ssi_char_t *file) { return new OSTransformFFT (file); };
	~OSTransformFFT ();

	OSTransformFFT::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component performs an FFT on a sequence of real values (one frame), the output is the complex domain result of the transform."; };

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

	virtual OSWindow *getWindow () {
		return _window;
	}

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		
		if (sample_dimension_in != 1) {
			ssi_err ("dimension > 1 not supported");
		}
		check_nfft ();
		return _options.nfft;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	OSTransformFFT (const ssi_char_t *file = 0);
	OSTransformFFT::Options _options;
	ssi_char_t *_file;
	ssi_char_t *_file_win;

	void check_nfft () {
		if (!smileMath_isPowerOf2(_options.nfft)) {
			ssi_wrn ("set nfft '%u' to next power of 2", _options.nfft);
			_options.nfft = smileMath_ceilToNextPowOf2(_options.nfft);  
		}
		if (_options.nfft < 4) _options.nfft = 4;
	}

	OSWindow *_window;
	long Ndst;
	int *ip;
    ssi_real_t *w;

	long _n_src_win;
	ssi_real_t *_src_win;
};

}

#endif
