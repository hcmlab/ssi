// CameraOptions.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/05/08
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

#ifndef SSI_SENSOR_CAMOPTIONS_H
#define	SSI_SENSOR_CAMOPTIONS_H

#include "ioput/option/OptionList.h"

namespace ssi {

class CameraOptions : public OptionList {

public:

	CameraOptions ();
	virtual ~CameraOptions ();

	virtual void setDeviceName (const ssi_char_t *device_name);
	virtual const ssi_char_t *getDeviceName () { return _device_name[0] == '\0' ? 0 : _device_name; }	
	virtual void setCodecName (const ssi_char_t *codec_name);
	virtual const ssi_char_t *getCodecName () { return _codec_name; }
	virtual void setSubTypeName (const ssi_char_t *subtype_name);
	virtual const ssi_char_t *getSubTypeName () { return _subtype_name; }
	virtual void udpateSubTypeName ();
	virtual void applySubTypeName ();	

	bool apply;
	bool flip;
	bool mirror;
	ssi_video_params_t params;

protected:

	bool _apply_gui_changes;
	ssi_char_t *_codec_name;
	ssi_char_t *_device_name;	
	ssi_char_t *_subtype_name;

};

}

#endif

									
