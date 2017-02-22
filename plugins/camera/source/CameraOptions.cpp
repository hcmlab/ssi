// CameraOptions.cpp
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

#include "CameraOptions.h"
#include "CameraCons.h"
#include "CameraTools.h"

namespace ssi {

CameraOptions::CameraOptions ()
: apply (true), flip (false), mirror (false) {

	ssi_video_params (params);
	_device_name = new ssi_char_t[SSI_MAX_CHAR];
	_subtype_name = new ssi_char_t[SSI_MAX_CHAR];
	_codec_name = new ssi_char_t[SSI_MAX_CHAR];

	setDeviceName (0);
	setCodecName (0);
	_subtype_name[0] = '\0';

	addOption ("apply", &apply, 1, SSI_BOOL, "Apply changes from graphical dialog?");		
	addOption ("device", _device_name, SSI_MAX_CHAR, SSI_CHAR, "Name of camera device (if empty a selection dialog will be shown).");	
	addOption ("codec", _codec_name, SSI_MAX_CHAR, SSI_CHAR, "Name of compression filter ('None' for none).");
	addOption ("width", &params.widthInPixels, 1, SSI_INT, "Width in pixel.");
	addOption ("height", &params.heightInPixels, 1, SSI_INT, "Height in pixel.");
	addOption ("fps", &params.framesPerSecond, 1, SSI_DOUBLE, "Frame rate per seconds.");		
	addOption ("subtype", _subtype_name, SSI_MAX_CHAR, SSI_CHAR, "Name of mediasubtype (if empty a selection dialog will be shown).");
	addOption ("channel", &params.numOfChannels, 1, SSI_INT, "Number of channels.");
	addOption ("depth", &params.depthInBitsPerChannel, 1, SSI_INT, "Depth in bits per channel.");	
	addOption ("closestFps", &params.useClosestFramerateForGraph, 1, SSI_BOOL, "Use closest frame rate?");
	addOption ("flip", &flip, 1, SSI_BOOL, "Flip camera image");	
	addOption ("mirror", &mirror, 1, SSI_BOOL, "mirror video image");
}

CameraOptions::~CameraOptions () {

	delete[] _device_name;
	delete[] _subtype_name;
	delete[] _codec_name;
};


void CameraOptions::setDeviceName (const ssi_char_t *device_name) {
	if (device_name) {
		ssi_strcpy (_device_name, device_name);
	} else {
		_device_name[0] = '\0';
	}
}

void CameraOptions::setCodecName (const ssi_char_t *codec_name) {
	if (codec_name) {
		ssi_strcpy (_codec_name, codec_name);
	} else {
		ssi_strcpy (_codec_name, SSI_CAMERA_USE_NO_COMPRESSION);	
	}
}

void CameraOptions::setSubTypeName (const ssi_char_t *subtype_name) {
	if (subtype_name) {
		ssi_strcpy (_subtype_name, subtype_name);	
	} else {
		_subtype_name[0] = '\0';
	}
}

void CameraOptions::udpateSubTypeName () {
	if (!CameraTools::GUIDToString (params.outputSubTypeOfCaptureDevice, _subtype_name, SSI_MAX_CHAR)) {
		_subtype_name[0] = '\0';
	}
}

void CameraOptions::applySubTypeName () {
	if (_subtype_name[0] == '\0' || !CameraTools::StringToGUID (_subtype_name, params.outputSubTypeOfCaptureDevice)) {
		params.outputSubTypeOfCaptureDevice = SSI_GUID_NULL;
	}
}

}

									

