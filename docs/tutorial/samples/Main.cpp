// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
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
#include "audio\include\ssiaudio.h"
#include "mouse\include\ssimouse.h"
#include "signal\include\ssisignal.h"
#include "image\include\ssiimage.h"
#include "camera\include\ssicamera.h"
#include "model\include\ssimodel.h"
#include "ssiml.h"
#include "emovoice\include\ssiev.h"
#include "torch\include\ssitorch.h"
using namespace ssi;

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

bool ex_filter (void *args);
bool ex_feature(void *args);
bool ex_audio(void *args);
bool ex_audio_tr(void *args);
bool ex_video(void *args);
bool ex_gestrec_online(void *args);
bool ex_gestrec_offline(void *args);
bool ex_gestrec_fusion(void *args);
bool ex_emorec(void *args);

int main () {

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("frame");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("mouse");
	Factory::RegisterDLL ("event");
	Factory::RegisterDLL ("audio");
	Factory::RegisterDLL ("ioput");
	Factory::RegisterDLL ("graphic");
	Factory::RegisterDLL ("signal");
	Factory::RegisterDLL ("model");
	Factory::RegisterDLL ("image");
	Factory::RegisterDLL ("camera");
	Factory::RegisterDLL ("emovoice");
	Factory::RegisterDLL ("torch");

	Exsemble exsemble;
	exsemble.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
	exsemble.add(ex_filter, 0, "FILTER", "filter example");
	exsemble.add(ex_feature, 0, "FEATURE", "feature example");
	exsemble.add(ex_audio, 0, "AUDIO", "audio example");
	exsemble.add(ex_audio_tr, 0, "TRIGGER", "trigger example");
	exsemble.add(ex_video, 0, "VIDEO", "video example");
	exsemble.add(ex_gestrec_online, 0, "GESTURE ONLINE", "gesture online example");
	exsemble.add(ex_gestrec_offline, 0, "GESTURE OFFLINE", "gesture offline example");
	exsemble.add(ex_gestrec_fusion, 0, "GESTURE FUSION", "gesture fusion example");
	exsemble.add(ex_emorec, 0, "EMOREC", "emotion recognition example");
	exsemble.show();

	Factory::Clear ();

	return 0;
}

