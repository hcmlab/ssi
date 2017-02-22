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
#include "ssiskeleton.h"

#define SSI_MICROSOFTKINECT_ENABLED
#define SSI_MICROSOFTKINECT2_ENABLED

#ifdef SSI_MICROSOFTKINECT_ENABLED
#include "microsoftkinect\include\ssimicrosoftkinect.h"
#endif

#ifdef SSI_MICROSOFTKINECT2_ENABLED
#include "microsoftkinect2\include\ssimicrosoftkinect2.h"
#endif

#ifdef SSI_SKEL_OPENNI_ENABLED
#include "opennikinect\include\ssiopennikinect.h"
#endif
#ifdef SSI_SKEL_XSENS_ENABLED
#include "xsens\include\ssixsens.h"
#include "xsens\include\ssixsensmvnsp.h"
#endif
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

bool ex_microsoftkinect(void *arg);
bool ex_microsoftkinect2(void *arg);
bool ex_opennikinect(void *arg);
bool ex_xsens(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssiskeleton.dll");

#ifdef SSI_MICROSOFTKINECT_ENABLED
	Factory::RegisterDLL ("ssimicrosoftkinect.dll");
#endif

#ifdef SSI_MICROSOFTKINECT2_ENABLED
	Factory::RegisterDLL ("ssimicrosoftkinect2.dll");
#endif

	Exsemble ex;
	ex.add(ex_microsoftkinect, 0, "KINECT 1", "");
	ex.add(ex_microsoftkinect2, 0, "KINECT 2", "");
	ex.add(ex_opennikinect, 0, "OPENNI KINECT", "");
	ex.add(ex_xsens, 0, "XSENSE", "");
	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_opennikinect(void *arg) {

#ifdef SSI_SKEL_OPENNI_ENABLED

	Factory::RegisterDLL ("ssiopennikinect.dll");

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	OpenNIKinect *kinect = ssi_create(OpenNIKinect, "opennikinect", true);
	kinect->getOptions()->setRecognizerXml("../../../plugins/opennikinect/include/SampleRecognizers.xml");
	kinect->getOptions()->users = 1;
	ITransformable *depth_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *scene_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME);
	board->RegisterSender(*kinect);
	frame->AddSensor(kinect);
	
	VideoPainter *vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("video");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	conv->getOptions()->n_skeletons = 1;
	ITransformable *conv_p = frame->AddTransformer(skel_p, conv, "1");	

	SkeletonPainter *skelpaint = ssi_create (SkeletonPainter, 0, true);
	ITransformable *skelpaint_t = frame->AddTransformer(conv_p, skelpaint, "1");

	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("skeleton");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(skelpaint_t, vplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();		
	frame->Clear();

#endif

	return true;

}

bool ex_microsoftkinect(void *arg) {

#ifdef SSI_MICROSOFTKINECT_ENABLED

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	MicrosoftKinect *kinect = ssi_create(MicrosoftKinect, 0, true);
	kinect->getOptions()->trackNearestPerson = false;
	ITransformable *rgb_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *skel_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_SKELETON_PROVIDER_NAME);	
	ITransformable *face_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_FACEPOINT3D_PROVIDER_NAME);	
	ITransformable *head_p = frame->AddProvider(kinect, SSI_MICROSOFTKINECT_HEADPOSE_PROVIDER_NAME);	
	frame->AddSensor(kinect);

	VideoPainter *vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("video");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	ITransformable **tf = new ITransformable*[2];
	tf[0] = head_p;
	tf[1] = face_p;
	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	ITransformable *conv_p = frame->AddTransformer(skel_p,2,tf,conv,"1");

	FileWriter *writer = 0;

	writer = ssi_create_id(FileWriter, 0, "writer");
	writer->getOptions()->setPath("skel-kinect1");
	frame->AddConsumer(skel_p, writer, "1");

	writer = ssi_create_id(FileWriter, 0, "writer");
	writer->getOptions()->setPath("skel-ssi");
	frame->AddConsumer(conv_p, writer, "1");

	SkeletonSelector *selector = ssi_create (SkeletonSelector, 0, true);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_X);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_Y);
	ITransformable *selector_t = frame->AddTransformer(conv_p, selector, "1");	

	SignalPainter *path_plot = ssi_create_id (SignalPainter, 0, "plot");
	path_plot->getOptions()->size = 3.0;
	path_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(selector_t, path_plot, "1");	

	SkeletonPainter *skelpaint = ssi_create (SkeletonPainter, 0, true);
	ITransformable *skelpaint_t = frame->AddTransformer(conv_p, skelpaint, "1");

	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("skeleton");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(skelpaint_t, vplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();		
	frame->Clear();

	return true;
	
#endif
}

