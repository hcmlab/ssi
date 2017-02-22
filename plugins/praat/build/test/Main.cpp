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
#include "ssipraat.h"
#include "audio\include\ssiaudio.h"
#include "ioput\include\ssiioput.h"
#include "signal\include\ssisignal.h"
using namespace ssi;

//#define SIMULATE

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_praat();
void ex_offline(const char* path, int id);

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssipraat.dll");	
	Factory::RegisterDLL("ssiaudio.dll");
	Factory::RegisterDLL("ssisignal.dll");

	ex_praat(); 

	//char path[256];
	//for (int i = 2; i <= 15; i++)
	//{
	//	for (int j = 1; j <= 4; j++)
	//	{
	//		sprintf(path, "D:\\Workspaces\\Publications\\2016\\ICMI\\study\\cc\\g%d", i);
	//		ssi_msg(SSI_LOG_LEVEL_BASIC, "computing %s (%d)", path, j);
	//		ex_offline(path, j);

	//		sprintf(path, "D:\\Workspaces\\Publications\\2016\\ICMI\\study\\ec\\g%d", i);
	//		ssi_msg(SSI_LOG_LEVEL_BASIC, "computing %s (%d)", path, j);
	//		ex_offline(path, j);
	//	}
	//}
	//ssi_msg(SSI_LOG_LEVEL_BASIC, "done");
	
	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_praat () {

	//general
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

	AudioActivity *activity = ssi_create (AudioActivity, 0, true); 
	activity->getOptions()->method = AudioActivity::INTENSITY;
	activity->getOptions()->threshold = 0.0001f;
	ssi::ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.01s");
	
	ZeroEventSender *zes = ssi_create (ZeroEventSender, 0, true);
	zes->getOptions()->hangin = 5;	
	zes->getOptions()->hangout = 30;
	zes->getOptions()->setSender("VAD");
	zes->getOptions()->setEvent("ACTIVITY");
	zes->getOptions()->mindur = 1;
	zes->getOptions()->maxdur = 10;
	frame->AddConsumer(activity_t, zes, "1");
	board->RegisterSender(*zes);
	
	PraatVoiceReport *praat_vr = ssi_create (PraatVoiceReport, "praatvoicereport", true);
	praat_vr->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat_vr->getOptions()->setScript("..\\..\\scripts\\voicereport.praat");
	praat_vr->getOptions()->setTmpWav("praatvoicereport.wav");
	frame->AddEventConsumer(audio_p, praat_vr, board, zes->getEventAddress());
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
	frame->AddEventConsumer(audio_p, praat_uni, board, zes->getEventAddress());
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
	frame->AddEventConsumer(audio_p, plot, board, zes->getEventAddress());

	FileEventWriter *ewriter = ssi_create (FileEventWriter, 0, true);
	ewriter->getOptions()->setPath("vad");
	board->RegisterListener(*ewriter, zes->getEventAddress());

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

}

void ex_offline(const char* path, int id) {

	/*
	* Read wav file
	*/
	ssi_stream_t audio;
	char file[256];
	ssi_sprint(file, "%s\\audio%d.wav", path, id);
	WavTools::ReadWavFile(file, audio, true);

	ssi_stream_t audio_snr;
	ssi_stream_t audio_snrf;

	/*
	* Annotate file for voice activty
	*/
	SNRatio *snratio = ssi_pcast(SNRatio, Factory::Create(SNRatio::GetCreateName()));
	SignalTools::Transform(audio, audio_snr, *snratio, ssi_cast(ssi_size_t, 0.01 * audio.sr + 0.5));

	MvgAvgVar *filter = ssi_pcast(MvgAvgVar, Factory::Create(MvgAvgVar::GetCreateName()));
	filter->getOptions()->win = 0.5;
	filter->getOptions()->format = MvgAvgVar::AVG;
	SignalTools::Transform(audio_snr, audio_snrf, *filter, ssi_cast(ssi_size_t, 0.01 * audio.sr + 0.5));

	char path_anno[256];
	sprintf(path_anno, "%s\\vad.anno", path);
	FileAnnotationWriter faw(path_anno, "vad");

	ThresEventSender *vad = ssi_create(ThresEventSender, 0, true);
	vad->getOptions()->setAddress("VoiceActivity@Audio");
	vad->getOptions()->thres = 4;
	vad->getOptions()->thresout = 2;
	vad->getOptions()->hangin = 20; //0.2s	
	vad->getOptions()->hangout = 50; //0.5s
	vad->getOptions()->mindur = 1;
	vad->getOptions()->maxdur = 10;
	vad->getOptions()->eager = false;
	vad->getOptions()->eall = true;
	vad->setEventListener(&faw);

	SignalTools::Consume(audio_snrf, *vad, 1);

	char path_snr[256];
	sprintf(path_snr, "%s\\snr", path);
	FileTools::WriteStreamFile(File::ASCII, path_snr, audio_snr);

	/*
	* Start-up PRAAT
	*/
	PraatUniversal *praat = ssi_factory_create(PraatUniversal, 0, true);
	praat->getOptions()->setExe("..\\bin\\praatcon.exe");
	praat->getOptions()->setScript("..\\..\\scripts\\ssi.praat");
	praat->getOptions()->setScriptArgs("0 2 yes");
	praat->getOptions()->setSender("Praat");

	FileEventWriter *writer = ssi_create(FileEventWriter, 0, true);
	char path_report[256];
	sprintf(path_report, "%s\\praat_new%d", path, id);
	writer->getOptions()->setPath(path_report);
	praat->setEventListener(writer);

	/*
	* Segment audio data using annotation and run segments through PRAAT
	*/
	Annotation vad_events;
	ModelTools::LoadAnnotation(vad_events, path_anno);
	Annotation::Entry *e;
	ssi_stream_t chunk;
	IConsumer::info info;
	info.status = IConsumer::COMPLETED;
	vad_events.reset();

	writer->listen_enter();
	praat->consume_enter(1, &chunk);

	while (e = vad_events.next()) {
		ssi_stream_copy(audio, chunk, ssi_cast(ssi_size_t, e->start * audio.sr + 0.5), ssi_cast(ssi_size_t, e->stop * audio.sr + 0.5));
		info.time = e->start;
		info.dur = e->stop - e->start;
		praat->consume(info, 1, &chunk);
		ssi_stream_destroy(chunk);
	}

	praat->consume_flush(1, &chunk);
	writer->listen_flush();

	ssi_stream_destroy(audio);
}