bool ex_filter(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->sr = 50.0;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse); 

	Selector *selector = ssi_create (Selector, 0, true);
	ssi_size_t inds[1] = {1};
	selector->getOptions()->set(1, inds);
	ITransformable *selector_t = frame->AddTransformer(cursor_p, selector, "0.05s");

	Noise *noise = ssi_create (Noise, 0, true);
	noise->getOptions()->ampl = 0.5f;
	ITransformable *noise_t = frame->AddTransformer(selector_t, noise, "0.05s");

	Butfilt *lowfilt = ssi_create (Butfilt, 0, true);
	lowfilt->getOptions()->type = Butfilt::LOW;
	lowfilt->getOptions()->low = 1;
	lowfilt->getOptions()->order = 3;
	lowfilt->getOptions()->norm = false;
	lowfilt->getOptions()->zero = true;
	ITransformable *lowfilt_t = frame->AddTransformer(noise_t, lowfilt, "0.05s");

	Butfilt *highfilt = ssi_create (Butfilt, 0, true);
	highfilt->getOptions()->type = Butfilt::HIGH;
	highfilt->getOptions()->high = 1;
	highfilt->getOptions()->norm = false;
	highfilt->getOptions()->order = 3;
	ITransformable *highfilt_t = frame->AddTransformer(noise_t, highfilt, "0.05s");

	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("raw");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(selector_t, sigpaint, "0.1s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("noisy");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(noise_t, sigpaint, "0.1s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("lowfilt");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(lowfilt_t, sigpaint, "0.1s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("highfilt");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(highfilt_t, sigpaint, "0.1s");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_feature(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse); 
	
	Selector *selector = ssi_create (Selector, 0, true);
	selector->getOptions()->set(0);
	ITransformable *selector_t = frame->AddTransformer(cursor_p, selector, "0.02s");

	Derivative *drvt = ssi_create (Derivative, 0, true);
	drvt->getOptions()->set(Derivative::D0TH | Derivative::D1ST | Derivative::D2ND);
	ITransformable *drvt_t = frame->AddTransformer(selector_t, drvt, "0.02s");

	Functionals *functs = ssi_create (Functionals, 0, true);
	functs->getOptions()->addName("mean");
	functs->getOptions()->addName("std");
	functs->getOptions()->addName("min");
	functs->getOptions()->addName("max");
	ITransformable *functs_t = frame->AddTransformer(drvt_t, functs, "0.02s", "0.08s");

	FileWriter *filewrite = ssi_create (FileWriter, 0, true);
	filewrite->getOptions()->type = File::ASCII;
	frame->AddConsumer(functs_t, filewrite, "0.02s");	

	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("cursor");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(selector_t, sigpaint, "0.1s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("derivative");
	sigpaint->getOptions()->size = 10.0;
	frame->AddConsumer(drvt_t, sigpaint, "0.1s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("functionals");
	sigpaint->getOptions()->size = 10.0;	
	frame->AddConsumer(functs_t, sigpaint, "0.1s");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_audio(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	Spectrogram *spect = ssi_create (Spectrogram, 0, true);
	Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank (512, audio_p->getSampleRate(), 50, 100, 5100, WINDOW_TYPE_RECTANGLE);
	spect->setFilterbank(filterbank);	
	delete filterbank;
	ITransformable *spect_t = frame->AddTransformer(audio_p, spect, "0.01s", "0.015s");

	MFCC *mfcc = ssi_create (MFCC, 0, true);
	mfcc->getOptions()->n_first = 0;
	mfcc->getOptions()->n_last = 13;
	ITransformable *mfcc_t = frame->AddTransformer(audio_p, mfcc, "0.01s", "0.015s");

	SignalPainter *sigpaint = 0;

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("audio");
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	sigpaint->getOptions()->size = 10.0;		
	frame->AddConsumer(audio_p, sigpaint, "0.02s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("spectrogram");
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(spect_t, sigpaint, "0.02s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("mfccs");
	sigpaint->getOptions()->size = 10.0;
	sigpaint->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(mfcc_t, sigpaint, "0.02s");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_audio_tr(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);

	AudioActivity *activity = ssi_create (AudioActivity, 0, true);
	activity->getOptions()->method = AudioActivity::LOUDNESS;
	activity->getOptions()->threshold = 0.15f;
	ITransformable *activity_t = frame->AddTransformer(audio_p, activity, "0.02s", "0.18s");

	ZeroEventSender *ezero = ssi_create (ZeroEventSender, 0, true);
	ezero->getOptions()->hangin = 5;
	ezero->getOptions()->hangin = 20;
	frame->AddConsumer(activity_t, ezero, "0.2s");
	board->RegisterSender(*ezero);

	SignalPainter *sigpaint = 0;	

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("audio(tr)");
	sigpaint->getOptions()->size = 0;	
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddEventConsumer(audio_p, sigpaint, board, ezero->getEventAddress());

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("audio");
	sigpaint->getOptions()->size = 10.0;		
	sigpaint->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, sigpaint, "0.02s");

	sigpaint = ssi_create_id (SignalPainter, 0, "plot");
	sigpaint->getOptions()->setTitle("activity");
	sigpaint->getOptions()->size = 10.0;			
	frame->AddConsumer(activity_t, sigpaint, "0.02s");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_video(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// sensor
	
	Camera *camera = ssi_create (Camera, "camera", true);
	ITransformable *camera_p = frame->AddProvider(camera, SSI_CAMERA_PROVIDER_NAME, 0, "2.0s");
	camera->setLogLevel(SSI_LOG_LEVEL_DEBUG);
	frame->AddSensor(camera);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->scale = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// transform
	
	CVColor *cv_color = ssi_create (CVColor, 0, true);
	cv_color->getOptions()->code = CVColor::RGB2GRAY;
	ITransformable *cv_color_t = frame->AddTransformer(camera_p, cv_color, "1");

	CVChange *cv_change = ssi_create (CVChange, 0, true);
	ITransformable *cv_chain_t = frame->AddTransformer(cv_color_t, cv_change, "1");

	ITransformable *channels[1] = {cursor_p};
	CVCrop *cv_crop = ssi_create (CVCrop, 0, true);
	cv_crop->getOptions()->width = 160;
	cv_crop->getOptions()->height = 120;
	cv_crop->getOptions()->region[2] = 0.5f;
	cv_crop->getOptions()->region[3] = 0.5f;
	cv_crop->getOptions()->scaled = true;	
	cv_crop->getOptions()->origin = CVCrop::ORIGIN::CENTER;
	ITransformable *cv_crop_t = frame->AddTransformer(camera_p, 1, channels, cv_crop, "1");

	CVResize *cv_resize = ssi_create (CVResize, 0, true);
	cv_resize->getOptions()->width = 0.5;
	cv_resize->getOptions()->height = 0.5;
	ITransformable *cv_resize_t = frame->AddTransformer(camera_p, cv_resize, "1");

	// save

	CVSave *cv_save = ssi_create (CVSave, 0, true);
	cv_save->getOptions()->set("image", CVSave::BMP);
	cv_save->getOptions()->number = false;
	frame->AddConsumer(camera_p, cv_save, "1");

	// plot

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("video");
	frame->AddConsumer(cv_color_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("change");
	frame->AddConsumer(cv_chain_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("crop");
	frame->AddConsumer(cv_crop_t, vidplot, "1");

	vidplot = ssi_create_id (VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("resize");
	vidplot->getOptions()->scale = false;	
	frame->AddConsumer(cv_resize_t, vidplot, "1");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_gestrec_online(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	SampleList sample_list;
	unsigned int iterations, class_num;
	char class_name[1024];

	// mouse	
	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// trigger
	ZeroEventSender *ezero = ssi_create (ZeroEventSender, 0, true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	EventConsumer *trigger = ssi_create (EventConsumer, 0, true);
	board->RegisterListener(*trigger, ezero->getEventAddress());

	// plot
	SignalPainter *plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse");
	plot->getOptions()->type = PaintSignalType::PATH;
	trigger->AddConsumer(cursor_p, plot);	

	// record
	ssi_print ("number of class> ");
	scanf ("%d", &class_num);
	ssi_print ("iterations> ");
	scanf ("%d", &iterations);

	Collector *collector = ssi_create (Collector, 0, true);
	collector->getOptions()->setUserName("user");
	collector->getOptions()->iter = iterations;
	collector->setSampleList(sample_list);	
	for (unsigned int i = 0; i < class_num; i++) {
		ssi_print ("name of class %d from %d> ", i+1, class_num);
		scanf ("%s", class_name);
		collector->getOptions()->addClassName(class_name);
	}
	trigger->AddConsumer(cursor_p, collector);
	
	ssi_print ("\n\n  draw gestures while pressing left mouse button\n\n\n");

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 400);
	decorator->add("monitor*", CONSOLE_WIDTH, 400, 400, 400);

	board->Start();
	frame->Start();
	collector->wait();
	frame->Stop();
	board->Stop();
	trigger->clear();

	trigger->AddConsumer(cursor_p, plot);

	// view
	sample_list.print (stdout);
	
	// create, eval and save model
	{
		ssi_print ("create trainer..\n");
		Dollar$1 *model = ssi_create (Dollar$1, 0, true);
		Trainer trainer (model);

		ssi_print ("evaluate trainer..\n");
		Evaluation eval;
		eval.evalLOO (&trainer, sample_list);
		eval.print ();

		ssi_print ("train classifier..\n");
		trainer.train (sample_list);
		trainer.save ("gesture.model");
	}

	Trainer trainer;
	Trainer::Load (trainer, "gesture.model");	

	// test
	ssi_print ("start testing..\n");

	Classifier *classifier = ssi_create (Classifier, 0, true);
	classifier->setTrainer(&trainer);
	trigger->AddConsumer(cursor_p, classifier);

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

bool ex_gestrec_offline(void *args) {

	// create and save sample list
	{
		ssi_char_t *movementpath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\cursor";
		ssi_char_t *annopath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\button.anno";

		ssi_stream_t movement;
		FileTools::ReadStreamFile (movementpath, movement);
		Annotation anno;
		ModelTools::LoadAnnotation (anno, annopath);
		SampleList samples;
		ModelTools::LoadSampleList (samples, movement, anno, "user");		
		ModelTools::SaveSampleList (samples, "gestrec", File::BINARY);

		ssi_stream_destroy (movement);
	}

	// create and save trainer
	{
		Dollar$1 *dollar = ssi_create (Dollar$1, 0, true);
		dollar->getOptions()->norm = true;
			
		Trainer trainer (dollar);				
		trainer.setSamples ("gestrec");
		trainer.save ("gestrec-template");
	}

	// load and train trainer
	{
		Trainer trainer;
		Trainer::Load (trainer, "gestrec-template");
		trainer.evalKFold (5);
		trainer.release ();
		trainer.train ();
		trainer.save ("gestrec");
	}

	return true;
}

bool ex_gestrec_fusion(void *args) {

	ssi_char_t *movementpath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\cursor";
	ssi_char_t *annopath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\button.anno";

	ssi_stream_t movement;
	FileTools::ReadStreamFile (movementpath, movement);

	Annotation anno;
	ModelTools::LoadAnnotation (anno, annopath);

	SampleList samples1;
	ModelTools::LoadSampleList (samples1, movement, anno, "user");

	SampleList samples2;
	ModelTools::LoadSampleList (samples2, movement, anno, "user");

	ISamples *slist[] = { &samples1, &samples2 };
	ISMergeStrms samples (2, slist);
		
	ModelTools::SaveSampleList (samples, "gestrecfusion", File::BINARY);

	if (!ssi_exists ("gestrecfusion.trainer")) {

		SVM *svm = ssi_create(SVM, 0, true);
		Dollar$1 *dollar = ssi_create (Dollar$1, 0, true);
		dollar->getOptions()->norm = true;
		IModel *mlist[] = { dollar, svm };

		SimpleFusion *fusion = ssi_create (SimpleFusion, 0, true);
		fusion->getOptions()->method = SimpleFusion::SUM;
			
		Functionals *func = ssi_create (Functionals, 0, true);
		Butfilt *butfilt = ssi_create (Butfilt, 0, true);
		butfilt->getOptions()->type = Butfilt::LOW;
		butfilt->getOptions()->low = 2.0;
		butfilt->getOptions()->norm = false;
		ITransformer *tlist[] = { 0, func };
		ssi_size_t flist[] = { 10, 0 };
		ssi_size_t dlist[] = { 0, 0 };

		ssi_size_t nslist[] = { 0, 10 };
		ssi_size_t slist2[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		ssi_size_t *slist[] = { 0, slist2 };

		Trainer trainer (2, mlist, fusion);				
		trainer.setTransformer (2, tlist, flist, dlist);
		trainer.setSelection (2, nslist, slist);
		trainer.train (samples);
		trainer.save ("gestrecfusion");

	}

	Trainer trainer;
	Trainer::Load (trainer, "gestrecfusion");

	Evaluation  eval;
	eval.evalKFold (&trainer, samples, 2);
	eval.print ();

	ssi_stream_destroy (movement);

	return true;
}

bool ex_gestrec(void *args) {

	ssi_char_t *movementpath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\cursor";
	ssi_char_t *annopath = "..\\..\\..\\model\\mouse\\data\\user\\2011-12-23_14-14-14\\button.anno";

	ssi_stream_t movement;
	FileTools::ReadStreamFile (movementpath, movement);

	Annotation anno;
	ModelTools::LoadAnnotation (anno, annopath);

	SampleList samples1;
	ModelTools::LoadSampleList (samples1, movement, anno, "user");

	SampleList samples2;
	ModelTools::LoadSampleList (samples2, movement, anno, "user");

	ISamples *slist[] = { &samples1, &samples2 };
	ISMergeStrms samples (2, slist);
		
	ModelTools::SaveSampleList (samples, "gestrec", File::BINARY);

	if (!ssi_exists ("gestrec.trainer")) {

		KNearestNeighbors *knn = ssi_create (KNearestNeighbors, 0, true);
		knn->getOptions()->distsum = true;
		Dollar$1 *dollar = ssi_create (Dollar$1, 0, true);
		dollar->getOptions()->norm = true;
		IModel *mlist[] = { knn, dollar };

		SimpleFusion *fusion = ssi_create (SimpleFusion, 0, true);
		fusion->getOptions()->method = SimpleFusion::SUM;
			
		Functionals *func = ssi_create (Functionals, 0, true);
		Butfilt *butfilt = ssi_create (Butfilt, 0, true);
		butfilt->getOptions()->type = Butfilt::LOW;
		butfilt->getOptions()->low = 2.0;
		butfilt->getOptions()->norm = false;
		ITransformer *tlist[] = { func, butfilt };
		ssi_size_t flist[] = { 0, 10 };
		ssi_size_t dlist[] = { 0, 0 };

		ssi_size_t nslist[] = { 10, 0 };
		ssi_size_t slist1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		ssi_size_t *slist[] = { slist1, 0 };

		Trainer trainer (2, mlist, fusion);				
		trainer.setTransformer (2, tlist, flist, dlist);
		trainer.setSelection (2, nslist, slist);
		trainer.train (samples);
		trainer.save ("gestrec");

	}

	Trainer trainer;
	Trainer::Load (trainer, "gestrec");

	Evaluation  eval;
	eval.eval (&trainer, samples);
	eval.print ();

	ssi_stream_destroy (movement);

	return true;
}

bool ex_emorec(void *args) {

	ssi_char_t *wavpath = "..\\..\\..\\model\\emovoice\\data\\user\\2013-10-16_07-35-48\\audio.wav";
	ssi_char_t *annopath = "..\\..\\..\\model\\emovoice\\data\\user\\2013-10-16_07-35-48\\turns.anno";

	ssi_stream_t audio;
	WavTools::ReadWavFile (wavpath, audio, true);

	Annotation anno;
	ModelTools::LoadAnnotation (anno, annopath);

	SampleList samples1;
	ModelTools::LoadSampleList (samples1, audio, anno, "user");

	SampleList samples2;
	ModelTools::LoadSampleList (samples2, audio, anno, "user");

	ISamples *slist[] = { &samples1, &samples2 };
	ISMergeStrms samples (2, slist);
		
	ModelTools::SaveSampleList (samples, "emorec", File::BINARY);

	if (!ssi_exists ("emorec.trainer")) {

		SVM *svm = ssi_create (SVM, 0, true);
		TorchHMM *hmm = ssi_create (TorchHMM, 0, true);
		IModel *mlist[] = { svm, hmm };

		SimpleFusion *fusion = ssi_create (SimpleFusion, 0, true);
		fusion->getOptions()->method = SimpleFusion::PRODUCT;
			
		EmoVoiceFeat *ev = ssi_create (EmoVoiceFeat, 0, true);
		MFCC *mfccs = ssi_create (MFCC, 0, true);
		ITransformer *tlist[] = { ev, mfccs };
		ssi_size_t flist[] = { 0, ssi_cast (ssi_size_t, 0.01 * audio.sr + 0.5) };
		ssi_size_t dlist[] = { 0, ssi_cast (ssi_size_t, 0.015 * audio.sr + 0.5) };

		ssi_size_t nslist[] = { 10, 0 };
		ssi_size_t slist1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		ssi_size_t *slist[] = { slist1, 0 };

		Trainer trainer (2, mlist, fusion);		
		trainer.setTransformer (2, tlist, flist, dlist);
		trainer.setSelection (2, nslist, slist);
		trainer.train (samples);
		trainer.save ("emorec");

	}

	Trainer trainer;
	Trainer::Load (trainer, "emorec");

	Evaluation  eval;
	eval.eval (&trainer, samples);
	eval.print ();

	ssi_stream_destroy (audio);

	return true;
}
