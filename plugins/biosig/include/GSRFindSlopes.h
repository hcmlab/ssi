// GSRFindSlopes.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/09/28 
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


#ifndef SSI_BIOSIG_GSRFINDSLOPES_H
#define SSI_BIOSIG_GSRFINDSLOPES_H

#pragma once

#include "SSI_Cons.h"

namespace ssi {

class OverlapBuffer;

class GSRFindSlopes {

public:

	class ICallback {
	public:
		virtual void slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t slope) = 0;
	};

	struct Params {
		ssi_time_t sr;
		ssi_real_t nstd;
		ssi_real_t mindur;
		ssi_real_t maxdur;
	};

	GSRFindSlopes (ICallback *callback, Params params);
	virtual ~GSRFindSlopes ();

	void process (ssi_size_t n, ssi_time_t sr, ssi_real_t *gsr, ssi_real_t *mvg);
	
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	void find_slopes (ssi_time_t sr, OverlapBuffer &gsr, OverlapBuffer &mvg);
	ssi_size_t look_for_minimum_on_left (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk);
	ssi_size_t look_for_minimum_on_right (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk);
	ssi_size_t look_for_maximum_on_left (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk);
	ssi_size_t look_for_maximum_on_right (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk);

	Params _params;
	ICallback *_callback;
	
	ssi_size_t _n_overlap;
	OverlapBuffer *_ob_gsr;
	OverlapBuffer *_ob_mvg;

};

}

#endif
