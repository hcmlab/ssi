// Main.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
// Copyright (C) University of Augsburg

#include "ssi.h"
#include "ssiartkplus.h"
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

bool ex_artk (void *args);
bool ex_mouse(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssicamera");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiartkplus");
	Factory::RegisterDLL ("ssimouse");

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Exsemble ex;
	ex.add(ex_artk, 0, "ARTK", "");
	ex.add(ex_mouse, 0, "ARTK", "Steer mouse");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_artk (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Camera *camera = ssi_create (Camera, "camera", true);
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(camera);	

	ARTKPlusTracker *tracker = ssi_create (ARTKPlusTracker, 0, true);
	tracker->getOptions()->setConfigPaths("config\\LogitechPro4000.dat", "config\\markerboard_1-20.cfg");
	ITransformable *tracker_t = frame->AddTransformer(camera_p, tracker, "1");

	ITransformable *source[] = { tracker_t };
	
	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("camera");	
	frame->AddConsumer(camera_p, cam_plot, "1");

	ARTKPlusWriter *writer = ssi_create (ARTKPlusWriter, 0, true);
	frame->AddConsumer(tracker_t, writer, "1");

	ARTKPlusSelector *selector = ssi_create (ARTKPlusSelector, 0, true);
	selector->id();
	selector->center_X();
	selector->center_Y();
	ITransformable *selector_t = frame->AddTransformer(tracker_t, selector, "1");

	FileWriter *selector_writer = ssi_create (FileWriter, 0, true);
	selector_writer->getOptions()->type = File::ASCII;
	selector_writer->getOptions()->setFormat(" ", ".1");
	frame->AddConsumer(selector_t, selector_writer, "1.0s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_mouse (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Camera *camera = ssi_create (Camera, "camera", true);
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");
	frame->AddSensor(camera);	

	ARTKPlusTracker *tracker = ssi_create (ARTKPlusTracker, 0, true);
	tracker->getOptions()->n_marker = 1;
	tracker->getOptions()->scale = true;
	tracker->getOptions()->setConfigPaths("config\\LogitechPro4000.dat", "config\\markerboard_1-20.cfg");
	ITransformable *tracker_t = frame->AddTransformer(camera_p, tracker, "1");

	ITransformable *source[] = { tracker_t };
	
	VideoPainter *cam_plot = ssi_create_id (VideoPainter, 0, "plot");
	cam_plot->getOptions()->setTitle("camera");	
	frame->AddConsumer(camera_p, cam_plot, "1");

	ARTKPlusSelector *selector = ssi_create (ARTKPlusSelector, 0, true);
	selector->center_X();
	selector->center_Y();
	ITransformable *selector_t = frame->AddTransformer(tracker_t, selector, "1");

	CursorMover *mouse = ssi_create (CursorMover, 0, true);
	mouse->getOptions()->scale = true;
	frame->AddConsumer(selector_t, mouse, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}
