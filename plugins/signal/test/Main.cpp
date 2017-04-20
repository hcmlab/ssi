// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/12/20
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
// version 3 of the License, or any later version.
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
		
bool ex_signal(void *args);
bool ex_cast(void *args);
bool ex_selector(void *args);
bool ex_windows(void *args);
bool ex_energy(void *args);
bool ex_fft(void *args);
bool ex_fftfeat(void *args);
bool ex_spectrogram(void *args);
bool ex_mfcc(void *args);
bool ex_butter_filter(void *args);
bool ex_butter_filter_2(void *args);
bool ex_butter_filter_3(void *args);
bool ex_block_boost_filter(void *args);
bool ex_derivative(void *args);
bool ex_integral(void *args);
bool ex_remove_trend(void *args);
bool ex_functionals(void *args);
bool ex_downsample(void *args);
bool ex_chain(void *args);
bool ex_moving(void *args);
bool ex_movingminmax(void *args);
bool ex_movingnorm(void *args);
bool ex_movingdrvtv(void *args);
bool ex_movingcondiv(void *args);
bool ex_movingpeakgate(void *args);
bool ex_movingmean(void *args);
bool ex_movingmedian(void *args);
bool ex_bundle(void *args);
bool ex_pulse(void *args);
bool ex_ethres(void *args);
bool ex_expression(void *args);
bool ex_statistics(void *args);

