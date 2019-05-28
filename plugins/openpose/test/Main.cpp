// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 14/6/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssiopenpose.h"

#include "skeleton\include\ssiskeleton.h"
#include "camera\include\ssicamera.h"
#include "image\include\ssiimage.h"
#include "ffmpeg\include\ssiffmpeg.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

bool ex_pose(void *arg);
bool ex_hand(void *arg);
bool ex_face(void *arg);
bool ex_full(void *arg);



int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
		
		Factory::RegisterDLL("camera");
		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("image");
		Factory::RegisterDLL("ffmpeg");
		Factory::RegisterDLL("ioput");
		Factory::RegisterDLL("openpose");
		Factory::RegisterDLL("ssiskeleton.dll");

		Exsemble ex;

		ex.add(ex_pose, 0, "Pose", "Demonstrates the use of openpose with only pose");
	    ex.add(ex_hand, 0, "Hand", "Demonstrates the use of openpose with pose and hand");
		ex.add(ex_face, 0, "Face", "Demonstrates the use of openpose with pose and face");
		ex.add(ex_full, 0, "Full", "Demonstrates the use of openpose with full detection (pose, hand, face)");




		ex.show();

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_pose(void *arg) {
	
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);
	
	//FOR VIDEOS
	/*CameraReader *camera = ssi_create(CameraReader,"videoReader", true );
	camera->getOptions()->setPath("../../karate.mp4");*/

	//FOR WEBCAM
	Camera *camera = ssi_create(Camera, "camera", true);

	camera->getOptions()->flip = true;
	//camera->getOptions()->params.framesPerSecond = 5;
	ssi_video_params_t video_params = camera->getFormat();

	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	Openpose *openpose = ssi_create(Openpose, 0, true);
	openpose->getOptions()->setModelFolder("../../models/");
	openpose->getOptions()->face = false;
	openpose->getOptions()->hand = false;
	openpose->getOptions()->numberOfMaxPeople = 10;
	//must be a multiple of 16, -1 means set automatically in regard of the video ratio aspect
	openpose->getOptions()->setNetResolution("-1x224");
	
	ITransformable *openpose_t = frame->AddTransformer(camera_p, openpose, "1");

	OpenposePainter *openpose_painter = ssi_create(OpenposePainter, 0, true);
	ITransformable* in[] = { openpose_t };
	ITransformable *openpose_painter_t = frame->AddTransformer(camera_p, 1, in, openpose_painter, "1");

	VideoPainter *vidplot = vidplot = ssi_create_id(VideoPainter, 0, "video");
	vidplot->getOptions()->setTitle("video");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(openpose_painter_t, vidplot, "1");

	OpenposeConverter* conv = ssi_create(OpenposeConverter, 0, true);
	//conv->getOptions()->n_skeletons = 1;
	ITransformable *conv_p = frame->AddTransformer(openpose_t, conv, "1");

	SkeletonPainter *skelpaint = ssi_create(SkeletonPainter, 0, true);
	ITransformable *skelpaint_t = frame->AddTransformer(conv_p, skelpaint, "1");

	VideoPainter * vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("skeleton");
	vplot->getOptions()->flip = false;
	frame->AddConsumer(skelpaint_t, vplot, "1");


	decorator->add("video*", 0, 0, 640, 480);
	decorator->add("console", 640, 0, 640, 480);
	decorator->add("plot*", 0, 480, 1280, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();
	
	return true;
}

