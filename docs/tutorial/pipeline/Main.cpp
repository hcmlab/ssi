// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
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
#include "MyConsumer.h"
#include "MySensor.h"
#include "MyTransformer.h"
#include "MyFilter.h"
#include "MyFilter2.h"
#include "MyFeature.h"
#include "MyFeature2.h"
#include "MyEventSender.h"
#include "MyEventListener.h"
#include "MyVideoConsumer.h"
#include "MyVideoFilter.h"
#include "MyVideoFeature.h"
using namespace ssi;

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

bool ex_pipeline (void *args);
bool ex_ioput(void *args);
bool ex_transf(void *args);
bool ex_filter(void *args);
bool ex_feature(void *args);
bool ex_chain(void *args);
bool ex_video(void *args);
bool ex_event(void *args);

int main () {

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("frame");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("ioput");
	Factory::RegisterDLL ("graphic");

	Factory::Register (MySensor::GetCreateName (), MySensor::Create);
	Factory::Register (MyConsumer::GetCreateName (), MyConsumer::Create);
	Factory::Register (MyTransformer::GetCreateName (), MyTransformer::Create);
	Factory::Register (MyFilter::GetCreateName (), MyFilter::Create);
	Factory::Register (MyFilter2::GetCreateName (), MyFilter2::Create);
	Factory::Register (MyFeature::GetCreateName (), MyFeature::Create);
	Factory::Register (MyFeature2::GetCreateName (), MyFeature2::Create);
	Factory::Register (MyEventSender::GetCreateName (), MyEventSender::Create);
	Factory::Register (MyEventListener::GetCreateName (), MyEventListener::Create);
	Factory::Register (MyVideoConsumer::GetCreateName(), MyVideoConsumer::Create);
	Factory::Register (MyVideoFilter::GetCreateName(), MyVideoFilter::Create);
	Factory::Register (MyVideoFeature::GetCreateName(), MyVideoFeature::Create);

	Exsemble exsemble;
	exsemble.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	exsemble.add(ex_pipeline, 0, "PIPELINE", "run a pipeline");
	exsemble.add(ex_ioput, 0, "IOPUT", "output a stream");
	exsemble.add(ex_transf, 0, "TRANSFORMER", "transform a stream");
	exsemble.add(ex_filter, 0, "FILTER", "filter a stream");
	exsemble.add(ex_feature, 0, "FEATURE", "extract a feature");
	exsemble.add(ex_chain, 0, "CHAIN", "create a transformation chain");
	exsemble.add(ex_video, 0, "VIDEO", "do some video processing");
	exsemble.add(ex_event, 0, "EVENT", "send an receive events");
	exsemble.show();

	Factory::Clear ();

	return 0;
}

