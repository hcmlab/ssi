// DspFiltersTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/20
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

#ifndef SSI_DSPFILTERS_TOOLS_H
#define SSI_DSPFILTERS_TOOLS_H

#include "SSI_Cons.h"

namespace Dsp {
	class Filter;
}

namespace ssi {

class DspFiltersTools {

public:	

	enum FILTER_FAMILY {
		RBJ = 1,
		BUTTERWORTH = 2,
		CHEBYSHEV_I = 3,
		CHEBYSHEV_II = 4,
		ELLIPTIC = 5,
		BESSEL = 6,
		LEGENDRE = 7,
		CUSTOM = 8
	};

	enum FILTER_TYPE {
		LOWPASS = 1,
		HIGHPASS = 2,
		BANDPASS1 = 3,
		BANDPASS2 = 4,
		BANDSTOP = 5,
		LOWSHELF = 6,
		HIGHSHELF = 7,
		BANDSHELF = 8,
		ALLPASS = 9
	};

	enum FILTER_STATE {
		DIRECTFORM_I = 1,
		DIRECTFORM_II = 2,
		TRANSPOSEDDIRECTFORM_I = 3,
		TRANSPOSEDDIRECTFORM_II = 4
	};

	struct FilterProperty {		
		ssi_time_t rate;
		ssi_size_t n_channels;
		FILTER_FAMILY family;
		FILTER_TYPE type;
		FILTER_STATE state;		
		ssi_size_t smooth;
		bool offset;
	};

public: 

	static void CreateFilter (Dsp::Filter** pFilter, FilterProperty prop);
	static const ssi_char_t *GetInfo (FilterProperty prop);

protected:

	template <class DesignType, class StateType>
	static void CreateFilterDesign (Dsp::Filter** pFilter, FilterProperty prop);
	template <class DesignType>
	static void CreateFilterState (Dsp::Filter** pFilter, FilterProperty prop); 

	static ssi_char_t InfoString[SSI_MAX_CHAR];	
	static ssi_char_t *ssi_log_name;

};

}

#endif
