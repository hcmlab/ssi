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

//#include <vld.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void test1();
void compare();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssiaudio");	
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssigraphic");

	Factory::RegisterDLL ("ssievent");

	Factory::RegisterDLL ("ssivad_webrtc");

	test1();
	//compare();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void test1 () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	/*
	WavReader *wav = ssi_create (WavReader, 0, true);
	wav->getOptions()->setPath("audio.wav");
	wav->getOptions()->blockInSamples = 1024;
	wav->getOptions()->scale = true;
	*/



	Audio *wav = ssi_create(Audio, "audio", true);
	wav->getOptions()->remember = true;
	wav->getOptions()->sr = 16000.0;
	wav->getOptions()->channels = 1;
	wav->getOptions()->bytes = 2;
	wav->getOptions()->blockInSamples = 1024;
	wav->getOptions()->scale = true;




	ITransformable *audio_p = frame->AddProvider(wav, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(wav);

	// plot
	SignalPainter *audio_plot_in = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_in->getOptions()->setTitle("audio in");
	audio_plot_in->getOptions()->size = 1.0;	
	audio_plot_in->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot_in, "0.01s");

	//VAD
	VadWebRTC *vad = ssi_create (VadWebRTC, 0, true);
	vad->getOptions()->aggr_mode = 3;
	vad->getOptions()->sendEvent = true;
	ITransformable *vad_t = frame->AddTransformer(audio_p, vad, "0.01s");	//just 10,20,30 ms is allowed!
	
	board->RegisterSender(*vad);

	// plot
	SignalPainter *audio_plot_out = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out->getOptions()->setTitle("VAD(webRTC)");
	audio_plot_out->getOptions()->size = 1.0;	
	audio_plot_out->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_t, audio_plot_out, "1");

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	frame->AddConsumer(vad_t, ezero, "1");
	board->RegisterSender(*ezero);

	// plot
	SignalPainter *vad_tr_plot = ssi_create_id (SignalPainter, 0, "plot");
	vad_tr_plot->getOptions()->setTitle("vad(tr)");
	frame->AddEventConsumer(audio_p, vad_tr_plot, board, ezero->getEventAddress()); 

	// audio out
	AudioPlayer *audioPlayer = ssi_create (AudioPlayer, 0, true);
	frame->AddConsumer(audio_p, audioPlayer, "0.1s");

	FileEventWriter *event_writer = ssi_create (FileEventWriter, 0, true);
	event_writer->getOptions()->setPath("vad_events");
	board->RegisterListener(*event_writer);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	board->Stop();

	frame->Clear();
	board->Clear();

}

void compare () {
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

	// plot
	SignalPainter *audio_plot_in = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_in->getOptions()->setTitle("audio in");
	audio_plot_in->getOptions()->size = 1.0;	
	audio_plot_in->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot_in, "0.01s");

	//VAD WEBRTC METHOD 3
	VadWebRTC *vad = ssi_create (VadWebRTC, 0, true);
	vad->getOptions()->aggr_mode = 3;
	vad->getOptions()->sendEvent = true;
	ITransformable *vad_t = frame->AddTransformer(audio_p, vad, "0.01s");	//just 10,20,30 ms is allowed!

	board->RegisterSender(*vad);

	//VAD WEBRTC METHOD 0
	VadWebRTC *vad_rtc_0 = ssi_create (VadWebRTC, 0, true);
	vad_rtc_0->getOptions()->aggr_mode = 0;
	vad_rtc_0->getOptions()->sendEvent = false;
	ITransformable *vad_rtc_0_t = frame->AddTransformer(audio_p, vad_rtc_0, "0.01s");	//just 10,20,30 ms is allowed!

	//VAD LOUDNESS
	AudioActivity *vad_audio_loud = ssi_create (AudioActivity, 0, true);
	vad_audio_loud->getOptions()->method = AudioActivity::LOUDNESS;
	vad_audio_loud->getOptions()->threshold = 0.05f;
	ITransformable *vad_audio_loud_t = frame->AddTransformer(audio_p, vad_audio_loud, "0.01s");

	//VAD INTENSITY
	AudioActivity *vad_audio_inten = ssi_create (AudioActivity, 0, true);
	vad_audio_inten->getOptions()->method = AudioActivity::INTENSITY;
	vad_audio_inten->getOptions()->threshold = 0.00f;
	ITransformable *vad_audio_inten_t = frame->AddTransformer(audio_p, vad_audio_inten, "0.01s");

	//VAD SNR ratio
	AudioActivity *vad_audio_snr = ssi_create (AudioActivity, 0, true);
	vad_audio_snr->getOptions()->method = AudioActivity::SNRATIO;
	vad_audio_snr->getOptions()->threshold = 0.00f;
	ITransformable *vad_audio_snr_t = frame->AddTransformer(audio_p, vad_audio_snr, "0.01s");

	// plot
	SignalPainter *audio_plot_out = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out->getOptions()->setTitle("VAD(webRTC aggressiveness 3)");
	audio_plot_out->getOptions()->size = 1.0;	
	audio_plot_out->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_t, audio_plot_out, "1");

	// plot
	SignalPainter *audio_plot_out1 = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out1->getOptions()->setTitle("VAD(webRTC aggressiveness 0)");
	audio_plot_out1->getOptions()->size = 1.0;	
	audio_plot_out1->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_rtc_0_t, audio_plot_out1, "1");

	// plot
	SignalPainter *audio_plot_out2 = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out2->getOptions()->setTitle("VAD(LOUDNESS)");
	audio_plot_out2->getOptions()->size = 1.0;	
	audio_plot_out2->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_loud_t , audio_plot_out2, "1");

	// plot
	SignalPainter *audio_plot_out3= ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out3->getOptions()->setTitle("VAD(INTENSITY)");
	audio_plot_out3->getOptions()->size = 1.0;	
	audio_plot_out3->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_inten_t , audio_plot_out3, "1");

	// plot
	SignalPainter *audio_plot_out4= ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out4->getOptions()->setTitle("VAD(SNR)");
	audio_plot_out4->getOptions()->size = 1.0;	
	audio_plot_out4->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(vad_audio_snr_t , audio_plot_out4, "1");

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	frame->AddConsumer(vad_t, ezero, "1");
	board->RegisterSender(*ezero);

	// plot
	SignalPainter *vad_tr_plot = ssi_create_id (SignalPainter, 0, "plot");
	vad_tr_plot->getOptions()->setTitle("vad(tr)");
	frame->AddEventConsumer(audio_p, vad_tr_plot, board, ezero->getEventAddress()); 

	// audio out
	AudioPlayer *audioPlayer = ssi_create (AudioPlayer, 0, true);
	frame->AddConsumer(audio_p, audioPlayer, "0.1s");

	FileEventWriter *event_writer = ssi_create (FileEventWriter, 0, true);
	event_writer->getOptions()->setPath("vad_events");
	board->RegisterListener(*event_writer);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	board->Stop();

	frame->Clear();
	board->Clear();
}