bool ex_pipeline(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	MySensor *sensor = ssi_create (MySensor, "sensor", true);
	sensor->getOptions()->sr = 5.0;
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyConsumer *consumer = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(sensor_p, consumer, "0.5s");		

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_ioput(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	MySensor *sensor = ssi_create (MySensor, "sensor", true);
	sensor->getOptions()->sr = 5.0;
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	FileWriter *filewrite = ssi_create (FileWriter, 0, true);
	filewrite->getOptions()->type = File::ASCII;
	filewrite->getOptions()->setPath("cursor.txt");
	frame->AddConsumer(sensor_p, filewrite, "0.5s");	

	SocketWriter *sockwrite = ssi_create (SocketWriter, 0, true);
	sockwrite->getOptions()->port = 1111;
	sockwrite->getOptions()->setHost("localhost");
	sockwrite->getOptions()->type = Socket::UDP;
	frame->AddConsumer(sensor_p, sockwrite, "0.5s");

	SignalPainter *sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("cursor");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(sensor_p, sigpaint, "0.5s");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	
	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_transf(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	MySensor *sensor = ssi_create (MySensor, 0, true);
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyTransformer *transf = ssi_create (MyTransformer, 0, true);
	ITransformable *transf_t = frame->AddTransformer(sensor_p, transf, "0.5s");	

	ITransformable *source[] = { sensor_p, transf_t };
	MyConsumer *consumer = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(2, source, consumer, "0.5s");

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_filter(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	MySensor *sensor = ssi_create (MySensor, 0, true);
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyFilter *filter = ssi_create (MyFilter, 0, true);
	ITransformable *filter_t = frame->AddTransformer(sensor_p, filter, "0.5s");	

	ITransformable *source[] = { sensor_p, filter_t };
	MyConsumer *consumer = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(2, source, consumer, "0.5s");

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_feature(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	MySensor *sensor = ssi_create (MySensor, 0, true);
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyFeature *feature = ssi_create (MyFeature, 0, true);
	ITransformable *feature_t = frame->AddTransformer(sensor_p, feature, "0.5s");

	MyFeature2 *feature2 = ssi_create (MyFeature2, 0, true);
	ITransformable *feature2_t = frame->AddTransformer(sensor_p, feature2, "0.5s");

	ITransformable *source[] = { sensor_p, feature_t, feature2_t };
	MyConsumer *consumer = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(3, source, consumer, "0.5s");	

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_chain(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	MySensor *sensor = ssi_create (MySensor, 0, true);
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyFilter *filter = ssi_create (MyFilter, 0, true);
	MyFeature *feature = ssi_create (MyFeature, 0, true);
	MyFeature2 *feature2 = ssi_create (MyFeature2, 0, true);
	IFilter *filters[1] = {filter};
	IFeature *features[2] = {feature, feature2};
	Chain *chain = ssi_create (Chain, 0, true);
	chain->set(1, filters, 2, features);
	ITransformable *chain_t = frame->AddTransformer(sensor_p, chain, "0.5s");	

	MyConsumer *printer = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(sensor_p, printer, "0.5s");	

	MyConsumer *printer_t = ssi_create (MyConsumer, 0, true);
	frame->AddConsumer(chain_t, printer_t, "0.5s");

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_video(void *args) {

	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	FakeSignal *video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_SSI;
	ITransformable *video_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	MyVideoFilter *filter = ssi_create(MyVideoFilter, 0, true);
	ITransformable *filter_t = frame->AddTransformer(video_t, filter, "1");

	MyVideoFeature *feature = ssi_create(MyVideoFeature, 0, true);
	ITransformable *feature_t = frame->AddTransformer(video_t, feature, "1");

	MyVideoConsumer *consumer = 0;	
	consumer = ssi_create_id(MyVideoConsumer, 0, "vplot");
	frame->AddConsumer(video_t, consumer, "1");
	consumer = ssi_create_id(MyVideoConsumer, 0, "vplot");
	frame->AddConsumer(filter_t, consumer, "1");

	SignalPainter *sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("DARKEST SPOT");
	sigpaint->getOptions()->size = 0.05;
	sigpaint->getOptions()->type = PaintSignalType::PATH;
	sigpaint->getOptions()->fix[0] = 0.0f;
	sigpaint->getOptions()->fix[1] = 1.0f;
	sigpaint->getOptions()->autoscale = false;
	frame->AddConsumer(feature_t, sigpaint, "1");
	
	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT / 2);
	decorator->add("vplot*", 2, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT / 2, 400, CONSOLE_HEIGHT / 2);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_event(void *args) {

	ITheFramework *frame = Factory::GetFramework ();
	ITheEventBoard *board = Factory::GetEventBoard ();
	
	MySensor *sensor = ssi_create (MySensor, 0, true);
	ITransformable *sensor_p = frame->AddProvider(sensor, MYSENSOR_PROVIDER_NAME);
	frame->AddSensor(sensor);

	MyEventSender *sender = ssi_create (MyEventSender, 0, true);
	frame->AddConsumer(sensor_p, sender, "2.5s");
	board->RegisterSender(*sender);

	MyEventListener *listener = ssi_create (MyEventListener, 0, true);
	board->RegisterListener(*listener, sender->getEventAddress());	

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
