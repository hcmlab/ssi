// Main.cpp
// author: Fabian Wildgrube
// created: 2021
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
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

#include "ssi.h"
#include "..\\..\\include\\ssishimmer3.h"

#include "..\\..\\..\\signal\\include\\MvgMedian.h"
#include "..\\..\\..\\signal\\include\\Derivative.h"
#include "..\\..\\..\\signal\\include\\MvgAvgVar.h"
#include "..\\..\\..\\signal\\include\\MvgMinMax.h"
#include "..\\..\\..\\signal\\include\\Expression.h"
#include "..\\..\\..\\signal\\include\\MvgPeakGate.h"
#include "..\\..\\..\\signal\\include\\FFTfeat.h"

#include "..\\..\\..\\xmpp\\include\\ssixmpp.h"

using namespace ssi;

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_shimmer3();

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("ssiframe");
		Factory::RegisterDLL("ssigraphic");
		Factory::RegisterDLL("ssievent");
		Factory::RegisterDLL("ssiioput");
		Factory::RegisterDLL("ssisignal");
		Factory::RegisterDLL("ssishimmer3");

		{
			ex_shimmer3();
		}

		ssi_print("\n\n\tpress a key to quit\n");
		getchar();

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_shimmer3() {

	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Shimmer3GSRPlus* gsrplus = ssi_create(Shimmer3GSRPlus, 0, true);
	// look up the "outgoing" COM port of the shimmer you want to talk to (Win->ControlPanel->Search for "Bluetooth" > Change Bluetooth Settings > Tab "COM Ports" -> Port COMX where X is the number you need to use here
	// use the OUTGOING port as we need to send stuff to the shimmer!
	gsrplus->getOptions()->port = 9;
	gsrplus->getOptions()->baud = 115200;
	gsrplus->getOptions()->sr = 128.0;

	ITransformable* ppg_p = frame->AddProvider(gsrplus, SSI_SHIMMER3_PPGRAW_PROVIDER_NAME);
	ITransformable* ppgcal_p = frame->AddProvider(gsrplus, SSI_SHIMMER3_PPGCALIBRATED_PROVIDER_NAME);
	//ITransformable* gsr_p = frame->AddProvider(gsrplus, SSI_SHIMMER3_GSRRAW_PROVIDER_NAME);
	//ITransformable* gsrR_p = frame->AddProvider(gsrplus, SSI_SHIMMER3_GSRCALIBRATEDRESISTANCE_PROVIDER_NAME);
	//ITransformable* gsrC_p = frame->AddProvider(gsrplus, SSI_SHIMMER3_GSRCALIBRATEDCONDUCTANCE_PROVIDER_NAME);

	frame->AddSensor(gsrplus);
	/*
	FFTfeat* fftfeat = ssi_create(FFTfeat, 0, true);
	fftfeat->getOptions()->nfft = 32;
	*/

	SignalPainter* plot = 0;
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("ppg raw");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(ppg_p, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("ppg calibrated");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(ppgcal_p, plot, "1");

	/*
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr raw");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(gsr_p, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr resistance");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(gsrR_p, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("gsr conductance");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(gsrC_p, plot, "1");
	*/

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);

	frame->Start();

	ssi_print("press enter to continue\n");
	getchar();

	frame->Stop();
	frame->Clear();

}