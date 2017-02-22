// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/12/20
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
#include "ssieyesig.h"
#include "theeyetribe/include/ssitheeyetribe.h"
#include "camera/include/ssicamera.h"

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

#define SIMULATE

void main_paint ();
void main_fixation ();
void main_area();
void main_blink();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssicamera.dll");
	Factory::RegisterDLL ("ssimouse.dll");
	Factory::RegisterDLL ("ssieyesig.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssitheeyetribe.dll");
	Factory::RegisterDLL ("ssiimage.dll");

//#ifndef SIMULATE
//	system("start C:\\progra~2\\EyeTribe\\Server\\EyeTribe.exe"); //Start Server
//	system("start C:\\progra~2\\EyeTribe\\Client\\EyeTribeUIWin.exe"); //Start UI for Calibration
//#endif
	
    //main_paint ();
	//main_fixation ();
	//main_area ();
	main_blink();
		
//#ifndef SIMULATE
//	system("taskkill /IM EyeTribeUIWin.exe");
//	system("taskkill /IM EyeTribe.exe");
//#endif

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void main_paint () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	// provider
#ifdef SIMULATE
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->scale = true;
	mouse->getOptions()->flip = false;
	ITransformable *eye_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
#else	
	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	ITransformable *eye_p = frame->AddProvider(eyetribe, "gazeraw");
	frame->AddSensor(eyetribe);
#endif
	
	CameraScreen *screen = ssi_create (CameraScreen, 0, true);
	screen->getOptions()->mouse = false;
	screen->getOptions()->fps = 15.0;
	screen->getOptions()->flip = false;
	screen->getOptions()->resize = true;
	screen->getOptions()->resize_width = 640;
	screen->getOptions()->resize_height = 480;

	ITransformable *screen_p = frame->AddProvider(screen, SSI_CAMERASCREEN_PROVIDER_NAME, 0, "5.0s");	
	frame->AddSensor(screen);

	// paint
	HeatMapPainter *eye_paint = ssi_create (HeatMapPainter, "eyepaint", true);
	eye_paint->getOptions()->decreaseovertime = true;
	eye_paint->getOptions()->verticalregions = 40;
	eye_paint->getOptions()->horizontalregions = 40;
	eye_paint->getOptions()->decreasefactor = 0.1;
	ITransformable *eye_paint_ids[] = {eye_p};
	ITransformable *eye_paint_t = frame->AddTransformer(screen_p, 1, eye_paint_ids, eye_paint, "1");

	// plot
	VideoPainter *eye_plot = ssi_create_id (VideoPainter, 0, "plot");
	eye_plot->getOptions()->setTitle("eye");	
	frame->AddConsumer(eye_paint_t, eye_plot, "1");

	// run framework

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
}

void main_area () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	char* CalibrationWindowName = "Scene"; //* Should be identical for CalibrationWindow Option and Name of Scene VideoPainter
	
	// provider
#ifdef SIMULATE
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->scale = false;
	mouse->getOptions()->flip = false;
	ITransformable *eye_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
#else	
	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	ITransformable *eye_p = frame->AddProvider(eyetribe, "gazeraw");
	frame->AddSensor(eyetribe);
#endif

	GazeArea *area = ssi_create (GazeArea, "area", true);
	area->getOptions()->norm = true;
	area->getOptions()->setArea(0.25,0.25,0.5,0.5);
	frame->AddConsumer(eye_p, area, "0.5s");
	board->RegisterSender(*area);

	EventMonitor* monitor = ssi_create ( EventMonitor, 0, true );
	board->RegisterListener(*monitor);

	// plot
	SignalPainter *eye_plot = ssi_create_id (SignalPainter, 0, "plot");
	eye_plot->getOptions()->setTitle("eye");
	eye_plot->getOptions()->size = 2.0;
	frame->AddConsumer(eye_p, eye_plot, "0.25s"); 

	// run framework

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
}

void main_fixation () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	char* CalibrationWindowName = "Scene"; //* Should be identical for CalibrationWindow Option and Name of Scene VideoPainter
	
	// provider
#ifdef SIMULATE
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->scale = false;
	mouse->getOptions()->flip = false;
	ITransformable *eye_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
#else	
	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	ITransformable *eye_p = frame->AddProvider(eyetribe, "gazeraw");
	frame->AddSensor(eyetribe);
#endif
	
	EyeFixation *fixation = ssi_create (EyeFixation, "fixation", true);
	fixation->getOptions()->thres = 90;
	fixation->getOptions()->mindur = 1.0;
	frame->AddConsumer(eye_p, fixation, "0.25s");
	board->RegisterSender(*fixation);

	EventMonitor* monitor = ssi_create ( EventMonitor, 0, true );
	board->RegisterListener(*monitor);

	// plot
	SignalPainter *eye_plot = ssi_create_id (SignalPainter, 0, "plot");
	eye_plot->getOptions()->setTitle("eye");
	eye_plot->getOptions()->staticImage = true;
	eye_plot->getOptions()->size = 2.0;
	frame->AddConsumer(eye_p, eye_plot, "0.25s"); 

	// run framework

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
}

void main_blink() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	char* CalibrationWindowName = "Scene"; //* Should be identical for CalibrationWindow Option and Name of Scene VideoPainter

	// provider
#ifdef SIMULATE
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->scale = false;
	mouse->getOptions()->flip = false;
	ITransformable *eye_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
#else	
	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	ITransformable *eye_p = frame->AddProvider(eyetribe, "gazeraw");
	frame->AddSensor(eyetribe);
#endif

	EyeBlink *blink = ssi_create(EyeBlink, 0, true);
	frame->AddConsumer(eye_p, blink, "1");
	board->RegisterSender(*blink);

	EventMonitor* monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	// plot
	SignalPainter *eye_plot = ssi_create_id (SignalPainter, 0, "plot");
	eye_plot->getOptions()->setTitle("eye");
	eye_plot->getOptions()->staticImage = true;
	eye_plot->getOptions()->size = 2.0;
	frame->AddConsumer(eye_p, eye_plot, "0.25s");

	// run framework

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
}

