// ExportMain.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 9/3/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
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

#include "ssishimmer3.h"
#include "base/Factory.h"
#include "ShimmerClosedLibraryAlgoFactory.h"

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

	//Load mixed mode dll for Shimmer PPGtoHR Algorithm
	result = ShimmerClosedLibraryAlgoFactory::load() && result;
	
	result = ssi::Factory::Register (ssi::Shimmer3GSRPlus::GetCreateName (), ssi::Shimmer3GSRPlus::Create) && result;
	result = ssi::Factory::Register (ssi::Shimmer3PPGToHR::GetCreateName (), ssi::Shimmer3PPGToHR::Create) && result;
				
	return result;
}
