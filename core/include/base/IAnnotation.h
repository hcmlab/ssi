// IAnnotation.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/17
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

#ifndef SSI_IANNOTATION
#define SSI_IANNOTATION

#include "SSI_Cons.h"

namespace ssi {

class IAnnotation
{

public:

	virtual ssi_size_t getSize() = 0;
	virtual ssi_label_t getLabel(ssi_size_t index) = 0;
	virtual const ssi_scheme_t *getScheme() = 0;
	virtual void sort() = 0;
};


}

#endif
