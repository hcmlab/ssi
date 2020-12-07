// TestMain.cpp (TEST)
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2020/10/26
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
#include "ssiemosim.h"
#include "MyMouseConsumer.h"
#include "signal/include/ssisignal.h"
#include "control/include/ssicontrol.h"

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

struct params_s {
	
};

ssi_char_t string[SSI_MAX_CHAR];

bool ex_EmoSim(void *args);

#define CONSOLE_WIDTH 600
#define CONSOLE_HEIGHT 1200

int main (int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssiemosim");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL("ssicontrol");

	ssi::Factory::Register(MyMouseConsumer::GetCreateName(), MyMouseConsumer::Create);

	params_s params;

	Exsemble ex;

	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);

	ex.add(ex_EmoSim, &params, "EMOSIM", "...");

	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_EmoSim(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework());
	ITheEventBoard *board = Factory::GetEventBoard();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 100;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::NONE;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create(Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	Selector *select_x = ssi_create(Selector, 0, true);
	select_x->getOptions()->set(0);
	ITransformable *select_x_t = frame->AddTransformer(norm_t, select_x, "0.1s");

	Selector *select_y = ssi_create(Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.1s");

	FunctionalsEventSender *meanx = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(meanx->getOptions()->names, "mean");
	meanx->getOptions()->setSenderName("mouse");
	meanx->getOptions()->setEventName("user_valence");
	frame->AddConsumer(select_x_t, meanx, "0.1s");
	board->RegisterSender(*meanx);

	FunctionalsEventSender *meany = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(meany->getOptions()->names, "mean");
	meany->getOptions()->setSenderName("mouse");
	meany->getOptions()->setEventName("user_arousal");
	frame->AddConsumer(select_y_t, meany, "0.1s");
	board->RegisterSender(*meany);

	SignalPainter *plot = 0;
	ssi_real_t p_size = 10.0f;
	//raw
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mouse");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(cursor_p, plot, "0.1s");
	//norm
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("norm");
	plot->getOptions()->size = p_size;
	frame->AddConsumer(norm_t, plot, "0.1s");

	SocketEventReader* feedback_pos = ssi_create(SocketEventReader, 0, true);
	feedback_pos->getOptions()->setAddress("pos@feedback");
	feedback_pos->getOptions()->setUrl("upd://localhost:2221");
	feedback_pos->getOptions()->osc = false;
	board->RegisterSender(*feedback_pos);

	SocketEventReader* feedback_neg = ssi_create(SocketEventReader, 0, true);
	feedback_neg->getOptions()->setAddress("neg@feedback");
	feedback_neg->getOptions()->setUrl("upd://localhost:2222");
	feedback_neg->getOptions()->osc = false;
	board->RegisterSender(*feedback_neg);

	EventAddress comb_adress;
	comb_adress.setAddress(meanx->getEventAddress());
	comb_adress.setAddress(meany->getEventAddress());
	comb_adress.setAddress(feedback_pos->getEventAddress());
	comb_adress.setAddress(feedback_neg->getEventAddress());

	EmoSim* sim = ssi_create_id(EmoSim, 0, "sim");
	sim->getOptions()->setTitle("EmotionSimulation");
	sim->getOptions()->setSenderName("simulation");
	sim->getOptions()->setEventName("emotion");
	sim->getOptions()->setPosition(CONSOLE_WIDTH, 500, CONSOLE_WIDTH + 200, CONSOLE_WIDTH + 200);
	sim->getOptions()->setUserValenceID("user_valence");
	sim->getOptions()->setUserArousalID("user_arousal");
	sim->getOptions()->setFeedbackPosID("pos");
	sim->getOptions()->setFeedbackNegID("neg");
	ssi_real_t openness = 0.5f;
	ssi_real_t conscientiousness = -0.5f;
	ssi_real_t extraversion = 0.25f;
	ssi_real_t agreeableness = 0.2f;
	ssi_real_t neuroticism = 0.1f;
	sim->getOptions()->openness = openness;
	sim->getOptions()->conscientiousness = conscientiousness;
	sim->getOptions()->extraversion = extraversion;
	sim->getOptions()->agreeableness = agreeableness;
	sim->getOptions()->neuroticism = neuroticism;
	board->RegisterListener(*sim, comb_adress.getAddress());
	board->RegisterSender(*sim);

	SocketEventWriter* emo_socket = ssi_create(SocketEventWriter, 0, true);
	emo_socket->getOptions()->setUrl("upd://localhost:1111");
	emo_socket->getOptions()->xml = true;
	emo_socket->getOptions()->osc = false;
	board->RegisterListener(*emo_socket, "emotion@simulation", 10000);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, "@feedback", 10000);

	ControlSlider* slider_openness = ssi_create(ControlSlider, "slider_openness", true);
	slider_openness->getOptions()->minval = -1.0f;
	slider_openness->getOptions()->maxval = 1.0f;
	slider_openness->getOptions()->setId("sim");
	slider_openness->getOptions()->setName("openness"); //option name!
	slider_openness->getOptions()->setTitle("openness");
	slider_openness->getOptions()->setPos(CONSOLE_WIDTH + 800, 0, CONSOLE_WIDTH, 100);
	frame->AddRunnable(slider_openness);

	ControlSlider* slider_conscientiousness = ssi_create(ControlSlider, "slider_conscientiousness", true);
	slider_conscientiousness->getOptions()->minval = -1.0f;
	slider_conscientiousness->getOptions()->maxval = 1.0f;
	slider_conscientiousness->getOptions()->setId("sim");
	slider_conscientiousness->getOptions()->setName("conscientiousness"); //option name!
	slider_conscientiousness->getOptions()->setTitle("conscientiousness");
	slider_conscientiousness->getOptions()->setPos(CONSOLE_WIDTH + 800, 100, CONSOLE_WIDTH, 100);
	frame->AddRunnable(slider_conscientiousness);

	ControlSlider* slider_extraversion = ssi_create(ControlSlider, "slider_extraversion", true);
	slider_extraversion->getOptions()->minval = -1.0f;
	slider_extraversion->getOptions()->maxval = 1.0f;
	slider_extraversion->getOptions()->setId("sim");
	slider_extraversion->getOptions()->setName("extraversion"); //option name!
	slider_extraversion->getOptions()->setTitle("extraversion");
	slider_extraversion->getOptions()->setPos(CONSOLE_WIDTH + 800, 200, CONSOLE_WIDTH, 100);
	frame->AddRunnable(slider_extraversion);

	ControlSlider* slider_agreeableness = ssi_create(ControlSlider, "slider_agreeableness", true);
	slider_agreeableness->getOptions()->minval = -1.0f;
	slider_agreeableness->getOptions()->maxval = 1.0f;
	slider_agreeableness->getOptions()->setId("sim");
	slider_agreeableness->getOptions()->setName("agreeableness"); //option name!
	slider_agreeableness->getOptions()->setTitle("agreeableness");
	slider_agreeableness->getOptions()->setPos(CONSOLE_WIDTH + 800, 300, CONSOLE_WIDTH, 100);
	frame->AddRunnable(slider_agreeableness);

	ControlSlider* slider_neuroticism = ssi_create(ControlSlider, "slider_neuroticism", true);
	slider_neuroticism->getOptions()->minval = -1.0f;
	slider_neuroticism->getOptions()->maxval = 1.0f;
	slider_neuroticism->getOptions()->setId("sim");
	slider_neuroticism->getOptions()->setName("neuroticism"); //option name!
	slider_neuroticism->getOptions()->setTitle("neuroticism");
	slider_neuroticism->getOptions()->setPos(CONSOLE_WIDTH + 800, 400, CONSOLE_WIDTH, 100);
	frame->AddRunnable(slider_neuroticism);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, 500);
	decorator->add("monitor", CONSOLE_WIDTH + 400, 0, 400, 500);
	
	board->Start();
	frame->Start();
	
	frame->Wait();

	board->Stop();
	frame->Stop();
	
	board->Clear();
	frame->Clear();

	return true;
}

