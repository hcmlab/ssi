// laugther_testMain.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/28
// Copyright (C) 2007-10 University of Augsburg, Florian Lingenfelser
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
#include "ssiopensmile.h"
#include "audio/include/ssiaudio.h"
#include "model/include/ssimodel.h"
using namespace ssi;

#ifdef _MSC_VER 
#ifdef _DEBUG
#pragma comment(lib, "ssid.lib")
#else
#pragma comment(lib, "ssi.lib")
#endif
#endif

#define SIMULATE

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_laughter ();

int main () {	

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssisignal.dll");			
	Factory::RegisterDLL ("ssiopensmile.dll");
	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssiaudio.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssigraphic.dll");

	ex_laughter ();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void ex_laughter () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->sr = 48000;
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

	SNRatio *laughter_vad = ssi_create (SNRatio, 0, true);
	ITransformable *laughter_vad_t = frame->AddTransformer(audio_p, laughter_vad, "0.2s");

	LaughterPreProcessor *laughter_pre = ssi_create (LaughterPreProcessor, 0, true);
	ITransformable *laughter_pre_t = frame->AddTransformer(audio_p, laughter_pre, "0.01s", "0.01s");

	LaughterFeatureExtractor *laughter_extract = ssi_create (LaughterFeatureExtractor, 0, true);
	Classifier *laughter_classifier = ssi_create (Classifier, 0, true);
	laughter_classifier->getOptions()->setTrainer("laughter");
	board->RegisterSender(*laughter_classifier);

	frame->AddConsumer(laughter_pre_t, laughter_classifier, "0.5s", "0.5s", laughter_extract, laughter_vad_t);

	SignalPainter *plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(audio_p, plot, "0.01s");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, laughter_classifier->getEventAddress(), 10000);	

#ifdef SIMULATE
	AudioPlayer *player = ssi_create (AudioPlayer, "player", true);	
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

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
