// Main.cpp
// author: Andreas Seiderer
// created: 2013/09/16
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
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "ssiml.h"
#include "ssipraat.h"
#include "audio\include\ssiaudio.h"
#include "ioput\include\ssiioput.h"
#include "signal\include\ssisignal.h"
using namespace ssi;

//#define SIMULATE

ssi_char_t sstring[SSI_MAX_CHAR];

bool ex_praat(void *args);
bool ex_offline(void *args);

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("frame");
	Factory::RegisterDLL ("graphic");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("ioput");
	Factory::RegisterDLL ("praat");	
	Factory::RegisterDLL ("audio");
	Factory::RegisterDLL ("signal");
	
	Exsemble ex;
	ex.add(ex_praat, 0, "ONLINE", "Extract praat features online");
	ex.add(ex_offline, 0, "OFFLINE", "Extract praat features offline");	
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_praat (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	ssi_pcast (TheEventBoard, board)->getOptions()->update = 250;

#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 16000;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");				
	audio->getOptions()->scale = true;
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(audio);
#endif

	AudioActivity *activity = ssi_pcast(AudioActivity, Factory::Create(AudioActivity::GetCreateName()));
	activity->getOptions()->method = ssi::AudioActivity::METHOD::LOUDNESS;
	activity->getOptions()->threshold = 0.05f;
	ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.01s", "0.02s");
	
	TriggerEventSender *vad = ssi_create(TriggerEventSender, 0, true);
	vad->getOptions()->setAddress("VoiceActivity@Audio");
	vad->getOptions()->hangOutSamples = 10;
	vad->getOptions()->hangInSamples = 3;
	vad->getOptions()->sendStartEvent = false;
	vad->getOptions()->maxDuration = 5.0;
	vad->getOptions()->minDuration = 0.5;
	vad->getOptions()->triggerType = ssi::TriggerEventSender::TRIGGER::NOT_EQUAL;
	frame->AddConsumer(activity_t, vad, "1");
	board->RegisterSender(*vad);
	
	PraatVoiceReport *praat_vr = ssi_create (PraatVoiceReport, "praatvoicereport", true);
	praat_vr->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat_vr->getOptions()->setScript("..\\..\\scripts\\voicereport.praat");
	praat_vr->getOptions()->setTmpWav("praatvoicereport.wav");
	frame->AddEventConsumer(audio_p, praat_vr, board, vad->getEventAddress());
	board->RegisterSender(*praat_vr);

	PraatVoiceReportT *praat_vr_transformer = ssi_create (PraatVoiceReportT, "praatvoicereport_t", true);
	praat_vr_transformer->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat_vr_transformer->getOptions()->setScript("..\\..\\scripts\\voicereport.praat");	
	praat_vr_transformer->getOptions()->setTmpWav("praatvoicereport_t.wav");
	ITransformable *praat_vr_t = frame->AddTransformer(audio_p, praat_vr_transformer, "1.0s");

	PraatUniversal *praat_uni = ssi_create (PraatUniversal, "praatuniversal", true);
	praat_uni->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat_uni->getOptions()->setScript("..\\..\\scripts\\voicereport_speechrate_formatted_intensity.praat");
	praat_uni->getOptions()->setScriptArgs("0 2 yes");
	praat_uni->getOptions()->setTmpWav("praatuniversal.wav");
	praat_uni->getOptions()->undefined_value = 0;
	frame->AddEventConsumer(audio_p, praat_uni, board, vad->getEventAddress());
	//frame->AddConsumer(audio_p, praat_uni, "1.0s");
	board->RegisterSender(*praat_uni);

	PraatUniversalT *praat_uni_transformer = ssi_create (PraatUniversalT, "praatuniversal_t", true);
	praat_uni_transformer->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat_uni_transformer->getOptions()->setScript("..\\..\\scripts\\voicereport_speechrate_formatted_intensity.praat");
	praat_uni_transformer->getOptions()->setScriptArgs("0 2 yes");
	praat_uni_transformer->getOptions()->dimensions = 35;
	praat_uni_transformer->getOptions()->setTmpWav("praatuniversal_t.wav");
	ITransformable *praat_uni_t = frame->AddTransformer(audio_p, praat_uni_transformer, "1.0s");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create (AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	SignalPainter *plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->size = 10.0;		
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, plot, "0.1s");

	EventPainter *eplot = ssi_create_id (EventPainter, 0, "plot");
	eplot->getOptions()->type = PaintBars::TYPE::BAR_POS;
	board->RegisterListener(*eplot, praat_uni->getEventAddress());

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("activity");
	plot->getOptions()->size = 10.0;		
	plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(activity_t, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("trigger");		
	frame->AddEventConsumer(audio_p, plot, board, vad->getEventAddress());

	FileEventWriter *ewriter = ssi_create (FileEventWriter, 0, true);
	ewriter->getOptions()->setPath("vad");
	board->RegisterListener(*ewriter, vad->getEventAddress());

	ewriter = ssi_create (FileEventWriter, 0, true);
	ewriter->getOptions()->setPath("universal");
	board->RegisterListener(*ewriter, praat_uni->getEventAddress());

	ewriter = ssi_create (FileEventWriter, 0, true);
	ewriter->getOptions()->setPath("voicereport");
	board->RegisterListener(*ewriter, praat_vr->getEventAddress());

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("universal");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(praat_uni_t, writer, "1");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("voicereport");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(praat_vr_t, writer, "1");

	EventMonitor* monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 20000);

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

bool ex_offline(void *args) {
	
	// Read wav file	

	ssi_stream_t audio_raw, audio;
	WavTools::ReadWavFile("audio.wav", audio_raw, true);

	// add silence
	ssi_size_t add_num = ssi_size_t(audio_raw.sr * 3);
	ssi_stream_init(audio, audio_raw.num + add_num, audio_raw.dim, audio_raw.byte, audio_raw.type, audio_raw.sr);
	ssi_stream_zero(audio);
	memcpy(audio.ptr, audio_raw.ptr, audio_raw.tot);

	ssi_stream_destroy(audio_raw);

	ssi_stream_t audio_aa;

	ssi_stream_t audio_mono;
	AudioMono *am = ssi_create(AudioMono, 0, true);
	SignalTools::Transform(audio, audio_mono, *am, ssi_cast(ssi_size_t, 0.01 * audio.sr + 0.5));

	// voice activity

	AudioActivity *activity = ssi_pcast(AudioActivity, Factory::Create(AudioActivity::GetCreateName()));
	activity->getOptions()->method = ssi::AudioActivity::METHOD::LOUDNESS;
	activity->getOptions()->threshold = 0.05f;
	SignalTools::Transform(audio_mono, audio_aa, *activity, "0.01s", "0.02s");
	
	old::FileAnnotationWriter faw("vad.csv", "vad");

	TriggerEventSender *vad = ssi_create(TriggerEventSender, 0, true);
	vad->getOptions()->setAddress("VoiceActivity@Audio");
	vad->getOptions()->hangOutSamples = 10;
	vad->getOptions()->hangInSamples = 3;
	vad->getOptions()->sendStartEvent = false;
	vad->getOptions()->maxDuration = 0;
	vad->getOptions()->minDuration = 0.5;
	vad->getOptions()->triggerType = ssi::TriggerEventSender::TRIGGER::NOT_EQUAL;
	vad->setEventListener(&faw);

	SignalTools::Consume(audio_aa, *vad, "0.1s");

	// run praat
	
	PraatVoiceReport *praat = ssi_factory_create(PraatVoiceReport, 0, true);
	praat->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat->getOptions()->setScript("..\\..\\scripts\\voicereport.praat");	
	praat->getOptions()->setTmpWav("praatvoicereport.wav");
	praat->getOptions()->setSender("Praat");

	FileEventWriter *writer = ssi_create(FileEventWriter, 0, true);	
	writer->getOptions()->setPath("praat");
	praat->setEventListener(writer);

	old::Annotation vad_events;

	ModelTools::LoadAnnotation(vad_events, "vad.csv");
	old::Annotation::Entry *e;
	ssi_stream_t chunk;
	IConsumer::info info;
	info.status = IConsumer::COMPLETED;
	vad_events.reset();

	writer->listen_enter();
	praat->consume_enter(1, &chunk);

	while (e = vad_events.next()) {
		ssi_size_t from = ssi_cast(ssi_size_t, e->start * audio.sr + 0.5);
		ssi_size_t to = ssi_cast(ssi_size_t, e->stop * audio.sr + 0.5);
		to = min(to, audio_mono.num);
		from = min(from, to);
		ssi_stream_copy(audio_mono, chunk, from, to);
		info.time = e->start;
		info.dur = e->stop - e->start;
		praat->consume(info, 1, &chunk);
		ssi_stream_destroy(chunk);
	}

	praat->consume_flush(1, &chunk);
	writer->listen_flush();

	ssi_stream_destroy(audio);
	ssi_stream_destroy(audio_mono);
	ssi_stream_destroy(audio_aa);

	return true;
}