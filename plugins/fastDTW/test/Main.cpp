// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 12/10/2016
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


#define SIMULATE

#include "ssi.h"
#include "SSI_FastDTW.h"
#include "eventinterpreter/include/MyEventinterpreter.h"
#include "signal/include/Relative.h"
#include "audio/include/ssiaudio.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_stream_mouse(void *arg);
bool ex_record_mouse(void *arg);

bool ex_stream_audio(void *arg);
bool ex_record_audio(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
	
	Exsemble ex;
	ex.add(ex_stream_mouse, 0, "Recognize: Mouse 2D", "Demonstrates the use of fastDTW with 2D data.");
#if _WIN32
    ex.add(ex_record_mouse, 0, "Record: Mouse 2D",	  "Record data for fastDTW with 2D data.");

	ex.add(ex_stream_audio, 0, "Recognize: Audio 1D", "Demonstrates the use of fastDTW with 1D data.");

    ex.add(ex_record_audio, 0, "Record: Audio 1D",	  "Record data for fastDTW with 1D data.");
    #endif
	ex.show();


#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

#if _WIN32
bool ex_record_mouse(void *arg) {

	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssimouse");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("ssiioput");
	Factory::RegisterDLL("ssisignal");
	Factory::RegisterDLL("ssieventinterpreter");


	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();


	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->empty = false;
	ezero->getOptions()->setString("triangle");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);


	MyEventinterpreter *eventinterpreter = ssi_create(MyEventinterpreter, 0, true);

	board->RegisterListener(*eventinterpreter, ezero->getEventAddress());
	board->RegisterSender(*eventinterpreter);


	Relative *rel = ssi_create(Relative, "relative", true);
	ITransformable * rel_t = frame->AddTransformer(cursor_p, rel, "1");



	FileSampleWriter *writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->setPath("gesture");
	writer->getOptions()->type = File::ASCII;
	writer->getOptions()->setClasses("triangle;rectangle;circle;w;z");	//please adapt the lua script according to these classnames!
	writer->getOptions()->setUser("user");
	//frame->AddConsumer(cursor_p, writer, "1");
	frame->AddEventConsumer(rel_t, writer, board, eventinterpreter->getEventAddress());

	SignalPainter *plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse(tr)");
	frame->AddEventConsumer(rel_t, plot, board, eventinterpreter->getEventAddress());

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, eventinterpreter->getEventAddress());

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	//frame->Clear();

	return true;
}

#endif

bool ex_stream_mouse(void *arg) {

	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssimouse");
    Factory::RegisterDLL("ssiioput");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("ssisignal");
	Factory::RegisterDLL("fastDTW");

	ITheFramework *frame = Factory::GetFramework();
#if _WIN32
	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);
#endif
	ITheEventBoard *board = Factory::GetEventBoard();


	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = true;
	mouse->getOptions()->single = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	ezero->getOptions()->empty = true;
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);


	Relative *rel = ssi_create(Relative, "relative", true);
	ITransformable * rel_t = frame->AddTransformer(cursor_p, rel, "1");


	SSI_FastDTW *fastDTW = ssi_create(SSI_FastDTW, 0, true);
	fastDTW->getOptions()->setSampleFilename("gesture");
	fastDTW->getOptions()->outputOption = 2;

	//frame->AddConsumer(cursor_p, fastDTW, "1");
	frame->AddEventConsumer(rel_t, fastDTW, board, ezero->getEventAddress());
	board->RegisterSender(*fastDTW);

#if _WIN32
	SignalPainter *plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse(tr)");
	frame->AddEventConsumer(rel_t, plot, board, ezero->getEventAddress());


	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);
#else
    FileEventWriter* ew = ssi_create(FileEventWriter, 0, true);
    ew->getOptions()->setPath("sensor_data");
    board->RegisterListener(*ew);

#endif
	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	//frame->Clear();

	return true;
}
#if _WIN32

