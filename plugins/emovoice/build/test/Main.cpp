// Main
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
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
#include "ssiev.h"
#include "audio/include/ssiaudio.h"
#include "model/include/ssimodel.h"
#include "ssiml.h"
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

void ex_bayes ();
void ex_svm ();
void ex_mfccpitch ();
void ex_online ();
void ex_offline ();

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

	Factory::RegisterDLL ("ssiframe.dll");	
	Factory::RegisterDLL ("ssievent.dll");	
	Factory::RegisterDLL ("ssiemovoice.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssiaudio.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssiioput.dll");	

	ex_bayes ();
	ex_svm ();
	ex_mfccpitch ();
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

void ex_bayes () {

	// training
	{
		EmoVoiceBayes *model = ssi_pcast (EmoVoiceBayes, Factory::Create (EmoVoiceBayes::GetCreateName (), 0));
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("bayes");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "bayes");			
		Evaluation eval;
		eval.eval (trainer, *test_samples);
		eval.print ();

		PlotSamples (*test_samples, "bayes");
	}

}

void ex_svm () {

	// training
	{
		EmoVoiceSVM *model = ssi_pcast (EmoVoiceSVM, Factory::Create (EmoVoiceSVM::GetCreateName (), 0));
		model->getOptions ()->svm_param.svm_type = 1;
		model->getOptions ()->svm_param.kernel_type = 3;
		Trainer trainer (model);
		trainer.train (*train_samples);
		trainer.save ("svm");
	}

	// evaluation
	{
		Trainer trainer;
		Trainer::Load (trainer, "svm");			
		Evaluation eval;
		eval.eval (trainer, *test_samples);
		eval.print ();

		PlotSamples (*test_samples, "svm");
	}
}

