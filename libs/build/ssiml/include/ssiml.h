// ssi.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/05
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
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

#ifndef SSI_ML_H
#define	SSI_ML_H

#include "model/SampleList.h"
#include "Trainer.h"
#include "Evaluation.h"
#include "Evaluation2Latex.h"
#include "Selection.h"
#include "ISHotClass.h"
#include "ISReClass.h"
#include "ISTransform.h"
#include "ISSelectDim.h"
#include "ISSelectSample.h"
#include "ISSelectClass.h"
#include "ISSelectUser.h"
#include "ISMergeStrms.h"
#include "ISAlignStrms.h"
#include "ISMissingData.h"
#include "ISSplitStream.h"
#include "ISMergeSample.h"
#include "ISOverSample.h"
#include "ISUnderSample.h"
#include "ISNorm.h"
#include "ISDuplStrms.h"
#include "ISTrigger.h"
#include "SampleArff.h"
#include "ElanDocument.h"
#include "ElanTools.h"
#include "Machine.h"

#endif
