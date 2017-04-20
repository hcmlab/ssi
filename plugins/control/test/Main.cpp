// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 20/11/2015
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
#include "ssicontrol.h"
#include "MyObject.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_controller(void *arg);
bool ex_slider(void *arg);
bool ex_checkbox(void *arg);
bool ex_button(void *arg);
bool ex_text(void *arg);
bool ex_grid(void *arg);
bool ex_event(void *arg);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssiframe"); 
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssimouse");
	Factory::RegisterDLL ("ssicontrol");
	Factory::RegisterDLL("ssigraphic");

	Factory::Register(MyObject::GetCreateName(), MyObject::Create);

	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);

#ifndef SSI_USE_SDL
	ex.add(ex_controller, 0, "CONTROLLER", "How to use a controller to manipulate objects in a pipeline");
	ex.add(ex_slider, 0, "SILDER", "How to use a slider to control an option of another object");
	ex.add(ex_checkbox, 0, "CHECKBOX", "How to use a checkbox to control an option of another object or turn an object on/off");
	ex.add(ex_button, 0, "BUTTON", "How to use a button to send messages to another object");
	ex.add(ex_text, 0, "TEXT", "How to use a text field to contorl an option of to another object");
	ex.add(ex_grid, 0, "CONTROL", "How to control options of another object through a property grid");
	ex.add(ex_event, 0, "EVENT", "How to control options of another object through an event");

#endif

	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

#ifndef SSI_USE_SDL

