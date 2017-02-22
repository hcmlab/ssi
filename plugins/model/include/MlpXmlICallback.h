// MLPEventCallback.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/24
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

#ifndef SSI_MLPXMLICALLBACK_H
#define SSI_MLPXMLICALLBACK_H

#include "SSI_Cons.h"

namespace ssi {

class MlpXmlICallback {

public:	

	virtual void call (ssi_time_t time, // time in seconds
		ssi_time_t duration,            // duration in seconds
		const char *label,              // label name, null if unclassified
		bool store,                     // if true, will be stored in annotation
		bool change) = 0;               // if true, may be used to change stimuli
	
};

}

#endif
