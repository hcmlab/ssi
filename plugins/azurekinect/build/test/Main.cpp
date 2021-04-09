// Main
// author: Fabian Wildgrube
// created: 2021
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
 
#define NOMINMAX

#include "ssi.h"
#include "ssiazurekinect.h"
using namespace ssi;

// load libraries
#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiazurekinect");
	Factory::RegisterDLL("ssisignal");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("ssiioput");

	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);

	ITransformable* rgb_p = frame->AddProvider(kinect, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	ITransformable* ir_p = frame->AddProvider(kinect, SSI_AZUREKINECT_IRIMAGE_PROVIDER_NAME, 0, "1.0s");
	frame->AddSensor(kinect);

	VideoPainter* vplot = 0;

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("rgb");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("infrared");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(ir_p, vplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 640, 360);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}