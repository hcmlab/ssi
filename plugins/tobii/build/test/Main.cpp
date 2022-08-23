// Main
// author: Tobias Baur <baur@hcm-lab.de>
// created: 2013/2/28
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
 
#include "ssi.h"
#include "ssiTobii.h"
#include "camera\include\ssicamera.h"
using namespace ssi;

//#define START_SERVER

bool ex_tracker();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssitobii.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiioput.dll");	



	ex_tracker();
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_tracker() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Tobii *tobii = ssi_create(Tobii, 0, true);
	tobii->getOptions()->sr = 120;
	ITransformable *stream_t[Tobii::CHANNELS::NUM];

	for (ssi_size_t i = 0; i < Tobii::CHANNELS::NUM; i++) {
		stream_t[i] = frame->AddProvider(tobii, tobii->getChannel(i)->getName());
	}
	frame->AddSensor(tobii);

	FileWriter *writer = 0;

	for (ssi_size_t i = 0; i < Tobii::CHANNELS::NUM; i++) {
		writer = ssi_create(FileWriter, 0, true);
		writer->getOptions()->setPath(tobii->getChannel(i)->getName());
		writer->getOptions()->type = File::ASCII;
		frame->AddConsumer(stream_t[i], writer, "1");
	}

	SignalPainter *plot = 0;
	
	for (ssi_size_t i = 0; i < Tobii::CHANNELS::NUM; i++) {
		plot = ssi_create_id (SignalPainter, 0, "plot");
		plot->getOptions()->setTitle(tobii->getChannel(i)->getName());
		plot->getOptions()->size = 1.0;
		frame->AddConsumer(stream_t[i], plot, "1");
	}




	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}



