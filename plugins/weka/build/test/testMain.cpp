// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/07/17
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
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
#include "ssiweka.h"
#include "audio\include\ssiaudio.h"
#include "signal\include\ssisignal.h"
#include "model\include\ssimodel.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define AUDIO_SIMULATE
#define AUDIO_SIMULATE_FILE "audio.wav"

#define FRAME_SIZE "0.5s"
#define DELTA_SIZE "0.5s"

void ex_online ();
void ex_offline ();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiaudio.dll");	
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssiweka.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssimodel.dll");

	ex_offline ();
	ex_online ();	  	 

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void ex_offline () {

	ssi_stream_t audio;
	WavTools::ReadWavFile (AUDIO_SIMULATE_FILE, audio, true);

	ssi_stream_t mfccs;
	MFCC *audio_mfcc = ssi_create (MFCC, 0, true);		
	audio_mfcc->getOptions()->n_first = 1;
	audio_mfcc->getOptions()->n_last = 13;
	SignalTools::Transform (audio, mfccs, *audio_mfcc, 160, 160);

	ssi_stream_t mfccs_dd;
	Derivative *audio_mfcc_dd = ssi_create (Derivative, 0, true);
	audio_mfcc_dd->getOptions()->set(Derivative::D0TH | Derivative::D1ST | Derivative::D2ND);
	SignalTools::Transform (mfccs, mfccs_dd, *audio_mfcc_dd, 0u);

	// audio intensity detection
	ssi_stream_t intensity;
	ClassifierT *audio_intensity_detect = ssi_create (ClassifierT, 0, true);
	audio_intensity_detect->getOptions()->setTrainer("..\\models\\mfcc-12{f160d160}_dd{f100d0}_mlp_intensity");
	SignalTools::Transform (mfccs_dd, intensity, *audio_intensity_detect, 1);
	
	FileTools::WriteStreamFile (File::BINARY, "intensity", intensity);

	ssi_stream_destroy (audio);
	ssi_stream_destroy (mfccs);
	ssi_stream_destroy (mfccs_dd);
}

void ex_online () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ("board");

	// audio	
#ifndef AUDIO_SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath(AUDIO_SIMULATE_FILE);
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(audio);
#endif

	// audio feature 
	MFCC *audio_mfcc = ssi_create (MFCC, 0, true);		
	audio_mfcc->getOptions()->n_first = 1;
	audio_mfcc->getOptions()->n_last = 13;
	ITransformable *audio_mfcc_t = frame->AddTransformer(audio_p, audio_mfcc, "160", "160");

	Derivative *audio_mfcc_dd = ssi_create (Derivative, 0, true);
	audio_mfcc_dd->getOptions()->set(Derivative::D0TH | Derivative::D1ST | Derivative::D2ND);
	ITransformable *audio_mfcc_dd_t = frame->AddTransformer(audio_mfcc_t, audio_mfcc_dd, "1");

	// audio intensity detection
	ClassifierT *audio_intensity_detect = ssi_create (ClassifierT, 0, true);
	audio_intensity_detect->getOptions()->setTrainer("..\\models\\mfcc-12{f160d160}_dd{f100d0}_mlp_intensity");
	ITransformable *audio_intensity_detect_t = frame->AddTransformer(audio_mfcc_dd_t, audio_intensity_detect, "1");

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(audio_intensity_detect_t, writer, "1");

	SignalPainter *paint = ssi_create_id (SignalPainter, 0, "plot");
	paint->getOptions()->type = PaintSignalType::AUDIO;
	paint->getOptions()->size = 10;
	frame->AddConsumer(audio_p, paint, "0.1s");

#ifdef AUDIO_SIMULATE
	AudioPlayer *aplayer = ssi_create (AudioPlayer, "aplayer", true);
	frame->AddConsumer(audio_p, aplayer, "0.1s");
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

