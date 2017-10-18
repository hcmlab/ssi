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

#include "ssibiosig.h"
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
	
	result = ssi::Factory::Register (ssi::GSRArousalCombination::GetCreateName (), ssi::GSRArousalCombination::Create) && result;
	result = ssi::Factory::Register (ssi::GSRArousalEstimation::GetCreateName (), ssi::GSRArousalEstimation::Create) && result;
	result = ssi::Factory::Register (ssi::GSRArtifactFilter::GetCreateName (), ssi::GSRArtifactFilter::Create) && result;
	result = ssi::Factory::Register (ssi::GSREventSender::GetCreateName (), ssi::GSREventSender::Create) && result;
	result = ssi::Factory::Register (ssi::GSRRemoveBaseline::GetCreateName (), ssi::GSRRemoveBaseline::Create) && result;
	result = ssi::Factory::Register (ssi::GSRBaselineMean::GetCreateName (), ssi::GSRBaselineMean::Create) && result;
	result = ssi::Factory::Register (ssi::QRSPreProcess::GetCreateName (), ssi::QRSPreProcess::Create) && result;		
	result = ssi::Factory::Register (ssi::QRSDetect::GetCreateName (), ssi::QRSDetect::Create) && result;		
	result = ssi::Factory::Register (ssi::QRSDetection::GetCreateName (), ssi::QRSDetection::Create) && result;		
	result = ssi::Factory::Register (ssi::QRSPulseEventListener::GetCreateName (), ssi::QRSPulseEventListener::Create) && result;
	result = ssi::Factory::Register (ssi::QRSHrvEventListener::GetCreateName (), ssi::QRSHrvEventListener::Create) && result;
	result = ssi::Factory::Register (ssi::QRSHeartRate::GetCreateName (), ssi::QRSHeartRate::Create) && result;
	result = ssi::Factory::Register (ssi::QRSHRVspectral::GetCreateName (), ssi::QRSHRVspectral::Create) && result;
	result = ssi::Factory::Register (ssi::QRSHRVtime::GetCreateName (), ssi::QRSHRVtime::Create) && result;
	result = ssi::Factory::Register (ssi::QRSHeartRateMean::GetCreateName (), ssi::QRSHeartRateMean::Create) && result;
	result = ssi::Factory::Register(ssi::GSRResponseEventSender::GetCreateName(), ssi::GSRResponseEventSender::Create) && result;
	result = ssi::Factory::Register(ssi::GSREventListener::GetCreateName(), ssi::GSREventListener::Create) && result;
	result = ssi::Factory::Register(ssi::GSRFeatures::GetCreateName(), ssi::GSRFeatures::Create) && result;
	result = ssi::Factory::Register(ssi::BVPBeatEventRawListener::GetCreateName(), ssi::BVPBeatEventRawListener::Create) && result;
	result = ssi::Factory::Register(ssi::BVPBeatEventStatisticalListener::GetCreateName(), ssi::BVPBeatEventStatisticalListener::Create) && result;
	result = ssi::Factory::Register(ssi::BVPBeatEventSender::GetCreateName(), ssi::BVPBeatEventSender::Create) && result;
	result = ssi::Factory::Register(ssi::BVPBeatEventRMSSDListener::GetCreateName(), ssi::BVPBeatEventRMSSDListener::Create) && result;
	result = ssi::Factory::Register(ssi::EMGRemoveBaseline::GetCreateName(), ssi::EMGRemoveBaseline::Create) && result;
	result = ssi::Factory::Register(ssi::EMGRectify::GetCreateName(), ssi::EMGRectify::Create) && result;
	result = ssi::Factory::Register(ssi::EMGDetermineNoise::GetCreateName(), ssi::EMGDetermineNoise::Create) && result;
	result = ssi::Factory::Register(ssi::EMGFeaturesTime::GetCreateName(), ssi::EMGFeaturesTime::Create) && result;
	result = ssi::Factory::Register(ssi::EMGFeaturesSpectral::GetCreateName(), ssi::EMGFeaturesSpectral::Create) && result;
	result = ssi::Factory::Register(ssi::RSPFeaturesTime::GetCreateName(), ssi::RSPFeaturesTime::Create) && result;
	result = ssi::Factory::Register(ssi::RSPFeaturesSpectral::GetCreateName(), ssi::RSPFeaturesSpectral::Create) && result;
	
	return result;
}
