// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 7/3/2015
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
#include "ssivad_webrtc.h"

#include "audio/include/ssiaudio.h"
using namespace ssi;

#define OFFLINE

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool vad(void *args);
bool compare(void *args);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("frame");
	Factory::RegisterDLL ("audio");	
	Factory::RegisterDLL ("ioput");
	Factory::RegisterDLL ("graphic");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("vad_webrtc");

	Exsemble ex;
	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	ex.add(&vad, 0, "VAD", "Voice activity with standard settings.");
	ex.add(&compare, 0, "COMPARE", "Comparing different vad settings.");
	ex.show();

	Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool vad(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

#ifdef OFFLINE
	WavReader *wav = ssi_create (WavReader, 0, true);
	wav->getOptions()->setPath("audio.wav");
	wav->getOptions()->blockInSamples = 1024;
	wav->getOptions()->scale = true;
#else
	Audio *wav = ssi_create(Audio, "audio", true);
	wav->getOptions()->remember = true;
	wav->getOptions()->sr = 16000.0;
	wav->getOptions()->channels = 1;
	wav->getOptions()->bytes = 2;
	wav->getOptions()->blockInSamples = 1024;
	wav->getOptions()->scale = true;
#endif
	ITransformable *audio_p = frame->AddProvider(wav, "audio");
	frame->AddSensor(wav);

	// VAD
	VadWebRTC *vad = ssi_create (VadWebRTC, 0, true);		
	ITransformable *vad_t = frame->AddTransformer(audio_p, vad, "10ms");	//just 10,20,30 ms is allowed!	

	TriggerEventSender *trigger = ssi_create (TriggerEventSender, 0, true);
	trigger->getOptions()->setAddress("vad@webrtc");
	trigger->getOptions()->hangInSamples = 15;
	trigger->getOptions()->hangOutSamples = 15;
	frame->AddConsumer(vad_t, trigger, "1");
	board->RegisterSender(*trigger);

	// plot
	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("AUDIO");
	plot->getOptions()->size = 10.0;
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, plot, "0.01s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD");
	plot->getOptions()->size = 10.0;
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_t, plot, "1");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD (Event)");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer(audio_p, plot, board, trigger->getEventAddress());

	// audio out
#ifdef OFFLINE
	AudioPlayer *audioPlayer = ssi_create (AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, audioPlayer, "0.1s");
#endif

	FileEventWriter *event_writer = ssi_create (FileEventWriter, 0, true);
	event_writer->getOptions()->setPath("vad");
	board->RegisterListener(*event_writer);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("plot*", 1, 0, CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("monitor*", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool compare (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	WavReader *wav = ssi_create (WavReader, 0, true);
	wav->getOptions()->setPath("audio.wav");
	wav->getOptions()->blockInSamples = 1024;
	wav->getOptions()->scale = true;

	ITransformable *audio_p = frame->AddProvider(wav, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(wav);

	//VAD WEBRTC METHOD 0
	VadWebRTC *vad = 0;
	
	vad = ssi_create(VadWebRTC, 0, true);
	vad->getOptions()->aggr_mode = 0;
	ITransformable *vad_rtc_0_t = frame->AddTransformer(audio_p, vad, "10ms");	//just 10,20,30 ms is allowed!

	//VAD WEBRTC METHOD 3
	vad = ssi_create(VadWebRTC, 0, true);
	vad->getOptions()->aggr_mode = 3;
	ITransformable *vad_rtc_3_t = frame->AddTransformer(audio_p, vad, "10ms");	//just 10,20,30 ms is allowed!

	//VAD LOUDNESS
	AudioActivity *vad_audio_loud = ssi_create (AudioActivity, 0, true);
	vad_audio_loud->getOptions()->method = AudioActivity::LOUDNESS;
	vad_audio_loud->getOptions()->threshold = 0.05f;
	ITransformable *vad_audio_loud_t = frame->AddTransformer(audio_p, vad_audio_loud, "10ms");

	//VAD INTENSITY
	AudioActivity *vad_audio_inten = ssi_create (AudioActivity, 0, true);
	vad_audio_inten->getOptions()->method = AudioActivity::INTENSITY;
	vad_audio_inten->getOptions()->threshold = 0.00f;
	ITransformable *vad_audio_inten_t = frame->AddTransformer(audio_p, vad_audio_inten, "10ms");

	//VAD SNR ratio
	AudioActivity *vad_audio_snr = ssi_create (AudioActivity, 0, true);
	vad_audio_snr->getOptions()->method = AudioActivity::SNRATIO;
	vad_audio_snr->getOptions()->threshold = 0.00f;
	ITransformable *vad_audio_snr_t = frame->AddTransformer(audio_p, vad_audio_snr, "10ms");

	// plot
	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("AUDIO");
	plot->getOptions()->size = 1.0;
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, plot, "0.01s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD (webRTC aggressiveness 0)");
	plot->getOptions()->size = 10.0; 
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_rtc_0_t, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD (webRTC aggressiveness 3)");
	plot->getOptions()->size = 1.0;
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_rtc_3_t, plot, "1");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD (LOUDNESS)");
	plot->getOptions()->size = 10.0; 
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_loud_t , plot, "1");

	plot= ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD (INTENSITY)");
	plot->getOptions()->size = 10.0; 
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_inten_t , plot, "1");

	plot= ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("VAD(SNR)");
	plot->getOptions()->size = 10.0; 
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_snr_t , plot, "1");

	// audio out
#ifdef OFFLINE
	AudioPlayer *audioPlayer = ssi_create (AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, audioPlayer, "0.1s");
#endif

	decorator->add("plot*", 1, 0, CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
