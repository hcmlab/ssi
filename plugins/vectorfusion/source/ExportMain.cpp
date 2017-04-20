// ExportMain.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#include "ssivectorfusion.h"
#include "base/Factory.h"

//#include <vld.h>

#ifndef DLLEXP
#if _WIN32||_WIN64
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
	
	ssi::Factory::Register (ssi::VectorFusionModality::GetCreateName (), ssi::VectorFusionModality::Create) && result;
	ssi::Factory::Register (ssi::VectorFusionGravity::GetCreateName (), ssi::VectorFusionGravity::Create) && result;
	ssi::Factory::Register (ssi::VectorFusionVA::GetCreateName (), ssi::VectorFusionVA::Create) && result;
	ssi::Factory::Register (ssi::VectorFusion::GetCreateName (), ssi::VectorFusion::Create) && result;

	ssi::Factory::Register (ssi::TupleConverter::GetCreateName (), ssi::TupleConverter::Create) && result;
	ssi::Factory::Register (ssi::TupleMap::GetCreateName (), ssi::TupleMap::Create) && result;
	ssi::Factory::Register (ssi::TupleScale::GetCreateName (), ssi::TupleScale::Create) && result;
	ssi::Factory::Register (ssi::TupleSelect::GetCreateName (), ssi::TupleSelect::Create) && result;
	ssi::Factory::Register (ssi::TupleThresh::GetCreateName (), ssi::TupleThresh::Create) && result;

	ssi::Factory::Register (ssi::VectorFusionWriter::GetCreateName (), ssi::VectorFusionWriter::Create) && result;
	ssi::Factory::Register(ssi::CombinerVA::GetCreateName(), ssi::CombinerVA::Create) && result;
	
	return result;
}
