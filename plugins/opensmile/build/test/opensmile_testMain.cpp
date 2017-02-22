// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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
#include "ssiopensmile.h"
#include "audio/include/ssiaudio.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define SIMULATE

bool ex_functionals (void *args);
bool ex_intensity(void *args);
bool ex_lpc(void *args);
bool ex_plp(void *args);
bool ex_mfcc(void *args);
bool ex_mfcc_chain(void *args);
bool ex_pitch(void *args);
bool ex_pitch_chain(void *args);
bool ex_vad(void *args);
bool ex_pitchdirection(void *args);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe");
	Factory::RegisterDLL ("ssiopensmile");
	Factory::RegisterDLL ("ssigraphic");
	Factory::RegisterDLL ("ssiaudio");
	Factory::RegisterDLL ("ssisignal");
	Factory::RegisterDLL ("ssiioput");
	Factory::RegisterDLL ("ssievent");

	Exsemble ex;
	ex.add(ex_functionals, 0, "FUNCTIONALS", "Demonstrates extraction of functionals");
	ex.add(ex_intensity, 0, "INTENSITY", "Demonstrates extraction of intensity");
	ex.add(ex_lpc, 0, "LPC", "Demonstrates extraction of LPC coefficients");
	ex.add(ex_plp, 0, "PLP", "Demonstrates extraction of PLP coefficients");
	ex.add(ex_mfcc, 0, "MFCC", "Demonstrates extraction of MFCC coefficients");
	ex.add(ex_mfcc_chain, 0, "MFCC-CHAIN", "Demonstrates extraction of MFCC coefficients");	
	ex.add(ex_pitch, 0, "PITCH", "Demonstrates extraction of pitch");
	ex.add(ex_pitch_chain, 0, "PITCH-CHAIN", "Demonstrates extraction of pitch");
	ex.add(ex_pitchdirection, 0, "PITCH-DIRECTION", "Demonstrates extraction of pitch direction");
	ex.add(ex_vad, 0, "VAD", "Demonstrates voice activity detection");
	ex.show();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

bool ex_functionals(void *args) {

	ssi_stream_t audio_s;
	WavTools::ReadWavFile ("audio.wav", audio_s, true);

	ssi_stream_t energy_s;
	OSEnergy *energy = ssi_create (OSEnergy, 0, true);
	energy->getOptions()->type = OSEnergy::BOTH;
	SignalTools::Transform (audio_s, energy_s, *energy, "0.01s", "0.01s");

	FileTools::WriteStreamFile (File::ASCII, "energy", energy_s);

	ssi_stream_t functionals_s;
	OSFunctionals *functionals = ssi_create (OSFunctionals, "functionals", true);
	SignalTools::Transform (energy_s, functionals_s, *functionals, 0u);
	
	FileTools::WriteStreamFile (File::ASCII, "functionals", functionals_s);

	ssi_stream_destroy (audio_s);
	ssi_stream_destroy (energy_s);
	ssi_stream_destroy (functionals_s);

	ssi_print("\n\n\tFunctionals extraction finished and result stored to 'functionals.stream'\n\n")

	return true;
}

