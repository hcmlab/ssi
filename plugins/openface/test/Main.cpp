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
#include "ssiopenface.h"
#include "camera\include\ssicamera.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_test(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("openface");
	Factory::RegisterDLL("camera");
	Factory::RegisterDLL("frame");
	Factory::RegisterDLL("graphic");

	Exsemble ex;
	ex.add(ex_test, 0, "TEST", "Demonstrates the use of openface");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_test(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	// sensor

	Camera *camera = ssi_create(ssi::Camera, "cam", true);
	camera->getOptions()->flip = true;
	ssi_video_params_t video_params = camera->getFormat();
	ITransformable* camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	frame->AddSensor(camera);

	Openface *openface = ssi_create(Openface, 0, true);
	openface->getOptions()->setModelPath("..\\lib\\local\\LandmarkDetector\\model");
	openface->getOptions()->setTriPath("..\\lib\\local\\LandmarkDetector\\model\\tris_68_full.txt");
	openface->getOptions()->setAuPath("..\\lib\\local\\FaceAnalyser\\AU_predictors\\AU_all_best.txt");
	ITransformable *openface_t = frame->AddTransformer(camera_p, openface, "1");

	OpenfaceSelector *sel_au_r = ssi_create(OpenfaceSelector, 0, true);
	sel_au_r->getOptions()->aureg = true;
	ITransformable *sel_au_r_t = frame->AddTransformer(openface_t, sel_au_r, "1");
	
	OpenfaceSelector *sel_au_c = ssi_create(OpenfaceSelector, 0, true);
	sel_au_c->getOptions()->auclass = true;
	ITransformable *sel_au_c_t = frame->AddTransformer(openface_t, sel_au_c, "1");

	OpenfacePainter *openface_painter = ssi_create(OpenfacePainter, 0, true);
	ITransformable* in[] = { openface_t };
	ITransformable *openface_painter_t = frame->AddTransformer(camera_p,1,in,openface_painter,"1");

	VideoPainter *vidplot = vidplot = ssi_create_id(VideoPainter, 0, "video");
	vidplot->getOptions()->setTitle("video");
	vidplot->getOptions()->flip = false;
	frame->AddConsumer(openface_painter_t, vidplot, "1");

	SignalPainter *sig_au_r = ssi_create_id(SignalPainter, 0, "plot");
	sig_au_r->getOptions()->type = PaintSignalType::BAR_POS;
	sig_au_r->getOptions()->setBarNames("Inner|Brow|Raiser,Outer|Brow|Raiser,Brow|Lowerer,Upper|Lid|Raiser,Cheek|Raiser,Nose|Wrinkler,Upper|Lip|Raiser,Lip|Corner|Puller,Dimpler,Lip|Corner|Depressor,Chin|Raiser,Lip|Stretcher,Lips|Part,Jaw|Drop");
	sig_au_r->getOptions()->setTitle("AU Regression");
	frame->AddConsumer(sel_au_r_t, sig_au_r, "1");

	SignalPainter *sig_au_c = ssi_create_id(SignalPainter, 0, "plot");
	sig_au_c->getOptions()->type = PaintSignalType::BAR_POS;
	sig_au_c->getOptions()->setTitle("AU Classification");
	sig_au_c->getOptions()->setBarNames("Brow|Lowerer,Lip|Corner|Puller,Lip|Corner|Depressor,Lip|Tightener,Lip|Suck,Blink");
	frame->AddConsumer(sel_au_c_t, sig_au_c, "1");

	decorator->add("video*", 0, 0, 640, 480);
	decorator->add("console", 640, 0, 640, 480);
	decorator->add("plot*", 0, 480, 1280, 480);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();


	return true;
}
