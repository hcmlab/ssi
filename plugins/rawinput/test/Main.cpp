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
#include <Rawinput.h>
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_rawinput (void *arg);
bool ex_rawinput_all (void *arg);
bool ex_rawinput_joy(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
	Factory::RegisterDLL ("ssirawinput");
	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiioput");

	Exsemble ex;
	ex.add(&ex_rawinput, 0, "RAWINPUT", "Mouse and keyboard.");
	ex.add(&ex_rawinput_all, 0, "RAWINPUT ALL", "Mouse, keyboard and joystick.");
	ex.add(&ex_rawinput_joy, 0, "RAWINPUT Joystick", "Joystick.");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_rawinput (void *arg) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	
	Rawinput *rawinput = ssi_create(Rawinput, 0, true);
	
	frame->AddRunnable(rawinput);
	board->RegisterSender(*rawinput);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");	
	board->RegisterListener(*monitor);

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

bool ex_rawinput_all(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Rawinput *rawinput = ssi_create(Rawinput, 0, true);

	bool list[] = {true, true, true};
	rawinput->getOptions()->setDevices(list);

	frame->AddRunnable(rawinput);
	board->RegisterSender(*rawinput);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");	
	board->RegisterListener(*monitor);

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

bool ex_rawinput_joy(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Rawinput *rawinput = ssi_create(Rawinput, 0, true);

	bool list[] = { false, false, true };
	rawinput->getOptions()->setDevices(list);

	frame->AddRunnable(rawinput);
	board->RegisterSender(*rawinput);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");	
	board->RegisterListener(*monitor);

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