bool ex_intensity(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	OSIntensity *intensity = ssi_create (OSIntensity, "OSIntensity", true);
	ITransformable *intensity_t = frame->AddTransformer(audio_p, intensity, "0.1s", "0.1s");

	SignalPainter *plot = 0;	

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;		
	frame->AddConsumer(audio_p, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("intensity");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(intensity_t, plot, "1"); 

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("intensity");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(intensity_t, writer, "1");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();	

	return true;
}

bool ex_lpc (void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create(WavReader, 0, true);
	audio->getOptions()->setPath("formants.wav");
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	OSLpc *lpc = ssi_create(OSLpc, "OSLpc", true);
	ITransformable *lpc_t = frame->AddTransformer(audio_p, lpc, "0.06s", "0.01s");

	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(audio_p, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("lpc");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(lpc_t, plot, "1");

	FileWriter *writer = ssi_create(FileWriter, 0, true);
	writer->getOptions()->setPath("lpc");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(lpc_t, writer, "1");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_plp(void *args) {

	ITheFramework *frame = Factory::GetFramework();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create(Audio, "audio", true);
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create(WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	OSPlpChain *plp = ssi_create(OSPlpChain, "OSPlpChain", true);
	ITransformable *plp_t = frame->AddTransformer(audio_p, plp, "0.06s", "0.01s");

	SignalPainter *plot = 0;

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(audio_p, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("lpc");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(plp_t, plot, "1");

	FileWriter *writer = ssi_create(FileWriter, 0, true);
	writer->getOptions()->setPath("plp_chain");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(plp_t, writer, "1");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();

	return true;
}

bool ex_pitch(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	// fft
	OSTransformFFT *fft = ssi_create (OSTransformFFT, "OSTransformFFT", true);
	fft->getWindow()->getOptions()->type = OSWindow::GAUSS;
	fft->getWindow()->getOptions()->gain = 1.0;
	fft->getWindow()->getOptions()->sigma = 0.4;	
	double frameSize = audio_p->getSampleRate() * 0.01;
	double deltaSize = audio_p->getSampleRate() * 0.04;
	fft->getOptions()->nfft = smileMath_ceilToNextPowOf2(frameSize + deltaSize); //we set the FFT frame size so it's a power of 2 but can also fit all the samples
	ITransformable *fft_t = frame->AddTransformer(audio_p, fft, "0.01s", "0.04s");

	// fftmag
	OSFFTmagphase *fftmag = ssi_create (OSFFTmagphase, "OSFFTmagphase", true);
	ITransformable *fftmag_t = frame->AddTransformer(fft_t, fftmag, "0.1s");

	// specscale
	OSSpecScale *specscale = ssi_create (OSSpecScale, "OSSpecScale", true);
	specscale->getOptions()->srcScale = OSSpecScale::LINEAR;
	specscale->getOptions()->dstScale = OSSpecScale::LOG;
	specscale->getOptions()->dstLogScaleBase = 2.0;	
	specscale->getOptions()->minF = 20;
	specscale->getOptions()->smooth = true;
	specscale->getOptions()->enhance = true;
	specscale->getOptions()->weight = true;
	specscale->getOptions()->fsSec = 0.064; // BUG?	
	ITransformable *specscale_t = frame->AddTransformer(fftmag_t, specscale, "0.1s");

	// pitchshs
	OSPitchShs *pitchshs = ssi_create (OSPitchShs, "OSPitchShs", false);	
	pitchshs->getOptions()->maxPitch = 620;
	pitchshs->getOptions()->minPitch = 42;
	pitchshs->getOptions()->nCandidates = 6;
	pitchshs->getOptions()->scores = true;
	pitchshs->getOptions()->voicing = true;
	pitchshs->getOptions()->F0C1 = false;
	pitchshs->getOptions()->voicingC1 = false;
	pitchshs->getOptions()->F0raw = false;
	pitchshs->getOptions()->voicingClip = false;
	pitchshs->getOptions()->voicingCutoff = 0.7f;
	pitchshs->getOptions()->octaveCorrection = false;
	pitchshs->getOptions()->fsSec = 0.064; // BUG?
	pitchshs->getOptions()->baseSr = audio_p->getSampleRate();
	ITransformable *pitchshs_t = frame->AddTransformer(specscale_t, pitchshs, "0.1s");

	// pitchsmooth
	OSPitchSmoother *pitchsmooth = ssi_create (OSPitchSmoother, "OSPitchSmoother", true);
	pitchsmooth->getOptions()->medianFilter0 = 0;
	pitchsmooth->getOptions()->postSmoothing = 0;
	pitchsmooth->getOptions()->postSmoothingMethod = OSPitchSmoother::SIMPLE;
	pitchsmooth->getOptions()->octaveCorrection = false;
	pitchsmooth->getOptions()->F0final = true;
	pitchsmooth->getOptions()->F0finalEnv = false;
	pitchsmooth->getOptions()->voicingFinalClipped = false;
	pitchsmooth->getOptions()->voicingFinalUnclipped = true;
	pitchsmooth->getOptions()->F0raw = false;
	pitchsmooth->getOptions()->voicingC1 = false;
	pitchsmooth->getOptions()->voicingClip = false;
	ITransformable *pitchsmooth_t = frame->AddTransformer(pitchshs_t, pitchsmooth, "0.1s");

	// plot
	SignalPainter *plot = 0;	

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;		
	frame->AddConsumer(audio_p, plot, "0.1s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("fft");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(fft_t, plot, "0.1s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("fftmag");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(fftmag_t, plot, "0.1s"); 

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("specscale");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(specscale_t, plot, "0.1s"); 

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pitchshs");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(pitchshs_t, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pitchsmooth");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(pitchsmooth_t, plot, "0.1s"); 

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("pitch");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(pitchsmooth_t, writer, "0.1s");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();	

	return true;
}

bool ex_pitch_chain(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	audio->getOptions()->block = 0.01;
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	// pitch
	OSPitchChain *pitch = ssi_create (OSPitchChain, "pitch", true);	
	pitch->getOSTransformFFT()->getOptions()->nfft = 1024;
	pitch->getOSPitchShs()->getOptions()->baseSr = audio_p->getSampleRate();
	ITransformable *pitch_t = frame->AddTransformer(audio_p, pitch, "0.01s", "0.01s");

	// plot
	SignalPainter *plot = 0;	

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;		
	frame->AddConsumer(audio_p, plot, "0.01s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("pitch");
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(pitch_t, plot, "0.01s"); 

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("pitch_chain");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(pitch_t, writer, "0.01s");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();	

	return true;
}

bool ex_mfcc(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	// preemph
	OSPreemphasis *preemph = ssi_create (OSPreemphasis, "preemph", true);
	ITransformable *preemph_t = frame->AddTransformer(audio_p, preemph, "0.02s");

	// energy
	OSEnergy *energy = ssi_create (OSEnergy, "energy", true);
	ITransformable *energy_t = frame->AddTransformer(preemph_t, energy, "0.015s", "0.01s");

	// fft
	OSTransformFFT *fft = ssi_create (OSTransformFFT, "fft", true);
	fft->getOptions()->nfft = 2048;
	ITransformable *fft_t = frame->AddTransformer(preemph_t, fft, "0.01s", "0.01s");

	// fftmag
	OSFFTmagphase *fftmag = ssi_create (OSFFTmagphase, "fftmag", true);
	ITransformable *fftmag_t = frame->AddTransformer(fft_t, fftmag, "0.1s");

	// melspec
	OSMelspec *melspec = ssi_create (OSMelspec, "melspec", true);
	ITransformable *melspec_t = frame->AddTransformer(fftmag_t, melspec, "0.1s");

	// mfcc
	OSMfcc *mfcc = ssi_create (OSMfcc, "mfcc", true);
	ITransformable *mfcc_t = frame->AddTransformer(melspec_t, mfcc, "0.1s");

	// plot
	SignalPainter *plot = 0;	

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;		
	frame->AddConsumer(preemph_t, plot, "0.1s");

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("energy");
	plot->getOptions()->size = 10.0;	
	frame->AddConsumer(energy_t, plot, "0.1s"); 
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("fft");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(fft_t, plot, "0.1s"); 
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("fftmag");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(fftmag_t, plot, "0.1s"); 

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("melspec");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;
	frame->AddConsumer(melspec_t, plot, "0.1s"); 

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mfcc");
	plot->getOptions()->size = 10.0;	
	plot->getOptions()->type = PaintSignalType::IMAGE;	
	frame->AddConsumer(mfcc_t, plot, "0.1s"); 

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("mfcc2");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(mfcc_t, writer, "0.1s");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();	

	return true;
}

bool ex_mfcc_chain(void *args) {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	// preemph
	OSPreemphasis *preemph = ssi_create (OSPreemphasis, "preemph", true);
	ITransformable *preemph_t = frame->AddTransformer(audio_p, preemph, "0.02s");

	// fft
	OSMfccChain *mfcc = ssi_create (OSMfccChain, "mfcc", true);	
	mfcc->getOSTransformFFT()->getOptions()->nfft = 2048;
	ITransformable *mfcc_t = frame->AddTransformer(preemph_t, mfcc, "0.01s", "0.01s");

	// plot
	SignalPainter *plot = 0;	

	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("audio");
	plot->getOptions()->type = PaintSignalType::AUDIO;
	plot->getOptions()->size = 10.0;		
	frame->AddConsumer(audio_p, plot, "0.1s");
	
	plot = ssi_create_id (SignalPainter, 0, "plot");
	plot->getOptions()->setTitle("mfcc");
	plot->getOptions()->type =  PaintSignalType::IMAGE;
	plot->getOptions()->size = 10.0;
	frame->AddConsumer(mfcc_t, plot, "0.1s"); 

	FileWriter *writer = ssi_create (FileWriter, 0, true);
	writer->getOptions()->setPath("mfcc_chain");
	writer->getOptions()->type = File::ASCII;
	frame->AddConsumer(mfcc_t, writer, "0.1s");

#ifdef SIMULATE
	AudioPlayer *player = ssi_create(AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

	// run framework
	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();	

	return true;
}

bool ex_vad(void *args) {

	//general
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast (TheEventBoard, board)->getOptions()->update = 250;

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);

	SignalPainter *audio_plot = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 10.0;		
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot, "0.2s");

	OSLpc * lpc = ssi_create (OSLpc, 0, true);
	lpc->getOptions()->lsp = true;
	ssi::ITransformable *lpc_p = frame->AddTransformer(audio_p, lpc, "160");
	
	OSPitchChain * pitch = ssi_create (OSPitchChain, 0, true);
	pitch->getOSPitchShs()->getOptions()->voicingC1 = true;
	pitch->getOSPitchSmoother()->getOptions()->voicingC1 = true;
	ssi::ITransformable *pitch_p = frame->AddTransformer(audio_p, pitch, "160", "160");
	
	OSEnergy * energy = ssi_create (OSEnergy, 0, true);
	energy->getOptions()->type = OSEnergy::TYPE::LOG;
	ssi::ITransformable *energy_p = frame->AddTransformer(audio_p, energy, "160");

	OSVad * vad = ssi_create (OSVad, 0, true);
	ssi::ITransformable * src[3] = {lpc_p, pitch_p, energy_p};
	frame->AddConsumer(3, src, vad, "0.01s");
	board->RegisterSender(*vad);

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, vad->getEventAddress());

#ifdef SIMULATE
	AudioPlayer *player = ssi_create (AudioPlayer, "player", true);
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

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

bool ex_pitchdirection(void *args) {

	//general
	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	ITheEventBoard *board = Factory::GetEventBoard ();
	ssi_pcast (TheEventBoard, board)->getOptions()->update = 250;

	// audio sensor	
#ifndef SIMULATE
	Audio *audio = ssi_create (Audio, "audio", true);
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_AUDIO_PROVIDER_NAME);
#else
	WavReader *audio = ssi_create (WavReader, 0, true);
	audio->getOptions()->setPath("audio.wav");
	audio->getOptions()->scale = true;		
	ITransformable *audio_p = frame->AddProvider(audio, SSI_WAVREADER_PROVIDER_NAME);
#endif	
	frame->AddSensor(audio);
	
	OSPitchChain * pitch = ssi_create (OSPitchChain, "pitch", true);
	pitch->getOSPitchSmoother()->getOptions()->F0final = true;
	pitch->getOSPitchSmoother()->getOptions()->F0finalEnv = true;
	double frameSize = audio_p->getSampleRate() * 0.01;
	double deltaSize = audio_p->getSampleRate() * 0.04;
	pitch->getOSTransformFFT()->getOptions()->nfft = smileMath_ceilToNextPowOf2(frameSize + deltaSize); //we set the FFT frame size so it's a power of 2 but can also fit all the samples	
	pitch->getOSPitchShs()->getOptions()->baseSr = audio_p->getSampleRate();
	ssi::ITransformable *pitch_p = frame->AddTransformer(audio_p, pitch, "0.01s", "0.04s");
	
	OSEnergy * energy = ssi_create (OSEnergy, 0, true);
	energy->getOptions()->type = OSEnergy::TYPE::RMS;
	ssi::ITransformable *energy_p = frame->AddTransformer(audio_p, energy, "0.01s");

	OSPitchDirection * dir = ssi_create (OSPitchDirection, 0, true);
	ssi::ITransformable * xtra_src[1] = {energy_p};
	ssi::ITransformable *dir_p = frame->AddTransformer(pitch_p, 1, xtra_src, dir, "0.01s");
	board->RegisterSender(*dir);

	SignalPainter *audio_plot = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot->getOptions()->setTitle("audio");
	audio_plot->getOptions()->size = 10.0;		
	audio_plot->getOptions()->type = PaintSignalType::AUDIO;
	frame->AddConsumer(audio_p, audio_plot, "0.2s");

	SignalPainter *pitch_plot = ssi_create_id (SignalPainter, 0, "plot");
	pitch_plot->getOptions()->setTitle("pitch");
	pitch_plot->getOptions()->size = 10.0;		
	pitch_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(pitch_p, pitch_plot, "0.2s");

	SignalPainter *energy_plot = ssi_create_id (SignalPainter, 0, "plot");
	energy_plot->getOptions()->setTitle("energy");
	energy_plot->getOptions()->size = 10.0;		
	energy_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(energy_p, energy_plot, "0.2s");
	
	SignalPainter *dir_plot = ssi_create_id (SignalPainter, 0, "plot");
	dir_plot->getOptions()->setTitle("direction");
	dir_plot->getOptions()->size = 10.0;		
	dir_plot->getOptions()->type = PaintSignalType::SIGNAL;
	frame->AddConsumer(dir_p, dir_plot, "0.2s");

	EventMonitor *monitor = ssi_create_id (EventMonitor, 0, "monitor");
	board->RegisterListener(*monitor, dir->getEventAddress());

#ifdef SIMULATE
	AudioPlayer *player = ssi_create (AudioPlayer, "player", true);
	player->getOptions()->bufferSize = 0.2;
	player->getOptions()->remember = true;
	frame->AddConsumer(audio_p, player, "0.01s");
#endif

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
