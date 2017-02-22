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

#include "ssiioput.h"
#include "base/Factory.h"

#ifndef DLLEXP
#if _WIN32|_WIN64
#define DLLEXP extern "C" __declspec( dllexport )
#else
#define DLLEXP extern "C" __attribute__((visibility("default")))
#endif
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
	
	result = ssi::Factory::Register (ssi::FileReader::GetCreateName (),ssi::FileReader::Create) && result;	
	result = ssi::Factory::Register (ssi::FileWriter::GetCreateName (),ssi::FileWriter::Create) && result;	
	result = ssi::Factory::Register (ssi::FileEventWriter::GetCreateName(), ssi::FileEventWriter::Create) && result;	
	result = ssi::Factory::Register (ssi::SocketReader::GetCreateName (),ssi::SocketReader::Create) && result;	
	result = ssi::Factory::Register (ssi::SocketWriter::GetCreateName (),ssi::SocketWriter::Create) && result;	
	result = ssi::Factory::Register (ssi::SocketEventWriter::GetCreateName (),ssi::SocketEventWriter::Create) && result;
	result = ssi::Factory::Register (ssi::SocketEventReader::GetCreateName (),ssi::SocketEventReader::Create) && result;	
	result = ssi::Factory::Register (ssi::FileSampleWriter::GetCreateName(), ssi::FileSampleWriter::Create) && result;
	result = ssi::Factory::Register (ssi::FakeSignal::GetCreateName(), ssi::FakeSignal::Create) && result;
	
	return result;
}
