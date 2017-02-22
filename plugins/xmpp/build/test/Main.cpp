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
#include "ssixmpp.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_events_to_xmpp(void *arg);
bool ex_events_to_xmpp_templates(void *arg);
bool ex_events_to_xmpp_publish(void *arg);
bool ex_xmpp_to_event_subscribe(void *arg);

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
{
#endif

	ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiframe");

	Factory::RegisterDLL("ssixmpp");

	Exsemble ex;

	ex.add(&ex_events_to_xmpp, 0,			"EVENTS TO XMPP", "Conversion of SSI events to XMPP messages (JSON).");
	ex.add(&ex_events_to_xmpp_templates, 0, "EVENTS TO XMPP", "Conversion of SSI events to XMPP messages (templates).");
	ex.add(&ex_events_to_xmpp_publish, 0,	"EVENTS TO XMPP (PUBLISH)", "Conversion of SSI events to XMPP pubsub.");
	ex.add(&ex_xmpp_to_event_subscribe, 0,	"XMPP TO EVENTS", "Conversion of XMPP events to SSI events.");

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

bool ex_events_to_xmpp(void *arg)
{

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
	
	XMPP *xmpp = ssi_create (XMPP, 0, true);
	xmpp->getOptions()->setJID("recommender@caretablet1");
	xmpp->getOptions()->setPw("123");

	xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_JSON;

	xmpp->getOptions()->setSenderName("XMPP");
	xmpp->getOptions()->setEventName("message");

	//send message
	{
		xmpp->getOptions()->pubNode = false;
		xmpp->getOptions()->setRecip("discrete_data_interpreter@caretablet1");
	} 

	board->RegisterSender(*xmpp);
	board->RegisterListener(*xmpp);

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

	Factory::Clear ();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();

	return true;
}

bool ex_events_to_xmpp_templates(void *arg)
{

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

	XMPP *xmpp = ssi_create(XMPP, 0, true);
	xmpp->getOptions()->setJID("test@andy");
	xmpp->getOptions()->setPw("123");

	//xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_JSON;
	xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_TEMPLATE;

	xmpp->getOptions()->setSenderName("XMPP");
	xmpp->getOptions()->setEventName("message");

	//send message
	{
		xmpp->getOptions()->pubNode = false;
		xmpp->getOptions()->setRecip("logger@andy");
	}

	board->RegisterSender(*xmpp);
	board->RegisterListener(*xmpp);

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

	Factory::Clear();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();

	return true;
}

bool ex_events_to_xmpp_publish(void *arg)
{
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

	XMPP *xmpp = ssi_create(XMPP, 0, true);
	xmpp->getOptions()->setJID("test@andy");
	xmpp->getOptions()->setPw("123");

	//xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_JSON;
	xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_TEMPLATE;

	xmpp->getOptions()->setSenderName("XMPP");
	xmpp->getOptions()->setEventName("message");

	//publish to node
	{
		xmpp->getOptions()->pubNode = true;
		xmpp->getOptions()->setPubNodeName("testnode2");
		xmpp->getOptions()->setPubNodeService("pubsub.andy");
	}

	board->RegisterSender(*xmpp);
	board->RegisterListener(*xmpp);

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

	Factory::Clear();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();

	return true;
}

bool ex_xmpp_to_event_subscribe(void *arg)
{

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	XMPP *xmpp = ssi_create(XMPP, 0, true);
	xmpp->getOptions()->setJID("recommender@caretablet1");
	xmpp->getOptions()->setPw("123");

	xmpp->getOptions()->setSenderName("XMPP");
	xmpp->getOptions()->setEventName("message");

	//subscribe to node
	if (true) {
		xmpp->getOptions()->subNode = true;
		xmpp->getOptions()->setSubNodeName("presence");
		xmpp->getOptions()->setSubNodeService("pubsub.caretablet1");
	}

	board->RegisterSender(*xmpp);
	board->RegisterListener(*xmpp);

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

	Factory::Clear();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();

	return true;
}
