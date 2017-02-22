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
#include "ssidspfilters.h"
#include "audio/include/ssiaudio.h"
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

bool ex_online_audio (void *args);
bool ex_online_mouse (void *args);
bool ex_butter (void *args);
bool ex_lowpass (void *args);
bool ex_highpass (void *args);
bool ex_bandpass (void *args);
bool ex_bandstop (void *args);
bool ex_lowshelf (void *args);
bool ex_highshelf (void *args);
bool ex_bandshelf (void *args);
bool ex_allpass (void *args);

std::vector<ICanvasClient *> plot_objects;
std::vector<ssi_char_t *> plot_names;
void plot_push (ssi_size_t num, ssi_size_t dim, ssi_real_t *signal, const ssi_char_t *name);
void plot_push (ssi_stream_t &signal, const ssi_char_t *name, bool as_image = false);
void plot();

struct testProperty {

	ssi_time_t sr;
	ssi_time_t frame;

	bool norm;
	DspFiltersTools::FILTER_TYPE type;
	DspFiltersTools::FILTER_STATE state;
	ssi_size_t order;
	ssi_time_t freq;
	ssi_time_t width;
	double q;
	double gain;
	double slope;
	ssi_size_t smooth;
	bool offset;

	ssi_time_t length;
	ssi_size_t n_sines;
	ssi_time_t *sine_f;
	ssi_real_t *sine_a;

	bool noise;
	ssi_time_t nfreq;
	ssi_time_t nwidth;
	double nmean;
	double nstdv;
	ssi_real_t nampl;	
};

void createSignal (ssi_stream_t &signal, testProperty prop);
IDspFilter *createFilter (const ssi_char_t *name, testProperty prop);
void testFilter (const ssi_char_t *name, testProperty prop, ssi_stream_t signal);
void transform (ssi_stream_t &signal, 
	ssi_stream_t &result,
	ITransformer &transformer, 
	ssi_size_t frame_size, 
	ssi_size_t delta_size);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssimouse.dll");
	Factory::RegisterDLL ("ssiaudio.dll");
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssidspfilters.dll");

	ssi_random_seed ();

	Exsemble ex;
	ex.console(0, 0, 650, 800);
	ex.add(ex_online_audio, 0, "ONLINE AUDIO", "");
	ex.add(ex_online_mouse, 0, "ONLINE MOUSE", "");
	ex.add(ex_lowpass, 0, "LOWPASS", "");
	ex.add(ex_highpass, 0, "HIGHPASS", "");
	ex.add(ex_bandpass, 0, "BANDPASS", "");
	ex.add(ex_bandstop, 0, "BANDSTOP", "");
	ex.add(ex_lowshelf, 0, "LOWSHELF", "");
	ex.add(ex_highshelf, 0, "HIGHSHELF", "");
	ex.add(ex_bandshelf, 0, "BANDSHELF", "");
	ex.add(ex_allpass, 0, "ALLPASS", "");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_online_audio (void *args) {

	ssi_size_t order = 3;
	DspFiltersTools::FILTER_STATE state = DspFiltersTools::DIRECTFORM_I;
	ssi_time_t freq = 1000.0;
	ssi_time_t width = 500.0;
	double gain = 12.0;
	ssi_size_t smooth = 0;
	bool offset = false;

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Audio *audio = ssi_create (Audio, "audio", true);	
	audio->getOptions()->scale = true;
	audio->getOptions()->block = 0.02;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
	frame->AddSensor(audio);
	
	ButterworthFilter *butter = ssi_create (ButterworthFilter, 0, true);
	butter->getOptions()->type = ButterworthFilter::LOW;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = freq;
	butter->getOptions()->order = order;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *butter_t = frame->AddTransformer(audio_p, butter, "0.02s");	

	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = 10;
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, plot, "0.02s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("lowpass");
	plot->getOptions()->size = 10;
	plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(butter_t, plot, "0.02s");

	WavWriter *writer = 0;

	writer = ssi_create (WavWriter, 0, true);
	writer->getOptions()->setPath("raw.wav");	
	frame->AddConsumer(audio_p, writer, "0.02s");

	writer = ssi_create (WavWriter, 0, true);
	writer->getOptions()->setPath("butter.wav");	
	frame->AddConsumer(butter_t, writer, "0.02s");

	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
};

