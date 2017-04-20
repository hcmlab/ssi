// ssisignal.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/04/13
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

#ifndef SSI_SIGNAL_H
#define	SSI_SIGNAL_H

#include "Derivative.h"
#include "Butfilt.h"
#include "Energy.h"
#include "Integral.h"
#include "Intensity.h"
#include "MFCC.h"
#include "Spectrogram.h"
#include "Functionals.h"
#include "FunctionalsEventSender.h"
#include "DownSample.h"
#include "Normalize.h"
#include "MvgAvgVar.h"
#include "MvgMinMax.h"
#include "MvgNorm.h"
#include "MvgPeakGate.h"
#include "MvgDrvtv.h"
#include "MvgConDiv.h"
#include "MvgMedian.h"
#include "Pulse.h"
#include "Multiply.h"
#include "Noise.h"
#include "FFTfeat.h"
#include "ConvPower.h"
#include "Expression.h"
#include "Limits.hpp"
#include "Gate.h"
#include "Bundle.h"
#include "Statistics.h"
#include "Sum.h"
#include "Relative.h"
#include "Mean.h"

#endif
