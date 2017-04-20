// Main.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/06/05
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
#include "ssipython.h"
#include "ssiml/include/ssiml.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_print(void *args);
bool ex_event(void *args);
bool ex_clone(void *args);
bool ex_energy(void *args);
bool ex_diff(void *args);
bool ex_pearson(void *args);
bool ex_imggray(void *args);
bool ex_imgavg(void *args);
bool ex_imgplot(void *args);
bool ex_sensor(void *args);
bool ex_offline(void *args);
bool ex_model(void *args);

int main () 
{

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL("frame");
	Factory::RegisterDLL("event");
	Factory::RegisterDLL("mouse");
	Factory::RegisterDLL("graphic");
	Factory::RegisterDLL("ioput");
	Factory::RegisterDLL("python");

	ssi_type_t type = SSI_INT;

#if SSI_RANDOM_LEGACY_FLAG	
	ssi_random_seed();
#endif

	Exsemble ex;
	ex.console(0, 0, 650, 600);		
	ex.add(ex_print, 0, "PRINT", "Print input stream(s).");
	ex.add(ex_event, 0, "EVENT", "Receive and send events.");
	ex.add(ex_clone, 0, "CLONE", "Copy input to output.");
	ex.add(ex_energy, 0, "ENERGY", "Calculate energy.");
	ex.add(ex_diff, 0, "DIFF", "Calculate 1st derivative.");
#ifndef DEBUG
	ex.add(ex_pearson, 0, "PPMCC", "Calculate Pearson product-moment correlation coefficient (requires numpy and scipy).");	
	ex.add(ex_imggray, 0, "IMG RGB2GRAY", "Convert rgb image to grayscale (requires numpy and opencv).");
	ex.add(ex_imgavg, 0, "IMG AVG", "Calculate average of each channel in an image (requires numpy and opencv).");
	ex.add(ex_imgplot, 0, "IMG PLOT", "Plot image (requires numpy and opencv).");
#endif	
	ex.add(ex_sensor, 0, "SENSOR", "Provide a stream.");
	ex.add(ex_offline, 0, "OFFLINE", "Run a script offline.");
	ex.add(ex_model, 0, "MODEL", "Train/Save/Load a machine learning model");
	ex.show();

	Factory::Clear();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_print(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *button_t = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	PythonConsumer *py_print = ssi_create(PythonConsumer, 0, true);
	py_print->getOptions()->setScript("ssi_print");
	ITransformable *input[] = { button_t, cursor_t };
	//py_print->getOptions()->add("path", "result.txt");
	frame->AddConsumer(2, input, py_print, "100ms");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_event(void *args)
{
	ITheFramework *frame = Factory::GetFramework();
	ITheEventBoard *board = Factory::GetEventBoard();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *button_t = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	PythonConsumer *py_send = ssi_create_id(PythonConsumer, 0, "pysend");
	py_send->getOptions()->setScript("ssi_send");
	ITransformable *input[] = { button_t, cursor_t };
	frame->AddConsumer(2, input, py_send, "100ms");
	board->RegisterSender(*py_send);

	PythonObject *py_listen = ssi_create_id(PythonObject, 0, "pylisten");
	py_listen->getOptions()->setScript("ssi_listen");
	board->RegisterListener(*py_listen, py_send->getEventAddress());

	EventMonitor *monitor = ssi_create_id(EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, 0, 10000u);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR");
	frame->AddEventConsumer(cursor_t, plot, board, py_send->getEventAddress());

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

bool ex_clone(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *button_t = frame->AddProvider(mouse, SSI_MOUSE_BUTTON_PROVIDER_NAME);
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	PythonTransformer *clone = 0;
	
	clone = ssi_create(PythonTransformer, 0, true);
	clone->getOptions()->setScript("ssi_clone");
	ITransformable *button_clone_t = frame->AddTransformer(button_t, clone, "0.1s");

	clone = ssi_create(PythonTransformer, 0, true);
	clone->getOptions()->setScript("ssi_clone");
	ITransformable *cursor_clone_t = frame->AddTransformer(cursor_t, clone, "0.1s");

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("BUTTON (RAW)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(button_t, plot, "0.1s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR (RAW)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(cursor_t, plot, "0.1s");
	
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("BUTTON (CLONE)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(button_clone_t, plot, "0.1s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR (CLONE)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(cursor_clone_t, plot, "0.1s");
		
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_energy(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	PythonFeature *py_energy = ssi_create(PythonFeature, 0, true);
	py_energy->getOptions()->setScript("ssi_energy");
	py_energy->getOptions()->setOptionFile("ssi_energy.opt");
	py_energy->getOptions()->add("global", true);
	ITransformable *py_energy_t = frame->AddTransformer(cursor_t, py_energy, "0.1s", "0.1s");

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("ENERGY (PYTHON)");
	plot->getOptions()->size = 10;
	frame->AddConsumer(py_energy_t, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_diff(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	mouse->getOptions()->mask = Mouse::LEFT;
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);
	
	PythonFilter *py_diff = ssi_create(PythonFilter, 0, true);
	py_diff->getOptions()->setScript("ssi_diff");
	ITransformable *py_diff_t = frame->AddTransformer(cursor_t, py_diff, "0.1s");

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("DIFF (PYTHON)");
	plot->getOptions()->size = 10;
	frame->AddConsumer(py_diff_t, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_pearson(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create(Mouse, 0, true);
	ITransformable *cursor_t = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	PythonFeature *ppmcc = ssi_create(PythonFeature, 0, true);
	ppmcc->getOptions()->setScript("ssi_ppmcc");
	ITransformable *ppmcc_t = frame->AddTransformer(cursor_t, ppmcc, "0.1s");

	SignalPainter *plot = 0;
	
	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("CURSOR");
	plot->getOptions()->size = 5;
	frame->AddConsumer(cursor_t, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("PPMCC");
	plot->getOptions()->size = 5;
	frame->AddConsumer(ppmcc_t, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_imggray(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FakeSignal *video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_RANDOM_RGB;
	ITransformable *rgb_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	PythonImageFilter *imggray = ssi_create(PythonImageFilter, 0, true);
	imggray->getOptions()->setScript("ssi_imggray");
	ITransformable *gray_t = frame->AddTransformer(rgb_t, imggray, "1");

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id(VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("RGB");
	frame->AddConsumer(rgb_t, vidplot, "1");

	vidplot = ssi_create_id(VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("GRAY");
	frame->AddConsumer(gray_t, vidplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_imgavg(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FakeSignal *video = 0;
	
	video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_RANDOM_RGB;
	ITransformable *rgb_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_RANDOM_GRAY;
	ITransformable *gray_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	PythonImageFeature *imgavg = 0;
	
	imgavg = ssi_create(PythonImageFeature, 0, true);
	imgavg->getOptions()->setScript("ssi_imgavg");
	ITransformable *rgb_avg_t = frame->AddTransformer(rgb_t, imgavg, "1");

	imgavg = ssi_create(PythonImageFeature, 0, true);
	imgavg->getOptions()->setScript("ssi_imgavg");
	ITransformable *gray_avg_t = frame->AddTransformer(gray_t, imgavg, "1");

	VideoPainter *vidplot = 0;

	vidplot = ssi_create_id(VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("RGB");
	frame->AddConsumer(rgb_t, vidplot, "1");

	vidplot = ssi_create_id(VideoPainter, 0, "plot");
	vidplot->getOptions()->setTitle("GRAY");
	frame->AddConsumer(gray_t, vidplot, "1");

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("AVG (RGB)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(rgb_avg_t, plot, "1");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("AVG (GRAY)");
	plot->getOptions()->size = 5;
	frame->AddConsumer(gray_avg_t, plot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_imgplot(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	FakeSignal *video = 0;

	video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_RANDOM_RGB;
	ITransformable *rgb_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	video = ssi_create(FakeSignal, 0, true);
	video->getOptions()->type = FakeSignal::SIGNAL::IMAGE_RANDOM_GRAY;
	ITransformable *gray_t = frame->AddProvider(video, "video");
	frame->AddSensor(video);

	PythonImageConsumer *imgplot = 0;

	imgplot = ssi_create(PythonImageConsumer, 0, true);
	imgplot->getOptions()->setScript("ssi_imgplot");
	imgplot->getOptions()->add("name", "RGB");
	imgplot->getOptions()->add("x", 650);
	imgplot->getOptions()->add("y", 0);
	imgplot->getOptions()->add("width", 200);
	imgplot->getOptions()->add("height", 200);
	frame->AddConsumer(rgb_t, imgplot, "1");

	imgplot = ssi_create(PythonImageConsumer, 0, true);
	imgplot->getOptions()->setScript("ssi_imgplot");
	imgplot->getOptions()->add("name", "GRAY");
	imgplot->getOptions()->add("x", 650);
	imgplot->getOptions()->add("y", 220);
	imgplot->getOptions()->add("width", 200);
	imgplot->getOptions()->add("height", 200);
	frame->AddConsumer(gray_t, imgplot, "1");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_sensor(void *args)
{
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create(Decorator, 0, true);
	frame->AddDecorator(decorator);

	PythonSensor *py_sensor = ssi_create(PythonSensor, 0, true);
	py_sensor->getOptions()->setScript("ssi_sensor");
	py_sensor->getOptions()->block = 0.5;
	py_sensor->getOptions()->add("sr", "1000");
	py_sensor->getOptions()->add("dim", "2");
	ITransformable *sine_t = frame->AddProvider(py_sensor, "sine");
	ITransformable *saw_t = frame->AddProvider(py_sensor, "saw");
	frame->AddSensor(py_sensor);

	SignalPainter *plot = 0;

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("SINE");
	plot->getOptions()->size = 5;
	frame->AddConsumer(sine_t, plot, "0.5s");

	plot = ssi_create_id(SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("SAW");
	plot->getOptions()->size = 5;
	frame->AddConsumer(saw_t, plot, "0.5s");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_offline(void *args)
{
	ssi_stream_t signal;
	FileTools::ReadStreamFile("signal", signal);
	
	PythonFeature *script = ssi_create (PythonFeature, 0, true);
	script->getOptions()->setScript("ssi_energy");

	ssi_stream_t result;	
	SignalTools::Transform(signal, result, *script, "0.1s", "0.1s", true);

	FileTools::WriteStreamFile(ssi::File::ASCII, "result", result);

	ssi_stream_destroy(signal);
	ssi_stream_destroy(result);

	return true;
}

bool ex_model(void *args)
{
	ssi_size_t n_classes = 2;
	ssi_size_t n_samples = 5;
	ssi_size_t n_streams = 3;
	ssi_real_t distr[][3] = { 0.3f, 0.3f, 0.2f, 0.3f, 0.6f, 0.2f };
	
	SampleList samples;
	ModelTools::CreateTestSamples(samples, n_classes, n_samples, n_streams, distr, "user");
	
	ModelTools::PrintInfo(samples);

	{
		PythonModel *model = ssi_create(PythonModel, 0, true);
		model->getOptions()->setScript("ssi_model");
		model->train(samples, 0);
		model->save("path/to/model");
	}
	
	{
		PythonModel *model = ssi_create(PythonModel, 0, true);
		model->getOptions()->setScript("ssi_model");
		model->load("path/to/model");
		ssi_size_t nProbs = n_classes;
		ssi_real_t* probs = new ssi_real_t[n_classes];
		for (ssi_size_t i = 0; i < nProbs; i++)
		{
			probs[i] = 1.0f;
		}
		model->forward(*samples.get(0)->streams[0], nProbs, probs);
	}

	return true;
}