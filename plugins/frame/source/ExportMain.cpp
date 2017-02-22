// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/20 
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

#ifndef DLLEXP
#if _WIN32|_WIN64
#define DLLEXP extern "C" __declspec( dllexport )
#else
#define DLLEXP extern "C" __attribute__((visibility("default")))
#endif
#endif

#include "ssiframe.h"
#include "base/Factory.h"

DLLEXP bool Register (ssi::Factory *factory, FILE *logfile, ssi::IMessage *message) {

	ssi::Factory::SetFactory (factory);

	if (logfile) {
		ssiout = logfile;
	}
	if (message) {
		ssimsg = message;
	}

	bool result = true;

	result = ssi::Factory::Register (ssi::TheFramework::GetCreateName(), ssi::TheFramework::Create) && result;
	result = ssi::Factory::Register (ssi::XMLPipeline::GetCreateName(), ssi::XMLPipeline::Create) && result;
	result = ssi::Factory::Register (ssi::Asynchronous::GetCreateName (), ssi::Asynchronous::Create) && result;
	result = ssi::Factory::Register (ssi::EventConsumer::GetCreateName (), ssi::EventConsumer::Create) && result;
	result = ssi::Factory::Register (ssi::Clone::GetCreateName (), ssi::Clone::Create) && result;	
	result = ssi::Factory::Register (ssi::Chain::GetCreateName (), ssi::Chain::Create) && result;
	result = ssi::Factory::Register (ssi::Cast::GetCreateName (), ssi::Cast::Create) && result;
	result = ssi::Factory::Register (ssi::Selector::GetCreateName (), ssi::Selector::Create) && result;
	result = ssi::Factory::Register (ssi::Merge::GetCreateName (), ssi::Merge::Create) && result;
	result = ssi::Factory::Register (ssi::Inverter::GetCreateName(), ssi::Inverter::Create) && result;
	result = ssi::Factory::Register (ssi::Decorator::GetCreateName(), ssi::Decorator::Create) && result;

	return result;
}

