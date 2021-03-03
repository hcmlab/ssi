// TestMain.cpp (TEST)
// Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2020/09/08
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
#include "pythonbridge.h"
#include "pythonbridgeentry.h"
#include "pythonbridgeexit.h"
#include "SSI_Tools.h"

#include "camera\include\ssicamera.h"

using namespace ssi;

//#include <vld.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

ssi_char_t string[SSI_MAX_CHAR];

void ex_pythonbridge();

int main(int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("event");
		Factory::RegisterDLL("ssimouse");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("ioput");
		Factory::RegisterDLL("camera");
		Factory::RegisterDLL("pythonbridge");
		Factory::RegisterDLL("signal");

		ex_pythonbridge();		

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_pythonbridge() {

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework());

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Mouse* mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	// mouse->getOptions()->sr = 2;
	ITransformable* cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable* button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	Selector* select = ssi_create(Selector, 0, true);
	ssi_sprint(select->getOptions()->indices, "1");
	ITransformable* cursor_y = frame->AddTransformer(cursor_p, select, "1");

	/*ZeroEventSender* ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);*/

	/*Camera* camera = ssi_create(Camera, 0, true);
	camera->getOptions()->flip = true;
	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0);
	frame->AddSensor(camera);*/

	PythonBridgeEntry *pybridgeentry = ssi_create(PythonBridgeEntry, 0, true);
	pybridgeentry->getOptions()->setAddress("entry@pythonbridge");
	frame->AddConsumer(cursor_y, pybridgeentry, "0.1s");
	board->RegisterSender(*pybridgeentry);

	//frame->AddEventConsumer(cursor_p, pybridgeentry, board, ezero->getEventAddress());
	//frame->AddConsumer(camera_p, pybridgeentry, "1");

	PythonBridge* pybridge = ssi_create(PythonBridge, 0, true);
	pybridge->getOptions()->setAddress("bridge@pythonbridge");
	pybridge->getOptions()->port = 5550;
	board->RegisterListener(*pybridge, "entry@pythonbridge", 10000);

	PythonBridgeExit *pybridgeexit = ssi_create(PythonBridgeExit, 0, true);
	pybridgeexit->getOptions()->setAddress("exit@pythonbridge");
	pybridgeexit->getOptions()->port = 5551;
	pybridgeexit->getOptions()->size = 4;
	frame->AddRunnable(pybridgeexit);
	board->RegisterSender(*pybridgeexit);

	//frame->AddEventConsumer(cursor_p, pybridgeentry, board, ezero->getEventAddress());
	//frame->AddConsumer(camera_p, pybridgeentry, "1");

	/*VideoPainter* vidplot = 0;

	vidplot = ssi_create_id(VideoPainter, 0, "plot0");
	vidplot->getOptions()->setTitle("raw");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(camera_p, vidplot, "1");*/

	SignalPainter* plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot_1");
	plot->getOptions()->setTitle("mouse");
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	plot->getOptions()->autoscale = true;
	plot->getOptions()->size = 20;
	frame->AddConsumer(cursor_y, plot, "1", "0");

	/*plot = ssi_create_id(SignalPainter, 0, "plot_2");
	plot->getOptions()->setTitle("bridge");
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	plot->getOptions()->autoscale = true;
	plot->getOptions()->size = 20;
	frame->AddConsumer(pybridge_t, plot, "1", "0");*/

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, "exit@", 10000);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 640, 960);
	decorator->add("monitor*", 1000, 0, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();

}