bool ex_online_mouse (void *args) {

	ssi_time_t sr = 250.0;

	ssi_size_t order = 5;
	DspFiltersTools::FILTER_STATE state = DspFiltersTools::DIRECTFORM_II;
	ssi_time_t freq = 5.0;
	ssi_time_t width = 40.0;
	double gain = 12.0;
	ssi_size_t smooth = 64;
	bool offset = true;

	ssi_time_t nfreq = 35.0;
	ssi_time_t nwidth = 5.0;
	double nmean = 1.0;
	double nstdv = 0.2;
	ssi_real_t namp = 1.0f;

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	Mouse *mouse = ssi_create (Mouse, 0, true);	
	mouse->getOptions()->sr = sr;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME);
	frame->AddSensor(mouse);

	Noise *noise = ssi_create (Noise, 0, true);
	noise->getOptions()->norm = false;
	noise->getOptions()->cutoff = nfreq;
	noise->getOptions()->width = nwidth;
	noise->getOptions()->ampl = namp;
	noise->getOptions()->mean = nmean;
	noise->getOptions()->stdv = nstdv;
	ITransformable *noise_t = frame->AddTransformer(cursor_p, noise, "0.2s");
	
	ButterworthFilter *lowpass = ssi_create (ButterworthFilter, "butter_low", true);
	lowpass->getOptions()->type = ButterworthFilter::LOW;
	lowpass->getOptions()->state = state;
	lowpass->getOptions()->norm = false;
	lowpass->getOptions()->freq = freq;
	lowpass->getOptions()->order = order;
	lowpass->getOptions()->smooth = smooth;
	lowpass->getOptions()->offset = offset;
	ITransformable *lowpass_t = frame->AddTransformer(noise_t, lowpass, "0.2s");	

	ButterworthFilter *butter = 0;

	butter = ssi_create (ButterworthFilter, "butter_high", true);
	butter->getOptions()->type = ButterworthFilter::HIGH;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = freq;
	butter->getOptions()->order = order;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *highpass_t = frame->AddTransformer(noise_t, butter, "0.2s");

	butter = ssi_create (ButterworthFilter, "butter_stop", true);
	butter->getOptions()->type = ButterworthFilter::STOP;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = nfreq;
	butter->getOptions()->width = nwidth;
	butter->getOptions()->order = order;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *stopband_t = frame->AddTransformer(noise_t, butter, "0.2s");

	butter = ssi_create (ButterworthFilter, "butter_band", true);
	butter->getOptions()->type = ButterworthFilter::BAND;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = nfreq;
	butter->getOptions()->width = nwidth;
	butter->getOptions()->order = order;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *bandpass_t = frame->AddTransformer(noise_t, butter, "0.2s");

	butter = ssi_create (ButterworthFilter, "butter_lowshelf", true);
	butter->getOptions()->type = ButterworthFilter::LOWSHELF;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = freq;
	butter->getOptions()->order = order;
	butter->getOptions()->gain = gain;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *lowshelf_t = frame->AddTransformer(noise_t, butter, "0.2s");

	butter = ssi_create (ButterworthFilter, "butter_highshelf", true);
	butter->getOptions()->type = ButterworthFilter::HIGHSHELF;
	butter->getOptions()->state = state;
	butter->getOptions()->norm = false;
	butter->getOptions()->freq = freq;
	butter->getOptions()->order = order;
	butter->getOptions()->gain = gain;
	butter->getOptions()->smooth = smooth;
	butter->getOptions()->offset = offset;
	ITransformable *highshelf_t = frame->AddTransformer(noise_t, butter, "0.2s");
	
	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("raw");
	plot->getOptions()->size = 10;
	frame->AddConsumer(cursor_p, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("raw + noise");
	plot->getOptions()->size = 10;
	frame->AddConsumer(noise_t, plot, "0.2s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("lowpass");
	plot->getOptions()->size = 10;
	frame->AddConsumer(lowpass_t, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("highpass");
	plot->getOptions()->size = 10;
	frame->AddConsumer(highpass_t, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("stopband");
	plot->getOptions()->size = 10;
	frame->AddConsumer(stopband_t, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("bandpass");
	plot->getOptions()->size = 10;
	frame->AddConsumer(bandpass_t, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("lowshelf");
	plot->getOptions()->size = 10;
	frame->AddConsumer(lowshelf_t, plot, "0.2s");

	plot = ssi_create_id (SignalPainter, 0, "plot");	
	plot->getOptions()->setTitle("highshelf");
	plot->getOptions()->size = 10;
	frame->AddConsumer(highshelf_t, plot, "0.2s");

	FileWriter *writer = 0;

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("raw");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(cursor_p, writer, "0.2s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("noise");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(noise_t, writer, "0.2s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("lowpass");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(lowpass_t, writer, "0.2s");

	writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("highpass");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(highpass_t, writer, "0.2s");

	decorator->add("plot*", 650, 0, 400, 800);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
};

bool ex_lowpass (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::LOWPASS;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;	
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 5.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);

	prop.q = 2.0;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_highpass (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::HIGHPASS;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;	
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 5.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);

	prop.q = 2.0;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_bandpass (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::BANDPASS2;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;	
	prop.width = 2.0;	
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 0;
	prop.nwidth = 0;
	prop.nmean = 1.0;
	prop.nstdv = 0.5;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {5.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);
	
	prop.q = 1.0;
	prop.width = 5.0;	

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	prop.type = DspFiltersTools::BANDPASS1;
	
	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_bandstop (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::BANDSTOP;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 15.0;	
	prop.width = 20.0;
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 10.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);

	prop.q = 2.0;
	prop.width = 0.5;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_lowshelf (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::LOWSHELF;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;
	prop.gain = 14.0;
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 5.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);

	prop.slope = 2.0;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_highshelf (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::HIGHSHELF;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;	
	prop.gain = -14.0;
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 5.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);

	prop.slope = 2.0;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_bandshelf (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::BANDSHELF;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 5.0;	
	prop.width = 2.0;	
	prop.gain = 14.0;
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 0;
	prop.nwidth = 0;
	prop.nmean = 1.0;
	prop.nstdv = 0.5;
	prop.nampl = 1.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {5.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	testFilter (ButterworthFilter::GetCreateName (), prop, signal);
	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

bool ex_allpass (void *args) {

	testProperty prop;

	// set paramters

	prop.sr = 250.0;
	prop.frame = 0.1;
	prop.length = 5.0;

	prop.norm = false;
	prop.type = DspFiltersTools::ALLPASS;
	prop.state = DspFiltersTools::DIRECTFORM_II;
	prop.order = 8;	
	prop.freq = 1.0;	
	prop.smooth = 0;
	prop.offset = true;

	prop.nfreq = 15.0;
	prop.nwidth = 5.0;
	prop.nmean = 1.0;
	prop.nstdv = 1.0;
	prop.nampl = 0.0f;

	prop.n_sines = 1;
	ssi_time_t sine_f[] = {1.0};
	prop.sine_f = sine_f;
	ssi_real_t sine_a[] = {1.0f};
	prop.sine_a = sine_a;

	// create signal
	
	ssi_stream_t signal;
	createSignal (signal, prop);	

	// test filter

	prop.q = 2.0;

	testFilter (RBJFilter::GetCreateName (), prop, signal);

	// clean up

	ssi_stream_destroy (signal);
	
	return true;
}

//////////////////////
// HELPER FUNCTIONS //
//////////////////////

IDspFilter *createFilter (const ssi_char_t *name, testProperty prop) {

	IDspFilter *filter = ssi_pcast (IDspFilter, Factory::Create (name, 0, false));
	OptionList *options = ssi_pcast (OptionList, filter->getOptions());
	options->setOptionValue("type", &prop.type);
	options->setOptionValue("state", &prop.state);
	options->setOptionValue("order", &prop.order);
	options->setOptionValue("norm", &prop.norm);
	options->setOptionValue("freq", &prop.freq);
	options->setOptionValue("width", &prop.width);
	options->setOptionValue("gain", &prop.gain);
	options->setOptionValue("q", &prop.q);
	options->setOptionValue("slope", &prop.slope);
	options->setOptionValue("smooth", &prop.smooth);
	options->setOptionValue("offset", &prop.offset);

	return filter;
}

void createSignal (ssi_stream_t &signal, testProperty prop) {

	ssi_time_t sr = prop.sr;

	ssi_size_t n_sines = prop.n_sines;
	ssi_time_t *sine_f = prop.sine_f;
	ssi_real_t *sine_a = prop.sine_a;
	ssi_time_t length = prop.length;

	ssi_stream_init (signal, 0, n_sines, sizeof (ssi_real_t), SSI_REAL, sr);
	SignalTools::Series (signal, length);	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	plot_push (signal, "Signal");

	bool noise = prop.noise;
	double nfreq = prop.nfreq;
	double nwidth = prop.nwidth;
	ssi_real_t nampl = prop.nampl;
	double nmean = prop.nmean;
	double nstdv = prop.nstdv;

	if (noise) {
		FilterTools::Noise (signal, nampl, nmean, nstdv, nfreq, nwidth);
		plot_push (signal, "Signal + Noise");
	}
}

void testFilter (const ssi_char_t *name, testProperty prop, ssi_stream_t signal) {

	ssi_time_t sr = prop.sr;
	ssi_size_t frame_size = ssi_cast (ssi_size_t, prop.frame * sr);

	// create filter

	IDspFilter *filter = createFilter (name, prop);

	// prepare impulse

	ssi_stream_t result;
	ssi_size_t n_impulse = 1024;
	ssi_real_t *impulse = new ssi_real_t[n_impulse];
	
	// apply filter to signal
		
	transform (signal, result, *filter, frame_size, 0); 
	plot_push (result, filter->info());
	filter->gain(sr, n_impulse, impulse);	
	plot_push (n_impulse, 1, impulse, "response");	

	plot();

	// clean up

	delete filter;
	ssi_stream_destroy (result);
	delete[] impulse;
}

void transform (ssi_stream_t &signal, 
	ssi_stream_t &result,
	ITransformer &transformer, 
	ssi_size_t frame_size, 
	ssi_size_t delta_size) {

	float tic = ssi_cast (float, clock ()) / CLOCKS_PER_SEC;
	SignalTools::Transform (signal, result, transformer, frame_size, delta_size, true, true);
	float toc = ssi_cast (float, clock ()) / CLOCKS_PER_SEC;
	ssi_print ("%f\n", toc - tic);
}

void plot() {

	ssi_size_t n_plots = (ssi_size_t) plot_objects.size();

	Canvas **canvas = new Canvas *[n_plots];
	Window **window = new Window *[n_plots];

	ssi_rect_t pos;
	pos.left = 650;
	pos.top = 0;
	pos.width = 400;
	pos.height = 800 / n_plots;
	for (ssi_size_t i = 0; i < n_plots; i++) {
		canvas[i] = new Canvas();
		canvas[i]->addClient(plot_objects[i]);
		window[i] = new Window();
		window[i]->setClient(canvas[i]);
		window[i]->setTitle(plot_names[i]);
		pos.top = i * pos.height;
		window[i]->setPosition(pos);
		window[i]->create();
		window[i]->show();
	}

	ssi_print ("\n\tpress a key to continue\n\n");
	getchar ();

	for (size_t i = 0; i < n_plots; i++) {
		window[i]->close();
		delete window[i];
		delete canvas[i];
		delete plot_objects[i];
		delete[] plot_names[i];
	}
	plot_objects.clear ();	
	plot_names.clear ();
	delete[] window;
	delete[] canvas;
}

void plot_push (ssi_size_t num, ssi_size_t dim, ssi_real_t *signal, const ssi_char_t *name) {
	
	ssi_stream_t *stream = new ssi_stream_t;
	ssi_stream_init(*stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, 0);
	stream->ptr = ssi_pcast(ssi_byte_t, signal);
	stream->tot = stream->tot_real = num * dim * sizeof(ssi_real_t);
	stream->num = stream->num_real = num;
	plot_push(*stream, name, false);
}

void plot_push (ssi_stream_t &signal, const ssi_char_t *name, bool as_image) {

	PaintData *plot = new PaintData;
	plot->setData(signal, as_image ? PaintData::TYPE::IMAGE : PaintData::TYPE::SIGNAL);
	plot_objects.push_back (plot);
	plot_names.push_back (ssi_strcpy (name));
}
