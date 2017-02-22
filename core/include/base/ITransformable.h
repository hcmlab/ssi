// IWiiMote.h
// author: Benjamin Hrzek <hrzek@arcor.de>
// created: 2009/12/22
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_ITRANSFORMABLE_H
#define SSI_ITRANSFORMABLE_H

namespace ssi
{

class ITransformable
{

public:

	virtual ~ITransformable() {}; 

	virtual int			getBufferId()			= 0;
	virtual ssi_time_t	getSampleRate()			= 0;
	virtual ssi_size_t	getSampleDimension()	= 0;
	virtual	ssi_size_t	getSampleBytes()		= 0;
	virtual ssi_type_t  getSampleType ()		= 0;
};

}

#endif
