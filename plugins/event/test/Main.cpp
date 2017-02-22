// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/11
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
using namespace ssi;

bool ex_address (void *arg);
bool ex_eboard(void *arg);
bool ex_sender(void *arg);
bool ex_clock(void *arg);
bool ex_thresclass(void *arg);
bool ex_xmlsender(void *arg);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

ssi_char_t string[SSI_MAX_CHAR];

class MySender : public IObject {

	IEventListener *_listener;
	ssi_event_t _e;
public:

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return "MySender"; };
	const ssi_char_t *getInfo() { return "Just a test sender"; };

	bool setEventListener (IEventListener *listener) {		
		static ssi_size_t _scount = 0;
		static ssi_size_t _ecount = 0;
		_listener = listener;
		ssi_sprint (string, "s%u", _scount++);
		ssi_size_t sid = Factory::AddString (string);
		ssi_sprint (string, "e%u", _ecount++);
		ssi_size_t eid = Factory::AddString (string);
		ssi_event_init (_e, SSI_ETYPE_EMPTY, sid, eid); 
		return true;
	}	
	void send_enter () {
		fire ();
	}
	void fire () {
		static ssi_size_t start_ms = ssi_time_ms ();
		_e.time = ssi_time_ms () - start_ms;
		_listener->update(_e);
	}
	void send_flush () {
		fire ();
	}
};

class MyListener : public IObject {

	IOptions *getOptions() { return 0; };
	const ssi_char_t *getName() { return "MyListener"; };
	const ssi_char_t *getInfo() { return "Just a test listener"; };

	bool update (IEvents &es, ssi_size_t n_new_events, ssi_size_t time_ms) {
		ssi_print ("CALLBACK\n\n");
		if (n_new_events > 0) {
			Factory::GetEventBoard ()->Print(es);
			ssi_print ("\n");
		} else {
			ssi_print ("no new events\n"); // must never happen
		}
		return true;
	}
};

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

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssisignal");

	ssi_random_seed ();

	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(&ex_address, 0, "ADDRESS", "How to use 'EventAddress' to compose an event address.");
	ex.add(&ex_eboard, 0, "EBOARD", "How to send/receive events using 'EventBoard'.");
	ex.add(&ex_sender, 0, "SENDER", "How to send/receive events in a pipeline.");
	ex.add(&ex_thresclass, 0, "THRESCLASS", "How to use 'ThresClassEventSender' in a pipeline.");
	ex.add(&ex_clock, 0, "CLOCK", "How to use 'ClockEventSender' in a pipeline.");
	ex.add(&ex_xmlsender, 0, "XMLSENDER", "How to use 'XmlEventSender' in a pipeline.");	
	ex.show();	

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_address (void *arg) {

	EventAddress ea;

	ea.print ();

	ea.setAddress ("e1@s1,s2");
	ea.print ();

	ea.setEvents ("e2,e3");
	ea.print ();

	ea.setSender ("s3");
	ea.print ();

	ea.setAddress ("e4,e5@s4");
	ea.print ();

	ea.clear ();
	ea.setAddress ("e1@s1,s2");
	ea.print ();

	return true;
}

bool ex_eboard (void *arg) {

	Decorator *decorator = ssi_create(Decorator, 0, true);
	ITheEventBoard *board = Factory::GetEventBoard();

	const ssi_size_t n_sender = 5;
	MySender sender[n_sender];
	for (ssi_size_t i = 0; i < n_sender; i++) {		
		board->RegisterSender(sender[i]);
	}

	EventAddress ea;
	ea.setEvents ("e0,e1,e2,e3,e4");
	ea.setSender ("s0,s1,s2,s3,s4");
	ssi_print ("address=%s\n", ea.getAddress ());
	MyListener listener;
	board->RegisterListener(listener, ea.getAddress());

	for (ssi_size_t i = 0; i < 200; i++) {
		ssi_size_t index = ssi_random (n_sender-1);
		sender[index].fire ();				
		Sleep (10);
	}

	board->Stop();
	board->Clear();

	return true;
}

