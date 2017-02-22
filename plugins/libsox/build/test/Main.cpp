// Main.cpp
// author: Andreas Seiderer
// created: 2013/02/08
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
#include "LibsoxFilter.h"
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

void testChain ();

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	Factory::RegisterDLL ("ssiframe.dll");
	Factory::RegisterDLL ("ssiaudio.dll");	
	Factory::RegisterDLL ("ssiioput.dll");
	Factory::RegisterDLL ("ssigraphic.dll");

	Factory::RegisterDLL ("ssilibsoxfilter.dll");

	testChain ();	  	 
	
	ssi_print ("\n\n\tpress enter to quit\n");
	getchar ();

	Factory::Clear ();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif
	
	return 0;
}

void testChain () {

	ITheFramework *frame = Factory::GetFramework ();

	Decorator *decorator = ssi_create (Decorator, 0, true);
	frame->AddDecorator(decorator);

	// wav reader	
	WavReader *wav = ssi_create (WavReader, 0, true);
	wav->getOptions()->setPath("audio.wav");
	//wav->getOptions()->block = 0.1;
	wav->getOptions()->blockInSamples = 1024;
	
	//use 32 bit float instead of 16 bit int?
	wav->getOptions()->scale = false;
	ITransformable *audio_p = frame->AddProvider(wav, SSI_WAVREADER_PROVIDER_NAME);
	frame->AddSensor(wav);

	// plot
	SignalPainter *audio_plot_in = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_in->getOptions()->setTitle("audio in");
	audio_plot_in->getOptions()->size = 1.0;	
	audio_plot_in->getOptions()->type = PaintSignalType::AUDIO;
	//frame->AddConsumer(audio_p, audio_plot_in, "0.1s");
	frame->AddConsumer(audio_p, audio_plot_in, "1024");

	//FILTER
	LibsoxFilter *filter = ssi_create (LibsoxFilter, "libsox", true);
	//ITransformable *filter_t = frame->AddTransformer(audio_p, filter, "0.1s");
	ITransformable *filter_t = frame->AddTransformer(audio_p, filter, "1024");	

	// audio out
	AudioPlayer *audioPlayer = ssi_create (AudioPlayer, 0, true);
	frame->AddConsumer(filter_t, audioPlayer, "0.1s");
	//frame->AddConsumer(filter_t, audioPlayer, "4096");

	// plot
	SignalPainter *audio_plot_out = ssi_create_id (SignalPainter, 0, "plot");
	audio_plot_out->getOptions()->setTitle("audio out");
	audio_plot_out->getOptions()->size = 1.0;	
	audio_plot_out->getOptions()->type = PaintSignalType::AUDIO;
	//frame->AddConsumer(filter_t, audio_plot_out, "0.1s");
	frame->AddConsumer(filter_t, audio_plot_out, "1024");

	// wav writer
	WavWriter *wavWrite = ssi_create (WavWriter, 0, true);
	wavWrite->getOptions()->setPath("out.wav");
	//frame->AddConsumer(filter_t, wavWrite, "0.1s");
	frame->AddConsumer(filter_t, wavWrite, "1024");

	decorator->add("console", 0, 0, 650, 800);
	decorator->add("plot*", 650, 0, 400, 400);
	decorator->add("monitor*", 650, 400, 400, 400);

	frame->Start();
	frame->Wait();
	frame->Stop();
	frame->Clear();
}
