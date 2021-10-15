// ExportMain.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2012/08/10
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "ssibodyfeatures.h"
#include "base/Factory.h"

#ifndef DLLEXP
#define DLLEXP extern "C" __declspec( dllexport )
#endif

DLLEXP bool Register(ssi::Factory *factory, FILE *logfile, ssi::IMessage *message) {
	ssi::Factory::SetFactory(factory);

	if (logfile) {
		ssiout = logfile;
	}
	if (message) {
		ssimsg = message;
	}

	bool result = true;

	result = ssi::Factory::Register(ssi::EnergyMovement::GetCreateName(), ssi::EnergyMovement::Create) && result;
	result = ssi::Factory::Register(ssi::FluidityMovement::GetCreateName(), ssi::FluidityMovement::Create) && result;
	result = ssi::Factory::Register(ssi::OAMovement::GetCreateName(), ssi::OAMovement::Create) && result;
	result = ssi::Factory::Register(ssi::SpatialExtentMovement::GetCreateName(), ssi::SpatialExtentMovement::Create) && result;
	result = ssi::Factory::Register(ssi::BodyProperties::GetCreateName(), ssi::BodyProperties::Create) && result;
	result = ssi::Factory::Register(ssi::EnergyAcc::GetCreateName(), ssi::EnergyAcc::Create) && result;
	result = ssi::Factory::Register(ssi::OAAcc::GetCreateName(), ssi::OAAcc::Create) && result;
	result = ssi::Factory::Register(ssi::Openness::GetCreateName(), ssi::Openness::Create) && result;
	result = ssi::Factory::Register(ssi::RelativeMovement::GetCreateName(), ssi::RelativeMovement::Create) && result;
	result = ssi::Factory::Register(ssi::BodyProperties2D::GetCreateName(), ssi::BodyProperties2D::Create) && result;


	return result;
}