void ex_mfccpitch () {

	IThePainter *painter = Factory::GetPainter ();
	ITheFramework *frame = Factory::GetFramework ();
	ssi_pcast (TheFramework, frame)->getOptions ()->setConsolePos (400,0,400,400);

	// audio	
	Audio *audio = ssi_pcast (Audio, Factory::Create (Audio::GetCreateName (), "audio"));
	audio->getOptions ()->sr = 16000.0;
	audio->getOptions ()->scale = true;
	ITransformable *audio_p = frame->AddProvider (audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor (audio);

	// mfccs
	EmoVoiceMFCC *ev_mfcc = ssi_pcast (EmoVoiceMFCC, Factory::Create (EmoVoiceMFCC::GetCreateName (), "ev_feat"));
	ITransformable *ev_mfcc_t  = frame->AddTransformer (audio_p, ev_mfcc, "0.2s");

	// pitch
	EmoVoicePitch *ev_pitch = ssi_pcast (EmoVoicePitch, Factory::Create (EmoVoicePitch::GetCreateName (), "ev_feat"));
	ITransformable *ev_pitch_t = frame->AddTransformer (audio_p, ev_pitch, "0.2s", "2.0");
	
	// plot	
	SignalPainter *sigpaint = 0;

	sigpaint = ssi_pcast (SignalPainter, Factory::Create (SignalPainter::GetCreateName ())); 
	sigpaint->getOptions ()->size = 10.0;
	sigpaint->getOptions ()->setName ("audio");
	sigpaint->getOptions ()->type = PaintSignalType::AUDIO;
	frame->AddConsumer (audio_p, sigpaint, "0.2s");

	sigpaint = ssi_pcast (SignalPainter, Factory::Create (SignalPainter::GetCreateName ())); 
	sigpaint->getOptions ()->size = 10.0;
	sigpaint->getOptions ()->setName ("mfcc");
	sigpaint->getOptions ()->type = PaintSignalType::IMAGE;
	frame->AddConsumer (ev_mfcc_t, sigpaint, "0.2s");
	
	sigpaint = ssi_pcast (SignalPainter, Factory::Create (SignalPainter::GetCreateName ())); 
	sigpaint->getOptions ()->size = 10.0;
	sigpaint->getOptions ()->setName ("pitch");
	frame->AddConsumer (ev_pitch_t, sigpaint, "0.2s");

	// run framework
	frame->Start ();	
	painter->Arrange (1, 3, 0, 0, 400, 800);
	frame->Wait ();
	frame->Stop ();
	frame->Clear ();
	painter->Clear ();
}

void ex_online () {

	IThePainter *painter = Factory::GetPainter ();
	ITheEventBoard *board = Factory::GetEventBoard ();
	ITheFramework *frame = Factory::GetFramework ();
	ssi_pcast (TheFramework, frame)->getOptions ()->setConsolePos (400,0,400,400);

	// audio	
	Audio *audio = ssi_pcast (Audio, Factory::Create (Audio::GetCreateName (), "audio"));
	audio->getOptions ()->sr = 16000.0;
	ITransformable *audio_p = frame->AddProvider (audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor (audio);

	// trigger
	SNRatio *snratio = ssi_pcast (SNRatio, Factory::Create (SNRatio::GetCreateName ()));
	ZeroEventSender *zerotr = ssi_pcast (ZeroEventSender, Factory::Create (ZeroEventSender::GetCreateName ()));	
	frame->AddConsumer (audio_p, zerotr, "0.2s", 0, snratio);
	board->RegisterSender (*zerotr);

	// feature
	EmoVoiceFeat *ev_feat = ssi_pcast (EmoVoiceFeat, Factory::Create (EmoVoiceFeat::GetCreateName (), "ev_feat"));
	ev_feat->getOptions ()->maj = 1;

	// classifier
	Trainer ev_class;
	if (!Trainer::Load (ev_class, "ev_model")) {
		ssi_err ("could not load model");
	}
	Classifier *classifier = ssi_pcast (Classifier, Factory::Create (Classifier::GetCreateName ()));
	classifier->setTrainer (&ev_class);
	classifier->getOptions ()->console = true;
	frame->AddEventConsumer (audio_p, classifier, board, zerotr->getEventAddress (), ev_feat);

	// plot	
	SignalPainter *sigpaint = 0;

	sigpaint = ssi_pcast (SignalPainter, Factory::Create (SignalPainter::GetCreateName ())); 
	sigpaint->getOptions ()->size = 10.0;
	sigpaint->getOptions ()->setName ("audio");
	sigpaint->getOptions ()->type = PaintSignalType::AUDIO;
	frame->AddConsumer (audio_p, sigpaint, "0.2s");

	sigpaint = ssi_pcast (SignalPainter, Factory::Create (SignalPainter::GetCreateName ())); 
	sigpaint->getOptions ()->setName ("audio (tr)");
	sigpaint->getOptions ()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer (audio_p, sigpaint, board, zerotr->getEventAddress ());

	// run framework
	frame->Start ();	
	board->Start ();
	painter->Arrange (1, 2, 0, 0, 400, 800);
	frame->Wait ();
	board->Stop ();
	frame->Stop ();	
	frame->Clear ();
	board->Clear ();
	painter->Clear ();

};

void PlotSamples (ISamples &samples, const ssi_char_t *name) {

	ThePainter *painter = (ThePainter *) Factory::Create(ThePainter::GetCreateName(), 0, false);

	int plot_id = painter->AddCanvas (name);

	ssi_size_t n_classes = samples.getClassSize ();	
	PaintData<ssi_real_t> **plots = new PaintData<ssi_real_t> *[n_classes];
	
	const ssi_real_t min[2] = {-0.20f, -0.2f};
	const ssi_real_t max[2] = {1.2f, 1.2f};
	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {

		plots[n_class] = new PaintData<ssi_real_t> (2, PAINT_TYPE_SCATTER);

		ssi_size_t n_samples = samples.getSize (n_class);
		ssi_size_t n_streams = samples.getStreamSize ();
		ssi_real_t *data = new ssi_real_t[n_samples * 2 * n_streams];		
		ISSelectClass samples_s (&samples);
		samples_s.setSelection (n_class);
		samples_s.reset ();
		ssi_sample_t *sample = 0;
		ssi_real_t *data_ptr = data;
		while (sample = samples_s.next ()) {
			for (ssi_size_t n_stream = 0; n_stream < n_streams; n_stream++) {
				memcpy (data_ptr, sample->streams[n_stream]->ptr, 2 * sizeof (ssi_real_t));
				data_ptr += 2;
			}
		}

		plots[n_class]->setPointSize (10);
		plots[n_class]->setFixedMinMax (min, max);
		plots[n_class]->setData (data, n_samples * n_streams);		
		plots[n_class]->setFillColor (COLORS[n_class][0],COLORS[n_class][1],COLORS[n_class][2]);
		painter->AddObject (plot_id, plots[n_class]);

		delete[] data;
	}

	painter->Paint (plot_id); 
	painter->Move (plot_id, 10, 10, 600, 600);

	ssi_print ("\n\n\t\tpress enter to close plot window!\n");
	getchar ();

	for (ssi_size_t n_class = 0; n_class < n_classes; n_class++) {
		delete plots[n_class];
	}
	delete[] plots;

	painter->Clear ();
}
