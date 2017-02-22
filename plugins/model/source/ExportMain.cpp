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

#include "ssimodel.h"
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
	
	result = ssi::Factory::Register (ssi::Collector::GetCreateName (),ssi::Collector::Create) && result;	
	result = ssi::Factory::Register (ssi::Classifier::GetCreateName (),ssi::Classifier::Create) && result;	
	result = ssi::Factory::Register (ssi::ClassifierT::GetCreateName (),ssi::ClassifierT::Create) && result;	
	result = ssi::Factory::Register (ssi::SimpleKNN::GetCreateName (),ssi::SimpleKNN::Create) && result;	
	result = ssi::Factory::Register (ssi::KNearestNeighbors::GetCreateName (), ssi::KNearestNeighbors::Create) && result;
	result = ssi::Factory::Register (ssi::Dollar$1::GetCreateName (),ssi::Dollar$1::Create) && result;	
	result = ssi::Factory::Register (ssi::SimpleFusion::GetCreateName (),ssi::SimpleFusion::Create) && result;	
	result = ssi::Factory::Register (ssi::SVM::GetCreateName (),ssi::SVM::Create) && result;	
	result = ssi::Factory::Register (ssi::NaiveBayes::GetCreateName (),ssi::NaiveBayes::Create) && result;	
	result = ssi::Factory::Register (ssi::Relief::GetCreateName (),ssi::Relief::Create) && result;	
	result = ssi::Factory::Register (ssi::Rank::GetCreateName (),ssi::Rank::Create) && result;	
	result = ssi::Factory::Register (ssi::FloatingSearch::GetCreateName (),ssi::FloatingSearch::Create) && result;
	result = ssi::Factory::Register (ssi::RandomFusion::GetCreateName (), ssi::RandomFusion::Create) && result;
	result = ssi::Factory::Register (ssi::FloatingCFS::GetCreateName (), ssi::FloatingCFS::Create) && result;
	result = ssi::Factory::Register (ssi::KMeans::GetCreateName (), ssi::KMeans::Create) && result;
	result = ssi::Factory::Register (ssi::PCA::GetCreateName (), ssi::PCA::Create) && result;
	result = ssi::Factory::Register (ssi::LDA::GetCreateName (), ssi::LDA::Create) && result;
	result = ssi::Factory::Register (ssi::Fisher::GetCreateName (), ssi::Fisher::Create) && result;
	result = ssi::Factory::Register (ssi::HierarchicalModel::GetCreateName(), ssi::HierarchicalModel::Create) && result;
	result = ssi::Factory::Register (ssi::DecisionSmoother::GetCreateName(), ssi::DecisionSmoother::Create) && result;

	return result;
}
