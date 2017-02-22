// ssiiom.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#pragma once

#ifndef SSI_BIOSIG_H
#define SSI_BIOSIG_H

#include "GSRArousalCombination.h"
#include "GSRArousalEstimation.h"
#include "GSRArtifactFilter.h"
#include "GSREventSender.h"
#include "GSRRemoveBaseline.h"
#include "GSRBaselineMean.h"
#include "QRSPreProcess.h"
#include "QRSDetect.h"
#include "QRSDetection.h"
#include "QRSPulseEventListener.h"
#include "QRSHrvEventListener.h"
#include "QRSHeartRate.h"
#include "QRSHRVspectral.h"
#include "QRSHRVtime.h"
#include "QRSHeartRateMean.h"

#include "GSRResponseEventSender.h"
#include "GSRResponseEventListener.h"
#include "GSRFeatures.h"

#include "BVPBeatEventSender.h"
#include "BVPBeatEventStatisticalListener.h"
#include "BVPBeatEventRawListener.h"
#include "BVPBeatEventRMSSDListener.h"

#include "EMGRemoveBaseline.h"
#include "EMGRectify.h"
#include "EMGDetermineNoise.h"
#include "EMGFeaturesTime.h"
#include "EMGFeaturesSpectral.h"
#include "RSPFeaturesTime.h"
#include "RSPFeaturesSpectral.h"




#endif