bool ex_controller(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();
	
	Decorator *decorator = ssi_create_id(Decorator, 0, "decorator");
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create_id (ZeroEventSender, 0, "ezero");
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	SignalPainter *mouse_plot = ssi_create_id (SignalPainter, 0, "plot");
	mouse_plot->getOptions()->setTitle("CURSOR");
	mouse_plot->getOptions()->size = 0;
	mouse_plot->getOptions()->fix[0] = 0;
	mouse_plot->getOptions()->fix[1] = 1;
	mouse_plot->getOptions()->autoscale = false;
	mouse_plot->getOptions()->type = PaintSignalType::PATH;
	frame->AddConsumer(cursor_p, mouse_plot, "1", "20");

	SignalPainter *mouse_tr_plot = ssi_create_id (SignalPainter, 0, "plot_tr");
	mouse_tr_plot->getOptions()->setTitle("CURSOR(TRIGGER)");
	frame->AddEventConsumer(cursor_p, mouse_tr_plot, board, ezero->getEventAddress());

	decorator->add("plot*", 1, 2, 0, 0, 400, CONSOLE_HEIGHT);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	monitor->getOptions()->chars = 1000;
	monitor->getOptions()->update_ms = 100;
	board->RegisterListener(*monitor, 0, 60000);

	decorator->add("monitor", 400, 0, 400, CONSOLE_HEIGHT/2);

	Controller *controller = ssi_create (Controller, "controller", true);
	controller->getOptions()->setPos(CONSOLE_WIDTH + 400, CONSOLE_HEIGHT / 2, 400, CONSOLE_HEIGHT / 2);
	controller->getOptions()->setTitle("CONTROLLER");
	frame->AddRunnable(controller);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_slider(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	MyObject *o = ssi_create_id(MyObject, 0, "object");
	o->getOptions()->value = 0.0;

	ControlSlider *slider = ssi_create(ControlSlider, "slider", true);
	slider->getOptions()->minval = -1.0f;
	slider->getOptions()->maxval = 1.0f;
	slider->getOptions()->setId("object");
	slider->getOptions()->setTitle("value");
	slider->getOptions()->setPos(CONSOLE_WIDTH, 0, 200, 100);
	frame->AddRunnable(slider);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_checkbox(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create_id(Decorator, 0, "decorator");
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);
	Decorator *decorator2 = ssi_create(Decorator, 0, true);
	decorator2->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator2);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create_id(ZeroEventSender, 0, "ezero");
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->eager = false;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	SignalPainter *plot = 0;
	
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR");
	plot->getOptions()->size = 10;
	frame->AddConsumer(cursor_p, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR(TRIGGER)");
	frame->AddEventConsumer(cursor_p, plot, board, ezero->getEventAddress());

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "plot");
	monitor->getOptions()->all = true;
	monitor->getOptions()->chars = 1000;
	monitor->getOptions()->update_ms = 100;
	board->RegisterListener(*monitor, 0, 60000);

	decorator->add("plot*", 1, 3, 0, 0, 400, CONSOLE_HEIGHT);
	ControlCheckBox *checkbox = 0;
	
	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");
	checkbox->getOptions()->setId("decorator");
	checkbox->getOptions()->setName("show");
	checkbox->getOptions()->setTitle("VISUALIZATION");
	checkbox->getOptions()->setLabel("SHOW");
	frame->AddRunnable(checkbox);

	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");
	checkbox->getOptions()->setId("ezero");
	checkbox->getOptions()->setName("eager");
	checkbox->getOptions()->setTitle("OPTION");
	checkbox->getOptions()->setLabel("EZERO:EAGER");
	frame->AddRunnable(checkbox);

	decorator2->add("checkbox*", 1, 2, 400, 0, 200, 150);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_button(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	MyObject *o = ssi_create_id(MyObject, 0, "object1");
	o->getOptions()->value = 0.0;

	o = ssi_create_id(MyObject, 0, "object2");
	o->getOptions()->value = 0.0;

	ControlButton *button = ssi_create(ControlButton, "button", true);	
	button->getOptions()->setLabel("click to reset");
	button->getOptions()->setTitle("BUTTON");
	button->getOptions()->setId("object*");
	button->getOptions()->setMessage("reset");	
	button->getOptions()->setPos(CONSOLE_WIDTH, 0, 200, 200);
	frame->AddRunnable(button);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_text(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	MyObject *o = ssi_create_id(MyObject, 0, "object1");
	o->getOptions()->value = 0.0;

	o = ssi_create_id(MyObject, 0, "object2");
	o->getOptions()->value = 0.0;

	ControlTextBox *text = 0;
	
	text = ssi_create(ControlTextBox, "text", true);
	text->getOptions()->setTitle("STRING");
	text->getOptions()->setId("object*");
	text->getOptions()->setName("string");
	text->getOptions()->setPos(CONSOLE_WIDTH, 0, 200, 200);
	frame->AddRunnable(text);

	text = ssi_create(ControlTextBox, 0, true);
	text->getOptions()->setTitle("VALUE");
	text->getOptions()->setId("object*");
	text->getOptions()->setName("value");
	text->getOptions()->setPos(CONSOLE_WIDTH, 200, 200, 200);
	frame->AddRunnable(text);

	text = ssi_create(ControlTextBox, 0, true);
	text->getOptions()->setTitle("CHECK");
	text->getOptions()->setId("object*");
	text->getOptions()->setName("check");
	text->getOptions()->setPos(CONSOLE_WIDTH, 400, 200, 200);
	frame->AddRunnable(text);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_grid(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	ssi_create_id(MyObject, "object1", "o1");
	ssi_create_id(MyObject, "object2", "o2");
	ssi_create_id(MyObject, "object3", "another");

	ControlGrid *grid = ssi_create(ControlGrid, "grid", true);
	grid->getOptions()->showEnabled = false;
	grid->getOptions()->setTitle("CONTROL OBJECTS");
	grid->getOptions()->setId("o*,another");	
	//grid->getOptions()->setId("");
	grid->getOptions()->setPos(CONSOLE_WIDTH, 0, 300, 300);
	frame->AddRunnable(grid);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_event(void *arg) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create_id(Decorator, 0, "decorator");
	decorator->getOptions()->setOrigin(CONSOLE_WIDTH, 0);
	frame->AddDecorator(decorator);


	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create_id(ZeroEventSender, 0, "ezero");
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->setString("test");
	frame->AddConsumer(button_p, ezero, "0.2s");
	board->RegisterSender(*ezero);

	ControlEvent *cevent = ssi_create_id(ControlEvent, 0, "cevent");
	board->RegisterListener(*cevent, "@mouse");

	decorator->add("plot*", 1, 2, 0, 0, 400, CONSOLE_HEIGHT);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	monitor->getOptions()->chars = 1000;
	monitor->getOptions()->update_ms = 100;
	board->RegisterListener(*monitor, 0, 60000);

	decorator->add("monitor", 400, 0, 400, CONSOLE_HEIGHT / 2);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}


#endif