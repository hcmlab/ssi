// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 9/3/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ssi.h"
#include "..\\..\\include\\ssigenericserial.h"

#include "..\\..\\..\\signal\\include\\MvgMedian.h"
#include "..\\..\\..\\signal\\include\\Derivative.h"
#include "..\\..\\..\\signal\\include\\MvgAvgVar.h"
#include "..\\..\\..\\signal\\include\\MvgMinMax.h"
#include "..\\..\\..\\signal\\include\\Expression.h"
#include "..\\..\\..\\signal\\include\\MvgPeakGate.h"

#include "..\\..\\..\\xmpp\\include\\ssixmpp.h"

using namespace ssi;

ssi_char_t sstring[SSI_MAX_CHAR];

void ex_arduino();
void ex_wax9UnstableSamplerate();
void ex_wax9();
void ex_capMatQuiteStableSampleratePlate();
void ex_capMatQuiteStableSamplerate();
void ex_capCushionMPR121();

void ex_arduinoXMPP();

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

		ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

		Factory::RegisterDLL("ssiframe");
		Factory::RegisterDLL("ssigraphic");
		Factory::RegisterDLL("ssievent");
		Factory::RegisterDLL("ssiioput");
		Factory::RegisterDLL("ssisignal");
		Factory::RegisterDLL("ssigeneric_serial");

		{
			//ex_arduino ();
			//ex_wax9 ();	
			//ex_wax9UnstableSamplerate ();
			//ex_capMatQuiteStableSampleratePlate ();
			//ex_capMatQuiteStableSamplerate();
			//ex_arduinoXMPP();

			ex_capCushionMPR121();
		}

		ssi_print("\n\n\tpress a key to quit\n");
		getchar();

		Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

/*	expects 7 values in the following format:
val1,val2,val3,val4,val5,val6,val7\n
val1,val2,val3,val4,val5,val6,val7\n
...
*/
void ex_arduino() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);
	genSer->getOptions()->port = 10;
	genSer->getOptions()->baud = 9600;
	genSer->getOptions()->dim = 7;
	genSer->getOptions()->sr = 70;
	genSer->getOptions()->size = 0.02;
	genSer->getOptions()->separator = ',';
	ITransformable *genSer_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();

	ssi_print("press enter to continue\n");
	getchar();

	frame->Stop();
	frame->Clear();

}

/*
* WARNING THIS SAMPLE DISABLES THE SAMPLE RATE CHECK IN SSI FOR THIS SENSOR! USE WITH CAUTION!
*/

void ex_wax9UnstableSamplerate() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);

	genSer->getOptions()->port = 12;
	genSer->getOptions()->baud = 9600;
	genSer->getOptions()->dim = 7;	//14
	genSer->getOptions()->sr = 50;
	genSer->getOptions()->size = 0.02;
	genSer->getOptions()->separator = ',';
	genSer->getOptions()->setStartCMD("stream\r");
	ITransformable *genSer_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME, 0, "10.0s", 0, 0);
	frame->AddSensor(genSer);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	ssi_wrn("THIS SAMPLE DISABLES THE SAMPLE RATE CHECK IN SSI FOR THIS SENSOR! USE WITH CAUTION!");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();

	ssi_print("press enter to continue\n");
	getchar();

	frame->Stop();
	frame->Clear();

}

void ex_wax9() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);
	genSer->getOptions()->port = 5;
	genSer->getOptions()->baud = 9600;
	genSer->getOptions()->dim = 14;
	genSer->getOptions()->sr = 50;
	genSer->getOptions()->size = 0.02;
	genSer->getOptions()->separator = ',';
	genSer->getOptions()->setStartCMD("stream\r");
	ITransformable *genSer_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();

	ssi_print("press enter to continue\n");
	getchar();

	frame->Stop();
	frame->Clear();

}

