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
#include "microsoftkinect\include\ssimicrosoftkinect.h"
#include "image\include\ssiimage.h"
#include "ffmpeg\include\ssiffmpeg.h"


#include <cstring>
#include <io.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <direct.h>
#include <ctime>
#include <sstream>


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
bool ex_test_kinect(void *arg);
bool ex_offline(void *arg);


std::vector<std::string> listFolders(std::string dir);
std::vector<std::string> listFiles(std::string dir);
std::vector<std::string> listSpecificFiles(std::string dir, std::string ending);


std::vector<std::string> listFolders(std::string dir)
{
	
	char originalDirectory[_MAX_PATH];

	// Get the current directory so we can return to it
	_getcwd(originalDirectory, _MAX_PATH);

	//Retrieving the absolute path from a reltive one
	char absPath[_MAX_PATH];
	_fullpath(absPath, dir.c_str(), _MAX_PATH);

	_chdir(absPath);  // Change to the working directory
	_finddata_t fileinfo;

	// This will grab the first file in the directory
	// "*" can be changed if you only want to look for specific files
	intptr_t handle = _findfirst("*", &fileinfo);

	std::vector<std::string> result;

	if (handle == -1) { // No files or directories found
		ssi_wrn("Error searching for folders");
	}

	do
	{
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;
		if (fileinfo.attrib & _A_SUBDIR) {// Use bitmask to see if this is a directory
			result.push_back(fileinfo.name);
			//ssi_print("%s\n", fileinfo.name);
		}

	} while (_findnext(handle, &fileinfo) == 0);

	_findclose(handle); // Close the stream

	_chdir(originalDirectory);
	return result;
}

std::vector<std::string> listSpecificFiles(std::string dir, std::string ending) {

	std::vector<std::string> r = listFiles(dir);
	std::vector<std::string> rr;

	for (int i = 0; i < r.size(); i++) {
		std::string ss = r[i].substr(r[i].size() - ending.size(), ending.size());
		if (0 != ss.compare(ending)) {
			//ssi_wrn("ignoring file %s\\%s (not ending in %s)\n", dir.c_str(), r[i].c_str(), ending.c_str());
		}
		else {
			rr.push_back(r[i]);
		}
	}

	return rr;

}

std::vector<std::string> listFiles(std::string dir)
{
	char originalDirectory[_MAX_PATH];

	// Get the current directory so we can return to it
	_getcwd(originalDirectory, _MAX_PATH);

	//Retrieving the absolute path from a reltive one
	char absPath[_MAX_PATH];
	_fullpath(absPath, dir.c_str(), _MAX_PATH);

	_chdir(absPath); // Change to the working directory
	_finddata_t fileinfo;

	// This will grab the first file in the directory
	// "*" can be changed if you only want to look for specific files
	intptr_t handle = _findfirst("*", &fileinfo);

	std::vector<std::string> result;

	if (handle == -1) { // No files or directories found
		ssi_wrn("Error searching for folders");
	}

	do
	{
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;
		if (fileinfo.attrib & _A_SUBDIR) {// Use bitmask to see if this is a directory
		}
		else { //file
			result.push_back(fileinfo.name);
		}

	} while (_findnext(handle, &fileinfo) == 0);

	_findclose(handle); // Close the stream

	_chdir(originalDirectory);
	return result;
}

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("openface");
		Factory::RegisterDLL("camera");
		Factory::RegisterDLL("frame");
		Factory::RegisterDLL("graphic");
		Factory::RegisterDLL("microsoftkinect");
		Factory::RegisterDLL("image");
		Factory::RegisterDLL("ffmpeg");
		Factory::RegisterDLL("ioput");

		Exsemble ex;
		ex.add(ex_test, 0, "TEST", "Demonstrates the use of openface");
		ex.add(ex_test_kinect, 0, "TEST Kinect", "Demonstrates the use of openface with kinect");
		ex.add(ex_offline, 0, "Offline", "Demonstrates the use of openface with a file");
		ex.show();

		Factory::Clear();

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

bool ex_test_kinect(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	MicrosoftKinect *kinect2 = ssi_create(MicrosoftKinect, 0, true);
	//kinect2->getOptions()->sr = 5.0;
	ITransformable *camera_p = frame->AddProvider(kinect2, SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME,0,"2.0s");
	frame->AddSensor(kinect2);

	
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
	ITransformable *openface_painter_t = frame->AddTransformer(camera_p, 1, in, openface_painter, "1");

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

bool ex_offline(void *arg) {
	//std::string basePath = "./";
	std::string basePath = "Z:/Korpora/aria-noxi/";
	std::vector<std::string> folders = listFolders(basePath);
	std::vector<std::string> files;
	std::string folderPath;
	std::string inputPath;
	std::string outputPath;
	std::string fileEnding = ".mp4";

	Openface *openface = ssi_create(Openface, 0, true);
	openface->getOptions()->setModelPath("..\\lib\\local\\LandmarkDetector\\model");
	openface->getOptions()->setTriPath("..\\lib\\local\\LandmarkDetector\\model\\tris_68_full.txt");
	openface->getOptions()->setAuPath("..\\lib\\local\\FaceAnalyser\\AU_predictors\\AU_all_best.txt");

	for (int i = 0; i < folders.size(); i++) {
		folderPath = basePath + folders.at(i) + "\\";
		files = listSpecificFiles(folderPath, fileEnding);
		for (int j = 0; j < files.size(); j++) {
			ssi_print("\n Found: %s", files.at(j).c_str());
			
			inputPath = folderPath + files.at(j);
			std::string temp = files.at(j).replace(files.at(j).length() - fileEnding.length(), fileEnding.length(), "_openface");
			outputPath = folderPath + temp;
			
			if (ssi_exists(outputPath.c_str(), ".stream")) {
				ssi_print("\n skipping file. %s already exists", outputPath.c_str());
				continue;
			}

			FFMPEGReader *reader = ssi_create(FFMPEGReader, 0, true);
			reader->getOptions()->setUrl(inputPath.c_str());
			reader->getOptions()->bestEffort = true;

			FileWriter *writer = ssi_create(FileWriter, 0, true);
			writer->getOptions()->setPath(outputPath.c_str());
			writer->getOptions()->type = File::ASCII;

			FileProvider provider(writer, openface);
			reader->setProvider(SSI_CAMERA_PROVIDER_NAME, &provider);

			reader->connect();
			reader->start();
			reader->wait();
			Sleep(1000);
			reader->stop();
			reader->disconnect();
		}
	}
	return true;
}

