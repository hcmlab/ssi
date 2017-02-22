// ssitorch.h
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

#pragma once

#ifndef SSI_OPENSMILE_H
#define SSI_OPENSMILE_H

#include "OSPreemphasis.h"
#include "OSEnergy.h"
#include "OSWindow.h"
#include "OSTransformFFT.h"
#include "OSFFTmagphase.h"
#include "OSMelspec.h"
#include "OSMfcc.h"
#include "OSSpecScale.h"
#include "OSPitchShs.h"
#include "OSPitchSmoother.h"
#include "OSFunctionals.h"
#include "Deltas.h"
#include "OSPlp.h"
#include "OSLpc.h"
#include "OSVad.h"
#include "OSPitchDirection.h"

#include "OSPitchChain.h"
#include "OSMfccChain.h"
#include "OSPlpChain.h"
#include "OSIntensity.h"

#include "LaughterFeatureExtractor.h"
#include "LaughterPreProcessor.h"

#endif
