// Main.cpp
// author: Dominik <dominik.schiller@student.uni-augsburg.de>
// created: 11/3/2015
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
#include "SSI_Tools.h"
#include "ssiemotext.h"
#include "TextFaker.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

void ex_text_input();

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
		Factory::RegisterDLL("ssiframe.dll");
		Factory::RegisterDLL("ssievent.dll");
		Factory::RegisterDLL("ssiioput.dll");
		Factory::RegisterDLL("ssiemotext.dll");
		ssi::Factory::Register(ssi::TextFaker::GetCreateName(), ssi::TextFaker::Create);

		Emotext *object = ssi_create(Emotext, 0, true);
		//object->print();

		ex_text_input();

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_text_input() {

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework("frame"));

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard("board");
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 200;

	TextFaker *tf = ssi_create(TextFaker, 0, true);
	tf->getOptions()->inputMaxLength = 100;
	tf->getOptions()->setSenderName("TextFaker");
	tf->getOptions()->setEventName("NewInput");
	board->RegisterSender(*tf);

	SocketEventReader *ser = ssi_create(SocketEventReader, 0, true);
	ser->getOptions()->port = 4000;
	ser->getOptions()->setSenderName("SocketAgent");
	ser->getOptions()->setEventName("AgentAction");
	board->RegisterListener(*ser);
	board->RegisterSender(*ser);

	Emotext *et = ssi_create(Emotext, 0, true);
	et->getOptions()->setSenderName("Emotext");
	et->getOptions()->setEventName("Proccessed Text");
	board->RegisterListener(*et, "@TextFaker,SocketAgent");
	board->RegisterSender(*et);

	EventMonitor* mo = ssi_create_id (EventMonitor, 0, "monitor");
	// mo->getOptions()->pos(1200, 750, 800, 250);
	board->RegisterListener(*mo, "@");
	
	// run framework

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	
	tf->start();
	ssi_print("\n\n\n Text eingeben: \n\n\n");
	while (!tf->exit)
		Sleep(500);	

	if (tf->exit) {
		tf->stop();

		frame->Stop();
		board->Stop();
		frame->Clear();
		board->Clear();
	}

	

	

}
