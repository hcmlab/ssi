// MvgMedianHelper.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/12
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_SIGNAL_MVGMEDIANHELPER_H
#define SSI_SIGNAL_MVGMEDIANHELPER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MvgMedianHelper {

public:

	MvgMedianHelper (ssi_size_t nwin);
	virtual ~MvgMedianHelper ();

	void init (ssi_real_t x);
	ssi_real_t move (ssi_real_t x);
	
protected:

	ssi_size_t _mpos;
	ssi_size_t _nwin;
	ssi_size_t _ndim;
	ssi_real_t *_vals;
	ssi_size_t *_order;
	ssi_size_t _pointer;

	void swap (ssi_size_t xpos,
		ssi_size_t ypos,
		ssi_size_t n,
		ssi_size_t *order, 
		ssi_real_t *values);

	ssi_size_t findmin (ssi_size_t n,
		ssi_real_t *values);

	ssi_size_t findmax (ssi_size_t n,
		ssi_real_t *values);
};

}

#endif
