// Fusion.h
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
#pragma once

#ifndef SSI_FUSION_H
#define SSI_FUSION_H

#include "AdaBoost.h"
#include "BKS.h"
#include "BordaCount.h"
#include "CascadingSpecialists.h"
#include "CascadingSpecialistsMS.h"
#include "DecisionTemplate.h"
#include "DempsterShafer.h"
#include "FeatureFusion.h"
#include "Grading.h"
#include "MajorityVoting.h"
#include "MaxRule.h"
#include "MeanRule.h"
#include "MedianRule.h"
#include "MinRule.h"
#include "OvR.h"
#include "OvRSpecialist.h"
#include "ProductRule.h"
#include "SingleFeatures.h"
#include "StackedGeneralization.h"
#include "SumRule.h"
#include "VACFusion.h"
#include "WeightedAverage.h"
#include "WeightedMajorityVoting.h"

#endif
