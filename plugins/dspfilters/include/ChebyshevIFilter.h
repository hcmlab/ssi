// ChebyshevIFilter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/26
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

/**

Provides butter low/high/band-pass filter

*/

#pragma once

#ifndef SSI_SIGNAL_CHEBYSHEVIFILTER_H
#define SSI_SIGNAL_CHEBYSHEVIFILTER_H

#include "IDspFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class ChebyshevIFilter : public IDspFilter {

public:

	enum TYPE {
		LOW = 1,
		HIGH = 2,
		BAND = 4,
		STOP = 5,
		LOWSHELF = 6,
		HIGHSHELF = 7,
		BANDSHELF = 8
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: type (LOW), state (DspFiltersTools::DIRECTFORM_I), order (1), norm (true), freq (0.1), width (0.1), gain (-6.0), smooth (0), offset (false) {

			addOption ("type", &type, 1, SSI_INT, "filter type (1=lowpass, 2=highpass, 4=bandpass, 5=bandstop, 6=lowshelf, 7=highshelf, 8=bandshelf)");		
			addOption ("state", &state, 1, SSI_INT, "filter state (1=directform I, 2=directform II, 3=transposed directform I, 4=transposed directform II)");		
			addOption ("order", &order, 1, SSI_UINT, "filter order");	
			addOption ("norm", &norm, 1, SSI_BOOL, "frequency values are normalized in interval [0..1], where 1 is the nyquist frequency (=half the sample rate)");
			addOption ("freq", &freq, 1, SSI_DOUBLE, "cutoff (lowpass/highpass/lowshelf/highshelf) or center (bandpass/bandstop/bandshelf) frequency either in hz or normalized (see -norm)");
			addOption ("width", &width, 1, SSI_DOUBLE, "bandpass/bandstop/bandshelf: band width either in hz or normalized (see -norm)");
			addOption ("gain", &gain, 1, SSI_DOUBLE, "lowshelf/highshelf/bandshelf: shelf gain in db");
			addOption ("smooth", &smooth, 1, SSI_SIZE, "number of transitions used to smooth filter paramaters");
			addOption ("offset", &offset, 1, SSI_BOOL, "subtract first sample before filtering");
		};

		TYPE type;
		DspFiltersTools::FILTER_STATE state;
		ssi_size_t order;
		bool norm;
		double freq;
		double width;
		double gain;
		ssi_size_t smooth;
		bool offset;
	};

public:

	static const ssi_char_t *GetCreateName () { return "ChebyshevIFilter"; };
	static IObject *Create (const ssi_char_t *file) { return new ChebyshevIFilter (file); };
	virtual ~ChebyshevIFilter ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "provides chebyshev I filtering"; };

	Dsp::Filter *createFilter (DspFiltersTools::FilterProperty &prop);

protected:

	ChebyshevIFilter (const ssi_char_t *file = 0);
	ChebyshevIFilter::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
};

}

#endif