void ex_capMatQuiteStableSampleratePlate() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);

	genSer->getOptions()->port = 8;
	genSer->getOptions()->baud = 9600;
	genSer->getOptions()->dim = 2;	//14
	genSer->getOptions()->sr = 72;
	genSer->getOptions()->size = 0.02;
	genSer->getOptions()->separator = ',';
	ITransformable *genSer_all_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	Selector *genSer_sel = ssi_create(Selector, 0, true);
	genSer_sel->getOptions()->set(0);
	ITransformable * genSer_p = frame->AddTransformer(genSer_all_p, genSer_sel, "1");

	Selector *genSer_sel2 = ssi_create(Selector, 0, true);
	genSer_sel2->getOptions()->set(1);
	ITransformable * genSer2_p = frame->AddTransformer(genSer_all_p, genSer_sel2, "1");

	//remove peaks
	MvgMedian *mvgMedian = ssi_create(MvgMedian, 0, true);
	mvgMedian->getOptions()->win = 0.25;
	ITransformable *filter_mvgMed_t = frame->AddTransformer(genSer_p, mvgMedian, "0.2s");

	MvgMedian *mvgMedian2 = ssi_create(MvgMedian, 0, true);
	mvgMedian2->getOptions()->win = 0.25;
	ITransformable *filter_mvgMed2_t = frame->AddTransformer(genSer2_p, mvgMedian2, "0.2s");

	//average signal with removed peaks
	MvgAvgVar *mvgAvg = ssi_create(MvgAvgVar, 0, true);
	mvgAvg->getOptions()->win = 0.25;
	mvgAvg->getOptions()->format = MvgAvgVar::AVG;
	mvgAvg->getOptions()->method = MvgAvgVar::MOVING;
	ITransformable *filter_mvgAvg_t = frame->AddTransformer(filter_mvgMed_t, mvgAvg, "0.2s");

	//average signal with removed peaks
	MvgAvgVar *mvgAvg2 = ssi_create(MvgAvgVar, 0, true);
	mvgAvg2->getOptions()->win = 0.25;
	mvgAvg2->getOptions()->format = MvgAvgVar::AVG;
	mvgAvg2->getOptions()->method = MvgAvgVar::MOVING;
	ITransformable *filter_mvgAvg2_t = frame->AddTransformer(filter_mvgMed2_t, mvgAvg2, "0.2s");

	//scale 32 bit signal to float
	Expression *expr = ssi_create(Expression, 0, true);
	expr->getOptions()->single = true;
	expr->getOptions()->setExpression("d / 65535");
	ITransformable *express_t = frame->AddTransformer(filter_mvgAvg_t, expr, "0.2s");

	Expression *expr2 = ssi_create(Expression, 0, true);
	expr2->getOptions()->single = true;
	expr2->getOptions()->setExpression("d / 65535");
	ITransformable *express2_t = frame->AddTransformer(filter_mvgAvg2_t, expr2, "0.2s");

	//peak gate on float values
	MvgPeakGate *moving = ssi_create(MvgPeakGate, 0, true);
	moving->getOptions()->thres = MvgPeakGate::FIX;
	moving->getOptions()->win = 0.25;		//0.25
	moving->getOptions()->fix = 0.340f;		//0.01	372
	moving->getOptions()->method = MvgPeakGate::MOVING;
	ITransformable *filter_peakGate_t = frame->AddTransformer(express_t, moving, "0.2s");

	MvgPeakGate *moving2 = ssi_create(MvgPeakGate, 0, true);
	moving2->getOptions()->thres = MvgPeakGate::FIX;
	moving2->getOptions()->win = 0.25;		//0.25
	moving2->getOptions()->fix = 0.100f;		//0.01	115
	moving2->getOptions()->method = MvgPeakGate::MOVING;
	ITransformable *filter_peakGate2_t = frame->AddTransformer(express2_t, moving2, "0.2s");

	Merge *merge = ssi_create(Merge, 0, true);
	merge->getOptions()->dims = 1;
	ITransformable * merge_p = frame->AddTransformer(filter_peakGate_t, 1, &filter_peakGate2_t, merge, "1");

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->hard = false;
	ezero->getOptions()->eager = true;
	ezero->getOptions()->hangin = 0.2;
	ezero->getOptions()->hangout = 0.2;
	ezero->getOptions()->maxdur = 3600;
	ezero->getOptions()->setEvent("Presence");
	frame->AddConsumer(merge_p, ezero, "1");
	board->RegisterSender(*ezero);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_all_p, plot, "1");

	SignalPainter *plotMvgMed = 0;
	plotMvgMed = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgMed->getOptions()->setTitle("MvgMedian");
	plotMvgMed->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgMed_t, plotMvgMed, "1");

	SignalPainter *plotMvgMed2 = 0;
	plotMvgMed2 = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgMed2->getOptions()->setTitle("MvgMedian 2");
	plotMvgMed2->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgMed2_t, plotMvgMed2, "1");

	SignalPainter *plotMvgAvg = 0;
	plotMvgAvg = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgAvg->getOptions()->setTitle("MvgAvg of MvgMedian");
	plotMvgAvg->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgAvg_t, plotMvgAvg, "1");

	SignalPainter *plotMvgAvg2 = 0;
	plotMvgAvg2 = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgAvg2->getOptions()->setTitle("MvgAvg of MvgMedian 2");
	plotMvgAvg2->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgAvg2_t, plotMvgAvg2, "1");

	SignalPainter *plotscale = 0;
	plotscale = ssi_create_id (SignalPainter, 0, "plot");
	plotscale->getOptions()->setTitle("Scale of(MvgAvg of MvgMedian)");
	plotscale->getOptions()->size = 10.0;
	frame->AddConsumer(express_t, plotscale, "1");

	SignalPainter *plotscale2 = 0;
	plotscale2 = ssi_create_id (SignalPainter, 0, "plot");
	plotscale2->getOptions()->setTitle("Scale of(MvgAvg of MvgMedian) 2");
	plotscale2->getOptions()->size = 10.0;
	frame->AddConsumer(express2_t, plotscale2, "1");

	SignalPainter *plotPeak = 0;
	plotPeak = ssi_create_id (SignalPainter, 0, "plot");
	plotPeak->getOptions()->setTitle("Peak Gate of Scaled(MvgAvg of MvgMedian) 1 and 2");
	plotPeak->getOptions()->size = 10.0;
	frame->AddConsumer(merge_p, plotPeak, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

}

