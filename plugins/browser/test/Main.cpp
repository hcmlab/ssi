// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 17/11/2014
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
#include "ssibrowser.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool browser(void *arg);
bool htmlstr(void *arg);
bool stimuli(void *arg);
bool images(void *arg);
bool tuple(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssiframe.dll");
	Factory::RegisterDLL("ssievent.dll");
	Factory::RegisterDLL("ssimouse.dll");
	Factory::RegisterDLL("ssibrowser.dll");
	Factory::RegisterDLL("ssiioput.dll");
	
	ssi_random_seed();

	Exsemble ex;
	ex.add(&browser, 0, "BROWSER", "How to browse web pages.");
	ex.add(&htmlstr, 0, "HTMLSTR", "How to display an HTML string.");
	ex.add(&stimuli, 0, "STIMULI", "How to browse stimuli files.");
	ex.add(&images, 0, "IMAGES", "How to browse images.");
	ex.add(&tuple, 0, "TUPLE", "How to visualize a tuple.");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool browser(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_pcast(TheFramework, frame)->getOptions()->countdown = 0;
	ITheEventBoard *board = Factory::GetEventBoard();
	
	ssi_event_t e_url;
	ssi_event_init(e_url, SSI_ETYPE_STRING, Factory::AddString("sender"), Factory::AddString("url"), 0, 0, SSI_MAX_CHAR);

	Browser *browser = ssi_create_id(Browser, 0, "plot");
	browser->getOptions()->setTitle("Browser");
	browser->getOptions()->setUrl("http://openssi.net");
	board->RegisterListener(*browser, "url@");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	ssi_print("\n\n\tpress enter to load another page\n");
	getchar();

	e_url.time = frame->GetRunTimeMs();
	ssi_strcpy(e_url.ptr, "..\\..\\..\\docs\\api\\index.html");
	board->update(e_url);

	ssi_print("\n\n\tpress a key to continue\n");
	getchar();

	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool htmlstr(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_pcast(TheFramework, frame)->getOptions()->countdown = 0;
	ITheEventBoard *board = Factory::GetEventBoard();

	ssi_event_t e_url;
	ssi_event_init(e_url, SSI_ETYPE_STRING, Factory::AddString("sender"), Factory::AddString("html"), 0, 0, SSI_MAX_CHAR);

	Browser *browser = ssi_create_id(Browser, 0, "plot");
	browser->getOptions()->setTitle("Browser");
	browser->getOptions()->HTMLstr = true;
	board->RegisterListener(*browser, "html@");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	ssi_print("\n\n\tpress enter to display html string\n");
	getchar();

	e_url.time = frame->GetRunTimeMs();
	ssi_strcpy(e_url.ptr, "<html><body>Hello World!</body></html>");
	board->update(e_url);

	ssi_print("\n\n\tpress enter to display another html string\n");
	getchar();

	e_url.time = frame->GetRunTimeMs();
	ssi_strcpy(e_url.ptr, "<html><body>Another HTML String</body></html>");
	board->update(e_url);

	ssi_print("\n\n\tpress a key to continue\n");
	getchar();

	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool stimuli(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_t = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// stimuli from folders

	ZeroEventSender *gate = ssi_create(ZeroEventSender, 0, true);	
	gate->getOptions()->setEvent("folder");
	gate->getOptions()->setSender("click");
	gate->getOptions()->empty = false;
	frame->AddConsumer(button_t, gate, "0.1s");
	board->RegisterSender(*gate);

	Stimuli *stimuli = ssi_create(Stimuli, 0, true);
	stimuli->getOptions()->setSender("stimuliFromFolder");
	stimuli->getOptions()->setFolder("stimuli/Numbers;stimuli/Shapes");	
	stimuli->getOptions()->labelFromFile = true;
	stimuli->getOptions()->setStartName("stimuli/start.html");
	stimuli->getOptions()->setEndName("stimuli/end.html");		
	stimuli->getOptions()->applyLabelToEvent = true;
	stimuli->getOptions()->randomize = true;
	stimuli->getOptions()->setAnnoPath("stimuli");
	board->RegisterListener(*stimuli, gate->getEventAddress());
	board->RegisterSender(*stimuli);

	Browser *browser = ssi_create_id(Browser, 0, "plot");
	browser->getOptions()->setTitle("Browser");
	browser->getOptions()->setPos(400, 0, 400, 300);
	board->RegisterListener(*browser, stimuli->getEventAddress());

	FileSampleWriter *writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setClasses("number;shape");
	writer->getOptions()->setUser("user");
	frame->AddEventConsumer(cursor_t, writer, board, gate->getEventAddress());

	writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->setPath("gesture");
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setClasses("number;shape");
	writer->getOptions()->setUser("user");
	frame->AddEventConsumer(cursor_t, writer, board, gate->getEventAddress());

	// stimuli from urls

	gate = ssi_create(ZeroEventSender, 0, true);
	gate->getOptions()->setEvent("url");
	gate->getOptions()->setSender("click");
	gate->getOptions()->empty = false;
	frame->AddConsumer(button_t, gate, "0.1s");
	board->RegisterSender(*gate);

	stimuli = ssi_create(Stimuli, 0, true);
	stimuli->getOptions()->setSender("stimuliFromUrl");
	stimuli->getOptions()->setNames("http://hcm-lab.de;http://openssi.net");
	stimuli->getOptions()->setLabels("lab;ssi");	
	stimuli->getOptions()->setStartName("stimuli/start.html");
	stimuli->getOptions()->setEndName("stimuli/end.html");
	stimuli->getOptions()->applyLabelToEvent = true;
	stimuli->getOptions()->randomize = true;
	stimuli->getOptions()->insertBlank = true;
	board->RegisterListener(*stimuli, gate->getEventAddress());
	board->RegisterSender(*stimuli);

	browser = ssi_create_id(Browser, 0, "plot");
	browser->getOptions()->setTitle("Browser");
	board->RegisterListener(*browser, stimuli->getEventAddress());
	
	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 10000);

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

bool images(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_t = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ClockEventSender *clock = ssi_create(ClockEventSender, 0, true);
	ssi_size_t clocks[] = { 1000, 3000 };
	clock->getOptions()->setClocks(2, clocks);
	frame->AddRunnable(clock);
	board->RegisterSender(*clock);

	Stimuli *stimuli = ssi_create(Stimuli, 0, true);
	stimuli->getOptions()->setSender("imagesFromFolder");
	stimuli->getOptions()->setFolder("images");	
	stimuli->getOptions()->setAnnoPath("images");
	stimuli->getOptions()->labelFromFile = false;
	stimuli->getOptions()->insertBlank = true;
	board->RegisterListener(*stimuli, clock->getEventAddress());
	board->RegisterSender(*stimuli);

	ImageViewer *viewer = ssi_create_id(ImageViewer, 0, "plot");
	viewer->getOptions()->setTitle("Viewer");
	board->RegisterListener(*viewer, stimuli->getEventAddress());

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 10000);

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

bool tuple(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	// create a tuple event
	ssi_event_t e;
	EventAddress ea;
	ea.setAddress("event@tuple");
	ssi_event_init(e, SSI_ETYPE_NTUPLE, Factory::AddString(ea.getSender(0)), Factory::AddString(ea.getEvent(0)), 0, 0, 5 * sizeof(ssi_event_tuple_t));
	ssi_event_tuple_t *ptr = ssi_pcast(ssi_event_tuple_t, e.ptr);
	ssi_char_t string[10];
	for (ssi_size_t i = 0; i < 5; i++) {
		ssi_sprint(string, "%02u", i + 1);
		ptr[i].id = Factory::AddString(string);
		ptr[i].value = ssi_cast(ssi_real_t, ssi_random());
	}
	
	ImageViewer *viewer = ssi_create_id(ImageViewer, 0, "plot");
	viewer->getOptions()->setTitle("Viewer");
	viewer->getOptions()->setDirectory("images");
	viewer->getOptions()->setExtension("jpg");
	board->RegisterListener(*viewer, ea.getAddress());

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 10000);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	
	Sleep(1000);
	e.time = frame->GetElapsedTimeMs();
	board->update(e);

	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	ssi_event_destroy(e);

	return true;
}
