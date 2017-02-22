// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2014/11/03
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
#include "..\..\include\ssitouchmouse.h"
using namespace ssi;

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_touchmouse ();

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssitouchmouse");

	{
		ex_touchmouse ();	
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

void ex_touchmouse () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	TouchMouse *touchmou = ssi_create (TouchMouse, 0, true);
	touchmou->getOptions()->scale = true;
	touchmou->getOptions()->sr = 50.0;
	ITransformable *touchmou_p = frame->AddProvider(touchmou, SSI_TOUCHMOUSE_PROVIDER_NAME);
	frame->AddSensor(touchmou);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pressure matrix");
	plot->getOptions()->size = 10.0;	
	frame->AddConsumer(touchmou_p, plot, "0.25s");

	SignalPainter *plot2 = 0;
	plot2 = ssi_create_id (SignalPainter, 0, "plot");
	plot2->getOptions()->setTitle("pressure map");
	plot2->getOptions()->size = 0;		
	plot2->getOptions()->type = PaintSignalType::IMAGE;
	plot2->getOptions()->indx = 15;
	plot2->getOptions()->indy = 13;
	plot2->getOptions()->staticImage = true;
	plot2->getOptions()->autoscale = false;
	plot2->getOptions()->fix[0] = 0.0;
	plot2->getOptions()->fix[1] = 1.0;
	plot2->getOptions()->colormap = ssi::Colormap::COLORMAP::GRAY64;
	frame->AddConsumer(touchmou_p, plot2, "1");

/*	FileWriter *data_write = ssi_create (FileWriter, 0, true);
	data_write->getOptions()->setPath("pressuredata");
	data_write->getOptions()->stream = true;
	data_write->getOptions()->type = File::ASCII;
	frame->AddConsumer(touchmou_p, data_write, "0.25s");*/

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

}
