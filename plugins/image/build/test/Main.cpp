// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/12/20
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
#include "ssiimage.h"
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

bool main_online (void *args);
bool main_offline(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiimage.dll");
	Factory::RegisterDLL ("ssicamera.dll");
	Factory::RegisterDLL ("ssimouse.dll");

	Exsemble ex;
	ex.console(0, 0, 650, 600);
	ex.add(main_online, 0, "CAMERA", "Online");
	ex.add(main_offline, 0, "CAMERA", "Offline");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool main_online (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// sensor
	
	Camera *camera = ssi_create (Camera, "camera", true);
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");
	camera->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(camera);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->scale = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// transform
	
	CVColor *cv_color = ssi_create (CVColor, 0, true);
	cv_color->getOptions()->code = CVColor::RGB2GRAY;
	ITransformable *cv_color_t = frame->AddTransformer(camera_p, cv_color, "1");

	CVChange *cv_change = ssi_create (CVChange, 0, true);
	ITransformable *cv_chain_t = frame->AddTransformer(cv_color_t, cv_change, "1");
	
	CVCrop *cv_crop = 0;

	cv_crop = ssi_create(CVCrop, 0, true);
	cv_crop->getOptions()->region[2] = 100;
	cv_crop->getOptions()->region[3] = 100;
	cv_crop->getOptions()->scaled = false;
	cv_crop->getOptions()->origin = CVCrop::ORIGIN::LEFTTOP;
	ITransformable *cv_crop_t = frame->AddTransformer(camera_p, cv_crop, "1");
	
	cv_crop  = ssi_create(CVCrop, 0, true);
	cv_crop->getOptions()->region[2] = 0.5f;
	cv_crop->getOptions()->region[3] = 0.5f;
	cv_crop->getOptions()->scaled = true;
	cv_crop->getOptions()->origin = CVCrop::ORIGIN::CENTER;
	ITransformable *channels[1] = { cursor_p };
	ITransformable *cv_crop2_t = frame->AddTransformer(camera_p, 1, channels, cv_crop, "1");

	CVResize *cv_resize = ssi_create (CVResize, 0, true);
	cv_resize->getOptions()->width = 0.1f;
	cv_resize->getOptions()->height = 0.1f;
	cv_resize->getOptions()->scaled = true;
	ITransformable *cv_resize_t = frame->AddTransformer(camera_p, cv_resize, "1");

	// save

	CVSave *cv_save = ssi_create (CVSave, 0, true);
	cv_save->getOptions()->set("image", CVSave::BMP);
	cv_save->getOptions()->number = false;
	frame->AddConsumer(camera_p, cv_save, "1");

	// plot

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("video");
	frame->AddConsumer(cv_color_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("change");
	frame->AddConsumer(cv_chain_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("crop (fixed)");
	frame->AddConsumer(cv_crop_t, vidplot, "1");

	vidplot = ssi_create_id(VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("crop (cursor)");
	frame->AddConsumer(cv_crop2_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("resize");
	vidplot->getOptions()->scale = false;	
	frame->AddConsumer(cv_resize_t, vidplot, "1");

	// run

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

void record () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Camera *camera = ssi_create (Camera, "camera", true);	
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME);
	camera->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(camera);	

	CameraWriter *writer = ssi_create (CameraWriter, 0, true);
	writer->getOptions()->setPath("video.avi");
	writer->getOptions()->setCompression("Microsoft Video 1");
	frame->AddConsumer(camera_p, writer, "1");

	VideoPainter *camera_plot = ssi_create_id (VideoPainter, 0, "plot");
	camera_plot->getOptions()->setTitle("camera");
	frame->AddConsumer(camera_p, camera_plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	ssi_print ("\nrecording video...\n");
	frame->Wait();
	frame->Stop();
	frame->Clear();
}

bool main_offline (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	const ssi_char_t *filename = "video.avi";
	
	if (!ssi_exists (filename)) {
		record ();
	}

	CameraReader *reader = ssi_create (CameraReader, 0, true);	
	reader->getOptions()->setPath(filename);
	reader->getOptions()->best_effort_delivery = true;
	frame->AddSensor(reader);

	CVColor *cv_color = ssi_create (CVColor, 0, true);
	cv_color->getOptions()->code = CVColor::RGB2GRAY;
	CVChange *cv_change = ssi_create (CVChange, 0, true);
	
	CVResize *cv_resize = ssi_create (CVResize, 0, true);
	cv_resize->getOptions()->width = 150;
	cv_resize->getOptions()->height = 150;

	// IFilter *filter[] = { cv_color, cv_change }; CRASHES, PROBABLY WRITING GRAY VIDEOS NOT SUPPORTED...
	IFilter *filter[] = { cv_resize };
	Chain *chain = ssi_create (Chain, 0, true);
	chain->set(1, filter, 0, 0);

	CameraWriter *writer = ssi_create (CameraWriter, 0, true);	
	writer->getOptions()->setPath("convert.avi");
	writer->getOptions()->setCompression("Microsoft Video 1");
	FileProvider video_p (writer, chain);
	reader->setProvider(SSI_CAMERAREADER_PROVIDER_NAME, &video_p);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	reader->wait();
	frame->Stop();
	frame->Clear();

	return true;
}

