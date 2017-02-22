// ExportMain.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 20/11/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssicontrol.h"
#include "base/Factory.h"

#ifndef DLLEXP
#define DLLEXP extern "C" __declspec( dllexport )
#endif

DLLEXP bool Register (ssi::Factory *factory, FILE *logfile, ssi::IMessage *message) {

	ssi::Factory::SetFactory (factory);

	if (logfile) {
		ssiout = logfile;
	}
	if (message) {
		ssimsg = message;
	}

	bool result = true;

#include "SSI_Define.h"

#ifndef SSI_USE_SDL
	
	result = ssi::Factory::Register(ssi::ControlSlider::GetCreateName(), ssi::ControlSlider::Create) && result;
	result = ssi::Factory::Register(ssi::Controller::GetCreateName(), ssi::Controller::Create) && result;
	result = ssi::Factory::Register(ssi::ControlCheckBox::GetCreateName(), ssi::ControlCheckBox::Create) && result;
	result = ssi::Factory::Register(ssi::ControlTextBox::GetCreateName(), ssi::ControlTextBox::Create) && result;
	result = ssi::Factory::Register(ssi::ControlButton::GetCreateName(), ssi::ControlButton::Create) && result;
	result = ssi::Factory::Register(ssi::ControlGrid::GetCreateName(), ssi::ControlGrid::Create) && result;
	
#endif

	return result;
}
