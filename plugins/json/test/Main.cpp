// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 24/10/2017
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
#include "ssijson.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

bool ex_xmltojson(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("frame");
	Factory::RegisterDLL("event");
	Factory::RegisterDLL("json");
	Factory::RegisterDLL("control");
	
	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(ex_xmltojson, 0, "XML TO JSON", "Convert xml string event to json");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_xmltojson(void *arg) {

	ITheEventBoard *board = Factory::GetEventBoard("board");	
	ITheFramework *frame = Factory::GetFramework("frame");

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_size_t n_xmlstr = 0;
	ssi_char_t *xmlstr = FileTools::ReadAsciiFile("test.xml", n_xmlstr);

	ClockEventSender *clock = ssi_create(ClockEventSender, 0, true);
	ssi_size_t clocks[] = { 1000 };
	clock->getOptions()->setAddress("string@xml");
	clock->getOptions()->setClocks(1, clocks);
	clock->getOptions()->empty = false;
	clock->getOptions()->setString(xmlstr);
	frame->AddRunnable(clock);
	board->RegisterSender(*clock);

	delete[] xmlstr;

	XmlToJson *xmltojson = ssi_create_id(XmlToJson, 0, "json");
	xmltojson->getOptions()->setAddress("string@json");
	xmltojson->getOptions()->numeric_support = false;
	board->RegisterSender(*xmltojson);
	board->RegisterListener(*xmltojson, clock->getEventAddress());

	EventMonitor *monitor = 0;

	monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->list = false;	
	monitor->getOptions()->lineReturn = true;
	board->RegisterListener(*monitor, clock->getEventAddress());

	monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->list = false;	
	monitor->getOptions()->lineReturn = true;
	board->RegisterListener(*monitor, xmltojson->getEventAddress());

	ControlCheckBox *checkbox = 0;
	
	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");	
	checkbox->getOptions()->setId("json");		
	checkbox->getOptions()->setName("prettify");
	frame->AddRunnable(checkbox);

	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");
	checkbox->getOptions()->setId("json");
	checkbox->getOptions()->setName("attributePrefix");
	frame->AddRunnable(checkbox);

	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");
	checkbox->getOptions()->setId("json");
	checkbox->getOptions()->setName("textPrefix");
	frame->AddRunnable(checkbox);

	checkbox = ssi_create_id(ControlCheckBox, 0, "checkbox");
	checkbox->getOptions()->setId("json");
	checkbox->getOptions()->setName("numericSupport");
	frame->AddRunnable(checkbox);

	decorator->add("monitor*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("checkbox*", 1, 0, CONSOLE_WIDTH+400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;

	return true;
}
