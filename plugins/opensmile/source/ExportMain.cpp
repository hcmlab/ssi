// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

#include "ssiopensmile.h"
#include "base/Factory.h"

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
	
	result = ssi::Factory::Register (ssi::OSPreemphasis::GetCreateName (), ssi::OSPreemphasis::Create) && result;	
	result = ssi::Factory::Register (ssi::OSEnergy::GetCreateName (), ssi::OSEnergy::Create) && result;	
	result = ssi::Factory::Register (ssi::OSWindow::GetCreateName (), ssi::OSWindow::Create) && result;
	result = ssi::Factory::Register (ssi::OSTransformFFT::GetCreateName (), ssi::OSTransformFFT::Create) && result;
	result = ssi::Factory::Register (ssi::OSFFTmagphase::GetCreateName (), ssi::OSFFTmagphase::Create) && result;
	result = ssi::Factory::Register (ssi::OSMelspec::GetCreateName (), ssi::OSMelspec::Create) && result;
	result = ssi::Factory::Register (ssi::OSMfcc::GetCreateName (), ssi::OSMfcc::Create) && result;
	result = ssi::Factory::Register (ssi::OSSpecScale::GetCreateName (), ssi::OSSpecScale::Create) && result;
	result = ssi::Factory::Register (ssi::OSPitchShs::GetCreateName (), ssi::OSPitchShs::Create) && result;
	result = ssi::Factory::Register (ssi::OSPitchSmoother::GetCreateName (), ssi::OSPitchSmoother::Create) && result;
	result = ssi::Factory::Register (ssi::OSPitchChain::GetCreateName (), ssi::OSPitchChain::Create) && result;
	result = ssi::Factory::Register (ssi::OSMfccChain::GetCreateName (), ssi::OSMfccChain::Create) && result;
	result = ssi::Factory::Register (ssi::OSFunctionals::GetCreateName (), ssi::OSFunctionals::Create) && result;
	result = ssi::Factory::Register (ssi::Deltas::GetCreateName(), ssi::Deltas::Create) && result;
	result = ssi::Factory::Register (ssi::OSIntensity::GetCreateName(), ssi::OSIntensity::Create) && result;
	result = ssi::Factory::Register (ssi::LaughterFeatureExtractor::GetCreateName(), ssi::LaughterFeatureExtractor::Create) && result;
	result = ssi::Factory::Register (ssi::LaughterPreProcessor::GetCreateName(), ssi::LaughterPreProcessor::Create) && result;
	result = ssi::Factory::Register (ssi::OSPlp::GetCreateName(), ssi::OSPlp::Create) && result;
	result = ssi::Factory::Register (ssi::OSPlpChain::GetCreateName(), ssi::OSPlpChain::Create) && result;
	result = ssi::Factory::Register (ssi::OSLpc::GetCreateName(), ssi::OSLpc::Create) && result;
	result = ssi::Factory::Register (ssi::OSVad::GetCreateName(), ssi::OSVad::Create) && result;
	result = ssi::Factory::Register (ssi::OSPitchDirection::GetCreateName(), ssi::OSPitchDirection::Create) && result;

	return result;
}
