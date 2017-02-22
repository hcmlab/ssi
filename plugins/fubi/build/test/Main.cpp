// Main.cpp
// author: Felix Kistler <kistler@informatik.uni-augsburg.de>, Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2012/09/10
// Copyright (C) 2012 University of Augsburg, Tobias Baur
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
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssifubi.h"
#include "microsoftkinect/include/ssimicrosoftkinect.h"
#if defined _MSC_VER && _MSC_VER > 1700
#include "microsoftkinect2/include/ssimicrosoftkinect2.h"
#endif
#include "skeleton/include/ssiskeleton.h"
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

ssi_char_t sstring[SSI_MAX_CHAR];

bool ex_kinect1(void *args);
bool ex_kinect2(void *args);

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimicrosoftkinect.dll");
#if defined _MSC_VER && _MSC_VER > 1700
	Factory::RegisterDLL ("ssimicrosoftkinect2.dll");
#endif
	Factory::RegisterDLL ("ssifubi.dll");
	Factory::RegisterDLL("ssiskeleton.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL("ssiioput.dll");

	Exsemble ex;
	ex.add(&ex_kinect1, 0, "KINECT 1", "Use FUBI with Kinect 1");
#if defined _MSC_VER && _MSC_VER > 1700
	ex.add(&ex_kinect2, 0, "KINECT 2", "Use FUBI with Kinect 2");
#endif
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

#if defined _MSC_VER && _MSC_VER > 1700
bool ex_kinect2(void *args) {

	//general
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast (TheEventBoard, board)->getOptions()->update = 250;

	MicrosoftKinect2 *kinect = ssi_create(MicrosoftKinect2, 0, true);
	kinect->getOptions()->sr = 25.0;
	kinect->getOptions()->trackNearestPersons = 2;
	kinect->getOptions()->showFaceTracking = true;
	kinect->getOptions()->showBodyTracking = true;
	kinect->getOptions()->rotQuaternion = false;
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME);
	ITransformable *face_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME);
	ITransformable *head_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME);
	ITransformable **tf = new ITransformable*[2];
	tf[0] = head_p;
	tf[1] = face_p;
	frame->AddSensor(kinect);

	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	conv->getOptions()->n_skeletons = 2;
	conv->getOptions()->n_faces = 2;
	conv->getOptions()->skeleton_type = 0; // SSI
	ITransformable *conv_p = frame->AddTransformer(skel_p, 2, tf, conv, "1");

	FubiGestures *fubi = ssi_create(FubiGestures,"eventfubi", true);
	fubi->getOptions()->setRecognizerXml("../../include/SampleRecognizers.xml");
	frame->AddConsumer(conv_p, fubi, "1");
	board->RegisterSender(*fubi);
	
	FubiScene* fubiScene = ssi_create(FubiScene, "scenefubi", true);
	ITransformable *scene_p = frame->AddProvider(fubiScene, SSI_FUBI_SCENE_PROVIDER_NAME);
	frame->AddSensor(fubiScene);

	VideoPainter *scene_plot = ssi_create_id (VideoPainter, 0, "plot");
	scene_plot->getOptions()->setTitle("scene");
	scene_plot->getOptions()->flip = false;
	scene_plot->getOptions()->mirror = false;
	frame->AddConsumer(scene_p, scene_plot, "1");

	VideoPainter *vplot = 0;
	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("kinect");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;	
	frame->AddConsumer(rgb_p, vplot, "1"); 
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
    EventAddress ea;
	ea.setAddress(fubi->getEventAddress());
	board->RegisterListener(*monitor, ea.getAddress(), 20000);

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
#endif

bool ex_kinect1(void *args) {

	//general
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast (TheEventBoard, board)->getOptions()->update = 250;

	MicrosoftKinect *kinect = ssi_create(MicrosoftKinect, 0, true);
	kinect->getOptions()->trackNearestPerson = true;
	kinect->getOptions()->seatedSkeletonMode = false;
	kinect->getOptions()->simpleFaceTracking = false;
	kinect->getOptions()->nearTrackingMode = true;
	kinect->getOptions()->showFaceTracking = false;
	kinect->getOptions()->showBodyTracking = true;
	kinect->getOptions()->deviceIndex = 0;
	kinect->getOptions()->rotQuaternion = false;
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME,0,"10.0s",1,0.5);
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_SKELETON_PROVIDER_NAME,0,"10.0s",1,0.5);
	frame->AddSensor(kinect);

	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	conv->getOptions()->n_skeletons = 1; //max 2
	conv->getOptions()->skeleton_type = 0; // SSI
	ITransformable *conv_p = frame->AddTransformer(skel_p, conv, "1");	

	FubiGestures *fubi = ssi_create(FubiGestures,"eventfubi", true);
	fubi->getOptions()->setRecognizerXml("../../include/SampleRecognizers.xml");
	frame->AddConsumer(conv_p, fubi, "1");
	board->RegisterSender(*fubi);
	
	FubiScene* fubiScene = ssi_create(FubiScene, "scenefubi", true);
	ITransformable *scene_p = frame->AddProvider(fubiScene, SSI_FUBI_SCENE_PROVIDER_NAME);
	frame->AddSensor(fubiScene);

	VideoPainter *scene_plot = ssi_create_id (VideoPainter, 0, "plot");
	scene_plot->getOptions()->setTitle("scene");
	scene_plot->getOptions()->flip = false;
	scene_plot->getOptions()->mirror = false;
	frame->AddConsumer(scene_p, scene_plot, "1");

	VideoPainter *vplot = 0;
	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("kinect");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;	
	frame->AddConsumer(rgb_p, vplot, "1"); 
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, 0, 60000);

	FileEventWriter *ewriter = ssi_create(FileEventWriter, 0, true);
	ewriter->getOptions()->setPath("fubi");
	board->RegisterListener(*ewriter);

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
