// Main
// author: Daniel Schork 
// created: 2015
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
#include "ssimicrosoftkinect2.h"
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

bool ex_microsoftkinect2 (void *args);
bool ex_skeleton(void *args);
bool ex_audio(void *args);
bool ex_smiledetector(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssimicrosoftkinect2");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");

	Exsemble ex;
	ex.add(ex_microsoftkinect2, 0, "KINECT 2", "All channels");
	ex.add(ex_skeleton, 0, "SKELETON", "Skeleton only");
	ex.add(ex_audio, 0, "AUDIO", "Audio only");
	ex.add(ex_smiledetector, 0, "KINECT", "Smile detection");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_microsoftkinect2 (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	MicrosoftKinect2 *kinect2 = ssi_create(MicrosoftKinect2, 0, true);
	kinect2->getOptions()->sr = 15.0;
	kinect2->getOptions()->trackNearestPersons = 6;
	//kinect2->getOptions()->screenWidth = 1920;
	//kinect2->getOptions()->screenHeight = 1080;
	//kinect2->getOptions()->showBodyTracking = false;
	//kinect2->getOptions()->showFaceTracking = false;

	ITransformable *face_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_FACEPOINT_PROVIDER_NAME);
	ITransformable *face3d_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME);
	ITransformable *skeleton_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME);
	ITransformable *skeleton2screen_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETON2SCREEN_PROVIDER_NAME);
	ITransformable *head_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME);
	ITransformable *rgb_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME,0,"1.0s");
	ITransformable *depth_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_DEPTHIMAGE_PROVIDER_NAME,0,"1.0s");
	ITransformable *depthr_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_DEPTHRAW_PROVIDER_NAME,0,"1.0s");
	ITransformable *ir_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_INFRAREDIMAGE_PROVIDER_NAME,0,"1.0s");
	ITransformable *irr_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_INFRAREDRAW_PROVIDER_NAME,0,"1.0s");
	ITransformable *conf_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_PROVIDER_NAME,0,"1.0s");
	ITransformable *au_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_ACTIONUNIT_PROVIDER_NAME);
	ITransformable *hand_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_HANDPOSE_PROVIDER_NAME);
	frame->AddSensor(kinect2);

	VideoPainter *vplot = 0;
		
	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("rgb");
	vplot->getOptions()->flip = false;
    vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");
	
	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("depth");
	vplot->getOptions()->flip = false;
    vplot->getOptions()->mirror = false;
	frame->AddConsumer(depth_p, vplot, "1");

	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("infrared");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(ir_p, vplot, "1");

	SignalPainter *splot;

	splot = ssi_create_id (SignalPainter, 0, "plot");
	splot->getOptions()->size = 5.0;
	splot->getOptions()->setTitle("hands");
	frame->AddConsumer(hand_p, splot, "1");

	splot = ssi_create_id (SignalPainter, 0, "plot");
	splot->getOptions()->size = 5.0;
	splot->getOptions()->setTitle("au");
	frame->AddConsumer(au_p, splot, "1");

	/*FileWriter *writer = 0;
	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("face");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(face_p, writer, "0.2s");*/

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

bool ex_skeleton(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	MicrosoftKinect2 *kinect2 = ssi_create(MicrosoftKinect2, 0, true);
	kinect2->getOptions()->sr = 15.0;
	kinect2->getOptions()->trackNearestPersons = 6;

	ITransformable *skeleton_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME);
	frame->AddSensor(kinect2);

	SignalPainter *splot;

	Selector *select = ssi_create(Selector, 0, true);
	select->getOptions()->set(2);

	splot = ssi_create_id(SignalPainter, 0, "plot");
	splot->getOptions()->size = 5.0;
	splot->getOptions()->setTitle("skeleton");
	frame->AddConsumer(skeleton_p, splot, "1", 0, select);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_audio(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	MicrosoftKinect2 *kinect2 = ssi_create(MicrosoftKinect2, 0, true);
	ITransformable *audio_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_AUDIO_PROVIDER_NAME);
	frame->AddSensor(kinect2);

	SignalPainter *splot;

	splot = ssi_create_id(SignalPainter, 0, "plot");
	splot->getOptions()->size = 5.0;
	splot->getOptions()->type = PaintSignalType::AUDIO;
	splot->getOptions()->setTitle("audio");
	frame->AddConsumer(audio_p, splot, "0.1s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 800);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_smiledetector (void *args) {
	
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	MicrosoftKinect2 *kinect2 = ssi_create(MicrosoftKinect2, 0, true);
	kinect2->getOptions()->trackNearestPersons = 1;
	kinect2->getOptions()->sr = 25.0;
	kinect2->getOptions()->showBodyTracking = false;
	kinect2->getOptions()->showFaceTracking = true;
	ITransformable *face_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_FACEPOINT_PROVIDER_NAME);
	ITransformable *au_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_ACTIONUNIT_PROVIDER_NAME);
	ITransformable *skeleton_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME);
	ITransformable *rgb_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	
	frame->AddSensor(kinect2);

	MicrosoftKinect2SmileDetector *kinect2smiledetector = ssi_create (MicrosoftKinect2SmileDetector, 0, true);
	ITransformable *kinect2smiledetector_t = frame->AddTransformer(au_p, kinect2smiledetector, "5");

	ThresEventSender *thresevent = ssi_create (ThresEventSender, 0, true);
	thresevent->getOptions()->thres = 0.0f;
	thresevent->getOptions()->hangin = 3;
	thresevent->getOptions()->hangout = 3;
	thresevent->getOptions()->mindur = 1.0;
	thresevent->getOptions()->setEvent("smile");
	thresevent->getOptions()->setSender("kinect");
	frame->AddConsumer(kinect2smiledetector_t, thresevent, "5");
	board->RegisterSender(*thresevent);

	SignalPainter *splot;
	
	splot = ssi_create_id (SignalPainter, 0, "plot");
	splot->getOptions()->size = 5.0;
	splot->getOptions()->setTitle("smile");
	frame->AddConsumer(kinect2smiledetector_t, splot, "1");
	
	VideoPainter *vplot = 0;
		
	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("video");
	vplot->getOptions()->flip = false;
    vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 20000);

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
