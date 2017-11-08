// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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
#include "ssicamera.h"
#include "..\..\..\audio\include\ssiaudio.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_cam (void *args);
bool ex_write(void *args);
bool ex_write_demand(void *args);
bool ex_read (void *args);
bool ex_screen (void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("frame");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("ioput");
	Factory::RegisterDLL ("graphic");
	Factory::RegisterDLL ("camera");
	Factory::RegisterDLL ("audio");
	Factory::RegisterDLL ("image");
	Factory::RegisterDLL ("control");

	Exsemble ex;
	ex.console(0, 0, 650, 600);
	ex.add(ex_cam, 0, "CAMERA", "Show camera");
	ex.add(ex_write, 0, "WRITE", "Write to a file");
	ex.add(ex_write_demand, 0, "WRITE ON DEMAND", "Write to a file");
	ex.add(ex_read, 0, "READ", "From from a file");
	ex.add(ex_screen, 0, "SCREEN", "Capture the screen");
	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_cam (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Camera *camera = ssi_create (Camera, "camera", true);	
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	VideoPainter *camera_plot = ssi_create_id (VideoPainter, 0, "plot");
	camera_plot->getOptions()->mirror = false;
	camera_plot->getOptions()->flip = true;
	camera_plot->getOptions()->setTitle("camera");
	frame->AddConsumer(camera_p, camera_plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_write (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Audio *audio = ssi_create(Audio, "audio", true);
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	Camera *camera = ssi_create (Camera, "camera", true);	
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	ITransformable *ids[] = { camera_p, audio_p };
	CameraWriter *cam_write = ssi_create(CameraWriter, "camerawrite", true);
	cam_write->getOptions()->setPath("video.avi");	
	frame->AddConsumer(2, ids, cam_write, "1");

	VideoPainter *camera_plot = ssi_create_id (VideoPainter, 0, "plot");
	camera_plot->getOptions()->mirror = false;
	camera_plot->getOptions()->flip = true;
	camera_plot->getOptions()->setTitle("camera");
	frame->AddConsumer(camera_p, camera_plot, "1");

	SignalPainter *audio_plot = ssi_create_id(SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 2.0;
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot, "0.1s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_write_demand(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Audio *audio = ssi_create(Audio, "audio", true);
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	Camera *camera = ssi_create(Camera, "camera", true);
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	ITransformable *ids[] = { camera_p, audio_p };
	CameraWriter *cam_write = ssi_create_id(CameraWriter, "camerawrite", "writer");
	cam_write->getOptions()->overwrite = false;
	cam_write->getOptions()->setPath("video.avi");;
	//frame->AddConsumer(2, ids, cam_write, "1");
	frame->AddConsumer(camera_p, cam_write, "1");

	VideoPainter *camera_plot = ssi_create_id(VideoPainter, 0, "plot");
	camera_plot->getOptions()->mirror = false;
	camera_plot->getOptions()->flip = true;
	camera_plot->getOptions()->setTitle("camera");
	frame->AddConsumer(camera_p, camera_plot, "1");

	SignalPainter *audio_plot = ssi_create_id(SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 2.0;
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot, "0.1s");

	ControlCheckBox *control = ssi_create_id(ControlCheckBox, 0, "control");
	control->getOptions()->setId("writer,plot*");
	control->getOptions()->setLabel("RECORD");
	frame->AddRunnable(control);

	decorator->add("console", 0, 0, 650, 600);
	decorator->add("control", 0, 600, 650, 200);
	decorator->add("plot*", 650, 0, 400, 800);	

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_read (void *args) {		

	const ssi_char_t *filename = "video.avi";	

	if (!ssi_exists (filename) && !ex_write(args))
	{
		return false;
	}

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	CameraReader *cam = ssi_create (CameraReader, 0, true);
	cam->getOptions()->setPath(filename);
	ITransformable *cam_p  = frame->AddProvider(cam, SSI_CAMERAREADER_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(cam);

	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("video");		
	//cam_plot->getOptions()->mirror = false;
	//cam_plot->getOptions()->flip = true;
	frame->AddConsumer(cam_p, cam_plot, "1");
	
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	cam->wait();
	frame->Stop();
	frame->Clear();

	return true;
}		

bool ex_screen (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Audio *audio = ssi_create (Audio, "audio", true);
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
	
	CameraScreen *screen = ssi_create (CameraScreen, "screen", true);
	screen->getOptions()->mouse = true;
	screen->getOptions()->mouse_size = 10;
	screen->getOptions()->setResize(640, 480);
	screen->getOptions()->fps = 10.0;
	screen->getOptions()->setRegion(100, 100, 800, 600);
	ITransformable *screen_p = frame->AddProvider(screen, SSI_CAMERASCREEN_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(screen);

	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("screen");
	ssi_video_params_t screen_format = screen->getFormat();
	frame->AddConsumer(screen_p, cam_plot, "1");

	ITransformable *ts[] = { screen_p, audio_p };
	CameraWriter *cam_write = ssi_create (CameraWriter, "camerawrite", true);
	cam_write->getOptions()->setPath("screen.avi");
	frame->AddConsumer(2, ts, cam_write, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}