bool ex_microsoftkinect2(void *arg) {

#ifdef SSI_MICROSOFTKINECT2_ENABLED

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	MicrosoftKinect2 *kinect2 = ssi_create(MicrosoftKinect2, 0, true);
	kinect2->getOptions()->trackNearestPersons = 6;
	kinect2->getOptions()->sr = 15;
	ITransformable *rgb_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME, 0, "2.0s");
	ITransformable *skel_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME);	
	ITransformable *face_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME);	
	ITransformable *head_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME);	
	frame->AddSensor(kinect2);

	VideoPainter *vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("video");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	ITransformable **tf = new ITransformable*[2];
	tf[0] = head_p;
	tf[1] = face_p;
	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	conv->getOptions()->skeleton_type = SSI_SKELETON_TYPE::MICROSOFT_KINECT2;
	conv->getOptions()->n_skeletons = 3;

	ITransformable *conv_p = frame->AddTransformer(skel_p,2,tf,conv,"1");
	//ITransformable *conv_p = frame->AddTransformer(skel_p, conv, "1");

	SkeletonSelector *selector = ssi_create (SkeletonSelector, 0, true);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_X);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_Y);
	ITransformable *selector_t = frame->AddTransformer(conv_p, selector, "1");	

	FileWriter *writer = 0;

	writer = ssi_create_id(FileWriter, 0, "writer");
	writer->getOptions()->setPath("skel-kinect2");
	frame->AddConsumer(skel_p, writer, "1");

	writer = ssi_create_id(FileWriter, 0, "writer");
	writer->getOptions()->setPath("skel-ssi");
	frame->AddConsumer(conv_p, writer, "1");


	SignalPainter *path_plot = ssi_create_id (SignalPainter, 0, "plot");
	path_plot->getOptions()->size = 3.0;
	path_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(selector_t, path_plot, "1");	

	SkeletonPainter *skelpaint = ssi_create (SkeletonPainter, 0, true);
	ITransformable *skelpaint_t = frame->AddTransformer(conv_p, skelpaint, "1");

	vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("skeleton");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(skelpaint_t, vplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;

#endif
}

bool ex_xsens(void *arg) {

#ifdef SSI_SKEL_XSENS_ENABLED

	Factory::RegisterDLL ("ssixsensmvnsp.dll");

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	XsensMVNSP* xsens = ssi_create(XsensMVNSP, 0, true);
	ITransformable *skel_p = frame->AddProvider(xsens, SSI_XSENSMVNSP_SKELETON_PROVIDER_NAME);
	frame->AddSensor(xsens);

	SkeletonConverter* conv = ssi_create (SkeletonConverter, 0, true);
	ITransformable *conv_p = frame->AddTransformer(skel_p, conv, "1");	

	SkeletonSelector *selector = ssi_create (SkeletonSelector, 0, true);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_X);	
	selector->select(SSI_SKELETON_JOINT::RIGHT_HAND, SSI_SKELETON_JOINT_VALUE::POS_Y);
	ITransformable *selector_t = frame->AddTransformer(conv_p, selector, "1");	

	SignalPainter *path_plot = ssi_create_id (SignalPainter, 0, "plot");
	path_plot->getOptions()->size = 3.0;
	path_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(selector_t, path_plot, "1");	

	SkeletonPainter *skelpaint = ssi_create (SkeletonPainter, 0, true);
	ITransformable *skelpaint_t = frame->AddTransformer(conv_p, skelpaint, "1");

	VideoPainter *vplot = ssi_create_id (VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("skeleton");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(skelpaint_t, vplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();		
	frame->Clear();

#endif

	return true;
}
