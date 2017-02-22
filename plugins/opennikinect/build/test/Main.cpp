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
#include "ssiopennikinect.h"
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

void ex_opennikinect ();
void ex_fubi ();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiopennikinect.dll");

	//ex_opennikinect ();
	ex_fubi ();	

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_opennikinect () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	OpenNIKinect *kinect = ssi_create(OpenNIKinect, "opennikinect", true);
	kinect->getOptions()->users = 1;	
	ITransformable *depth_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *scene_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME);
	frame->AddSensor(kinect);
	
	VideoPainter *depth_plot = ssi_create_id (VideoPainter, 0, "plot");
	depth_plot->getOptions()->setTitle("depth");
	depth_plot->getOptions()->flip = false;
	depth_plot->getOptions()->mirror = false;
	depth_plot->getOptions()->maxValue = SSI_OPENNI_KINECT_DEPTH_MAX_VALUE;
	frame->AddConsumer(depth_p, depth_plot, "1");

	VideoPainter *scene_plot = ssi_create_id (VideoPainter, 0, "plot");
	scene_plot->getOptions()->setTitle("scene");
	scene_plot->getOptions()->flip = false;
	scene_plot->getOptions()->mirror = false;
	frame->AddConsumer(scene_p, scene_plot, "1");

	VideoPainter *rgb_plot = ssi_create_id (VideoPainter, 0, "plot");
	rgb_plot->getOptions()->setTitle("rgb");
	rgb_plot->getOptions()->flip = false;
	rgb_plot->getOptions()->mirror = false;
	rgb_plot->getOptions()->maxValue = 255;
	frame->AddConsumer(rgb_p, rgb_plot, "1");

	OpenNIKinectSelector *selector = ssi_create (OpenNIKinectSelector, 0, true);	
	selector->select(OpenNIKinect::SkeletonJoint::RIGHT_HAND, OpenNIKinect::JOINT_VALUES::POS_X);	
	selector->select(OpenNIKinect::SkeletonJoint::RIGHT_HAND, OpenNIKinect::JOINT_VALUES::POS_Y);
	ITransformable *selector_t = frame->AddTransformer(skel_p, selector, "1");	

	SignalPainter *path_plot = ssi_create_id (SignalPainter, 0, "plot");
	path_plot->getOptions()->size = 3.0;
	path_plot->getOptions()->type = PaintSignalType::SIGNAL;
	path_plot->getOptions()->indx = 2;
	path_plot->getOptions()->indy = 3;
	frame->AddConsumer(selector_t, path_plot, "0.1s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();		
	frame->Clear();
}

void ex_fubi () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ("board");

	OpenNIKinect *kinect = ssi_create(OpenNIKinect, "opennikinect", true);
	kinect->getOptions()->users = 1;
	kinect->getOptions()->setRecognizerXml("../../../plugins/opennikinect/include/SampleRecognizers.xml");
	kinect->getOptions()->jointsToRender = 0x0000001 | 0x0000010 | 0x0000100 | 0x0000002 | 0x0000004 | 0x0000080| 0x0000800;
	ITransformable *depth_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *scene_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME);
	board->RegisterSender(*kinect);
	frame->AddSensor(kinect);

	VideoPainter *scene_plot = ssi_create_id (VideoPainter, 0, "plot");
	scene_plot->getOptions()->setTitle("scene");
	scene_plot->getOptions()->flip = false;
	scene_plot->getOptions()->mirror = false;
	frame->AddConsumer(scene_p, scene_plot, "1");

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
