// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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
#include "ssiaudio.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_audio(void *arg);
bool ex_audioplay(void *arg);
bool ex_mixer(void *arg);
bool ex_vad(void *arg);
bool ex_gate(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssiaudio.dll");

	Exsemble ex;
	ex.add(&ex_audio, 0, "AUDIO", "How to record an audio file.");
	ex.add(&ex_audioplay, 0, "AUDIOPLAY", "How to replay an audio file.");
	ex.add(&ex_mixer, 0, "MIXER", "How to mix two audio streams.");
	ex.add(&ex_vad, 0, "VAD", "How to use voice activity detection.");
	ex.add(&ex_gate, 0, "GATE", "How to use a noise gate.");
	ex.show();

	ssi_print ("\n\n\tpress enter to quit\n");
	getchar ();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_audio(void *arg) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->remember = true;
	audio->getOptions()->sr = 16000.0;
	audio->getOptions()->channels = 1;
	audio->getOptions()->bytes = 2;		
	audio->getOptions()->scale = false;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
	
	AudioActivity *vad = ssi_create (AudioActivity, 0, true);
	vad->getOptions()->method = AudioActivity::LOUDNESS;
	vad->getOptions()->threshold = 0.05f;
	ITransformable *vad_t = frame->AddTransformer(audio_p, vad, "0.03s", "0.015s");
	
	ZeroEventSender *ezero = ssi_create (ZeroEventSender, 0, true);	
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->hangin = 3;
	ezero->getOptions()->hangout = 10;	
	frame->AddConsumer(vad_t, ezero, "0.1s", "0");
	board->RegisterSender(*ezero);
	
	WavWriter *wavwrite = ssi_create (WavWriter, 0, true);
	wavwrite->getOptions()->setPath("audio");	
	frame->AddConsumer(audio_p, wavwrite, "0.1s");

	SignalPainter *sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio");
	sigplot->getOptions()->size = 10.0;		
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("activity");		
	sigplot->getOptions()->size = 10.0;		
	frame->AddConsumer(vad_t, sigplot, "0.1s");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	const ssi_char_t *address = "@";	
	board->RegisterListener(*monitor, address, 10000);

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

	return true;
}
	
bool ex_audioplay(void *arg) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->block = 0.1;
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->loop = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(audio);
	
	AudioPlayer *aplay = ssi_create (AudioPlayer, 0, true);
	frame->AddConsumer(audio_p, aplay, "0.1s");

	SignalPainter *sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio");
	sigplot->getOptions()->size = 2.0;	
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}		

bool ex_mixer(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Audio *audio = ssi_create(Audio, "audio1", true);
	audio->getOptions()->remember = true;
	audio->getOptions()->sr = 16000.0;
	audio->getOptions()->channels = 1;
	audio->getOptions()->bytes = 2;
	audio->getOptions()->scale = true;
	ITransformable *audio_1_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	audio = ssi_create(Audio, "audio2", true);
	audio->getOptions()->remember = true;
	audio->getOptions()->sr = 16000.0;
	audio->getOptions()->channels = 1;
	audio->getOptions()->bytes = 2;
	audio->getOptions()->scale = true;
	ITransformable *audio_2_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	AudioMixer *mixer = ssi_create(AudioMixer, 0, true);
	ITransformable *audio_mix = frame->AddTransformer(audio_1_p, 1, &audio_2_p, mixer, "0.1s");

	WavWriter *wavwrite = ssi_create(WavWriter, 0, true);
	wavwrite->getOptions()->setPath("audio_mix");
	frame->AddConsumer(audio_mix, wavwrite, "0.1s");

	SignalPainter *sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio 1");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_1_p, sigplot, "0.1s");

	sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio 2");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_2_p, sigplot, "0.1s");

	sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio 1+2");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_mix, sigplot, "0.1s");

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

	return true;
}

bool ex_vad(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 16000.0;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);	
	
	AudioLpc *lpc = ssi_create(AudioLpc, 0, true);
	ITransformable *lpc_t = frame->AddTransformer(audio_p, lpc, "0.01s", "0.05s");

	VoiceActivitySender *vad = ssi_create(VoiceActivitySender, "vad", true);
	vad->getAudioActivityOptions()->method = AudioActivity::LOUDNESS;
	vad->getAudioActivityOptions()->threshold = 0.05f;	
	vad->getZeroEventSenderOptions()->mindur = 0.5;
	vad->getZeroEventSenderOptions()->incdur = 1.0;
	vad->getZeroEventSenderOptions()->maxdur = 5.0;
	vad->getZeroEventSenderOptions()->hangin = 3;
	vad->getZeroEventSenderOptions()->hangout = 10;
	ITransformable *vad_t = frame->AddTransformer(audio_p, vad, "0.03s", "0.015s");
	board->RegisterSender(*vad);

	VoiceActivityVerifier *vadok = ssi_create(VoiceActivityVerifier, "vadok", true);
	vadok->getOptions()->threshold = 3.0f;
	vadok->getOptions()->from = 1;
	vadok->getOptions()->to = 6;
	frame->AddEventConsumer(audio_p, vadok, board, vad->getEventAddress());
	board->RegisterSender(*vadok);

	SignalPainter *sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("audio");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("activity");
	sigplot->getOptions()->size = 10.0;
	frame->AddConsumer(vad_t, sigplot, "0.1s");

	sigplot = ssi_create_id (SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("lpc");
	sigplot->getOptions()->size = 10.0;
	frame->AddConsumer(lpc_t, sigplot, "0.1s");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	const ssi_char_t *address = "@";
	board->RegisterListener(*monitor, address, 10000);

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

	return true;
}

bool ex_gate(void *arg) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 16000.0;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	AudioNoiseGate *gate = ssi_create(AudioNoiseGate, 0, true);
	gate->getOptions()->threshold = -35;
	gate->getOptions()->attack = 250;
	gate->getOptions()->decay = 250;
	ITransformable *gate_t = frame->AddTransformer(audio_p, gate, "0.01s");

	SignalPainter *sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("AUDIO");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigplot, "0.1s");

	sigplot = ssi_create_id(SignalPainter, 0, "plot");
	sigplot->getOptions()->setTitle("GATE");
	sigplot->getOptions()->size = 10.0;
	sigplot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(gate_t, sigplot, "0.1s");

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	const ssi_char_t *address = "@";
	board->RegisterListener(*monitor, address, 10000);

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

	return true;
}
