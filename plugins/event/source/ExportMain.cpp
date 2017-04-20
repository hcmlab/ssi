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

#include "ssievent.h"
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

	result = ssi::Factory::Register (ssi::TheEventBoard::GetCreateName(), ssi::TheEventBoard::Create) && result;
#ifndef HEADLESS
	result = ssi::Factory::Register (ssi::EventMonitor::GetCreateName(), ssi::EventMonitor::Create) && result;
#endif
	result = ssi::Factory::Register (ssi::MapEventSender::GetCreateName(), ssi::MapEventSender::Create) && result;
	result = ssi::Factory::Register (ssi::TupleEventSender::GetCreateName(), ssi::TupleEventSender::Create) && result;
	result = ssi::Factory::Register (ssi::StringEventSender::GetCreateName(), ssi::StringEventSender::Create) && result;
	result = ssi::Factory::Register (ssi::ZeroEventSender::GetCreateName(), ssi::ZeroEventSender::Create) && result;
	result = ssi::Factory::Register (ssi::ThresEventSender::GetCreateName(), ssi::ThresEventSender::Create) && result;	
	result = ssi::Factory::Register(ssi::TriggerEventSender::GetCreateName(), ssi::TriggerEventSender::Create) && result;
	result = ssi::Factory::Register (ssi::FixationEventSender::GetCreateName(), ssi::FixationEventSender::Create) && result;		
	result = ssi::Factory::Register (ssi::ThresClassEventSender::GetCreateName(), ssi::ThresClassEventSender::Create) && result;		
	result = ssi::Factory::Register (ssi::XMLEventSender::GetCreateName(), ssi::XMLEventSender::Create) && result;
	result = ssi::Factory::Register(ssi::ClockEventSender::GetCreateName(), ssi::ClockEventSender::Create) && result;
	result = ssi::Factory::Register(ssi::EventToStream::GetCreateName(), ssi::EventToStream::Create) && result;
	
	return result;
}