bool ex_sender (void *arg) {

	ITheEventBoard *board = Factory::GetEventBoard ("board");
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 10;
	ITheFramework *frame = Factory::GetFramework ("frame");

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);
	
	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;	
	ezero->getOptions()->maxdur = 5.0;
	ezero->getOptions()->incdur = 1.0;
	ezero->getOptions()->setAddress("mouse@click");	
	ezero->getOptions()->eager = true;
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);
	
	TupleEventSender *tuple_sender = ssi_create (TupleEventSender, "tuple_sender", true);
	tuple_sender->getOptions()->setAddress("pos-floats@mouse");
	frame->AddConsumer(cursor_p, tuple_sender, "2.0s");
	board->RegisterSender(*tuple_sender);

	MapEventSender *map_sender = ssi_create(MapEventSender, "map_sender", true);
	map_sender->getOptions()->setAddress("pos-tuples@mouse");
	map_sender->getOptions()->setKeys("x,y");
	frame->AddEventConsumer(cursor_p, tuple_sender, board, ezero->getEventAddress());
	board->RegisterSender(*tuple_sender);

	StringEventSender *string_sender = ssi_create(StringEventSender, "string_sender", true);
	string_sender->getOptions()->setAddress("pos-string@mouse");
	string_sender->getOptions()->mean = true;
	string_sender->getOptions()->n_buffer = 10000;
	frame->AddEventConsumer(cursor_p, string_sender, board, ezero->getEventAddress());
	board->RegisterSender(*string_sender);
    
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	monitor->getOptions()->chars = 10000;
	monitor->getOptions()->update_ms = 100;
	board->RegisterListener(*monitor, 0, 60000, IEvents::EVENT_STATE_FILTER::ALL);

	decorator->add("monitor*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_thresclass (void *arg) {

	ITheEventBoard *board = Factory::GetEventBoard ("board");
	ITheFramework *frame = Factory::GetFramework ("frame");

	Decorator *decorator = ssi_create(Decorator, 0, true);
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->scale = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
	
	Selector *select = ssi_create(Selector, 0, true);
	select->getOptions()->set(1);
	ITransformable* selector_t = frame->AddTransformer(cursor_p, select, "1");

	ThresClassEventSender *tc = ssi_create (ThresClassEventSender, 0, true);
	tc->getOptions()->setClasses("low, med, high");
	tc->getOptions()->setThresholds("0.1, 0.5, 0.7");
	tc->getOptions()->minDiff = 0.01f;
	frame->AddConsumer(selector_t, tc, "1");
	board->RegisterSender(*tc);	

	SignalPainter* paint = ssi_create_id(SignalPainter, 0, "plot");
	paint->getOptions()->type = PaintSignalType::SIGNAL;
	paint->getOptions()->size = 10;
	paint->getOptions()->setTitle("Mouse Y-Coord");
	frame->AddConsumer(selector_t, paint, "0.1s");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->update_ms = 100;	
	board->RegisterListener(*monitor, "@", 10000);

	decorator->add("plot*", 0, 0, 400, CONSOLE_HEIGHT/2);
	decorator->add("monitor*", 0, CONSOLE_HEIGHT / 2, 400, CONSOLE_HEIGHT / 2);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_clock(void *arg) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->scale = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	Selector *select = ssi_create(Selector, 0, true);
	select->getOptions()->set(1);
	ITransformable* selector_t = frame->AddTransformer(cursor_p, select, "1");

	ClockEventSender *clock = ssi_create(ClockEventSender, 0, true);	
	ssi_size_t clocks[] = { 500, 1500, 3000 };
	clock->getOptions()->setAddress("tick@clock");
	clock->getOptions()->setClocks(3, clocks);
	clock->getOptions()->init = true;
	frame->AddRunnable(clock);
	board->RegisterSender(*clock);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->update_ms = 100;
	board->RegisterListener(*monitor, "@", 10000);

	decorator->add("monitor*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_xmlsender(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 10;

	Cast *button_cast = ssi_create(Cast, 0, true);
	button_cast->getOptions()->cast = SSI_FLOAT;

	Selector *cursor_select = ssi_create(Selector, 0, true);
	ssi_size_t indices[10] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };
	cursor_select->getOptions()->set(10, indices);

	Mouse *mouse = ssi_create(Mouse, 0, "mouse");
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME, cursor_select);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME, button_cast);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->mindur = 0.5;
	ezero->getOptions()->setAddress("click@mouse");	
	frame->AddConsumer(button_p, ezero, "5");
	board->RegisterSender(*ezero);

	TupleEventSender *tuple_sender = ssi_create(TupleEventSender, 0, true);
	tuple_sender->getOptions()->setAddress("features@mouse");
	frame->AddEventConsumer(cursor_p, tuple_sender, board, ezero->getEventAddress());
	board->RegisterSender(*tuple_sender);
	
	XMLEventSender *xmlsender = ssi_create_id(XMLEventSender, 0, "monitor");
	xmlsender->getOptions()->setPath("template");
	xmlsender->getOptions()->monitor = true;
	xmlsender->getOptions()->console = false;
	xmlsender->getOptions()->update = 100;
	xmlsender->getOptions()->coldelim = ' ';
	xmlsender->getOptions()->setAddress("xml@sender");
	ITransformable *xmlsender_pins[] = { cursor_p, button_p };
	frame->AddConsumer(2, xmlsender_pins, xmlsender, "0.5s");
	board->RegisterListener(*xmlsender, 0, 0, IEvents::EVENT_STATE_FILTER::COMPLETED);
	board->RegisterSender(*xmlsender);
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, "click,features@mouse");

	decorator->add("monitor*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