void ex_capMatQuiteStableSamplerate() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);

	genSer->getOptions()->port = 8;
	genSer->getOptions()->baud = 9600;
	genSer->getOptions()->dim = 2;	//14
	genSer->getOptions()->sr = 65;
	genSer->getOptions()->size = 0.02;
	genSer->getOptions()->separator = ',';
	ITransformable *genSer_all_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	Selector *genSer_sel = ssi_create(Selector, 0, true);
	genSer_sel->getOptions()->set(0);
	ITransformable * genSer_p = frame->AddTransformer(genSer_all_p, genSer_sel, "1");

	//remove peaks
	MvgMedian *mvgMedian = ssi_create(MvgMedian, 0, true);
	mvgMedian->getOptions()->win = 0.25;
	ITransformable *filter_mvgMed_t = frame->AddTransformer(genSer_p, mvgMedian, "0.2s");

	//average signal with removed peaks
	MvgAvgVar *mvgAvg = ssi_create(MvgAvgVar, 0, true);
	mvgAvg->getOptions()->win = 0.25;
	mvgAvg->getOptions()->format = MvgAvgVar::AVG;
	mvgAvg->getOptions()->method = MvgAvgVar::MOVING;
	ITransformable *filter_mvgAvg_t = frame->AddTransformer(filter_mvgMed_t, mvgAvg, "0.2s");

	MvgMinMax *mvgMin = ssi_create(MvgMinMax, 0, true);
	mvgMin->getOptions()->win = 0.25;
	mvgMin->getOptions()->format = MvgMinMax::MIN;
	mvgMin->getOptions()->method = MvgMinMax::MOVING;
	ITransformable *filter_mvgMin_t = frame->AddTransformer(filter_mvgAvg_t, mvgMin, "0.2s");

	//scale 32 bit signal to float
	Expression *expr = ssi_create(Expression, 0, true);
	expr->getOptions()->single = true;
	expr->getOptions()->setExpression("d / 65535");
	ITransformable *express_t = frame->AddTransformer(filter_mvgAvg_t, expr, "0.2s");

	//peak gate on float values
	MvgPeakGate *moving = ssi_create(MvgPeakGate, 0, true);
	moving->getOptions()->thres = MvgPeakGate::FIXAVGSTD;
	moving->getOptions()->win = 0.25;		//0.25
	moving->getOptions()->fix = 0.005f;		//0.01
	moving->getOptions()->method = MvgPeakGate::MOVING;
	ITransformable *filter_peakGate_t = frame->AddTransformer(express_t, moving, "0.2s");

	ZeroEventSender *ezero = ssi_create(ZeroEventSender, "ezero", true);
	ezero->getOptions()->eager = true;
	ezero->getOptions()->hangin = 0.2;
	ezero->getOptions()->hangout = 0.2;
	ezero->getOptions()->maxdur = 3600;
	ezero->getOptions()->setEvent("Presence");
	frame->AddConsumer(filter_peakGate_t, ezero, "1");
	board->RegisterSender(*ezero);

	Derivative *derv = ssi_create(Derivative, 0, true);
	derv->getOptions()->set(Derivative::D1ST);
	ITransformable *filter_derv_t = frame->AddTransformer(filter_mvgAvg_t, derv, "0.2s");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	SignalPainter *plotMvgMed = 0;
	plotMvgMed = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgMed->getOptions()->setTitle("MvgMedian");
	plotMvgMed->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgMed_t, plotMvgMed, "1");

	SignalPainter *plotMvgAvg = 0;
	plotMvgAvg = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgAvg->getOptions()->setTitle("MvgAvg of MvgMedian");
	plotMvgAvg->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgAvg_t, plotMvgAvg, "1");

	SignalPainter *plotMvgMin = 0;
	plotMvgMin = ssi_create_id (SignalPainter, 0, "plot");
	plotMvgMin->getOptions()->setTitle("Min of(MvgAvg of MvgMedian)");
	plotMvgMin->getOptions()->size = 10.0;
	frame->AddConsumer(filter_mvgMin_t, plotMvgMin, "1");

	SignalPainter *plotintegral = 0;
	plotintegral = ssi_create_id (SignalPainter, 0, "plot");
	plotintegral->getOptions()->setTitle("Scale of(MvgAvg of MvgMedian)");
	plotintegral->getOptions()->size = 10.0;
	frame->AddConsumer(express_t, plotintegral, "1");

	SignalPainter *plotDerv = 0;
	plotDerv = ssi_create_id (SignalPainter, 0, "plot");
	plotDerv->getOptions()->setTitle("Derivative");
	plotDerv->getOptions()->size = 10.0;
	frame->AddConsumer(filter_derv_t, plotDerv, "1");

	SignalPainter *plotPeak = 0;
	plotPeak = ssi_create_id (SignalPainter, 0, "plot");
	plotPeak->getOptions()->setTitle("Peak Gate of Scaled(MvgAvg of MvgMedian)");
	plotPeak->getOptions()->size = 10.0;
	frame->AddConsumer(filter_peakGate_t, plotPeak, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

}

