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
#include "ssifacecrop.h"
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
void ex_facecrop_offline(int argc, char *argv[]);

int main(int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("event");
		Factory::RegisterDLL("camera");
		Factory::RegisterDLL("facecrop");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("ioput");
		Factory::RegisterDLL("ffmpeg");

		//ex_facecrop();
		ex_facecrop_offline(argc, argv);
		
		

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
	camera->getOptions()->params.framesPerSecond = 15;
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0);
	frame->AddSensor(camera);

	Facecrop *crop = ssi_create(Facecrop, 0, true);
	crop->getOptions()->setAddress("face@video");
	crop->getOptions()->setDependenciesPath("../dependencies/");
	crop->getOptions()->color_code = true;
	crop->getOptions()->resize_offset = 25;
	ITransformable *crop_t = frame->AddTransformer(camera_p, crop, "1");
	board->RegisterSender(*crop);

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id(VideoPainter, 0, "plot1");
	vidplot->getOptions()->setTitle("raw");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(camera_p, vidplot, "1");

	vidplot = ssi_create_id(VideoPainter, 0, "plot2");
	vidplot->getOptions()->setTitle("facecrop");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(crop_t, vidplot, "1");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, crop->getEventAddress(), 10000);

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

void ex_facecrop_offline(int argc, char *argv[]) {

	if (argc == 3)
	{

	FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, true);
	reader->getOptions()->setUrl(argv[1]);
	reader->getOptions()->bestEffort = true;


	FFMPEGWriter *writer = ssi_create(FFMPEGWriter, 0, true);
	writer->getOptions()->setUrl(argv[2]);
	

	Facecrop *crop = ssi_create(Facecrop, 0, true);
	crop->getOptions()->setAddress("face@video");
	crop->getOptions()->setDependenciesPath(".\\");
	crop->getOptions()->color_code = false;
	crop->getOptions()->resize_offset = 25;
	//ITransformable *crop_t = frame->AddTransformer(camera_p, crop, "1");
	//board->RegisterSender(*crop);


	FileProvider provider(writer, crop);
	reader->setProvider(SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME, &provider);


	reader->connect();
	reader->start();
	reader->wait();
	Sleep(1000);
	reader->stop();
	reader->disconnect();

	}

	else {
		printf("Add InputPath OutputPath to command line arguments");
	}



}


