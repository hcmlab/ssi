// TestMain.cpp (TEST)
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/10/20
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
#include "ssivectorfusion.h"
#include "MyMouseConsumer.h"
#include "signal/include/ssisignal.h"
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
	ssi_real_t FusionSpeed;
	ssi_real_t EventSpeed;
	bool Accel; 
	bool DecayWeights;
};

ssi_char_t string[SSI_MAX_CHAR];

bool ex_TupleConverter(void *args);
bool ex_TupleMap(void *args);
bool ex_TupleScale(void *args);
bool ex_VectorFusionSimple(void *args);
bool ex_VectorFusionSimple_Baseline(void *args);
bool ex_VectorFusionSimple_Baseline_1D(void *args);
bool ex_VectorFusionModality(void *args);
bool ex_VectorFusionGravity(void *args);
bool ex_VectorFusionVA(void *args);
bool ex_VectorFusionWriter(void *args);
bool ex_CombinerVA(void *args);

#define CONSOLE_WIDTH 650
#define CONSOLE_HEIGHT 600

int main (int argc, char *argv[]) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssievent");
	Factory::RegisterDLL ("ssivectorfusion");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssimodel");	
	Factory::RegisterDLL ("ssimouse");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiioput");

	ssi::Factory::Register(MyMouseConsumer::GetCreateName(), MyMouseConsumer::Create);

	params_s params;

	Exsemble ex;

	ex.console(0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);

	ex.add(ex_TupleConverter, 0, "TUPLE CONVERTER", "");
	ex.add(ex_TupleMap, 0, "TUPLE MAP", "");
	ex.add(ex_TupleScale, 0, "TUPLE SCALE", "");	

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = false;
	params.DecayWeights = false;
	ex.add(ex_VectorFusionSimple, &params, "SIMPLE", "accel=false, decay=false");

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = false;
	params.DecayWeights = true;
	ex.add(ex_VectorFusionSimple, &params, "SIMPLE", "accel=false, decay=true");

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = true;
	params.DecayWeights = true;
	ex.add(ex_VectorFusionSimple, &params, "SIMPLE", "accel=true, decay=true");

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = false;
	params.DecayWeights = false;
	ex.add(ex_VectorFusionSimple_Baseline_1D, &params, "BASELINE 1D", "");

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = false;
	params.DecayWeights = false;
	ex.add(ex_VectorFusionSimple_Baseline, &params, "BASELINE", "");

	params.FusionSpeed = 0.1f;
	params.EventSpeed = 0.2f;
	params.Accel = false;
	params.DecayWeights = false;
	ex.add(ex_VectorFusionModality, &params, "MODALITY", "");

	params.FusionSpeed = 0.1f;
	params.Accel = false;
	ex.add(ex_VectorFusionGravity, &params, "GRAVITY", "");

	params.FusionSpeed = 0.1f;
	params.Accel = false;
	ex.add(ex_VectorFusionVA, &params, "VA", "");	
	
	ex.add(ex_VectorFusionWriter, 0, "FUSION WRITER", "");

	ex.add(ex_CombinerVA, 0, "COMBINER VA", "");

	ex.show();
	
	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_TupleConverter(void *args){

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 100;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setAddress("mean@mouse");	
	frame->AddConsumer(norm_t, mean, "5.0s");
	board->RegisterSender(*mean);
		
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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.1s");
	board->RegisterSender(*ezero);

	EventAddress ea;
	ea.setAddress(mean->getEventAddress());
	ea.setAddress(ezero->getEventAddress());

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setAddress("tuple@converter");	
	board->RegisterListener(*converter, ea.getAddress());
	board->RegisterSender(*converter);
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, 0, 10000);
	
	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();	
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_TupleMap(void *args){

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_pcast(TheEventBoard, board)->getOptions()->update = 100;

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("mouse");
	mean->getOptions()->setEventName("mean");
	frame->AddConsumer(norm_t, mean, "5.0s");
	board->RegisterSender(*mean);

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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.1s");
	board->RegisterSender(*ezero);

	TupleConverter *converter_klick = ssi_create(TupleConverter, 0, true);
	converter_klick->getOptions()->setSenderName("klick_converter");
	converter_klick->getOptions()->setEventName("klick_vector");
	board->RegisterListener(*converter_klick, ezero->getEventAddress());
	board->RegisterSender(*converter_klick);

	TupleMap *vec_map_klick = ssi_create (TupleMap, 0, true);
	vec_map_klick->getOptions()->dimension = 2;
	vec_map_klick->getOptions()->mapped = true;
	vec_map_klick->getOptions()->setSenderName("klick_mapper");
	vec_map_klick->getOptions()->setEventName("vector");
	ssi_strcpy (vec_map_klick->getOptions()->mapping, "1.0,-1.0");
	board->RegisterListener(*vec_map_klick, converter_klick->getEventAddress());
	board->RegisterSender(*vec_map_klick);

	TupleConverter *converter_mean = ssi_create(TupleConverter, 0, true);
	converter_mean->getOptions()->setSenderName("mean_converter");
	converter_mean->getOptions()->setEventName("mean_vector");
	board->RegisterListener(*converter_mean, mean->getEventAddress());
	board->RegisterSender(*converter_mean);

	TupleMap *vec_map_mean = ssi_create (TupleMap, 0, true);
	vec_map_mean->getOptions()->dimension = 2;
	vec_map_mean->getOptions()->mapped = true;
	vec_map_mean->getOptions()->setSenderName("mean_mapper");
	vec_map_mean->getOptions()->setEventName("vector");
	ssi_strcpy (vec_map_mean->getOptions()->mapping, "-1.0,-1.0");
	board->RegisterListener(*vec_map_mean, converter_mean->getEventAddress());
	board->RegisterSender(*vec_map_mean);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, 0, 9000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();
	
	return true;
}

