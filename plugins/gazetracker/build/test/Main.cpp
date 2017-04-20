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
#include "ssigazetracker.h"

#include "camera/include/ssicamera.h"
#include "ffmpeg\include\ssiffmpeg.h"
#include "signal\include\ssisignal.h"

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

void gazetracker ();
void justRecordVideo();


int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssigazetracker.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssiffmpeg.dll");
	Factory::RegisterDLL ("ssicamera.dll");
	Factory::RegisterDLL ("ssisignal.dll");

	gazetracker ();
	//justRecordVideo();


	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}


void gazetracker () {

	bool eyeFromFile = false;
	bool sceneFromFile = false;
	bool shiftableGazePoint = false;

	bool record = true;

	char* CalibrationWindowName = "Scene"; //* Should be identical for CalibrationWindow Option and Name of Scene VideoPainter

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);


	std::string baseFolder = "D:\\Recordings\\"; //your path to the recorded scenecamera and eyecamera files

	// eye camera

	ITransformable *eyeCamera_p;
	if (eyeFromFile){
		std::string eyeFilePath = baseFolder + "eye_video.mp4";
		if (!ssi_exists(eyeFilePath.c_str())){
			ssi_err("eye file not found: %s\n", eyeFilePath.c_str());
			return;
		}
				
		FFMPEGReader *vreadeye = ssi_create(FFMPEGReader, "eye", true);
		vreadeye->getOptions()->setUrl(eyeFilePath.c_str());
		vreadeye->getOptions()->stream = false;
		vreadeye->getOptions()->buffer = 1.0;
		vreadeye->getOptions()->fps = 30.0;
		vreadeye->getOptions()->width = 640;
		vreadeye->getOptions()->height = 480;
		eyeCamera_p = frame->AddProvider(vreadeye, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME);
		frame->AddSensor(vreadeye);
	}
	else{
		Camera *eyeCamera = ssi_create(Camera, "eyeCamera", true);
		eyeCamera->getOptions()->flip = true;
		eyeCamera_p = frame->AddProvider(eyeCamera, SSI_CAMERA_PROVIDER_NAME, 0, "1.0s");
		frame->AddSensor(eyeCamera);
	}

	// scene camera

	ITransformable *sceneCamera_p;
	if (sceneFromFile){
		std::string sceneFilePath = baseFolder + "scene_video_raw.mp4";
		if (!ssi_exists(sceneFilePath.c_str())){
			ssi_err("scene file not found: %s\n", sceneFilePath.c_str());
			return;
		}
				
		FFMPEGReader *vreadscene = ssi_create(FFMPEGReader, "scene", true);
		vreadscene->getOptions()->setUrl(sceneFilePath.c_str());
		vreadscene->getOptions()->stream = false;
		vreadscene->getOptions()->buffer = 1.0;
		vreadscene->getOptions()->fps = 30.0;
		vreadscene->getOptions()->width = 640;
		vreadscene->getOptions()->height = 480;
		sceneCamera_p = frame->AddProvider(vreadscene, SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME);
		frame->AddSensor(vreadscene);
	}
	else{
		Camera *sceneCamera = ssi_create(Camera, "sceneCamera", true);
		sceneCamera->getOptions()->flip = true;
		sceneCamera_p = frame->AddProvider(sceneCamera, SSI_CAMERA_PROVIDER_NAME, 0, "1.0s");
		frame->AddSensor(sceneCamera);
	}
	
	// eye tracker
	EyeTracker *et = ssi_create(EyeTracker, 0, true);
	et->getOptions()->showDebugImage = false;
	et->getOptions()->liveAdjustOptions = false;
	ITransformable *et_t = frame->AddTransformer(eyeCamera_p, et, "1");

	// scene tracker
	SceneTracker *st = ssi_create(SceneTracker, 0, true);
	st->getOptions()->setCalibrationWindow(CalibrationWindowName);
	st->getOptions()->preciseCalibration = true;
	st->getOptions()->trackHeadMovement = true;
	st->getOptions()->showHeadtrackingDebugImage = false;
	ITransformable *st_t = frame->AddTransformer(sceneCamera_p, 1, &et_t, st, "1");

	ITransformable *gps_t;
	if (shiftableGazePoint) {
		// gaze point shifter
		GazePointShifter *gps = ssi_create(GazePointShifter, 0, true);
		gps_t = frame->AddTransformer(st_t, gps, "1");
	}
	


	// eye painter
	EyePainter *eyepainter = ssi_create(EyePainter, 0, true);
	ITransformable *eyepainter_t = frame->AddTransformer(eyeCamera_p, 1, &et_t, eyepainter, "1");

	// scene painter
	ScenePainter *scenepainter = ssi_create(ScenePainter, 0, true);
	#ifdef _DEBUG
		scenepainter->getOptions()->drawHeatmap = false; //heatmap painting is resource intensive and should only be enabled in release builds
	#else
		scenepainter->getOptions()->drawHeatmap = true;
	#endif

	ITransformable *scenepainter_t;
	if (shiftableGazePoint) {
		scenepainter_t = frame->AddTransformer(sceneCamera_p, 1, &gps_t, scenepainter, "1");
	}
	else {
		scenepainter_t = frame->AddTransformer(sceneCamera_p, 1, &st_t, scenepainter, "1");
	}

	// eye plot
	VideoPainter *eyePlot = ssi_create_id (VideoPainter, 0, "plot_eye");
	eyePlot->getOptions()->setTitle("Eye");
	eyePlot->getOptions()->flip = false;
	frame->AddConsumer(eyepainter_t, eyePlot, "1");

	// scene plot
	VideoPainter *scenePlot = ssi_create_id (VideoPainter, 0, "plot_scene");
	scenePlot->getOptions()->setTitle(CalibrationWindowName);
	scenePlot->getOptions()->flip = false;
	frame->AddConsumer(scenepainter_t, scenePlot, "1");
	
	if (record){

		char datetime[80];
		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(datetime, 80, "rec %m_%d_%y %H_%M_%S", timeinfo);

		char path[SSI_MAX_CHAR];
		ssi_mkdir("..\\Recordings");
		ssi_sprint(path, "..\\Recordings\\%s", datetime);
		ssi_mkdir(path);

		//record eye gaze
		char path_eyegaze[SSI_MAX_CHAR];
		ssi_sprint(path_eyegaze, "%s\\eye_gaze", path);
		FileWriter *eyeWriter = ssi_create(FileWriter, 0, true);
		eyeWriter->getOptions()->setPath(path_eyegaze);
		eyeWriter->getOptions()->type = File::ASCII;
		frame->AddConsumer(et_t, eyeWriter, "1");

		//record scene gaze
		char path_scenegaze[SSI_MAX_CHAR];
		ssi_sprint(path_scenegaze, "%s\\scene_gaze", path);
		FileWriter *sceneWriter = ssi_create(FileWriter, 0, true);
		sceneWriter->getOptions()->setPath(path_scenegaze);
		sceneWriter->getOptions()->type = File::ASCII;
		frame->AddConsumer(st_t, sceneWriter, "1");

		char path_eyeVideo[SSI_MAX_CHAR];
		ssi_sprint(path_eyeVideo, "%s\\eye_video.mp4", path);
		char path_sceneVideo[SSI_MAX_CHAR];
		ssi_sprint(path_sceneVideo, "%s\\scene_video.mp4", path);

		//record eye video
		FFMPEGWriter *fwEye = ssi_create(FFMPEGWriter, 0, true);
		fwEye->getOptions()->setUrl(path_eyeVideo);

		//record scene video
		FFMPEGWriter *fwScene = ssi_create(FFMPEGWriter, 0, true);
		fwScene->getOptions()->setUrl(path_sceneVideo);

	}

	if (st->getOptions()->trackHeadMovement){
		if (std::abs(eyeCamera_p->getSampleRate() - sceneCamera_p->getSampleRate()) > 0.0001){
			ssi_wrn("Eyecamera and scenecamera framerate are different (%.0f and %.0f)\nThis will cause problems with head tracking", eyeCamera_p->getSampleRate(), sceneCamera_p->getSampleRate())
		}
	}

	// run framework
	decorator->add("console", 0, 480, 640, 240);
	decorator->add("plot_eye", 0, 0, 640, 480);
	decorator->add("plot_scene", 640, 0, 640, 480);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();
}

