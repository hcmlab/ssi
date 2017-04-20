// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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

#include "audio/include/ssiaudio.h"

#include "model/include/ssimodel.h"
#include "ssiml/include/ssiml.h"
#include "ssiev.h"

using namespace ssi;

#define N_COLORS 5
unsigned short COLORS[][3] = {
	128,0,0,
	0,128,0,
	0,0,128,
	128,0,128,
	0,128,128
};

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#if __gnu_linux__

#if __ANDROID__
    #define Audio AudioOpenSL
    #define SSI_AUDIO_PROVIDER_NAME SSI_AUDIOOPENSL_PROVIDER_NAME
#else
    #define Audio AudioPortaudio
    #define SSI_AUDIO_PROVIDER_NAME SSI_AUDIOPORTAUDIO_PROVIDER_NAME
#endif
#endif

bool ex_bayes (void *args);
bool ex_svm(void *args);
bool ex_mfccpitch(void *args);
bool ex_online(void *args);
bool ex_continuous(void *args);

void PlotSamples (ISamples &samples, const ssi_char_t *name);

ssi_size_t n_classes = 4;
ssi_size_t n_sampels = 50;
ssi_size_t n_streams = 1;
ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
SampleList *train_samples = 0;
SampleList *test_samples = 0;

int main (int argc, const char* argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif
	
	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);
	
	ssi_random_seed ();

	SampleList training;
	ModelTools::CreateTestSamples (training, n_classes, n_sampels, n_streams, distr);			
	SampleList testing;
	ModelTools::CreateTestSamples (testing, n_classes, n_sampels, n_streams, distr);	
	train_samples = &training;
	test_samples = &testing;

	Factory::RegisterDLL ("ssiframe");	
	Factory::RegisterDLL ("ssievent");	
	Factory::RegisterDLL ("ssiemovoice");
#if __ANDROID__
#else
	Factory::RegisterDLL ("ssigraphic");
#endif
	Factory::RegisterDLL ("ssimodel");
	Factory::RegisterDLL ("ssiaudio");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssiioput");	

	Exsemble ex;
	ex.add(ex_mfccpitch, 0, "MFCCs + Pitch", "");
	ex.add(ex_online, 0, "ONLINE TRIGGERED", "");
	ex.add(ex_continuous, 0, "ONLINE CONTINUOUS", "");
	ex.show();	

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_mfccpitch(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio	

    Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = false;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	// mfccs
	EmoVoiceMFCC *ev_mfcc = ssi_create (EmoVoiceMFCC, 0, true);
	ITransformable *ev_mfcc_t  = frame->AddTransformer(audio_p, ev_mfcc, "0.2s");

	// pitch
	//EmoVoicePitch *ev_pitch = ssi_create (EmoVoicePitch, 0, true);
	//ITransformable *ev_pitch_t = frame->AddTransformer(audio_p, ev_pitch, "0.2s", "2.0s");
	
	// plot	
#if __ANDROID__
#else
	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot"); 
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->setTitle("audio");
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigpaint, "0.2s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot"); 
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->setTitle("mfcc");
	sigpaint->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(ev_mfcc_t, sigpaint, "0.2s");
	
	//sigpaint = ssi_create_id (SignalPainter, 0, "plot"); 
	//sigpaint->getOptions()->size = 10.0;
	//sigpaint->getOptions()->setTitle("pitch");
	//frame->AddConsumer(ev_pitch_t, sigpaint, "0.2s");
#endif

	// run framework
    decorator->add("console", 0, 0, 650, 800);
    decorator->add("plot*", 650, 0, 400, 400);
    decorator->add("monitor*", 650, 400, 400, 400);

    frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_online(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard ();
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

    // audio

    Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = false;
    ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
    frame->AddSensor(audio);

	// trigger
	AudioActivity *activity = ssi_create (AudioActivity, 0, true);
	activity->getOptions()->method = AudioActivity::INTENSITY;
	activity->getOptions()->threshold = 0.05f;
	ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.03s", "0.015s");

	ZeroEventSender *zerotr = ssi_create (ZeroEventSender, 0, true);	
	zerotr->getOptions()->mindur = 0.3;
	zerotr->getOptions()->maxdur = 5.0;
	zerotr->getOptions()->eager = false;
	zerotr->getOptions()->hangin = 3;
	zerotr->getOptions()->hangout = 10;
	frame->AddConsumer(activity_t, zerotr, "0.1s");
	board->RegisterSender(*zerotr);

	// feature
	EmoVoiceFeat *ev_feat = ssi_create (EmoVoiceFeat, "ev_feat", true);
	ev_feat->getOptions()->maj = 1;

	// classifier
	Trainer ev_class;
	if (!Trainer::Load (ev_class, "ev_model")) {
		ssi_err ("could not load model");
	}
	Classifier *classifier = ssi_create (Classifier, 0, true);
	classifier->setTrainer(&ev_class);
	classifier->getOptions()->console = true;
	frame->AddEventConsumer(audio_p, classifier, board, zerotr->getEventAddress(), IEvents::EVENT_STATE_FILTER::COMPLETED, ev_feat);

#if __ANDROID__
#else
	// plot	
	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot"); 
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->setTitle("audio");
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigpaint, "0.2s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot"); 
	sigpaint->getOptions()->setTitle("audio(tr)");
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer(audio_p, sigpaint, board, zerotr->getEventAddress());
#endif
	// run framework

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
};

bool ex_continuous(void *args) {

	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

    // audio

    Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;
    audio->getOptions()->sr = 16000.0;
    ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);

    frame->AddSensor(audio);

	// classifier
	Trainer ev_class;
	if (!Trainer::Load(ev_class, "ev_cont")) {
		ssi_err("could not load model");
	}
	Classifier *classifier = ssi_create (Classifier, 0, true);
	classifier->setTrainer(&ev_class);
	classifier->getOptions()->console = true;
	frame->AddConsumer(audio_p, classifier, "1.0s");
#if __ANDROID__
#else
	// plot	
	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->setTitle("audio");
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigpaint, "0.2s");
#endif
	// run framework

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
};

