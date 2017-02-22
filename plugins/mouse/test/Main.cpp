// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_mouse (void *arg);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssigraphic");

	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(&ex_mouse, 0, "MOUSE", "How to use 'Mouse' sensor in a pipeline.");	
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_mouse (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();
	ITheEventBoard *board = Factory::GetEventBoard ();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);
	
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);
	
	FileWriter *cursor_write = ssi_create (FileWriter, 0, true);
	cursor_write->getOptions()->setPath("cursor");
	cursor_write->getOptions()->stream = true;
	cursor_write->getOptions()->type = File::ASCII;
	frame->AddConsumer(cursor_p, cursor_write, "0.25s");

	FileWriter *button_write = ssi_create (FileWriter, 0, true);
	button_write->getOptions()->setPath("button");
	button_write->getOptions()->stream = true;
	button_write->getOptions()->type = File::ASCII;
	frame->AddConsumer(button_p, button_write, "0.25s");

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;	
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse");
	plot->getOptions()->size = 0;
	plot->getOptions()->fix[0] = 0;
	plot->getOptions()->fix[1] = 1;
	plot->getOptions()->autoscale = false;
	plot->getOptions()->type = PaintSignalType::PATH;
	frame->AddConsumer(cursor_p, plot, "1", "20");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse(tr)");
	frame->AddEventConsumer(cursor_p, plot, board, ezero->getEventAddress());

	decorator->add("plot*", 1, 2, 0, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