void justRecordVideo() {

	bool useFFMPEGWriter = true;

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// eye camera

	ITransformable *eyeCamera_p;
	
	Camera *eyeCamera = ssi_create(Camera, "eyeCamera", true);
	eyeCamera->getOptions()->flip = true;
	eyeCamera_p = frame->AddProvider(eyeCamera, SSI_CAMERA_PROVIDER_NAME, 0, "4.0s");
	frame->AddSensor(eyeCamera);

	// scene camera

	ITransformable *sceneCamera_p;
	
	Camera *sceneCamera = ssi_create(Camera, "sceneCamera", true);
	sceneCamera->getOptions()->flip = true;
	sceneCamera_p = frame->AddProvider(sceneCamera, SSI_CAMERA_PROVIDER_NAME, 0, "4.0s");
	frame->AddSensor(sceneCamera);
	
	if (true){

		char datetime[80];
		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(datetime, 80, "rec %m_%d_%y %H_%M_%S", timeinfo);

		char path[SSI_MAX_CHAR];
		ssi_mkdir("..\\Recordings");
		ssi_sprint(path, "..\\Recordings\\%s", datetime);
		ssi_mkdir(path);

		if (useFFMPEGWriter){

			char path_eyeVideo[SSI_MAX_CHAR];
			ssi_sprint(path_eyeVideo, "%s\\eye_video.mp4", path);
			char path_sceneVideo[SSI_MAX_CHAR];
			ssi_sprint(path_sceneVideo, "%s\\scene_video.mp4", path);

			//record eye video
			FFMPEGWriter *fwEye = ssi_create(FFMPEGWriter, 0, true);
			fwEye->getOptions()->setUrl(path_eyeVideo);
			frame->AddConsumer(eyeCamera_p, fwEye, "1");

			//record scene video
			FFMPEGWriter *fwScene = ssi_create(FFMPEGWriter, 0, true);
			fwScene->getOptions()->setUrl(path_sceneVideo);
			frame->AddConsumer(sceneCamera_p, fwScene, "1");

		}
		else{

			char path_eyeVideo[SSI_MAX_CHAR];
			ssi_sprint(path_eyeVideo, "%s\\eye_video.avi", path);
			char path_sceneVideo[SSI_MAX_CHAR];
			ssi_sprint(path_sceneVideo, "%s\\scene_video.avi", path);

			char* compression = "Microsoft Video 1";

			//record eye video
			CameraWriter *cwEye = ssi_create(CameraWriter, 0, true);
			cwEye->getOptions()->setPath(path_eyeVideo);
			cwEye->getOptions()->flip = false;
			cwEye->getOptions()->setCompression(compression);
			frame->AddConsumer(eyeCamera_p, cwEye, "1");

			//record scene video
			CameraWriter *cwScene = ssi_create(CameraWriter, 0, true);
			cwScene->getOptions()->setPath(path_sceneVideo);
			cwScene->getOptions()->flip = false;
			cwScene->getOptions()->setCompression(compression);
			frame->AddConsumer(sceneCamera_p, cwScene, "1");
		}

	}

	// run framework
	decorator->add("console", 0, 480, 640, 240);
	decorator->add("plot*", 640, 0, 640, 480);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();
}

