// MainOnline.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/11
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
#include "ssimodel.h"
#include "ssiml.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_gesture (void *args); 
bool ex_smooth(void *args);
bool ex_events(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssimouse.dll");
	Factory::RegisterDLL ("ssimodel.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssisignal.dll");

	ssi_random_seed();

	Exsemble exsemble;
	exsemble.add(&ex_gesture, 0, "GESTURE", "Online gesture recognizer");
	exsemble.add(&ex_smooth, 0, "SMOOTH", "Online decision smoothing");
	exsemble.add(&ex_events, 0, "EVENT", "Online recognition from events");
	exsemble.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_gesture (void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();

	SampleList sample_list;
	unsigned int iterations, class_num;
	char class_name[1024];

	// mouse
	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	// trigger
	ZeroEventSender *ezero = ssi_create (ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	frame->AddConsumer(button_p, ezero, "0.25s");	
	board->RegisterSender(*ezero);
	
	EventConsumer *trigger = ssi_create (EventConsumer, 0, true);	
	board->RegisterListener(*trigger, ezero->getEventAddress());

	// plot
	SignalPainter *plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse");
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

	// run

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	ssi_print ("\n\n\tPerform gesture while pressing left mouse button..\n\n\n");
	collector->wait();
	frame->Stop();
	board->Stop();
	trigger->clear();

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
	trigger->AddConsumer(cursor_p, plot);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	classifier->wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

struct fake_tuple_sender_arg {
	ITheEventBoard *b;
	ssi_event_t *e;
	ssi_size_t ms;
	ssi_size_t n;
};

void fake_tuple_sender(void *ptr) {
	fake_tuple_sender_arg *arg = ssi_pcast(fake_tuple_sender_arg, ptr);
	ssi_event_map_t *tuple = ssi_pcast(ssi_event_map_t, arg->e->ptr);
	for (ssi_size_t i = 0; i < arg->n; i++) {
		tuple[i].value = ssi_cast (ssi_real_t, ssi_random());
	}
	arg->b->update(*arg->e);
	ssi_sleep(arg->ms);
}

bool ex_smooth(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	ssi_size_t n_classes = 4;	
	ssi_event_t e;
	ssi_event_init(e, SSI_ETYPE_MAP, Factory::AddString("decision"), Factory::AddString("faked"));
	ssi_event_adjust(e, sizeof(ssi_event_map_t) * n_classes);
	ssi_event_map_t *tuple = ssi_pcast(ssi_event_map_t, e.ptr);
	ssi_char_t string[100];
	for (ssi_size_t i = 0; i < n_classes; i++) {
		ssi_sprint(string, "%c", ssi_cast(char, 'A' + i));
		tuple[i].id = Factory::AddString(string);
	}
	fake_tuple_sender_arg arg;
	arg.e = &e;
	arg.b = board;
	arg.ms = 3000;
	arg.n = n_classes;

	DecisionSmoother *smoother = ssi_create(DecisionSmoother, "smoother", true);
	smoother->getOptions()->update_ms = 100;
	smoother->getOptions()->decay = 0.05f;
	smoother->getOptions()->speed = 0.2f;
	board->RegisterListener(*smoother, "faked@decision");
	board->RegisterSender(*smoother);

	EventPainter *paint;

	paint = ssi_create_id (EventPainter, 0, "plot");
	paint->getOptions()->type = PaintEvent::TYPE::BAR_POS;
	paint->getOptions()->fix = 1.0f;
	paint->getOptions()->autoscale = false;
	paint->getOptions()->reset = false;
	board->RegisterListener(*paint, "faked@decision");

	paint = ssi_create_id (EventPainter, 0, "plot");
	paint->getOptions()->type = PaintEvent::TYPE::BAR_POS;
	paint->getOptions()->autoscale = false;
	paint->getOptions()->reset = false;
	board->RegisterListener(*paint, smoother->getEventAddress());

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	RunAsThread thread(&fake_tuple_sender, &arg);
	thread.start();
	frame->Wait();
	thread.stop();

	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	ssi_event_destroy(e);

	return true;

}

struct fake_floats_sender_arg {
	ITheEventBoard *b;
	ssi_event_t *e;
	ssi_size_t ms;
	ssi_size_t n;
};

void fake_floats_sender(void *ptr) {
	fake_floats_sender_arg *arg = ssi_pcast(fake_floats_sender_arg, ptr);
	ssi_real_t *floats = ssi_pcast(ssi_real_t, arg->e->ptr);
	for (ssi_size_t i = 0; i < arg->n; i++) {
		floats[i] = ssi_cast(ssi_real_t, ssi_random());
	}
	arg->b->update(*arg->e);
	ssi_sleep(arg->ms);
}

bool ex_events(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	ssi_size_t n_classes = 4;
	ssi_size_t n_samples = 25;
	ssi_size_t n_streams = 1;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f, 0.6f, 0.3f, 0.2f, 0.6f, 0.6f, 0.2f };
	SampleList samples;
	ModelTools::CreateTestSamples(samples, n_classes, n_samples, n_streams, distr, "user");

	ssi_event_t e;
	ssi_event_init(e, SSI_ETYPE_TUPLE, Factory::AddString("faked"), Factory::AddString("features"), 0, 0, 2 * sizeof(ssi_real_t));
	fake_floats_sender_arg arg;
	arg.e = &e;
	arg.b = board;
	arg.ms = 3000;
	arg.n = 2;
	
	SVM *svm = ssi_create(SVM, 0, true);
	Trainer trainer(svm);
	trainer.train(samples);

	Classifier *classifier = ssi_create(Classifier, 0, true);
	classifier->setTrainer(&trainer);
	classifier->getOptions()->console = true;
	board->RegisterListener(*classifier, "features@faked");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();
	
	RunAsThread thread(&fake_floats_sender, &arg);
	thread.start();
	frame->Wait();
	thread.stop();

	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
