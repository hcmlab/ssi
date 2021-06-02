// Main
// author: Fabian Wildgrube
// created: 2021
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
 
#define NOMINMAX

#include "ssi.h"
#include "ssiazurekinect.h"
#include "signal/include/ssisignal.h"
#include "websocket/include/websocket.h"
#include "GarbageTestDataSensor.h"
#include <iostream>
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

bool ex_allvideostreams(void* args);
bool ex_pointcloudwebsocketserver(void* args);
bool ex_bodytrackingvideo(void* args);
bool ex_skeleton(void* args);
bool ex_skeletonwebsocketserver(void* args);
bool ex_garbagedatawebsocketserver(void* args);
bool ex_pointcloudtcpsender(void* args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiazurekinect");
	Factory::RegisterDLL("ssisignal");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("ssiioput");
	Factory::RegisterDLL("ssiwebsocket");

	Exsemble ex;
	ex.add(ex_allvideostreams, 0, "AZURE KINECT VIDEO", "All video channels (rgb, depth, IR)");
	ex.add(ex_bodytrackingvideo, 0, "Bodytracking Video", "rgb video channel with bodytracking information visualized");
	ex.add(ex_skeleton, 0, "SKELETON", "Raw numerical values of the Skeleton joints only");
	ex.add(ex_skeletonwebsocketserver, 0, "SKELETON Websocket server", "Skeleton only websocket server on port 8000");
	ex.add(ex_pointcloudwebsocketserver, 0, "POINT CLOUD websocket server", "Point cloud only websocket server on port 9000");
	ex.add(ex_garbagedatawebsocketserver, 0, "Garbage Data websocket server", "Garbage Data only websocket server 9000");
	ex.add(ex_pointcloudtcpsender, 0, "Pointcloud TCP sender", "Send Pointcloud data via TCP socket on port 8888");
	ex.show();

	Factory::Clear();
	

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_allvideostreams(void* args)
{
	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);
	kinect->getOptions()->nrOfBodiesToTrack = 1;
	kinect->getOptions()->showBodyTracking = true;

	ITransformable* rgb_p = frame->AddProvider(kinect, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	ITransformable* ir_p = frame->AddProvider(kinect, SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME, 0, "1.0s");
	ITransformable* depth_p = frame->AddProvider(kinect, SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME, 0, "1.0s");
	frame->AddSensor(kinect);

	VideoPainter* vplot = 0;

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("rgb");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("infrared");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(ir_p, vplot, "1");

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("depth");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(depth_p, vplot, "1");

	decorator->add("console", 0, 0, 500, 800);
	decorator->add("plot*", 500, 0, 900, 1000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_bodytrackingvideo(void* args)
{
	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);
	kinect->getOptions()->nrOfBodiesToTrack = 1;
	kinect->getOptions()->showBodyTracking = true;

	ITransformable* rgb_p = frame->AddProvider(kinect, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	frame->AddSensor(kinect);

	VideoPainter* vplot = 0;

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("rgb");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	decorator->add("console", 0, 0, 500, 800);
	decorator->add("plot*", 500, 0, 900, 1000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_skeleton(void* args)
{
	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);
	kinect->getOptions()->nrOfBodiesToTrack = 1;

	ITransformable* skeleton_p = frame->AddProvider(kinect, SSI_AZUREKINECT_SKELETON_PROVIDER_NAME, 0, "1.0s");
	frame->AddSensor(kinect);

	SignalPainter* paint = ssi_create_id(SignalPainter, 0, "plot");
	paint->getOptions()->type = PaintSignalType::SIGNAL;
	paint->getOptions()->setTitle("Azure Kinect Skeleton");
	frame->AddConsumer(skeleton_p, paint, "0.1s");

	decorator->add("console", 0, 0, 500, 800);
	decorator->add("plot*", 500, 0, 900, 1000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_skeletonwebsocketserver(void* args)
{
	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);
	kinect->getOptions()->nrOfBodiesToTrack = 1;
	kinect->getOptions()->showBodyTracking = true;
	kinect->getOptions()->lowpassFilterJointRotations = false;

	ITransformable* rgb_p = frame->AddProvider(kinect, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME, 0, "1.0s");
	ITransformable* skeleton_p = frame->AddProvider(kinect, SSI_AZUREKINECT_SKELETON_PROVIDER_NAME, 0, "10.0s");
	frame->AddSensor(kinect);

	/*
	MvgAvgVar* slidingAverage = ssi_create(MvgAvgVar, "sliding", true);
	slidingAverage->getOptions()->win = 1.0;
	slidingAverage->getOptions()->format = MvgAvgVar::AVG;
	slidingAverage->getOptions()->method = MvgAvgVar::MOVING;
	ITransformable* averaged_skeleton_t = frame->AddTransformer(skeleton_p, slidingAverage, "1");
	*/

	MvgMedian* movingMedian = ssi_create(MvgMedian, "movingmedian", true);
	ITransformable* medianed_skeleton_t = frame->AddTransformer(skeleton_p, movingMedian, "1");

	VideoPainter* vplot = 0;

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("rgb");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(rgb_p, vplot, "1");

	Websocket* websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->send_info = false;
	websocket->getOptions()->send_own_events = true;
	//setting to a little less than what 1 frame at 30fps would take to make sure the websocket sends each frame as soon as possible (setting to 33 lead to the client receiving only every 40-45ms...)
	websocket->getOptions()->queue_check_interval = 30;
	frame->AddConsumer(medianed_skeleton_t, websocket, "1");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 500, 0, 900, 1000);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_pointcloudwebsocketserver(void* args)
{
	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	AzureKinect* kinect = ssi_create(AzureKinect, 0, true);

	ITransformable* pc_p = frame->AddProvider(kinect, SSI_AZUREKINECT_POINTCLOUD_PROVIDER_NAME, 0, "2.0s");
	ITransformable* depth_p = frame->AddProvider(kinect, SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME, 0, "1.0s");
	frame->AddSensor(kinect);

	VideoPainter* vplot = 0;

	vplot = ssi_create_id(VideoPainter, 0, "plot");
	vplot->getOptions()->setTitle("depth");
	vplot->getOptions()->flip = false;
	vplot->getOptions()->mirror = false;
	frame->AddConsumer(depth_p, vplot, "1");

	
	Websocket* websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->send_info = false;
	websocket->getOptions()->send_own_events = true;
	websocket->getOptions()->http_port = 9000;
	//setting to a little less than what 1 frame at 30fps would take to make sure the websocket sends each frame as soon as possible (setting to 33 lead to the client receiving only every 40-45ms...)
	websocket->getOptions()->queue_check_interval = 30;
	frame->AddConsumer(pc_p, websocket, "1");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor* monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 700, 800);
	decorator->add("plot*", 700, 0, 512, 512);
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

bool ex_garbagedatawebsocketserver(void* args)
{
	ssi::Factory::Register(ssi::GarbageDataSensor::GetCreateName(), ssi::GarbageDataSensor::Create);

	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard* board = Factory::GetEventBoard();

	GarbageDataSensor* garbageDataSensor = ssi_create(GarbageDataSensor, 0, true);
	garbageDataSensor->getOptions()->sr = 30.0;
	garbageDataSensor->getOptions()->bytesPerSample = 3000000; //3 MBytes / sample 

	ITransformable* garbage_p = frame->AddProvider(garbageDataSensor, SSI_GARBAGEDATA_PROVIDER, 0, "2.0s");
	frame->AddSensor(garbageDataSensor);
	
	Websocket* websocket = ssi_create(Websocket, 0, true);
	websocket->getOptions()->send_info = false;
	websocket->getOptions()->send_own_events = true;
	//setting to a little less than what 1 frame at 30fps would take to make sure the websocket sends each frame as soon as possible (setting to 33 lead to the client receiving only every 40-45ms...)
	websocket->getOptions()->queue_check_interval = 30;
	frame->AddConsumer(garbage_p, websocket, "1");

	board->RegisterSender(*websocket);
	board->RegisterListener(*websocket);

	EventMonitor* monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 700, 800);
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

bool ex_pointcloudtcpsender(void* args) {
	ssi::Factory::Register(ssi::GarbageDataSensor::GetCreateName(), ssi::GarbageDataSensor::Create);

	ITheFramework* frame = Factory::GetFramework();

	Decorator* decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	GarbageDataSensor* garbageDataSensor = ssi_create(GarbageDataSensor, 0, true);
	garbageDataSensor->getOptions()->sr = 30.0;
	garbageDataSensor->getOptions()->bytesPerSample = 1500000; //1.5 MBytes / sample 

	ITransformable* garbage_p = frame->AddProvider(garbageDataSensor, SSI_GARBAGEDATA_PROVIDER, 0, "2.0s");
	frame->AddSensor(garbageDataSensor);

	// start sender
	SocketWriter* socket_writer_bin = ssi_create(SocketWriter, 0, true);
	socket_writer_bin->getOptions()->setUrl(Socket::TYPE::TCP, "localhost", 8888);
	socket_writer_bin->getOptions()->format = SocketWriter::Options::FORMAT::BINARY;
	frame->AddConsumer(garbage_p, socket_writer_bin, "1");

	decorator->add("console", 0, 0, 700, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}