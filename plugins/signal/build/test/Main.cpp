// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/12/20
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
#include "ssisignal.h"
#include "IFFT.h"
#include "FFT.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
		
bool signal (void *arg);
bool cast (void *arg);
bool selector (void *arg);
bool windows (void *arg);
bool energy (void *arg);
bool fft (void *arg);
bool fftfeat (void *arg);
bool spectrogram (void *arg);
bool mfcc (void *arg);
bool butter_filter (void *arg);
bool butter_filter_2 (void *arg);
bool butter_filter_3 (void *arg);
bool block_boost_filter (void *arg);
bool derivative (void *arg);
bool integral (void *arg);
bool remove_trend (void *arg);
bool functionals (void *arg);
bool downsample (void *arg);
bool chain (void *arg);
bool moving (void *arg);
bool movingminmax (void *arg);
bool movingnorm (void *arg);
bool movingdrvtv (void *arg);
bool movingcondiv (void *arg);
bool movingpeakgate (void *arg);
bool movingmean (void *arg);
bool movingmedian (void *arg);
bool bundle(void *arg);
bool pulse (void *arg);
bool ethres (void *arg);
bool expression (void *arg);
bool slider(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssievent.dll");
	Factory::RegisterDLL ("ssisignal.dll");
	Factory::RegisterDLL ("ssigraphic.dll");
	Factory::RegisterDLL ("ssimouse.dll");

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ssi_random_seed ();

	Exsemble ex;	
	ex.add(&signal, 0, "SIGNAL", "");
	ex.add(&cast, 0, "CAST", "");
	ex.add(&windows, 0, "WINDOWS", "");
	ex.add(&butter_filter, 0, "BUTTER I", "");
	ex.add(&butter_filter_2, 0, "BUTTER II", "");
	ex.add(&butter_filter_3, 0, "BUTTER III", "");
	ex.add(&block_boost_filter, 0, "BLOCK & BOOST", "");
	ex.add(&derivative, 0, "DERIVATIVE", "");
	ex.add(&integral, 0, "INTEGRAL", "");
	ex.add(&energy, 0, "ENERGY", "");
	ex.add(&fft, 0, "FFT", "");
	ex.add(&fftfeat, 0, "FFT FEATURES", "");
	ex.add(&spectrogram, 0, "SPECTROGRAM", "");
	ex.add(&mfcc, 0, "MFCC", "");
	ex.add(&functionals, 0, "FUNCTIONALS", "");
	ex.add(&downsample, 0, "SOWNSAMPLE", "");
	ex.add(&selector, 0, "SELECTOR", "");
	ex.add(&chain, 0, "CHAIN", "");
	ex.add(&moving, 0, "MOVING", "");
	ex.add(&movingminmax, 0, "MOVING MIN/MAX", "");
	ex.add(&movingnorm, 0, "MOVING NORM", "");
	ex.add(&movingdrvtv, 0, "MOVING DERIVATIVE", "");
	ex.add(&movingcondiv, 0, "MOVING CONDIV", "");
	ex.add(&movingpeakgate, 0, "MOVING PEAK GATE", "");
	ex.add(&movingmedian, 0, "MOVING MEDIAN", "");
	ex.add(&bundle, 0, "BUNDLE", "");
	ex.add(&pulse, 0, "PULSE", "");
	ex.add(&ethres, 0, "THRESHOLD", "");
	ex.add(&expression, 0, "EXPRESSION", "");
	ex.add(&slider, 0, "SLIDER", "");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

std::vector<ICanvasClient *> plot_objects;
std::vector<ssi_char_t *> plot_names;

void transform (ssi_stream_t &signal, 
	ssi_stream_t &result,
	ITransformer &transformer, 
	ssi_size_t frame_size, 
	ssi_size_t delta_size) {

	float tic = ssi_cast (float, clock ()) / CLOCKS_PER_SEC;
	SignalTools::Transform (signal, result, transformer, frame_size, delta_size, true);
	float toc = ssi_cast (float, clock ()) / CLOCKS_PER_SEC;
	ssi_print ("%f\n", toc - tic);
}

void plot (IThePainter *painter, int numv, int numh) {

	int id;
	for (size_t i = 0; i < plot_objects.size (); i++) {
		id = painter->AddCanvas (plot_names[i]);
		painter->AddObject (id, plot_objects[i]);
		painter->Update (id);
	}

	painter->Arrange (numh, numv, 0, 0, 800, 600);
	painter->MoveConsole (800, 0, 400, 600);

	ssi_print ("\n\n\tpress enter to continue\n\n");
	getchar ();

	for (size_t i = 0; i < plot_objects.size (); i++) {
		delete plot_objects[i];
	}
	plot_objects.clear ();
	plot_names.clear ();
}

void plot_push (Matrix<ssi_real_t> *matrix, ssi_char_t *name, bool transpose, bool as_image = false) {

	ssi_size_t num = transpose ? matrix->cols : matrix->rows;
	ssi_size_t dim = transpose ? matrix->rows : matrix->cols;

	ssi_stream_t stream;
	ssi_stream_init(stream, num, dim, sizeof(ssi_real_t), SSI_REAL, 0);
	memcpy(stream.ptr, matrix->data, stream.tot);
	
	PaintData *plot = new PaintData();
	plot->setData (stream, as_image ? PaintData::TYPE::IMAGE : PaintData::TYPE::SIGNAL);
	plot_objects.push_back (plot);
	plot_names.push_back (name);

	ssi_stream_destroy(stream);
}

void plot_push (ssi_stream_t &signal, ssi_char_t *name, bool as_image = false) {

	PaintData *plot = new PaintData();
	plot->setData(signal, as_image ? PaintData::TYPE::IMAGE : PaintData::TYPE::SIGNAL);
	plot_objects.push_back(plot);
	plot_names.push_back(name);
}

void save (ssi_stream_t &signal, ssi_char_t *filename) {
	
	FileTools::WriteStreamFile (File::BINARY, filename, signal);	
}

void load (ssi_stream_t &signal, ssi_char_t *filename) {
		
	FileTools::ReadStreamFile (filename, signal);
}

bool signal (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Signal Example ...\n");

	ssi_stream_t series;
	ssi_stream_init (series, 0, 2, sizeof (ssi_real_t), SSI_REAL, 100.0);
	SignalTools::Series (series, 5.0);

	plot_push (series, "Series");
	save (series, "series");
	
	ssi_time_t sine_f[2] = {5.0f, 20.0};
	ssi_real_t sine_a[2] = {2.0f, 2.5f};	
	SignalTools::Sine (series, sine_f, sine_a);
	
	plot_push (series, "Sine");
	save (series, "sine");

	ssi_real_t noise_a[2] = {0.05f, 1.0f};	
	FilterTools::Noise (series, noise_a);
	
	plot_push (series, "Sine+Noise");
	save (series, "noise");

	plot (painter, 3, 1);

	ssi_stream_destroy (series);

	painter->Clear ();

	return true;
}

bool cast (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Cast Example ...\n");

	ssi_stream_t series, result_double, result_short;
	ssi_stream_init (series, 0, 2, sizeof (ssi_real_t), SSI_REAL, 100.0);
	SignalTools::Series (series, 5.0);

	Cast *filter = ssi_create (Cast, "cast", true);
	
	ssi_time_t sine_f[2] = {5.0f, 1.0f};
	ssi_real_t sine_a[2] = {2.0f, 2.5f};	
	SignalTools::Sine (series, sine_f, sine_a);
	
	plot_push (series, "Float");
	save (series, "cast_float");

	filter->getOptions ()->cast = SSI_DOUBLE;
	transform (series, result_double, *filter, 10, 0); 

	plot_push (result_double, "Double");
	save (result_double, "cast_double");

	filter->getOptions ()->cast = SSI_SHORT;
	transform (series, result_short, *filter, 10, 0); 

	plot_push (result_short, "Short");
	save (result_short, "cast_short");

	plot (painter, 3, 1);

	ssi_stream_destroy (series);
	ssi_stream_destroy (result_short);
	ssi_stream_destroy (result_double);

	painter->Clear ();

	return true;
}

bool windows (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Window Example ...\n");

	Matrix<ssi_real_t> *window;
	int winlen = 50;

	window = FilterTools::Window (winlen, WINDOW_TYPE_TRIANGLE, MATRIX_DIMENSION_ROW);
	plot_push (window, "Signal", true);
	delete window;

	window = FilterTools::Window (winlen, WINDOW_TYPE_GAUSS, MATRIX_DIMENSION_ROW);
	plot_push (window, "Signal", true);
	delete window;

	window = FilterTools::Window (winlen, WINDOW_TYPE_HAMMING, MATRIX_DIMENSION_ROW);
	plot_push (window, "Signal", true);
	delete window;

	plot (painter, 3, 1);

	painter->Clear ();

	return true;
}

bool butter_filter (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("IIR Filter Example ...\n");

	ssi_stream_t signal;
	ssi_stream_init (signal, 0, 3, sizeof (ssi_real_t), SSI_REAL, 200.0);
	SignalTools::Series (signal, 1.0);

	ssi_time_t sine_f[3] = {5.0, 25.0, 45.0};
	ssi_real_t sine_a[3] = {1.0f, 1.0f, 1.0f};	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	double lcutoff = 0.15;
	double hcutoff = 0.35;
	int order = 8;

	plot_push (signal, "Signal");

	FFT fft (signal.num, signal.dim);
	ssi_stream_t fftmag;
	ssi_stream_init (fftmag, fft.rfft, 1, sizeof (ssi_real_t), SSI_REAL, 1.0);

	fft.transform (signal.num, ssi_pcast (float, signal.ptr), ssi_pcast (float, fftmag.ptr));

	// FFT

	plot_push (fftmag, "FFT");

	ssi_stream_t result;
	Matrix<ssi_real_t> *magnitude = 0;
	Matrix<std::complex<ssi_real_t>> *response = 0;

	ssi_stream_destroy (fftmag);
	
	// filter

	Butfilt *filter = ssi_pcast (Butfilt, Factory::Create (Butfilt::GetCreateName (), "butfilt"));
	filter->getOptions ()->low = lcutoff;
	filter->getOptions ()->high = hcutoff;
	filter->getOptions ()->order = order;

	// lowpass
	
	filter->getOptions ()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Lowpass Response", true);
	plot_push (result, "Lowpassed Signal");
	save (result, "lowpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// bandpass

	filter->getOptions ()->type = Butfilt::BAND;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Bandpass Response", true);
	plot_push (result, "Bandpassed Signal");
	save (result, "bandpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// highpass

	filter->getOptions ()->type = Butfilt::HIGH;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Highpass Response", true);
	plot_push (result, "Highpassed Signal");
	save (result, "highpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// plot

	plot (painter, 4, 2);

	ssi_stream_destroy (signal);

	painter->Clear ();

	return true;
}

bool butter_filter_2 (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("IIR Filter Example ...\n");

	ssi_time_t sr = 200.0;

	ssi_stream_t signal;
	ssi_stream_init (signal, 0, 2, sizeof (ssi_real_t), SSI_REAL, sr);
	SignalTools::Series (signal, 5.0);

	ssi_time_t sine_f[2] = {5.0, 30.0};
	ssi_real_t sine_a[2] = {0.5f, 0.5f};	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * sr);
	double lcutoff = 10.0;
	double hcutoff = 20.0;
	int order = 8;	

	plot_push (signal, "Signal");

	FilterTools::Noise (signal, 1.0, 0.0, 1.0, lcutoff, hcutoff - lcutoff);

	FILE *fid = fopen ("signal+noise.txt", "w");
	ssi_stream_print (signal, fid);
	fclose (fid);

	plot_push (signal, "Signal + Noise");

	ssi_stream_t result;
	Matrix<ssi_real_t> *magnitude = 0;
	Matrix<std::complex<ssi_real_t>> *response = 0;
	
	// filter

	Butfilt *filter = ssi_pcast (Butfilt, Factory::Create (Butfilt::GetCreateName (), "butfilt"));
	filter->getOptions ()->low = 2 * lcutoff / sr;
	filter->getOptions ()->high = 2 * hcutoff / sr;
	filter->getOptions ()->order = order;

	// lowpass
	
	filter->getOptions ()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Lowpass Response", true);
	plot_push (result, "Lowpassed Signal");
	save (result, "lowpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// bandpass

	filter->getOptions ()->type = Butfilt::BAND;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Bandpass Response", true);
	plot_push (result, "Bandpassed Signal");
	save (result, "bandpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// highpass

	filter->getOptions ()->type = Butfilt::HIGH;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (filter->getCoefs (signal.sr), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Highpass Response", true);
	plot_push (result, "Highpassed Signal");
	save (result, "highpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// plot

	plot (painter, 4, 2);

	ssi_stream_destroy (signal);

	painter->Clear ();

	return true;
}

bool butter_filter_3 (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Lowpass Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	// filter

	Butfilt *filter = ssi_pcast (Butfilt, Factory::Create (Butfilt::GetCreateName (), "butfilt"));
	filter->getOptions ()->low = 0.1;
	filter->getOptions ()->order = 4;
	filter->getOptions ()->zero = true;

	// lowpass

	plot_push (signal, "Signal");

	ssi_stream_t result;
	filter->getOptions ()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 

	plot_push (result, "Lowpassed");
	save (result, "lowpass");

	plot (painter, 2,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool block_boost_filter (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("IIR Filter Example 2 ...\n");

	ssi_stream_t signal;
	ssi_stream_init (signal, 0, 3, sizeof (ssi_real_t), SSI_REAL, 200.0);
	SignalTools::Series (signal, 10.0);

	ssi_time_t sine_f[3] = {0.25, 4.0, 50.0};
	ssi_real_t sine_a[3] = {1.0f, 1.0f, 0.1f};	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * 200.0);
	double R = 0.9;

	plot_push (signal, "Signal");

	FFT fft (signal.num, signal.dim);
	ssi_stream_t fftmag;
	ssi_stream_init (fftmag, fft.rfft, 1, sizeof (ssi_real_t), SSI_REAL, 1.0);
	fft.transform (signal.num, ssi_pcast (ssi_real_t, signal.ptr), ssi_pcast (ssi_real_t, fftmag.ptr));

	// FFT

	plot_push (fftmag, "FFT");

	ssi_stream_t result;
	Matrix<ssi_real_t> *coefs = 0;
	Matrix<ssi_real_t> *magnitude = 0;
	Matrix<std::complex<ssi_real_t>> *response = 0;

	ssi_stream_destroy (fftmag);
	
	// filter

	IIR *filter = ssi_pcast (IIR, Factory::Create (IIR::GetCreateName ()));

	// DC blocker
	
	coefs = FilterTools::DCBlocker (R);
	filter->setCoefs (coefs);	

	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (coefs, 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Blocker Response", true);
	plot_push (result, "Blocked Signal");
	save (result, "blocked");
	
	ssi_stream_destroy (result);
	delete magnitude;
	delete response;
	delete coefs;

	// Boost
	
	coefs = FilterTools::Boost (signal.sr, 10.0, 50.0, 5.0);
	filter->setCoefs (coefs);	

	transform (signal, result, *filter, frame_size, 0); 
			
	response = IIR::Response (coefs, 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Boost Response", true);
	plot_push (result, "Boosed Signal");
	save (result, "boosed");
	
	ssi_stream_destroy (result);
	delete magnitude;
	delete response;
	delete coefs;

	// plot

	plot (painter, 3, 2);

	ssi_stream_destroy (signal);

	painter->Clear ();

	return true;
}

bool derivative (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Derivative Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	Derivative *derivative = ssi_pcast (Derivative, Factory::Create (Derivative::GetCreateName (), "derivative"));
	derivative->getOptions ()->set (Derivative::D4TH | Derivative::D2ND);

	ssi_stream_t result;
	transform (signal, result, *derivative, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Derivative");
	save (result, "derivative");

	plot (painter, 2,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool integral (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Integral Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	Integral *integral = ssi_pcast (Integral, Factory::Create (Integral::GetCreateName (), "integral"));
	integral->getOptions ()->set (Integral::I1ST | Integral::I0TH | Integral::I4TH);

	ssi_stream_t result;
	transform (signal, result, *integral, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Integral");
	save (result, "integral");

	plot (painter, 2,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool energy (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Energy Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);

	Energy *energy = ssi_pcast (Energy, Factory::Create (Energy::GetCreateName ()));
	ssi_stream_t result_energy;
	transform (signal, result_energy, *energy, frame_size, delta_size);

	Intensity *intensity = ssi_pcast (Intensity, Factory::Create (Intensity::GetCreateName ()));
	ssi_stream_t result_intensity;
	transform (signal, result_intensity, *intensity, frame_size, delta_size);
	
	plot_push (signal, "Signal");
	plot_push (result_energy, "Energy");
	save (result_energy, "energy");
	plot_push (result_intensity, "Intensity");
	save (result_intensity, "intensity");

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result_energy);
	ssi_stream_destroy (result_intensity);

	painter->Clear ();

	return true;
}

bool fft (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("FFT Example ...\n");

	ssi_stream_t signal;
	ssi_size_t nfft = 512;
	ssi_stream_init (signal, 0, 3, sizeof (ssi_real_t), SSI_REAL, nfft);
	SignalTools::Series (signal, 1.0);

	ssi_time_t sine_f[3] = {5.0, 25.0, 45.0};
	ssi_real_t sine_a[3] = {0.5f, 1.0f, 1.5f};	
	SignalTools::Sine (signal, sine_f, sine_a);

	plot_push (signal, "Sines", false);
	SignalTools::Sum (signal);

	ssi_stream_t signal2;
	ssi_stream_init (signal2, signal.num, 2, signal.byte, signal.type, signal.sr);
	ssi_real_t *src = ssi_pcast (ssi_real_t, signal.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, signal2.ptr);
	for (ssi_size_t i = 0; i < signal.num; i++) {
		for (ssi_size_t j = 0; j < signal2.dim; j++) {
			*dst++ = *src;
		}
		src++;
	}

	plot_push (signal2, "Sum (Sines)", false);

	FFT fft (nfft, signal2.dim);
	IFFT ifft (fft.rfft, signal2.dim);

	Matrix<ssi_real_t> signal_m (signal2.num, signal2.dim, ssi_pcast (ssi_real_t, signal2.ptr));

	float tic = ssi_cast (float, clock ()) / CLOCKS_PER_SEC;

	Matrix<ssi_real_t> *magnitude = new Matrix<ssi_real_t> (1, fft.rfft * signal2.dim);
	fft.transform (&signal_m, magnitude);
	Matrix<std::complex<ssi_real_t>> *fftpoints = new Matrix<std::complex<ssi_real_t>> (1, fft.rfft * signal2.dim);
	fft.transform (&signal_m, fftpoints);
	Matrix<ssi_real_t> result (fft.nfft, signal2.dim);
	ifft.transform (fftpoints, &result);

	float toc = static_cast<float> (clock ()) / CLOCKS_PER_SEC;
	ssi_print ("%f\n", toc - tic);

	plot_push (magnitude, "FFT", true);
	plot_push (&result, "IFFT", false);

	plot (painter, 4, 1);

	delete fftpoints;
	delete magnitude;
	ssi_stream_destroy (signal);

	painter->Clear ();	

	return true;
}

bool fftfeat (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("FFT Feature Example...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "audio");

	ssi_size_t frame_size = 16;
	ssi_size_t delta_size = 16;

	FFTfeat *fftfeat =  ssi_pcast (FFTfeat, Factory::Create (FFTfeat::GetCreateName ()));
	fftfeat->getOptions ()->nfft = 32;
	transform (signal, result, *fftfeat, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "FFT Features");

	save (result, "fftfeat");

	plot (painter, 2, 1);

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool spectrogram (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Spectrogram Example ...\n");

	ssi_stream_t signal;
	load (signal, "audio");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.015 * signal.sr);

	int fft_size = 512;

	Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank (fft_size, signal.sr, 50, 100, 5100, WINDOW_TYPE_RECTANGLE);
	Spectrogram *spectrogram = ssi_pcast (Spectrogram, Factory::Create (Spectrogram::GetCreateName (), "spect", true));
	//spectrogram->setFilterbank (filterbank, WINDOW_TYPE_RECTANGLE, true);
	delete filterbank;

	ssi_stream_t result;
	transform (signal, result, *spectrogram, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "Spectrogram", true);
	save (result, "spect");

	plot (painter, 2,1);

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool mfcc (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	int n_first = 0;
	int n_last = 13;

	ssi_print ("MFCC Example ...\n");

	ssi_stream_t signal;
	load (signal, "audio");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.015 * signal.sr);

	int fft_size = 512;

	Matrix<ssi_real_t> *filterbank = FilterTools::MelBank ((fft_size >> 1) + 1, 16000.0f);

	Matrix<ssi_real_t> *dctmatrix = FilterTools::DCTMatrix (40, n_first, n_last);
	MFCC *mfcc = ssi_pcast (MFCC, Factory::Create (MFCC::GetCreateName ()));
	mfcc->getOptions ()->n_ffts = fft_size;
	mfcc->getOptions ()->n_first = n_first;
	mfcc->getOptions ()->n_last = n_last;
	ssi_stream_t result;
	transform (signal, result, *mfcc, frame_size, delta_size);
	
	plot_push (signal, "Signal");
	plot_push (dctmatrix, "DCT Matrix", false, true);
	plot_push (result, "MFCCs", true);
	save (result, "mfcc");

	plot (painter, 3,1);

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	delete filterbank;
	delete dctmatrix;

	painter->Clear ();

	return true;
}

bool functionals (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Functionals Example ...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "audio");
	
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);

	Functionals *functs = ssi_pcast (Functionals, Factory::Create (Functionals::GetCreateName (), "functs"));
	functs->getOptions ()->delta = 10;	
	transform (signal, result, *functs, frame_size, delta_size);

	ssi_print ("selected functionals:\n");
	for (ssi_size_t i = 0; i < functs->getSize (); i++) {
		ssi_print ("\t%s\n", functs->getName (i));
	}

	save (result, "statistics");
	plot_push (signal, "Signal");
	plot_push (result, "Statistics");

	plot (painter, 2, 1);

	ssi_stream_destroy (result);
	ssi_stream_destroy (signal);

	painter->Clear ();

	return true;
}

bool downsample (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Down Sample Example ...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "rsp");

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);
	ssi_size_t delta_size = 0;
	ssi_size_t factor = 10;
	
	DownSample *downsample = ssi_pcast (DownSample, Factory::Create (DownSample::GetCreateName (), "downsample"));
	downsample->getOptions ()->factor = factor;
	transform (signal, result, *downsample, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "Downsampled");

	save (result, "downsampled");

	plot (painter, 2, 1);

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool selector (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Signal Example ...\n");

	ssi_stream_t series;
	ssi_stream_init (series, 0, 5, sizeof (ssi_real_t), SSI_REAL, 100.0);
	SignalTools::Series (series, 5.0);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * 100.0);

	ssi_time_t sine_f[] = {1.0, 5.0, 20.0, 30.0, 50.0};
	ssi_real_t sine_a[] = {2.0f, 0.5f, 0.4f, 0.3f, 0.2f};	
	SignalTools::Sine (series, sine_f, sine_a);
	
	plot_push (series, "Signal");
	
	Selector *selector = ssi_pcast (Selector, Factory::Create (Selector::GetCreateName (), "selector"));
	ssi_size_t inds[] = {4,2};
	selector->getOptions ()->set (2, inds);

	ssi_stream_t result;
	transform (series, result, *selector, frame_size, 0);
	
	plot_push (result, "Selector");
	save (result, "selector");

	plot (painter, 2, 1);

	ssi_stream_destroy (series);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool chain (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Chain Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	Chain *chain = ssi_pcast (Chain, Factory::Create (Chain::GetCreateName (), "mychain"));

	ssi_stream_t result;
	transform (signal, result, *chain, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Chain");
	save (result, "chain");

	plot (painter, 2,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	painter->Clear ();

	return true;
}

bool moving (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving/Sliding Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgAvgVar *moving = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), "moving"));
	moving->getOptions ()->win = 1.0;
	moving->getOptions ()->format = MvgAvgVar::ALL;
	moving->getOptions ()->method = MvgAvgVar::MOVING;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgAvgVar *sliding = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), "sliding"));
	sliding->getOptions ()->win = 1.0;
	sliding->getOptions ()->format = MvgAvgVar::ALL;
	sliding->getOptions ()->method = MvgAvgVar::SLIDING;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "Moving");
	plot_push (result2, "Sliding");	
	save (result, "moving");
	save (result2, "sliding");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingminmax (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving/Sliding Min/Max Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgMinMax *moving = ssi_pcast (MvgMinMax, Factory::Create (MvgMinMax::GetCreateName (), "movingminmax"));
	moving->getOptions ()->win = 1.0;
	moving->getOptions ()->format = MvgMinMax::ALL;
	moving->getOptions ()->method = MvgMinMax::MOVING;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgMinMax *sliding = ssi_pcast (MvgMinMax, Factory::Create (MvgMinMax::GetCreateName (), "slidingminmax"));
	sliding->getOptions ()->win = 1.0;
	sliding->getOptions ()->format = MvgMinMax::ALL;
	sliding->getOptions ()->method = MvgMinMax::SLIDING;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingMinMax");
	plot_push (result2, "SlidingMinMax");	
	save (result, "movingminmax");
	save (result2, "slidingminmax");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingnorm (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving/Sliding Norm Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	MvgNorm::NORM norm = MvgNorm::SUBMIN;

	MvgNorm *moving = ssi_pcast (MvgNorm, Factory::Create (MvgNorm::GetCreateName (), "movingnorm"));
	moving->getOptions ()->win = 1.0;	
	moving->getOptions ()->method = MvgNorm::MOVING;
	moving->getOptions ()->norm = norm;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgNorm *sliding = ssi_pcast (MvgNorm, Factory::Create (MvgNorm::GetCreateName (), "slidingnorm"));
	sliding->getOptions ()->win = 1.0;
	sliding->getOptions ()->method = MvgNorm::SLIDING;
	sliding->getOptions ()->norm = norm;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingNorm");
	plot_push (result2, "SlidingNorm");	
	save (result, "movingnorm");
	save (result2, "slidingnorm");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingdrvtv (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving/Sliding Derivative Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgDrvtv *moving = ssi_pcast (MvgDrvtv, Factory::Create (MvgDrvtv::GetCreateName (), "movingdrvtv"));
	moving->getOptions ()->win = 0.1;	
	moving->getOptions ()->method = MvgDrvtv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgDrvtv *sliding = ssi_pcast (MvgDrvtv, Factory::Create (MvgDrvtv::GetCreateName (), "slidingdrvtv"));
	sliding->getOptions ()->win = 0.1;
	sliding->getOptions ()->method = MvgDrvtv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingDrvtv");
	plot_push (result2, "SlidingDrvtv");	
	save (result, "movingdrvtv");
	save (result2, "slidingdrvtv");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingcondiv (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving/Sliding Condiv Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	MvgConDiv *moving = ssi_pcast (MvgConDiv, Factory::Create (MvgConDiv::GetCreateName (), "movingcondiv"));
	moving->getOptions ()->wins = wins;	
	moving->getOptions ()->winl = winl;	
	moving->getOptions ()->method = MvgConDiv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgConDiv *sliding = ssi_pcast (MvgConDiv, Factory::Create (MvgConDiv::GetCreateName (), "slidingcondiv"));
	sliding->getOptions ()->wins = wins;	
	sliding->getOptions ()->winl = winl;	
	sliding->getOptions ()->method = MvgConDiv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingpeakgate (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving PeakGate Example ...\n");

	ssi_stream_t signal;
	ssi_real_t fix = 0.002f;
	ssi_real_t win = 0.5f;
	MvgPeakGate::THRESHOLD thres = MvgPeakGate::FIXAVGSTD;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgPeakGate *moving = ssi_pcast (MvgPeakGate, Factory::Create (MvgPeakGate::GetCreateName (), "movingpeakgate"));
	moving->getOptions ()->thres = thres;
	moving->getOptions ()->win = win;
	moving->getOptions ()->fix = fix;
	moving->getOptions ()->method = MvgPeakGate::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgPeakGate *sliding = ssi_pcast (MvgPeakGate, Factory::Create (MvgPeakGate::GetCreateName (), "slidingpeakgate"));
	sliding->getOptions ()->thres = thres;
	sliding->getOptions ()->win = win;
	sliding->getOptions ()->fix = fix;	
	sliding->getOptions ()->method = MvgPeakGate::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool movingmedian (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Moving Median Example ...\n");

	ssi_stream_t signal;
	ssi_size_t win = 128;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.2 * signal.sr);

	MvgMedian *moving = ssi_pcast (MvgMedian, Factory::Create (MvgMedian::GetCreateName (), "movingmedian"));
	moving->getOptions ()->winInSamples = win;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingMedian");
	save (result, "movingmedian");	

	plot (painter, 2,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);	

	painter->Clear ();

	return true;
}

bool bundle(void *arg) {

	IThePainter *painter = Factory::GetPainter();

	ssi_print("Bundle Example ...\n");

	ssi_stream_t tmp, signal;	
	load(tmp, "noise");
	ssi_size_t select = 0;
	ssi_stream_select(tmp, signal, 1, &select);	
	ssi_size_t frame_size = 1;

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, signal.ptr);
	for (ssi_size_t i = 0; i < signal.num; i++) {
		if (abs(ptr[i * signal.dim]) > 1.5 && ssi_random () > 0.7) {
			ptr[i * signal.dim] *= -5.0;
			i += 10;
		}
	}

	ssi_size_t hang_in = 2;
	ssi_size_t hang_out = 2;

	Bundle *bundle = ssi_pcast(Bundle, Factory::Create(Bundle::GetCreateName(), "bundle"));
	bundle->getOptions()->hang_in = hang_in;
	bundle->getOptions()->replace_below = Bundle::REPLACE::PREVIOUS;
	bundle->getOptions()->set_below = 0;
	bundle->getOptions()->hang_out = hang_out;
	bundle->getOptions()->replace_above = Bundle::REPLACE::PREVIOUS;
	bundle->getOptions()->set_above = 1;
	bundle->getOptions()->set_all = false;
	bundle->getOptions()->thres = 0;

	ssi_stream_t result;
	transform(signal, result, *bundle, frame_size, max(hang_in, hang_out));

	plot_push(signal, "Signal");
	plot_push(result, "Bundle");
	save(result, "bundle");

	plot(painter, 2, 1);
	ssi_stream_destroy(tmp);
	ssi_stream_destroy(signal);
	ssi_stream_destroy(result);

	painter->Clear();

	return true;
}

bool pulse (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Pulse Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	MvgConDiv *moving = ssi_pcast (MvgConDiv, Factory::Create (MvgConDiv::GetCreateName (), "movingcondiv"));
	moving->getOptions ()->wins = wins;	
	moving->getOptions ()->winl = winl;	
	moving->getOptions ()->method = MvgConDiv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgConDiv *sliding = ssi_pcast (MvgConDiv, Factory::Create (MvgConDiv::GetCreateName (), "slidingcondiv"));
	sliding->getOptions ()->wins = wins;	
	sliding->getOptions ()->winl = winl;	
	sliding->getOptions ()->method = MvgConDiv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot (painter, 3,1);
	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	painter->Clear ();

	return true;
}

bool ethres (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Threshold Event Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	FileAnnotationWriter awrite;

	ThresEventSender *tes = ssi_create (ThresEventSender, 0, true);
	tes->getOptions ()->thres = 0;
	tes->setEventListener (&awrite);
	SignalTools::Consume (signal, *tes, frame_size);

	plot_push (signal, "Signal");	
	plot (painter, 1,1);

	ssi_stream_destroy (signal);
	
	painter->Clear ();

	return true;
}

bool expression (void *arg) {

	IThePainter *painter = Factory::GetPainter ();

	ssi_print ("Expression Example ...\n");

	ssi_stream_t signal, result;
	load (signal, "rsp");
	
	Expression *expression = ssi_create (Expression, 0, true);
	//expression->getOptions ()->single = true;
	//expression->getOptions ()->setExpression ("sin(sqrt(d0*d1))");
	expression->getOptions ()->single = false;
	expression->getOptions ()->join = Expression::JOIN::MULT;
	expression->getOptions ()->setExpression ("(100000*d) ^2");
	
	SignalTools::Transform (signal, result, *expression, 0);
	plot_push (signal, "Signal");
	plot_push (result, expression->getOptions ()->expression);	
	plot (painter,1,2);

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	
	painter->Clear ();

	return true;
}

bool slider (void *arg) {

	ITheEventBoard *board = Factory::GetEventBoard("board");
	ITheFramework *frame = Factory::GetFramework("frame");
	IThePainter *painter = Factory::GetPainter();

	Selector *select = ssi_create(Selector, 0, true);
	select->getOptions()->set(1);

	Mouse *mouse = ssi_create(Mouse, "mouse", true);
	mouse->getOptions()->scale = true;
	ITransformable *cursor_p = frame->AddProvider(mouse, SSI_MOUSE_CURSOR_PROVIDER_NAME, select);
	frame->AddSensor(mouse);

	EventSlider *slider = 0;
	EventAddress address;

	slider = ssi_create(EventSlider, 0, true);
	slider->getOptions()->setEvent("min");
	slider->getOptions()->setSender("slider");
	slider->getOptions()->setValue("min");
	slider->getOptions()->maxval = 0.5f;
	slider->getOptions()->defval = 0.0f;
	slider->getOptions()->setSliderPos(410, 405, 400, 75);
	board->RegisterSender(*slider);
	address.setAddress(slider->getEventAddress());

	slider = ssi_create(EventSlider, 0, true);
	slider->getOptions()->setEvent("max");
	slider->getOptions()->setSender("slider");
	slider->getOptions()->setValue("max");
	slider->getOptions()->minval = 0.5f;
	slider->getOptions()->defval = 1.0f;
	slider->getOptions()->setSliderPos(410, 490, 400, 75);
	board->RegisterSender(*slider);
	address.setAddress(slider->getEventAddress());
	
	Limits *limits = ssi_create (Limits, 0, true);
	ITransformable* limits_t = frame->AddTransformer (cursor_p, limits, "10");
	board->RegisterListener (*limits, address.getAddress ());
	
	SignalPainter* paint = 0;

	paint = ssi_create(SignalPainter, 0, true);
	paint->getOptions()->type = PaintSignalType::SIGNAL;
	paint->getOptions()->size = 10;
	paint->getOptions()->setName("Cursor");
	paint->getOptions()->autoscale = false;
	paint->getOptions()->fix[0] = 0;
	paint->getOptions()->fix[1] = 1;
	frame->AddConsumer(cursor_p, paint, "0.1s");

	paint = ssi_create(SignalPainter, 0, true);
	paint->getOptions ()->type = PaintSignalType::SIGNAL;
	paint->getOptions ()->size = 10;
	paint->getOptions ()->setName("Limits");
	paint->getOptions ()->autoscale = false;
	paint->getOptions ()->fix[0] = 0;
	paint->getOptions ()->fix[1] = 1;
	frame->AddConsumer(limits_t, paint, "0.1s");

	frame->Start();
	board->Start();
	painter->Arrange(1, 2, 400, 0, 600, 400);
	painter->MoveConsole(0, 0, 400, 600);
	frame->Wait();
	board->Stop();
	frame->Stop();
	board->Clear();
	frame->Clear();
	painter->Clear();

	return true;
}