// ExportMain.h
// author: Andreas Seiderer
// created: 2013/08/05
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "ssipraat.h"
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
	
	result = ssi::Factory::Register (ssi::PraatVoiceReport::GetCreateName (), ssi::PraatVoiceReport::Create) && result;
	result = ssi::Factory::Register (ssi::PraatUniversal::GetCreateName (), ssi::PraatUniversal::Create) && result;
	result = ssi::Factory::Register (ssi::PraatUniversalT::GetCreateName (), ssi::PraatUniversalT::Create) && result;
	result = ssi::Factory::Register (ssi::PraatVoiceReportT::GetCreateName (), ssi::PraatVoiceReportT::Create) && result;
			
	return result;
}
