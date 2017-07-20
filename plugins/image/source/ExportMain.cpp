// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/11 
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

#include "ssiimage.h"
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
	
	result = ssi::Factory::Register (ssi::CVChange::GetCreateName (), ssi::CVChange::Create) && result;	
	result = ssi::Factory::Register (ssi::CVColor::GetCreateName (), ssi::CVColor::Create) && result;
	result = ssi::Factory::Register (ssi::CVResize::GetCreateName (), ssi::CVResize::Create) && result;	
	result = ssi::Factory::Register (ssi::CVSave::GetCreateName (), ssi::CVSave::Create) && result;	
	result = ssi::Factory::Register (ssi::CVCrop::GetCreateName (), ssi::CVCrop::Create) && result;	
	result = ssi::Factory::Register (ssi::CVFlip::GetCreateName (), ssi::CVFlip::Create) && result;	
	result = ssi::Factory::Register (ssi::CVMean::GetCreateName(), ssi::CVMean::Create) && result;

	return result;
}