int main() {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	Factory::RegisterDLL("ssiframe.dll");
	Factory::RegisterDLL("ssievent.dll");
	Factory::RegisterDLL("ssisignal.dll");
	Factory::RegisterDLL("ssigraphic.dll");
	Factory::RegisterDLL("ssimouse.dll");
	Factory::RegisterDLL("ssiioput.dll");

	ssi_print("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ssi_random_seed();

	Exsemble ex;
	ex.console(0, 0, 650, 800);
	ex.add(ex_signal, 0, "SIGNAL", "");
	ex.add(ex_cast, 0, "CAST", "");
	ex.add(ex_windows, 0, "WINDOWS", "");
	ex.add(ex_butter_filter, 0, "BUTTER I", "");
	ex.add(ex_butter_filter_2, 0, "BUTTER II", "");
	ex.add(ex_butter_filter_3, 0, "BUTTER III", "");
	ex.add(ex_block_boost_filter, 0, "BLOCK & BOOST", "");
	ex.add(ex_derivative, 0, "DERIVATIVE", "");
	ex.add(ex_integral, 0, "INTEGRAL", "");
	ex.add(ex_energy, 0, "ENERGY", "");
	ex.add(ex_fft, 0, "FFT", "");
	ex.add(ex_fftfeat, 0, "FFT FEATURES", "");
	ex.add(ex_spectrogram, 0, "SPECTROGRAM", "");
	ex.add(ex_mfcc, 0, "MFCC", "");
	ex.add(ex_functionals, 0, "FUNCTIONALS", "");
	ex.add(ex_downsample, 0, "DOWNSAMPLE", "");
	ex.add(ex_selector, 0, "SELECTOR", "");
	ex.add(ex_chain, 0, "CHAIN", "");
	ex.add(ex_moving, 0, "MOVING", "");
	ex.add(ex_movingminmax, 0, "MOVING MIN/MAX", "");
	ex.add(ex_movingnorm, 0, "MOVING NORM", "");
	ex.add(ex_movingdrvtv, 0, "MOVING DERIVATIVE", "");
	ex.add(ex_movingcondiv, 0, "MOVING CONDIV", "");
	ex.add(ex_movingpeakgate, 0, "MOVING PEAK GATE", "");
	ex.add(ex_movingmedian, 0, "MOVING MEDIAN", "");
	ex.add(ex_bundle, 0, "BUNDLE", "");
	ex.add(ex_pulse, 0, "PULSE", "");
	ex.add(ex_ethres, 0, "THRESHOLD", "");
	ex.add(ex_expression, 0, "EXPRESSION", "");
	ex.add(ex_statistics, 0, "STATISTICS", "Statistics on a Gaussian signal (sigma = 10, mean = 0, sample rate = 133, window = 20s) " \
		"Dimensions: 0 - kurtosis " \
		"1 - skewness " \
		"2 - mean " \
		"3 - stddev " \
		"4 - var " \
		"5 - number-of-values ");
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

void plot() {

	ssi_size_t n_plots = (ssi_size_t)plot_objects.size();

	Canvas **canvas = new Canvas *[n_plots];
    ssi::Window **window = new ssi::Window *[n_plots];

	ssi_rect_t pos;
	pos.left = 650;
	pos.top = 0;
	pos.width = 400;
	pos.height = 800 / n_plots;
	for (ssi_size_t i = 0; i < n_plots; i++) {
		canvas[i] = new Canvas();
		canvas[i]->addClient(plot_objects[i]);
        window[i] = new ssi::Window();
		window[i]->setClient(canvas[i]);
		window[i]->setTitle(plot_names[i]);
		pos.top = i * pos.height;
		window[i]->setPosition(pos);
		window[i]->create();
		window[i]->show();
	}

	ssi_print("\n\tpress a key to continue\n\n");
	getchar();

	for (size_t i = 0; i < n_plots; i++) {
		window[i]->close();
		delete window[i];
		delete canvas[i];
		delete plot_objects[i];
		//delete[] plot_names[i];
	}
	plot_objects.clear();
	plot_names.clear();
	delete[] window;
	delete[] canvas;
}

void plot_push (Matrix<ssi_real_t> *matrix, ssi_char_t *name, bool transpose, bool as_image = false) {

	ssi_size_t num = transpose ? matrix->cols : matrix->rows;
	ssi_size_t dim = transpose ? matrix->rows : matrix->cols;

	ssi_stream_t stream;
	ssi_stream_init(stream, num, dim, sizeof(ssi_real_t), SSI_REAL, 0);
	memcpy(stream.ptr, matrix->data, stream.tot);
	
	PaintData *plot = new PaintData();
	plot->setData(stream, as_image ? PaintData::TYPE::IMAGE : PaintData::TYPE::SIGNAL);
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

bool ex_signal(void *args) {

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

	plot();

	ssi_stream_destroy (series);

	return true;
}

bool ex_cast(void *args) {

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

	filter->getOptions()->cast = SSI_DOUBLE;
	transform (series, result_double, *filter, 10, 0); 

	plot_push (result_double, "Double");
	save (result_double, "cast_double");

	filter->getOptions()->cast = SSI_SHORT;
	transform (series, result_short, *filter, 10, 0); 

	plot_push (result_short, "Short");
	save (result_short, "cast_short");

	plot();

	ssi_stream_destroy (series);
	ssi_stream_destroy (result_short);
	ssi_stream_destroy (result_double);

	return true;
}

bool ex_windows(void *args) {

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

	plot();

	return true;
}

bool ex_butter_filter(void *args) {

	ssi_print ("IIR Filter Example ...\n");

	ssi_stream_t signal;
	ssi_stream_init (signal, 0, 3, sizeof (ssi_real_t), SSI_REAL, 200.0);
	SignalTools::Series (signal, 1.0);

	ssi_time_t sine_f[3] = {5.0, 25.0, 45.0};
	ssi_real_t sine_a[3] = {1.0f, 1.0f, 1.0f};	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_real_t lcutoff = 0.15f;
	ssi_real_t hcutoff = 0.35f;
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
	IIR *iir = ssi_create(IIR, 0, false);

	ssi_stream_destroy (fftmag);
	
	// filter

	Butfilt *filter = ssi_create (Butfilt, "butfilt", true);
	filter->getOptions()->low = lcutoff;
	filter->getOptions()->high = hcutoff;
	filter->getOptions()->order = order;

	// lowpass
	
	filter->getOptions()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 
	response = iir->Response(filter->getCoefs(), 512);	
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Lowpass Response", true);
	plot_push (result, "Lowpassed Signal");
	save (result, "lowpass");

	plot();

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// bandpass

	filter->getOptions()->type = Butfilt::BAND;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(filter->getCoefs(), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Bandpass Response", true);
	plot_push (result, "Bandpassed Signal");
	save (result, "bandpass");

	plot();

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// highpass

	filter->getOptions()->type = Butfilt::HIGH;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(filter->getCoefs(), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Highpass Response", true);
	plot_push (result, "Highpassed Signal");
	save (result, "highpass");

	plot();

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;
	ssi_stream_destroy (signal);
	delete iir;

	return true;
}

bool ex_butter_filter_2(void *args) {

	ssi_print ("IIR Filter Example ...\n");

	ssi_time_t sr = 200.0;

	IIR *iir = ssi_create(IIR, 0, false);

	ssi_stream_t signal;
	ssi_stream_init (signal, 0, 2, sizeof (ssi_real_t), SSI_REAL, sr);
	SignalTools::Series (signal, 5.0);

	ssi_time_t sine_f[2] = {5.0, 30.0};
	ssi_real_t sine_a[2] = {0.5f, 0.5f};	
	SignalTools::Sine (signal, sine_f, sine_a);
	SignalTools::Sum (signal);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * sr);
	ssi_real_t lcutoff = 10.0f;
	ssi_real_t hcutoff = 20.0f;
	int order = 8;	

	plot_push (signal, "Signal");

	FilterTools::Noise (signal, 1.0, 0.0, 1.0, lcutoff, hcutoff - lcutoff);
	plot_push (signal, "Signal + Noise");

	ssi_stream_t result;
	Matrix<ssi_real_t> *magnitude = 0;
	Matrix<std::complex<ssi_real_t>> *response = 0;
	
	// filter

	Butfilt *filter = ssi_create (Butfilt, "butfilt", true);
	filter->getOptions()->low = ssi_real_t(2 * lcutoff / sr);
	filter->getOptions()->high = ssi_real_t(2 * hcutoff / sr);
	filter->getOptions()->order = order;

	// lowpass
	
	filter->getOptions()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(filter->getCoefs(), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Lowpass Response", true);
	plot_push (result, "Lowpassed Signal");
	save (result, "lowpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// bandpass

	filter->getOptions()->type = Butfilt::BAND;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(filter->getCoefs(), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Bandpass Response", true);
	plot_push (result, "Bandpassed Signal");
	save (result, "bandpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// highpass

	filter->getOptions()->type = Butfilt::HIGH;
	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(filter->getCoefs(), 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Highpass Response", true);
	plot_push (result, "Highpassed Signal");
	save (result, "highpass");

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;

	// plot
	plot();

	ssi_stream_destroy (signal);
	delete iir;

	return true;
}

bool ex_butter_filter_3(void *args) {

	ssi_print ("Lowpass Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	// filter

	Butfilt *filter = ssi_create (Butfilt, "butfilt", true);
	filter->getOptions()->low = 0.1f;
	filter->getOptions()->order = 4;
	filter->getOptions()->zero = true;

	// lowpass

	plot_push (signal, "Signal");

	ssi_stream_t result;
	filter->getOptions()->type = Butfilt::LOW;
	transform (signal, result, *filter, frame_size, 0); 

	plot_push (result, "Lowpassed");
	save (result, "lowpass");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_block_boost_filter(void *args) {

	ssi_print ("IIR Filter Example 2 ...\n");

	IIR *iir = ssi_create(IIR, 0, false);

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

	IIR *filter = ssi_create (IIR, 0, true);

	// DC blocker
	
	coefs = FilterTools::DCBlocker (R);
	filter->setCoefs(coefs);	

	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(coefs, 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Blocker Response", true);
	plot_push (result, "Blocked Signal");
	save (result, "blocked");

	plot();

	ssi_stream_destroy (result);
	delete magnitude;
	delete response;
	delete coefs;

	// Boost
	
	coefs = FilterTools::Boost (signal.sr, 10.0, 50.0, 5.0);
	filter->setCoefs(coefs);	

	transform (signal, result, *filter, frame_size, 0); 
			
	response = iir->Response(coefs, 512);
	magnitude = MatrixOps<ssi_real_t>::Abs (response);	

	plot_push (magnitude, "Boost Response", true);
	plot_push (result, "Boosed Signal");
	save (result, "boosed");
	
	ssi_stream_destroy (result);
	delete magnitude;
	delete response;
	delete coefs;

	// plot
	plot();

	ssi_stream_destroy (signal);
	delete iir;

	return true;
}

bool ex_derivative(void *args) {

	ssi_print ("Derivative Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	Derivative *derivative = ssi_create (Derivative, "derivative", true);
	derivative->getOptions()->set(Derivative::D4TH | Derivative::D2ND);

	ssi_stream_t result;
	transform (signal, result, *derivative, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Derivative");
	save (result, "derivative");

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_integral(void *args) {

	ssi_print ("Integral Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.5 * signal.sr);

	Integral *integral = ssi_create (Integral, "integral", true);
	integral->getOptions()->set(Integral::I1ST | Integral::I0TH | Integral::I4TH);

	ssi_stream_t result;
	transform (signal, result, *integral, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Integral");
	save (result, "integral");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_energy(void *args) {

	ssi_print ("Energy Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);

	Energy *energy = ssi_create (Energy, 0, true);
	ssi_stream_t result_energy;
	transform (signal, result_energy, *energy, frame_size, delta_size);

	Intensity *intensity = ssi_create (Intensity, 0, true);
	ssi_stream_t result_intensity;
	transform (signal, result_intensity, *intensity, frame_size, delta_size);
	
	plot_push (signal, "Signal");
	plot_push (result_energy, "Energy");
	save (result_energy, "energy");
	plot_push (result_intensity, "Intensity");
	save (result_intensity, "intensity");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result_energy);
	ssi_stream_destroy (result_intensity);

	return true;
}

bool ex_fft(void *args) {

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

    plot();

	delete fftpoints;
	delete magnitude;
	ssi_stream_destroy (signal);

	return true;
}

bool ex_fftfeat(void *args) {

	ssi_print ("FFT Feature Example...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "audio");

	ssi_size_t frame_size = 16;
	ssi_size_t delta_size = 16;

	FFTfeat *fftfeat =  ssi_create (FFTfeat, 0, true);
	fftfeat->getOptions()->nfft = 32;
	transform (signal, result, *fftfeat, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "FFT Features");

	save (result, "fftfeat");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_spectrogram(void *args) {

	ssi_print ("Spectrogram Example ...\n");

	ssi_stream_t signal;
	load (signal, "audio");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.01 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.015 * signal.sr);

	int fft_size = 512;

	Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank (fft_size, signal.sr, 50, 100, 5100, WINDOW_TYPE_RECTANGLE);
	Spectrogram *spectrogram = ssi_create (Spectrogram, "spect", true);
	//spectrogram->setFilterbank(filterbank, WINDOW_TYPE_RECTANGLE, true);
	delete filterbank;

	ssi_stream_t result;
	transform (signal, result, *spectrogram, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "Spectrogram", true);
	save (result, "spect");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_mfcc(void *args) {

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
	MFCC *mfcc = ssi_create (MFCC, 0, true);
	mfcc->getOptions()->n_ffts = fft_size;
	mfcc->getOptions()->n_first = n_first;
	mfcc->getOptions()->n_last = n_last;
	ssi_stream_t result;
	transform (signal, result, *mfcc, frame_size, delta_size);
	
	plot_push (signal, "Signal");
	plot_push (dctmatrix, "DCT Matrix", false, true);
	plot_push (result, "MFCCs", true);
	save (result, "mfcc");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	delete filterbank;
	delete dctmatrix;

	return true;
}

bool ex_functionals(void *args) {

	ssi_print ("Functionals Example ...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "audio");
	
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);
	ssi_size_t delta_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);

	Functionals *functs = ssi_create (Functionals, "functs", true);
	functs->getOptions()->delta = 10;	
	transform (signal, result, *functs, frame_size, delta_size);

	ssi_print ("selected functionals:\n");
	for (ssi_size_t i = 0; i < functs->getSize(); i++) {
		ssi_print ("\t%s\n", functs->getName(i));
	}

	save (result, "statistics");
	plot_push (signal, "Signal");
	plot_push (result, "Statistics");

	plot();

	ssi_stream_destroy (result);
	ssi_stream_destroy (signal);

	return true;
}

bool ex_downsample(void *args) {

	ssi_print ("Down Sample Example ...\n");

	ssi_stream_t signal;
	ssi_stream_t result;
	load (signal, "rsp");

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.05 * signal.sr);
	ssi_size_t delta_size = 0;
	ssi_size_t factor = 10;
	
	DownSample *downsample = ssi_create (DownSample, "downsample", true);
	downsample->getOptions()->keep = factor;
	transform (signal, result, *downsample, frame_size, delta_size);

	plot_push (signal, "Signal");
	plot_push (result, "Downsampled");
	save (result, "downsampled");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_selector(void *args) {

	ssi_print ("Signal Example ...\n");

	ssi_stream_t series;
	ssi_stream_init (series, 0, 5, sizeof (ssi_real_t), SSI_REAL, 100.0);
	SignalTools::Series (series, 5.0);

	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * 100.0);

	ssi_time_t sine_f[] = {1.0, 5.0, 20.0, 30.0, 50.0};
	ssi_real_t sine_a[] = {2.0f, 0.5f, 0.4f, 0.3f, 0.2f};	
	SignalTools::Sine (series, sine_f, sine_a);
	
	plot_push (series, "Signal");
	
	Selector *selector = ssi_create (Selector, "selector", true);
	ssi_size_t inds[] = {4,2};
	selector->getOptions()->set(2, inds);

	ssi_stream_t result;
	transform (series, result, *selector, frame_size, 0);
	
	plot_push (result, "Selector");
	save (result, "selector");

	plot();

	ssi_stream_destroy (series);
	ssi_stream_destroy (result);

	return true;
}

bool ex_chain(void *args) {

	ssi_print ("Chain Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	Chain *chain = ssi_create(Chain, "chain", true);
	chain->getOptions()->set("mychain");

	ssi_stream_t result;
	transform (signal, result, *chain, frame_size, 0);
	
	plot_push (signal, "Signal");
	plot_push (result, "Chain");
	save (result, "chain");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_moving(void *args) {

	ssi_print ("Moving/Sliding Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgAvgVar *moving = ssi_create (MvgAvgVar, "moving", true);
	moving->getOptions()->win = 1.0;
	moving->getOptions()->format = MvgAvgVar::ALL;
	moving->getOptions()->method = MvgAvgVar::MOVING;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgAvgVar *sliding = ssi_create (MvgAvgVar, "sliding", true);
	sliding->getOptions()->win = 1.0;
	sliding->getOptions()->format = MvgAvgVar::ALL;
	sliding->getOptions()->method = MvgAvgVar::SLIDING;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "Moving");
	plot_push (result2, "Sliding");	
	save (result, "moving");
	save (result2, "sliding");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingminmax(void *args) {

	ssi_print ("Moving/Sliding Min/Max Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgMinMax *moving = ssi_create (MvgMinMax, "movingminmax", true);
	moving->getOptions()->win = 1.0;
	moving->getOptions()->format = MvgMinMax::ALL;
	moving->getOptions()->method = MvgMinMax::MOVING;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgMinMax *sliding = ssi_create (MvgMinMax, "slidingminmax", true);
	sliding->getOptions()->win = 1.0;
	sliding->getOptions()->format = MvgMinMax::ALL;
	sliding->getOptions()->method = MvgMinMax::SLIDING;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingMinMax");
	plot_push (result2, "SlidingMinMax");	
	save (result, "movingminmax");
	save (result2, "slidingminmax");

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingnorm(void *args) {

	ssi_print ("Moving/Sliding Norm Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	MvgNorm::NORM norm = MvgNorm::SUBMIN;

	MvgNorm *moving = ssi_create (MvgNorm, "movingnorm", true);
	moving->getOptions()->win = 1.0;	
	moving->getOptions()->method = MvgNorm::MOVING;
	moving->getOptions()->norm = norm;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgNorm *sliding = ssi_create (MvgNorm, "slidingnorm", true);
	sliding->getOptions()->win = 1.0;
	sliding->getOptions()->method = MvgNorm::SLIDING;
	sliding->getOptions()->norm = norm;

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingNorm");
	plot_push (result2, "SlidingNorm");	
	save (result, "movingnorm");
	save (result2, "slidingnorm");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingdrvtv(void *args) {

	ssi_print ("Moving/Sliding Derivative Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgDrvtv *moving = ssi_create (MvgDrvtv, "movingdrvtv", true);
	moving->getOptions()->win = 0.1;	
	moving->getOptions()->method = MvgDrvtv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgDrvtv *sliding = ssi_create (MvgDrvtv, "slidingdrvtv", true);
	sliding->getOptions()->win = 0.1;
	sliding->getOptions()->method = MvgDrvtv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingDrvtv");
	plot_push (result2, "SlidingDrvtv");	
	save (result, "movingdrvtv");
	save (result2, "slidingdrvtv");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingcondiv(void *args) {

	ssi_print ("Moving/Sliding Condiv Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	MvgConDiv *moving = ssi_create (MvgConDiv, "movingcondiv", true);
	moving->getOptions()->wins = wins;	
	moving->getOptions()->winl = winl;	
	moving->getOptions()->method = MvgConDiv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgConDiv *sliding = ssi_create (MvgConDiv, "slidingcondiv", true);
	sliding->getOptions()->wins = wins;	
	sliding->getOptions()->winl = winl;	
	sliding->getOptions()->method = MvgConDiv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingpeakgate(void *args) {

	ssi_print ("Moving PeakGate Example ...\n");

	ssi_stream_t signal;
	ssi_real_t fix = 0.002f;
	ssi_real_t win = 0.5f;
	MvgPeakGate::THRESHOLD thres = MvgPeakGate::FIXAVGSTD;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);

	MvgPeakGate *moving = ssi_create (MvgPeakGate, "movingpeakgate", true);
	moving->getOptions()->thres = thres;
	moving->getOptions()->win = win;
	moving->getOptions()->fix = fix;
	moving->getOptions()->method = MvgPeakGate::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgPeakGate *sliding = ssi_create (MvgPeakGate, "slidingpeakgate", true);
	sliding->getOptions()->thres = thres;
	sliding->getOptions()->win = win;
	sliding->getOptions()->fix = fix;	
	sliding->getOptions()->method = MvgPeakGate::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_movingmedian(void *args) {

	ssi_print ("Moving Median Example ...\n");

	ssi_stream_t signal;
	ssi_size_t win = 128;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.2 * signal.sr);

	MvgMedian *moving = ssi_create (MvgMedian, "movingmedian", true);
	moving->getOptions()->winInSamples = win;

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingMedian");
	save (result, "movingmedian");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);	

	return true;
}

bool ex_bundle(void *args) {

	ssi_print("Bundle Example ...\n");

	ssi_stream_t tmp, signal;	
	load(tmp, "noise");
	ssi_size_t select = 0;
	ssi_stream_select(tmp, signal, 1, &select);	
	ssi_size_t frame_size = 1;

	Randomf random(0, 1);

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, signal.ptr);
	for (ssi_size_t i = 0; i < signal.num; i++) {
		if (abs(ptr[i * signal.dim]) > 1.5 && random.next () > 0.7) {
			ptr[i * signal.dim] *= -5.0;
			i += 10;
		}
	}

	ssi_size_t hang_in = 2;
	ssi_size_t hang_out = 2;

	Bundle *bundle = ssi_create (Bundle, "bundle", true);
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

	plot();

	ssi_stream_destroy(tmp);
	ssi_stream_destroy(signal);
	ssi_stream_destroy(result);

	return true;
}

bool ex_pulse(void *args) {

	ssi_print ("Pulse Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	MvgConDiv *moving = ssi_create (MvgConDiv, "movingcondiv", true);
	moving->getOptions()->wins = wins;	
	moving->getOptions()->winl = winl;	
	moving->getOptions()->method = MvgConDiv::MOVING;	

	ssi_stream_t result;
	transform (signal, result, *moving, frame_size, 0);
	
	MvgConDiv *sliding = ssi_create (MvgConDiv, "slidingcondiv", true);
	sliding->getOptions()->wins = wins;	
	sliding->getOptions()->winl = winl;	
	sliding->getOptions()->method = MvgConDiv::SLIDING;	

	ssi_stream_t result2;
	transform (signal, result2, *sliding, frame_size, 0);

	plot_push (signal, "Signal");
	plot_push (result, "MovingConDiv");
	plot_push (result2, "SlidingConDiv");	
	save (result, "movingcondiv");
	save (result2, "slidingcondiv");	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);
	ssi_stream_destroy (result2);	

	return true;
}

bool ex_ethres(void *args) {

	ssi_print ("Threshold Event Example ...\n");

	ssi_stream_t signal;
	load (signal, "rsp");
	ssi_size_t frame_size = ssi_cast (ssi_size_t, 0.1 * signal.sr);
	ssi_time_t wins = 0.1;
	ssi_time_t winl = 5.0;

	old::FileAnnotationWriter awrite;

	ThresEventSender *tes = ssi_create (ThresEventSender, 0, true);
	tes->getOptions()->thres = 0;
	tes->setEventListener(&awrite);
	SignalTools::Consume (signal, *tes, frame_size);

	plot_push (signal, "Signal");	

	plot();

	ssi_stream_destroy (signal);

	return true;
}

bool ex_expression(void *args) {

	ssi_print ("Expression Example ...\n");

	ssi_stream_t signal, result;
	load (signal, "rsp");
	
	Expression *expression = ssi_create (Expression, 0, true);
	//expression->getOptions()->single = true;
	//expression->getOptions()->setExpression("sin(sqrt(d0*d1))");
	expression->getOptions()->single = false;
	expression->getOptions()->join = Expression::JOIN::MULT;
	expression->getOptions()->setExpression("(100000*d) ^2");
	
	SignalTools::Transform (signal, result, *expression, 0u);
	plot_push (signal, "Signal");
	plot_push (result, expression->getOptions()->expression);	

	plot();

	ssi_stream_destroy (signal);
	ssi_stream_destroy (result);

	return true;
}

bool ex_statistics(void *arg)
{
	ITheEventBoard *board = Factory::GetEventBoard();
	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	FileReader *fileReader = ssi_create(FileReader, 0, true);
	fileReader->getOptions()->setPath("gauss_m0_sigma_10");
	fileReader->getOptions()->loop = true;

	ITransformable *gauss_p = frame->AddProvider(fileReader, SSI_FILEREADER_PROVIDER_NAME, 0, "21.0s");
	frame->AddSensor(fileReader);

	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("raw Gaussian noise");
	plot->getOptions()->size = 20;
	frame->AddConsumer(gauss_p, plot, "1");

	Statistics *statistics = ssi_create (Statistics, "statistics", true);
	statistics->getOptions()->kurtosis = true;
	statistics->getOptions()->skewness = true;
	statistics->getOptions()->mean = true;
	statistics->getOptions()->stddev = true;
	statistics->getOptions()->var = true;
	statistics->getOptions()->number_vals = true;

	ITransformable * statistics_t = frame->AddTransformer(gauss_p, statistics, "1s", "19s", "21s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("statistical evaluation");
	plot->getOptions()->size = 20;
	frame->AddConsumer(statistics_t, plot, "1s");

	decorator->add("plot*", 650, 0, 400, 800);

	board->Start();
	frame->Start();
	frame->Wait();
	frame->Stop();
	board->Stop();
	frame->Clear();
	board->Clear();

	return true;
}
