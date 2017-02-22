// RBJFilter.h
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

#ifndef SSI_SIGNAL_RBJFILTER_H
#define SSI_SIGNAL_RBJFILTER_H

#include "IDspFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class RBJFilter : public IDspFilter {

public:

	enum TYPE {
		LOW = 1,
		HIGH = 2,
		BAND1 = 3,
		BAND2 = 4,
		STOP = 5,
		LOWSHELF = 6,
		HIGHSHELF = 7,
		BANDSHELF = 8,
		ALL = 9
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: type (LOW), state (DspFiltersTools::DIRECTFORM_I), order (1), norm (true), freq (0.1), width (0.1), q (1.0), gain (-6.0), slope (1.0), smooth (false), offset (false) {

			addOption ("type", &type, 1, SSI_INT, "filter type (1=lowpass, 2=highpass, 3=bandpass1 (constant skirt gain), 4=bandpass2 (constant 0 dB peak gain), 5=bandstop, 6=lowshelf, 7=highshelf, 8=bandshelf, 9=allpass)");		
			addOption ("state", &state, 1, SSI_INT, "filter state (1=directform I, 2=directform II, 3=transposed directform I, 4=transposed directform II)");		
			addOption ("order", &order, 1, SSI_UINT, "filter order");	
			addOption ("norm", &norm, 1, SSI_BOOL, "frequency values are normalized in interval [0..1], where 1 is the nyquist frequency (=half the sample rate)");
			addOption ("freq", &freq, 1, SSI_DOUBLE, "cutoff (lowpass/highpass/lowshelf/highshelf) or center (bandpass1/bandpass2/bandstop/bandshelf) or phase (allpass) frequency either in hz or normalized (see -norm)");
			addOption ("width", &width, 1, SSI_DOUBLE, "bandpass1/bandpass2/bandstop/bandshelf: band width either in hz or normalized (see -norm)");
			addOption ("q", &q, 1, SSI_DOUBLE, "lowpass/highpass/allpass: peak gain q");
			addOption ("gain", &gain, 1, SSI_DOUBLE, "lowshelf/highshelf/bandshelf: shelf gain in db");
			addOption ("slope", &slope, 1, SSI_DOUBLE, "lowshelf/highshelf: shelf slope");
			addOption ("smooth", &smooth, 1, SSI_SIZE, "number of transitions used to smooth filter paramaters");
			addOption ("offset", &offset, 1, SSI_BOOL, "subtract first sample before filtering");
		};

		TYPE type;
		DspFiltersTools::FILTER_STATE state;
		ssi_size_t order;
		bool norm;
		double freq;
		double width;
		double q;
		double gain;
		double slope;
		ssi_size_t smooth;
		bool offset;
	};

public:

	static const ssi_char_t *GetCreateName () { return "RBJFilter"; };
	static IObject *Create (const ssi_char_t *file) { return new RBJFilter (file); };
	virtual ~RBJFilter ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "provides rbj filtering"; };

	Dsp::Filter *createFilter (DspFiltersTools::FilterProperty &prop);

protected:

	RBJFilter (const ssi_char_t *file = 0);
	RBJFilter::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
};

}

#endif