bool ex_TupleScale(void *args){

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, 0, 100000);

	Mouse *mouse = ssi_create (Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, 0, true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	MyMouseConsumer *mouse_c = ssi_create(MyMouseConsumer, 0, true);
	board->RegisterListener(*mouse_c, ezero->getEventAddress());
	board->RegisterSender(*mouse_c);
	
	TupleScale *vec_send = ssi_create (TupleScale, 0, true);
	vec_send->getOptions()->dimension = 1;
	board->RegisterListener(*vec_send, mouse_c->getEventAddress());
	board->RegisterSender(*vec_send);

	VectorFusion *fusion = ssi_create_id(VectorFusion, 0, "fusion");
	fusion->getOptions()->dimension = 1;
	fusion->getOptions()->fusionspeed = 0.01f;
	fusion->getOptions()->threshold = 0.01f;
	fusion->getOptions()->print = false;
	fusion->getOptions()->paint = true;
	board->RegisterListener(*fusion, vec_send->getEventAddress());

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionGravity(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.5s");

	Selector *select_y = ssi_create (Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.5s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("mouse");
	mean->getOptions()->setEventName("tuple");
	frame->AddConsumer(select_y_t, mean, "0.5s");
	board->RegisterSender(*mean);
		
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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 1;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("mouse");
	vec_send->getOptions()->setEventName("baseline");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	fusion_adress.setAddress(mean->getEventAddress());
	VectorFusionGravity *fusion = ssi_create_id(VectorFusionGravity, 0, "fusion");
	fusion->getOptions()->dimension = 1;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = 0.5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->threshold = 0.05f;
	fusion->getOptions()->print = true;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->update_ms = 500;
	fusion->getOptions()->setPath("fusion.modality");
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, mean->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionVA(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_pcast(TheEventBoard, board)->getOptions()->update = 100;

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("mouse");
	mean->getOptions()->setEventName("mean");
	frame->AddConsumer(norm_t, mean, "0.1s");
	board->RegisterSender(*mean);
			
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
		
	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, mean->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 2;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("mouse");
	vec_send->getOptions()->setEventName("tuple");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0,-1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	//fusion_adress.setAddress(mean->getEventAddress());
	VectorFusionVA *fusion = ssi_create_id(VectorFusionVA, 0, "fusion");
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = 0.5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->threshold = 0.05f;
	fusion->getOptions()->print = false;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->update_ms = 100;
	fusion->getOptions()->setPath("fusion.modality");
	fusion->getOptions()->setTitle("VA Fusion");
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, fusion->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionModality(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.5s");

	Selector *select_y = ssi_create (Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.5s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("mouse");
	mean->getOptions()->setEventName("baseline");
	frame->AddConsumer(select_y_t, mean, "0.5s");
	board->RegisterSender(*mean);
		
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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 1;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("mouse");
	vec_send->getOptions()->setEventName("tuple");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	fusion_adress.setAddress(mean->getEventAddress());
	VectorFusionModality *fusion = ssi_create_id(VectorFusionModality, 0, "fusion");
	fusion->getOptions()->dimension = 1;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = 0.5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->threshold = 0.05f;
	fusion->getOptions()->print = false;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->update_ms = 500;
	fusion->getOptions()->setPath("fusion.modality");
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, mean->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionSimple_Baseline_1D(void *args){

	params_s *params = ssi_pcast(params_s, args);
	
	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	Selector *select_y = ssi_create (Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.1s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("baseline_sender");
	mean->getOptions()->setEventName("baseline");
	frame->AddConsumer(select_y_t, mean, "0.1s");
	board->RegisterSender(*mean);
		
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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 1;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("Mouse");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	fusion_adress.setAddress(mean->getEventAddress());
	VectorFusion *fusion = ssi_create_id(VectorFusion, 0, "fusion");
	fusion->getOptions()->dimension = 1;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = .5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->eventspeed = params->EventSpeed;
	fusion->getOptions()->threshold = .05f;
	fusion->getOptions()->print = true;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->decay_weights = params->DecayWeights;
	fusion->getOptions()->update_ms = 500;
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, mean->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionSimple_Baseline(void *args){

	params_s *params = ssi_pcast(params_s, args);
	
	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	FunctionalsEventSender *mean = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean->getOptions()->names, "mean");
	mean->getOptions()->setSenderName("baseline_sender");
	mean->getOptions()->setEventName("baseline");
	frame->AddConsumer(norm_t, mean, "0.1s");
	board->RegisterSender(*mean);
	
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
	
	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 2;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("Mouse");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0,1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	fusion_adress.setAddress(mean->getEventAddress());
	VectorFusion *fusion = ssi_create_id(VectorFusion, 0, "fusion");
	fusion->getOptions()->dimension = 2;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = .5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->eventspeed = params->EventSpeed;
	fusion->getOptions()->threshold = .05f;
	fusion->getOptions()->print = false;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->decay_weights = params->DecayWeights;
	fusion->getOptions()->update_ms = 500;	
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, mean->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionSimple(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 500;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create (Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.1s");

	Selector *select_x = ssi_create (Selector, 0, true);
	select_x->getOptions()->set(0);
	ITransformable *select_x_t = frame->AddTransformer(norm_t, select_x, "0.1s");

	FunctionalsEventSender *mean_x = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean_x->getOptions()->names, "mean");
	mean_x->getOptions()->setSenderName("MeanX");
	frame->AddConsumer(select_x_t, mean_x, "0.1s");
	board->RegisterSender(*mean_x);

	Selector *select_y = ssi_create (Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.1s");

	FunctionalsEventSender *mean_y = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(mean_y->getOptions()->names, "mean");
	mean_y->getOptions()->setSenderName("MeanY");
	frame->AddConsumer(select_y_t, mean_y, "0.1s");
	board->RegisterSender(*mean_y);

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
	//select
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("select(x)");
	plot->getOptions()->size = p_size;		
	frame->AddConsumer(select_x_t, plot, "0.1s");
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("select(y)");
	plot->getOptions()->size = p_size;		
	frame->AddConsumer(select_y_t, plot, "0.1s");

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 2;
	vec_send->getOptions()->mapped = true;
	vec_send->getOptions()->setSenderName("Mouse");
	ssi_strcpy (vec_send->getOptions()->mapping, "1.0,1.0");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	EventAddress fusion_adress;
	fusion_adress.setAddress(vec_send->getEventAddress());
	VectorFusion *fusion = ssi_create_id(VectorFusion, 0, "fusion");
	fusion->getOptions()->dimension = 2;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion->getOptions()->gradient = .5f;
	fusion->getOptions()->fusionspeed = params->FusionSpeed;
	fusion->getOptions()->eventspeed = params->EventSpeed;
	fusion->getOptions()->threshold = .05f;
	fusion->getOptions()->print = true;
	fusion->getOptions()->paint = true;
	fusion->getOptions()->accelerate = params->Accel;
	fusion->getOptions()->decay_weights = params->DecayWeights;
	fusion->getOptions()->update_ms = 500;
	board->RegisterListener(*fusion, fusion_adress.getAddress());
	board->RegisterSender(*fusion);
		
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, vec_send->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_VectorFusionWriter(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework ());
	TheEventBoard *board = ssi_pcast(TheEventBoard, Factory::GetEventBoard());
	board->getOptions()->update = 100;

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);	
	frame->AddSensor(mouse);

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->mindur = 0.2;
	ezero->getOptions()->setAddress("click@mouse");
	frame->AddConsumer(button_p, ezero, "0.25s");
	board->RegisterSender(*ezero);

	TupleConverter *converter = ssi_create(TupleConverter, 0, true);
	converter->getOptions()->setSenderName("converter");
	converter->getOptions()->setEventName("tuple");
	board->RegisterListener(*converter, ezero->getEventAddress());
	board->RegisterSender(*converter);

	TupleMap *vec_send = ssi_create (TupleMap, 0, true);
	vec_send->getOptions()->dimension = 2;
	vec_send->getOptions()->mapped = true;
	ssi_strcpy (vec_send->getOptions()->mapping, "0.5, -0.5");
	board->RegisterListener(*vec_send, converter->getEventAddress());
	board->RegisterSender(*vec_send);

	VectorFusion *fusion = ssi_create_id(VectorFusion, 0, "fusion");
	fusion->getOptions()->dimension = 2;
	fusion->getOptions()->update_ms = 100;
	fusion->getOptions()->decay_type = EVector::DECAY_TYPE_EXP;
	fusion->getOptions()->gradient = 0.7f;
	fusion->getOptions()->fusionspeed = 0.075f;
	fusion->getOptions()->print = false;
	fusion->getOptions()->paint = true;
	board->RegisterListener(*fusion, vec_send->getEventAddress());
	board->RegisterSender(*fusion);
	
	VectorFusionWriter *writer = ssi_create (VectorFusionWriter, 0, true);
	writer->getOptions()->setPath("test");
	writer->getOptions()->f_update_ms = fusion->getUpdateRate();
	writer->getOptions()->dim = fusion->getDim();
	board->RegisterListener(*writer, fusion->getEventAddress());
	
	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, fusion->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

bool ex_CombinerVA(void *args){

	params_s *params = ssi_pcast(params_s, args);

	TheFramework *frame = ssi_pcast(TheFramework, Factory::GetFramework());
	ITheEventBoard *board = Factory::GetEventBoard();
	ssi_pcast(TheEventBoard, board)->getOptions()->update = 100;

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->mask = Mouse::LEFT;
	mouse->getOptions()->flip = true;
	mouse->getOptions()->scale = false;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	ITransformable *button_p = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	frame->AddSensor(mouse);

	Normalize *norm = ssi_create(Normalize, 0, true);
	norm->getOptions()->maxval = 1.0f;
	norm->getOptions()->minval = -1.0f;
	ITransformable *norm_t = frame->AddTransformer(cursor_p, norm, "0.5s");

	Selector *select_x = ssi_create(Selector, 0, true);
	select_x->getOptions()->set(0);
	ITransformable *select_x_t = frame->AddTransformer(norm_t, select_x, "0.5s");

	Selector *select_y = ssi_create(Selector, 0, true);
	select_y->getOptions()->set(1);
	ITransformable *select_y_t = frame->AddTransformer(norm_t, select_y, "0.5s");

	FunctionalsEventSender *meanx = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(meanx->getOptions()->names, "mean");
	meanx->getOptions()->setSenderName("mouse");
	meanx->getOptions()->setEventName("valence");
	frame->AddConsumer(select_x_t, meanx, "0.5s");
	board->RegisterSender(*meanx);

	FunctionalsEventSender *meany = ssi_create(FunctionalsEventSender, 0, true);
	ssi_strcpy(meany->getOptions()->names, "mean");
	meany->getOptions()->setSenderName("mouse");
	meany->getOptions()->setEventName("arousal");
	frame->AddConsumer(select_y_t, meany, "0.5s");
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

	EventAddress fusion_x_adress;
	fusion_x_adress.setAddress(meanx->getEventAddress());
	VectorFusionGravity *fusion_x = ssi_create_id(VectorFusionGravity, 0, "fusion_x");
	fusion_x->getOptions()->dimension = 1;
	fusion_x->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion_x->getOptions()->gradient = 0.5f;
	fusion_x->getOptions()->fusionspeed = 1.0f;
	fusion_x->getOptions()->threshold = 0.05f;
	fusion_x->getOptions()->print = false;
	fusion_x->getOptions()->paint = true;
	fusion_x->getOptions()->accelerate = false;
	fusion_x->getOptions()->update_ms = 100;
	fusion_x->getOptions()->eventspeed = 0.5f;
	fusion_x->getOptions()->setEventName("valence");
	fusion_x->getOptions()->setAxisCaption("Valence");
	board->RegisterListener(*fusion_x, fusion_x_adress.getAddress());
	board->RegisterSender(*fusion_x);

	EventAddress fusion_y_adress;
	fusion_y_adress.setAddress(meany->getEventAddress());
	VectorFusionGravity *fusion_y = ssi_create_id(VectorFusionGravity, 0, "fusion_y");
	fusion_y->getOptions()->dimension = 1;
	fusion_y->getOptions()->decay_type = EVector::DECAY_TYPE_LIN;
	fusion_y->getOptions()->gradient = 0.5f;
	fusion_y->getOptions()->fusionspeed = 1.0f;
	fusion_y->getOptions()->threshold = 0.05f;
	fusion_y->getOptions()->print = false;
	fusion_y->getOptions()->paint = true;
	fusion_y->getOptions()->accelerate = false;
	fusion_y->getOptions()->update_ms = 100;
	fusion_y->getOptions()->eventspeed = 0.5f;
	fusion_y->getOptions()->setEventName("arousal");
	fusion_y->getOptions()->setAxisCaption("Arousal");
	board->RegisterListener(*fusion_y, fusion_y_adress.getAddress());
	board->RegisterSender(*fusion_y);

	EventAddress comb_adress;
	comb_adress.setAddress(fusion_x->getEventAddress());
	comb_adress.setAddress(fusion_y->getEventAddress());
	CombinerVA *comb = ssi_create_id(CombinerVA, 0, "comb");
	comb->getOptions()->paint = true;
	board->RegisterListener(*comb, comb_adress.getAddress());
	board->RegisterSender(*comb);

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	monitor->getOptions()->all = true;
	board->RegisterListener(*monitor, comb->getEventAddress(), 10000);

	decorator->add("plot*", CONSOLE_WIDTH, 0, 400, CONSOLE_HEIGHT);
	decorator->add("fusion_x,fusion_y,comb,monitor", CONSOLE_WIDTH + 400, 0, 400, CONSOLE_HEIGHT);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}

