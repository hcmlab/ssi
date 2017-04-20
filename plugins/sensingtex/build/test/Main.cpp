// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2014/09/02
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#include "ssi.h"
#include "../../include/ssisensingtex.h"
using namespace ssi;

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_sensingtex ();

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssisensingtex");

	{
		ex_sensingtex ();	
	}

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_sensingtex () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	SensingTex *senstex = ssi_create (SensingTex, 0, true);
	senstex->getOptions()->scale = true;
	senstex->getOptions()->setSerialDev("COM14");
	senstex->getOptions()->cols = 16;	//floor mat: 16		//sitting mat: 8
	senstex->getOptions()->rows = 14;	//floor mat: 14		//sitting mat: 8
	senstex->getOptions()->sr = 20.0;
	senstex->getOptions()->fps = true;
	ITransformable *senstex_p = frame->AddProvider(senstex, SSI_SENSINGTEX_PROVIDER_NAME);
	frame->AddSensor(senstex);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pressure matrix");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(senstex_p, plot, "0.25s");

	SignalPainter *plot2 = 0;
	plot2 = ssi_create_id (SignalPainter, 0, "plot");
	plot2->getOptions()->setTitle("pressure map");
	plot2->getOptions()->size = 0;		
	plot2->getOptions()->type = PaintSignalType::IMAGE;
	plot2->getOptions()->indx = senstex->getOptions()->cols;
	plot2->getOptions()->indy = senstex->getOptions()->rows;
	plot2->getOptions()->staticImage = true;
	plot2->getOptions()->autoscale = false;
	plot2->getOptions()->fix[0] = 0.0;
	plot2->getOptions()->fix[1] = 0.5;
	plot2->getOptions()->colormap = ssi::Colormap::COLORMAP::GRAY64;
	frame->AddConsumer(senstex_p, plot2, "1");

/*	FileWriter *data_write = ssi_create (FileWriter, 0, true);
	data_write->getOptions()->setPath("pressuredata");
	data_write->getOptions()->stream = true;
	data_write->getOptions()->type = File::ASCII;
	frame->AddConsumer(senstex_p, data_write, "0.25s");*/

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

}
