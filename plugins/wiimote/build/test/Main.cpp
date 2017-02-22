// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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
#include "ssiwiimote.h"
using namespace ssi;

// load libraries
#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_wiimote (void *args);
bool ex_wiiboard(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiwiimote.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiouput.dll");

	Exsemble ex;
	ex.add(ex_wiimote, 0, "WIIMOTE", "");
	ex.add(ex_wiiboard, 0, "WIIBOARD", "");
	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_wiimote (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
				
	WiiRemote *wii = ssi_create (WiiRemote, "wiimote", true);		
	ITransformable *acc_p = frame->AddProvider(wii, SSI_WII_ACC_PROVIDER_NAME);
	ITransformable *ori_p = frame->AddProvider(wii, SSI_WII_ORI_PROVIDER_NAME);
	ITransformable *but_p = frame->AddProvider(wii, SSI_WII_BUT_PROVIDER_NAME);
	ITransformable *ir_p = frame->AddProvider(wii, SSI_WII_IRFLT_PROVIDER_NAME);
	ITransformable *irraw_p = frame->AddProvider(wii, SSI_WII_IRRAW_PROVIDER_NAME);
	wii->SetLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(wii);

	// trigger
	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	frame->AddConsumer(but_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	SignalPainter *acc_plot = ssi_create_id (SignalPainter, 0, "plot");
	acc_plot->getOptions()->setTitle("acceleration");
	acc_plot->getOptions()->size = 2.0;
	frame->AddConsumer(acc_p, acc_plot, "0.1s"); 

	SignalPainter *acc_plot_tr = ssi_create_id (SignalPainter, 0, "plot");
	acc_plot_tr->getOptions()->setTitle("acceleration(tr)");
	frame->AddEventConsumer(acc_p, acc_plot_tr, board, ezero->getEventAddress());

	SignalPainter *ir_plot = ssi_create_id (SignalPainter, 0, "plot");
	ir_plot->getOptions()->setTitle("ir");
	frame->AddConsumer(ir_p, ir_plot, "0.1s");

	SignalPainter *irraw_plot = ssi_create_id (SignalPainter, 0, "plot");
	irraw_plot->getOptions()->setTitle("irraw");
	frame->AddConsumer(irraw_p, irraw_plot, "0.1s");

	SignalPainter *ori_plot = ssi_create_id (SignalPainter, 0, "plot");
	ori_plot->getOptions()->setTitle("orientation");
	ori_plot->getOptions()->size = 2.0;
	frame->AddConsumer(ori_p, ori_plot, "0.1s");

	SignalPainter *but_plot = ssi_create_id (SignalPainter, 0, "plot");
	but_plot->getOptions()->setTitle("buttons");
	but_plot->getOptions()->size = 2.0;
	frame->AddConsumer(but_p, but_plot, "0.1s");

	WiiRemoteButtonEventSender *wii_but_sender = ssi_create (WiiRemoteButtonEventSender, 0, true);
	frame->AddEventConsumer(but_p, wii_but_sender, board, ezero->getEventAddress());
	board->RegisterSender(*wii_but_sender);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, "button@wiimote", 5000);

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

bool ex_wiiboard (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	WiiBBoard *bboard = ssi_create (WiiBBoard, "wiibboard", true);		
	ITransformable *raw_p = frame->AddProvider(bboard, SSI_WIIBBOARD_RAW_PROVIDER_NAME);
	ITransformable *flt_p = frame->AddProvider(bboard, SSI_WIIBBOARD_FLT_PROVIDER_NAME);
	bboard->SetLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(bboard);

	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = 2.0;
	frame->AddConsumer(raw_p, plot, "0.1s"); 

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("flt");
	plot->getOptions()->size = 2.0;
	frame->AddConsumer(flt_p, plot, "0.1s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();	
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}