//CARE complex test
void ex_arduinoXMPP() {

	Factory::RegisterDLL("ssixmpp");

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard();

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);
	genSer->getOptions()->port = 7;
	genSer->getOptions()->baud = 115200;
	genSer->getOptions()->dim = 6;
	genSer->getOptions()->sr = 4;
	genSer->getOptions()->size = 0.5;
	genSer->getOptions()->separator = ';';
	ITransformable *genSer_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	XMPP *xmpp = ssi_create(XMPP, 0, true);
	xmpp->getOptions()->setJID("test@andy");
	xmpp->getOptions()->setPw("123");

	//xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_JSON;
	xmpp->getOptions()->msgFormat = XMPP::MESSAGEBODYFORMAT_TEMPLATE;

	xmpp->getOptions()->setSenderName("XMPP");
	xmpp->getOptions()->setEventName("message");

	xmpp->getOptions()->pubNode = false;
	xmpp->getOptions()->setRecip("discrete_data_interpreter@andy");

	board->RegisterListener(*xmpp);

	//IR presence
	Selector *select_IR = ssi_create(Selector, 0, true);
	select_IR->getOptions()->set(0);
	ITransformable * select_IR_t = frame->AddTransformer(genSer_p, select_IR, "1");

	ThresEventSender *ethres = ssi_create(ThresEventSender, 0, true);
	ethres->getOptions()->eall = true;
	ethres->getOptions()->eager = true;
	ethres->getOptions()->hangin = 2;
	ethres->getOptions()->hangout = 5;
	ethres->getOptions()->thres = 250;
	ethres->getOptions()->maxdur = 1800;	//max 30 minutes
	ethres->getOptions()->setEvent("presence");
	ethres->getOptions()->setSender("userIRpresence");
	ethres->getOptions()->empty = false;
	ethres->getOptions()->setString("{thres:250;}");
	frame->AddConsumer(select_IR_t, ethres, "1");
	board->RegisterSender(*ethres);

	//Temperature
	Selector *select_Temp = ssi_create(Selector, 0, true);
	select_Temp->getOptions()->set(1);
	ITransformable * select_Temp_t = frame->AddTransformer(genSer_p, select_Temp, "1");

	TupleEventSender *fles = ssi_create(TupleEventSender, 0, true);
	fles->getOptions()->setSenderName("temperature");
	fles->getOptions()->setEventName("val");
	fles->getOptions()->mean = false;
	fles->getOptions()->valchanges = true;
	frame->AddConsumer(select_Temp_t, fles, "1");
	board->RegisterSender(*fles);

	//Humidity
	Selector *select_Hum = ssi_create(Selector, 0, true);
	select_Hum->getOptions()->set(2);
	ITransformable * select_Hum_t = frame->AddTransformer(genSer_p, select_Hum, "1");

	TupleEventSender *flesHum = ssi_create(TupleEventSender, 0, true);
	flesHum->getOptions()->setSenderName("humidity");
	flesHum->getOptions()->setEventName("val");
	flesHum->getOptions()->mean = false;
	flesHum->getOptions()->valchanges = true;
	frame->AddConsumer(select_Hum_t, flesHum, "1");
	board->RegisterSender(*flesHum);

	//Magnetometer
	/*Selector *select_Mag = ssi_create(Selector, 0, true);
	ssi_size_t indices[3] = { 3, 4, 5 };
	select_Mag->getOptions()->set(3, indices);
	ITransformable * select_Mag_t = frame->AddTransformer(genSer_p, select_Mag, "1");

	TupleEventSender *flesMag = ssi_create(TupleEventSender, 0, true);
	flesMag->getOptions()->setSenderName("magnetometer");
	flesMag->getOptions()->setEventName("val");
	flesMag->getOptions()->mean = false;
	flesMag->getOptions()->valchanges = true;
	frame->AddConsumer(select_Mag_t, flesMag, "1");
	board->RegisterSender(*flesMag);*/

	SignalPainter *plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor);

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	board->Start();
	frame->Start();

	frame->Wait();

	frame->Stop();
	frame->Clear();

}

void ex_capCushionMPR121() {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	GenericSerial *genSer = ssi_create(GenericSerial, 0, true);
	genSer->getOptions()->port = 14;
	genSer->getOptions()->baud = 115200;
	genSer->getOptions()->dim = 12;
	genSer->getOptions()->sr = 9;
	genSer->getOptions()->skipLinesAfterStart = 20;
	genSer->getOptions()->size = 1;
	genSer->getOptions()->separator = ',';
	ITransformable *genSer_p = frame->AddProvider(genSer, SSI_GENERICSERIAL_PROVIDER_NAME);
	frame->AddSensor(genSer);

	SignalPainter *plot = 0;
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("serial values");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(genSer_p, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();

	ssi_print("press enter to continue\n");
	getchar();

	frame->Stop();
	frame->Clear();

}
