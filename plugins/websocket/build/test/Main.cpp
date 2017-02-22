// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 13/3/2015
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssiwebsocket.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_events_empty(void *arg);
bool ex_events_string(void *arg);

bool ex_stream(void *arg);
bool ex_stream_info(void *arg);

bool ex_mobile_events(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiframe");

	Factory::RegisterDLL ("ssiwebsocket");	

	Exsemble ex;
	ex.add(&ex_events_empty, 0,  "EVENTS EMPTY", "How to use Websocket to send SSI events with type EMPTY to the browser and how to receive data from the browser.");
	ex.add(&ex_events_string, 0, "EVENTS STRING", "How to use Websocket to send SSI events with type STRING to the browser and how to receive data from the browser.");

	ex.add(&ex_stream, 0, "STREAMING TO BROWSER", "How to use Websocket to stream SSI streams to the browser.");
	ex.add(&ex_stream_info, 0, "STREAMING TO BROWSER WITH INFO", "How to use Websocket to stream SSI streams to the browser. Stream info message is sent at beginning.");

	ex.add(&ex_mobile_events, 0, "EVENTS FROM BROWSER", "How to use Websocket to receive sensor data from mobile devices.");
	ex.show();

	Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	std::cout << std::endl << "Press Enter to close." << std::endl; 
	getchar();

	return 0;
}

bool ex_events_string(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	ClockEventSender *eclock = ssi_create(ClockEventSender, 0, true);
	eclock->getOptions()->clock = 1000;
	eclock->getOptions()->empty = false;
	eclock->getOptions()->setString("test");
	board->RegisterSender(*eclock);
	frame->AddRunnable(eclock);

	Websocket *websocket = ssi_create(Websocket, 0, true);

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_events_empty(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	ClockEventSender *eclock = ssi_create(ClockEventSender, 0, true);
	eclock->getOptions()->clock = 1000;
	board->RegisterSender(*eclock);
	frame->AddRunnable(eclock);

	Websocket *websocket = ssi_create(Websocket, 0, true);

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_stream(void *arg) {

	Factory::RegisterDLL("ssimouse");

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//ClockEventSender *eclock = ssi_create(ClockEventSender, 0, true);

	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	mouse->getOptions()->sendEvent = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	//ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);
	board->RegisterSender(*mouse);

	Websocket *websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->setHttpRoot("./stream_to_browser");
	//frame->AddConsumer(cursor_p, websocket, "0.25s");
	frame->AddConsumer(cursor_p, websocket, "1");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_stream_info(void *arg) {

	Factory::RegisterDLL("ssimouse");

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	//ClockEventSender *eclock = ssi_create(ClockEventSender, 0, true);

	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	mouse->getOptions()->sendEvent = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);
	board->RegisterSender(*mouse);

	Websocket *websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->setHttpRoot("stream_to_browser_info");
	websocket->getOptions()->send_info = true;
	//frame->AddConsumer(cursor_p, websocket, "0.25s");

	ssi::ITransformable *src[2] = { cursor_p, button_p };
	frame->AddConsumer(2, src, websocket, "1");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_mobile_events(void *arg)
{

	Factory::RegisterDLL("ssiioput");

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Websocket *websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->setHttpRoot("mobile_browser_to_SSI");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	FileEventWriter *writer = ssi_create(FileEventWriter, 0, true);
	writer->getOptions()->setPath("sensor_data");
	board->RegisterListener(*writer);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

	return true;
}
