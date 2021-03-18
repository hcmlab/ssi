// TestMain.cpp (TEST)
// Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2018/12/06
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
#include "ssipupiltracker.h"
#include "SSI_Tools.h"
#include "..\libs\build\ssiml\include\ssiml.h"
#include "camera\include\ssicamera.h"
#include "ffmpeg/include/ssiffmpeg.h"

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

void ex_facecrop();
void ex_facecrop_offline();

int main(int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("event");
		Factory::RegisterDLL("camera");
		Factory::RegisterDLL("pupiltracker");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("ioput");
		Factory::RegisterDLL("ffmpeg");

		ex_facecrop();
		//ex_facecrop_offline();
		
		

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_facecrop() {

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework());

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Camera *camera = ssi_create(Camera, "camera", true);
	camera->getOptions()->flip = true;
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0);
	frame->AddSensor(camera);

	PupilTracker *pupil = ssi_create(PupilTracker, 0, true);
	pupil->getOptions()->setAddress("face@video");
	ITransformable *pupil_t = frame->AddTransformer(camera_p, pupil, "1");
	board->RegisterSender(*pupil);

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id(VideoPainter, 0, "plot1");
	vidplot->getOptions()->setTitle("raw");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(camera_p, vidplot, "1");

	SignalPainter* paint = ssi_create_id(SignalPainter, 0, "plot");
	paint->getOptions()->type = PaintSignalType::SIGNAL;
	paint->getOptions()->size = 10;
	paint->getOptions()->setTitle("Title");
	frame->AddConsumer(pupil_t, paint, "0.1s");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, pupil->getEventAddress(), 10000);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 640, 960);
	decorator->add("monitor*", 0, 0, 400, 400);

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


void ex_facecrop_offline()
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, true);
	reader->getOptions()->setUrl("C:\\Users\\wildgrfa\\Desktop\\pupilTrackingVideos\\test_10fps.mp4");
	reader->getOptions()->bestEffort = false;
	//reader->getOptions()->buffer = 1.0;
	//reader->getOptions()->fps = 10.0;
	ITransformable *reader_p = frame->AddProvider(reader, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(reader);	

	PupilTracker *pupil = ssi_create(PupilTracker, 0, true);
	pupil->getOptions()->setAddress("face@video");
	ITransformable *pupil_t = frame->AddTransformer(reader_p, pupil, "1");

	SignalPainter *paint = ssi_create_id(SignalPainter, 0, "plot");		
	paint->getOptions()->type = PaintSignalType::SIGNAL;
	paint->getOptions()->size = 10;
	paint->getOptions()->setTitle("Title");
	frame->AddConsumer(pupil_t, paint, "0.1s");
	/*
	VideoPainter *vidplot = 0;
	vidplot = ssi_create_id(VideoPainter, 0, "plot1");
	vidplot->getOptions()->setTitle("raw");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(reader_p, vidplot, "1");
	*/

	FileWriter *streamWriter = 0;
	streamWriter = ssi_create_id(FileWriter, 0, "writer1");
	streamWriter->getOptions()->setPath("C:\\Users\\wildgrfa\\Desktop\\pupilTrackingVideos\\test_10fps_output");
	frame->AddConsumer(pupil_t, streamWriter, "1");	

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot", 650, 0, 400, 800);
	//decorator->add("plot1", 1050, 0, 640, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	ssi_print("\n\n\tpress a key to quit\n");
	getchar();
}