bool ex_record_audio(void *arg) {

	Factory::RegisterDLL("ssiaudio");
	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("ssiioput");
	Factory::RegisterDLL("ssieventinterpreter");


	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();


#ifndef SIMULATE
	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 44100;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
#else
	WavReader *audio = ssi_create(WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(audio);
#endif


	AudioActivity *activity = ssi_create(AudioActivity, 0, true);
	activity->getOptions()->method = AudioActivity::LOUDNESS;
	activity->getOptions()->threshold = 0.02f;
	//ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.03s", "0.015s");
	ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.1s");

	
	ZeroEventSender *zes = ssi_create(ZeroEventSender, 0, true);
	//zes->getOptions()->hangin = 0;		//5
	//zes->getOptions()->hangout = 0;	//30
	zes->getOptions()->mindur = 0.05;
	zes->getOptions()->maxdur = 5;
	frame->AddConsumer(activity_t, zes, "0.1s");
	board->RegisterSender(*zes);


	MyEventinterpreter *eventinterpreter = ssi_create(MyEventinterpreter, 0, true);
	eventinterpreter->getOptions()->setLuaScriptName("audio.lua");

	board->RegisterListener(*eventinterpreter, zes->getEventAddress());
	board->RegisterSender(*eventinterpreter);



	FileSampleWriter *writer = ssi_create(FileSampleWriter, 0, true);
	writer->getOptions()->setPath("audio");
	writer->getOptions()->type = File::BINARY;
	writer->getOptions()->setClasses("start;stop;next;previous;restart");	//please adapt the lua script according to these classnames!
	writer->getOptions()->setUser("user");
	frame->AddEventConsumer(audio_p, writer, board, eventinterpreter->getEventAddress());


#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	SignalPainter *sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("activity");
	sigplot->getOptions()->size = 10.0;
	frame->AddConsumer(activity_t, sigplot, "0.1s");

	sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio(tr)");
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer(audio_p, sigplot, board, eventinterpreter->getEventAddress());

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	//frame->Clear();

	return true;
}



bool ex_stream_audio(void *arg) {
	Factory::RegisterDLL("ssiframe");
	Factory::RegisterDLL("ssievent");
	Factory::RegisterDLL("ssiaudio");
	Factory::RegisterDLL("ssigraphic");
	Factory::RegisterDLL("fastDTW");


	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

#ifndef SIMULATE
	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 22050;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
#else
	WavReader *audio = ssi_create(WavReader, 0, true);
	audio->getOptions()->setPath("audio_test.wav");
	audio->getOptions()->scale = true;
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(audio);
#endif


	AudioActivity *activity = ssi_create(AudioActivity, 0, true);
	activity->getOptions()->method = AudioActivity::LOUDNESS;
	activity->getOptions()->threshold = 0.02f;
	//ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.03s", "0.015s");
	ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.1s");

	ZeroEventSender *zes = ssi_create(ZeroEventSender, 0, true);
	//zes->getOptions()->hangin = 0;		//5
	//zes->getOptions()->hangout = 0;	//30
	zes->getOptions()->mindur = 0.3;
	zes->getOptions()->maxdur = 5;
	frame->AddConsumer(activity_t, zes, "0.1s");
	board->RegisterSender(*zes);


	SSI_FastDTW *fastDTW = ssi_create(SSI_FastDTW, 0, true);
	fastDTW->getOptions()->setSampleFilename("audio");
	fastDTW->getOptions()->outputOption = 2;

	//frame->AddConsumer(cursor_p, fastDTW, "1");
	frame->AddEventConsumer(audio_p, fastDTW, board, zes->getEventAddress());
	board->RegisterSender(*fastDTW);


#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif


	SignalPainter *sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("activity");
	sigplot->getOptions()->size = 10.0;
	frame->AddConsumer(activity_t, sigplot, "0.1s");

	sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio(tr)");
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer(audio_p, sigplot, board, zes->getEventAddress());


	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	//frame->Clear();

	return true;
}
#endif
