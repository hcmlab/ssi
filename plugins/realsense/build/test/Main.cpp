// Main
// author: Tobias Baur <baur@hcm-lab.de>
// created: 2013/2/28
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
#include "ssitheeyetribe.h"
#include "eyesig\include\ssieyesig.h"
#include "camera\include\ssicamera.h"
using namespace ssi;

//#define START_SERVER

bool ex_tracker(void *arg);
bool ex_screen (void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssitheeyetribe.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssicamera.dll");
	Factory::RegisterDLL ("ssiioput.dll");	
	Factory::RegisterDLL ("ssieyesig.dll");

#ifdef START_SERVER
	//These are hard coded. Anyways, when installing the drivers they will be installed in these paths (by default)
	system("start C:\\progra~2\\EyeTribe\\Server\\EyeTribe.exe"); //Start Server is necesarry
	system("start C:\\progra~2\\EyeTribe\\Client\\EyeTribeUIWin.exe"); //Start UI for Calibration
#endif
	
#ifdef START_SERVER
	system("taskkill /IM EyeTribeUIWin.exe");
	system("taskkill /IM EyeTribe.exe");
#endif

	Exsemble ex;
	ex.add(&ex_tracker, 0, "TRACKER", "How to use TheEyeTribe tracker.");
	ex.add(&ex_screen, 0, "SCREEN", "How to visualize gaze on screen.");
	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_tracker(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	eyetribe->getOptions()->keep = 30;
	ITransformable *stream_t[TheEyeTribe::CHANNELS::NUM];
	for (ssi_size_t i = 0; i < TheEyeTribe::CHANNELS::NUM; i++) {
		stream_t[i] = frame->AddProvider(eyetribe, eyetribe->getChannel(i)->getName());
	}
	frame->AddSensor(eyetribe);

	FileWriter *writer = 0;

	for (ssi_size_t i = 0; i < TheEyeTribe::CHANNELS::NUM; i++) {
		writer = ssi_create(FileWriter, 0, true);
		writer->getOptions()->setPath(eyetribe->getChannel(i)->getName());
		writer->getOptions()->type = File::ASCII;
		frame->AddConsumer(stream_t[i], writer, "1");
	}

	SignalPainter *plot = 0;
	
	for (ssi_size_t i = 0; i < TheEyeTribe::CHANNELS::NUM; i++) {
		plot = ssi_create_id (SignalPainter, 0, "plot");
		plot->getOptions()->setTitle(eyetribe->getChannel(i)->getName());
		plot->getOptions()->size = 10.0;
		frame->AddConsumer(stream_t[i], plot, "1");
	}

	CameraScreen *screen = ssi_create (CameraScreen, 0, true);
	screen->getOptions()->fps = 5.0;
	ITransformable *screen_p = frame->AddProvider(screen, SSI_CAMERASCREEN_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(screen);

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

bool ex_screen(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	TheEyeTribe *eyetribe = ssi_create(TheEyeTribe, "eyetribe", true);
	ITransformable *eyegaze_t = frame->AddProvider(eyetribe, TheEyeTribe::GazeAvgChannel::GetName());
	frame->AddSensor(eyetribe);

	SignalPainter *eye_plot = ssi_create_id (SignalPainter, 0, "plot");
	eye_plot->getOptions()->setTitle("eye");
	eye_plot->getOptions()->size = 2.0;
	frame->AddConsumer(eyegaze_t, eye_plot, "0.25s");

	CameraScreen *screen = ssi_create(CameraScreen, 0, true);
	screen->getOptions()->fps = 5.0;
	ITransformable *screen_p = frame->AddProvider(screen, SSI_CAMERASCREEN_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(screen);

	EyePainter *eye_paint = ssi_create(EyePainter, 0, true);
	ITransformable *eye_paint_t = frame->AddTransformer(screen_p, 1, &eyegaze_t, eye_paint, "1");

	VideoPainter *eye_video = ssi_create_id (VideoPainter, 0, "plot");
	eye_video->getOptions()->setTitle("eye");
	frame->AddConsumer(eye_paint_t, eye_video, "1");

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

