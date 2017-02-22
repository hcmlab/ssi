// OSFFTmagphase.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/22
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

#pragma once

#ifndef SSI_OPENSMILE_FFTMAGPHASE_H
#define SSI_OPENSMILE_FFTMAGPHASE_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSFFTmagphase : public IFilter {

public:

	enum TYPE {
		MAGNITUDE = 0,
		PHASE = 1,
		BOTH = 2
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: type (MAGNITUDE), norm (false), power (false), dBpsd (false), dBpnorm (90.302f) {

			addOption ("type", &type, 1, SSI_INT, "0=compute magnitude, 1=compute phase, 2=compute both");		
			addOption ("norm", &norm, 1, SSI_BOOL, "Normalise FFT magnitudes to input window length, to obtain spectral densities.");
			addOption ("power", &power, 1, SSI_BOOL, "Square FFT magnitudes to obtain power spectrum.");
			addOption ("dBpsd", &dBpsd, 1, SSI_BOOL, "output logarithmic (dB SPL) power spectral density instead of linear magnitude spectrum (you should use a Hann window for analysis in this case). Setting this option also sets 'norm=true' and 'power=true'.");
			addOption ("dBpnorm", &dBpnorm, 1, SSI_REAL, "Value for dB power normalisation when 'dBpsd=true' (in dB SPL). Default is according to MPEG-1, psy I model.");
		};

		TYPE type;
		bool norm, power;
		int dBpsd;
		ssi_real_t dBpnorm;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSFFTmagphase"; };
	static IObject *Create (const ssi_char_t *file) { return new OSFFTmagphase (file); };
	~OSFFTmagphase ();

	OSFFTmagphase::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes magnitude and phase of each array in the input level (it thereby assumes that the arrays contain complex numbers with real and imaginary parts alternating, as computed by the cTransformFFT component)."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		switch (_options.type) {
			case MAGNITUDE:
				return sample_dimension_in / 2 + 1;
			case PHASE:
				return sample_dimension_in / 2;
			case BOTH:
				return 2 * (sample_dimension_in / 2) + 1;
		}
		return 0;
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

	OSFFTmagphase (const ssi_char_t *file = 0);
	OSFFTmagphase::Options _options;
	ssi_char_t *_file;

	int magnitude;
    int phase;
    int joinMagphase;
    int power;
    int normalise, dBpsd;
    FLOAT_DMEM dBpnorm;
};

}

#endif
