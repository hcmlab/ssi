// AudioCons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#ifndef SSI_SENSOR_AUDIOLIBCONS_H
#define	SSI_SENSOR_AUDIOLIBCONS_H

#include "SSI_Cons.h"

#include "base/ISensor.h"

namespace ssi {

#define SSI_AUDIO_BLOCK_COUNT				8
#define SSI_AUDIO_DEFAULT_DEVICE			WAVE_MAPPER
#define SSI_AUDIO_DEFAULT_SAMPLE_TYPE		short
#define SSI_AUDIO_PROVIDER_NAME				"audio"

}

#endif

									