bool ex_hand(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	//FOR VIDEOS
	/*CameraReader *camera = ssi_create(CameraReader,"videoReader", true );
	camera->getOptions()->setPath("../../karate.mp4");*/

	//FOR WEBCAM
	Camera *camera = ssi_create(Camera, "camera", true);

	camera->getOptions()->flip = true;
	//camera->getOptions()->params.framesPerSecond = 5;
	ssi_video_params_t video_params = camera->getFormat();

	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	Openpose *openpose = ssi_create(Openpose, 0, true);
	openpose->getOptions()->setModelFolder("../../models/");
	openpose->getOptions()->face = false;
	openpose->getOptions()->hand = true;
	openpose->getOptions()->numberOfMaxPeople = 2;
	//must be a multiple of 16, -1 means set automatically in regard of the video ratio aspect
	openpose->getOptions()->setNetResolution("-1x224");

	ITransformable *openpose_t = frame->AddTransformer(camera_p, openpose, "1");

	OpenposePainter *openpose_painter = ssi_create(OpenposePainter, 0, true);
	ITransformable* in[] = { openpose_t };
	ITransformable *openpose_painter_t = frame->AddTransformer(camera_p, 1, in, openpose_painter, "1");

	VideoPainter *vidplot = vidplot = ssi_create_id(VideoPainter, 0, "video");
	vidplot->getOptions()->setTitle("video");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(openpose_painter_t, vidplot, "1");

	decorator->add("video*", 0, 0, 640, 480);
	decorator->add("console", 640, 0, 640, 480);
	decorator->add("plot*", 0, 480, 1280, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_face(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	//FOR VIDEOS
	/*CameraReader *camera = ssi_create(CameraReader,"videoReader", true );
	camera->getOptions()->setPath("../../karate.mp4");*/

	//FOR WEBCAM
	Camera *camera = ssi_create(Camera, "camera", true);

	camera->getOptions()->flip = true;
	//camera->getOptions()->params.framesPerSecond = 5;
	ssi_video_params_t video_params = camera->getFormat();

	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	Openpose *openpose = ssi_create(Openpose, 0, true);
	openpose->getOptions()->setModelFolder("../../models/");
	openpose->getOptions()->face = true;
	openpose->getOptions()->hand = false;
	openpose->getOptions()->numberOfMaxPeople = 2;
	//must be a multiple of 16, -1 means set automatically in regard of the video ratio aspect
	openpose->getOptions()->setNetResolution("-1x224");

	ITransformable *openpose_t = frame->AddTransformer(camera_p, openpose, "1");

	OpenposePainter *openpose_painter = ssi_create(OpenposePainter, 0, true);
	ITransformable* in[] = { openpose_t };
	ITransformable *openpose_painter_t = frame->AddTransformer(camera_p, 1, in, openpose_painter, "1");

	VideoPainter *vidplot = vidplot = ssi_create_id(VideoPainter, 0, "video");
	vidplot->getOptions()->setTitle("video");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(openpose_painter_t, vidplot, "1");


	decorator->add("video*", 0, 0, 640, 480);
	decorator->add("console", 640, 0, 640, 480);
	decorator->add("plot*", 0, 480, 1280, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_full(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	//FOR VIDEOS
	/*CameraReader *camera = ssi_create(CameraReader,"videoReader", true );
	camera->getOptions()->setPath("../../karate.mp4");*/

	//FOR WEBCAM
	Camera *camera = ssi_create(Camera, "camera", true);

	camera->getOptions()->flip = true;
	//camera->getOptions()->params.framesPerSecond = 5;
	ssi_video_params_t video_params = camera->getFormat();

	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	Openpose *openpose = ssi_create(Openpose, 0, true);
	openpose->getOptions()->setModelFolder("../../models/");
	openpose->getOptions()->face = true;
	openpose->getOptions()->hand = true;
	openpose->getOptions()->numberOfMaxPeople = 2;
	//must be a multiple of 16, -1 means set automatically in regard of the video ratio aspect
	openpose->getOptions()->setNetResolution("-1x224");

	ITransformable *openpose_t = frame->AddTransformer(camera_p, openpose, "1");

	OpenposePainter *openpose_painter = ssi_create(OpenposePainter, 0, true);
	ITransformable* in[] = { openpose_t };
	ITransformable *openpose_painter_t = frame->AddTransformer(camera_p, 1, in, openpose_painter, "1");

	VideoPainter *vidplot = vidplot = ssi_create_id(VideoPainter, 0, "video");
	vidplot->getOptions()->setTitle("video");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(openpose_painter_t, vidplot, "1");


	decorator->add("video*", 0, 0, 640, 480);
	decorator->add("console", 640, 0, 640, 480);
	decorator->add("plot*", 0, 480, 1280, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}


