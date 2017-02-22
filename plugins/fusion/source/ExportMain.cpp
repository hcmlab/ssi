// ExportMain.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/12/07 
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
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

#include "ssifusion.h"
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
	
	result = ssi::Factory::Register (ssi::AdaBoost::GetCreateName (), ssi::AdaBoost::Create) && result;
	result = ssi::Factory::Register (ssi::BKS::GetCreateName (), ssi::BKS::Create) && result;
	result = ssi::Factory::Register (ssi::BordaCount::GetCreateName (), ssi::BordaCount::Create) && result;
	result = ssi::Factory::Register (ssi::CascadingSpecialists::GetCreateName (), ssi::CascadingSpecialists::Create) && result;
	result = ssi::Factory::Register (ssi::CascadingSpecialistsMS::GetCreateName (), ssi::CascadingSpecialistsMS::Create) && result;
	result = ssi::Factory::Register (ssi::DecisionTemplate::GetCreateName (), ssi::DecisionTemplate::Create) && result;
	result = ssi::Factory::Register (ssi::DempsterShafer::GetCreateName (), ssi::DempsterShafer::Create) && result;
	result = ssi::Factory::Register (ssi::FeatureFusion::GetCreateName (), ssi::FeatureFusion::Create) && result;
	result = ssi::Factory::Register (ssi::Grading::GetCreateName (), ssi::Grading::Create) && result;
	result = ssi::Factory::Register (ssi::MajorityVoting::GetCreateName (), ssi::MajorityVoting::Create) && result;
	result = ssi::Factory::Register (ssi::MaxRule::GetCreateName (), ssi::MaxRule::Create) && result;
	result = ssi::Factory::Register (ssi::MeanRule::GetCreateName (), ssi::MeanRule::Create) && result;
	result = ssi::Factory::Register (ssi::MedianRule::GetCreateName (), ssi::MedianRule::Create) && result;
	result = ssi::Factory::Register (ssi::MinRule::GetCreateName (), ssi::MinRule::Create) && result;
	result = ssi::Factory::Register (ssi::OvR::GetCreateName (), ssi::OvR::Create) && result;
	result = ssi::Factory::Register (ssi::OvRSpecialist::GetCreateName (), ssi::OvRSpecialist::Create) && result;
	result = ssi::Factory::Register (ssi::ProductRule::GetCreateName (), ssi::ProductRule::Create) && result;
	result = ssi::Factory::Register (ssi::SingleFeatures::GetCreateName (), ssi::SingleFeatures::Create) && result;
	result = ssi::Factory::Register (ssi::StackedGeneralization::GetCreateName (), ssi::StackedGeneralization::Create) && result;
	result = ssi::Factory::Register (ssi::SumRule::GetCreateName (), ssi::SumRule::Create) && result;
	result = ssi::Factory::Register (ssi::VACFusion::GetCreateName (), ssi::VACFusion::Create) && result;
	result = ssi::Factory::Register (ssi::WeightedAverage::GetCreateName (), ssi::WeightedAverage::Create) && result;
	result = ssi::Factory::Register (ssi::WeightedMajorityVoting::GetCreateName (), ssi::WeightedMajorityVoting::Create) && result;
	
	return result;
}
