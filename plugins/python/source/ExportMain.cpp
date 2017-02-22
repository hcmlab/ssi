// ExportMain.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssipython.h"
#include "base/Factory.h"
#include "PythonManager.h"
extern "C"
{
#include "ssipy.h"
}


#ifndef DLLEXP
#if _WIN32||_WIN64
#define DLLEXP extern "C" __declspec( dllexport )
#else
#define DLLEXP extern "C" __attribute__((visibility("default")))
#endif
#endif

DLLEXP bool Register (ssi::Factory *factory, FILE *logfile, ssi::IMessage *message)
{
	ssi::Factory::SetFactory (factory);
	ssi::PythonManager::Instance().Init();

	if (logfile) {
		ssiout = logfile;
	}
	if (message) {
		ssimsg = message;
	}

	bool result = true;
	
	result = ssi::Factory::Register(ssi::PythonTransformer::GetCreateName(), ssi::PythonTransformer::Create) && result;
	result = ssi::Factory::Register(ssi::PythonFeature::GetCreateName(), ssi::PythonFeature::Create) && result;
	result = ssi::Factory::Register(ssi::PythonFilter::GetCreateName(), ssi::PythonFilter::Create) && result;
	result = ssi::Factory::Register(ssi::PythonConsumer::GetCreateName(), ssi::PythonConsumer::Create) && result;
	result = ssi::Factory::Register(ssi::PythonObject::GetCreateName(), ssi::PythonObject::Create) && result;
	result = ssi::Factory::Register(ssi::PythonSensor::GetCreateName(), ssi::PythonSensor::Create) && result;
	result = ssi::Factory::Register(ssi::PythonImageFilter::GetCreateName(), ssi::PythonImageFilter::Create) && result;
	result = ssi::Factory::Register(ssi::PythonImageFeature::GetCreateName(), ssi::PythonImageFeature::Create) && result;
	result = ssi::Factory::Register(ssi::PythonImageConsumer::GetCreateName(), ssi::PythonImageConsumer::Create) && result;
	result = ssi::Factory::Register(ssi::PythonModel::GetCreateName(), ssi::PythonModel::Create) && result;

	return result;
}

DLLEXP void UnRegister() 
{
	ssi::PythonManager::Instance().Quit();
}
