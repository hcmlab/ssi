// test_portaudio.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/02/29
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

#include <stdio.h>
#if _WIN32||_WIN64
#include "PortAudio.h"
#else
#include <portaudio.h>
#endif
#include "ioput/file/FileBinary.h"
#include "ioput/wav/WavTools.h"
using namespace ssi;

static File **wavfile;
static unsigned long counter;
static int n_channels;

void input ();
void output ();

static int MyOutputStreamCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {

	ssi_stream_t *wavstream = ssi_pcast (ssi_stream_t, userData);
	if (wavstream->num <= counter + frameCount) {	
		return paAbort;
	}

	memcpy (output, wavstream->ptr + counter * sizeof(float) * n_channels, frameCount * sizeof(float) * n_channels);

	counter += frameCount;
	printf (".");	

	return paContinue;
}

static int MyInputStreamCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {

	const float *data = (const float *) input;
	const float *ptr = data;

	for (unsigned long i = 0; i < frameCount; i++) {
		for (unsigned long j = 0; j < n_channels; j++) {
			short value = ssi_cast (short, *ptr++ * 32768.0f);
			wavfile[j]->write(&value, sizeof(short), 1);	
		}
	}

	counter += frameCount;
	printf (".");	

	return paContinue;
}

int main(int argc, char* argv[]) {

	ssi_random_seed ();

    input ();
	output ();

	return 0;
}

void output () {

	int n_frames = 256;
	double sample_rate = 44100.0;	

	PaError status;
	if((status = Pa_Initialize()) != paNoError)
	{
		printf ("Could not initialize portaudio: %s", Pa_GetErrorText(status));
		return;
	}

	int cDevices = Pa_GetDeviceCount();

	PaStreamParameters params = { NULL };
	params.device = Pa_GetDefaultOutputDevice();	
	PaDeviceInfo const* info = Pa_GetDeviceInfo (params.device);
	//n_channels = info->maxOutputChannels;
	n_channels = 1;
	params.channelCount = n_channels;
	params.sampleFormat = paFloat32;	
	params.suggestedLatency = info->defaultLowInputLatency;
	params.hostApiSpecificStreamInfo = NULL;	

	ssi_stream_t wavstream;
	WavTools::ReadWavFile ("check0.wav", wavstream, true);
	counter = 0;

	PaStream *stream;
	status = Pa_OpenStream (&stream, NULL, &params, sample_rate, n_frames, paClipOff, &MyOutputStreamCallback, &wavstream);
	if (status != paNoError)
	{
		printf ("Could not open stream: %s", Pa_GetErrorText (status));
		return;
	}

	status = Pa_StartStream (stream);
	if (status != paNoError)
	{
		printf ("Could not start stream: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}
	
	printf ("\n\n\tpress enter to quit\n\n");
	getchar ();

	status = Pa_StopStream (stream);
	if(status != paNoError)
	{
		printf ("Disconnect failed: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}

	Pa_Terminate();

}

void input ()
{
	int n_frames = 256;
	double sample_rate = 44100.0;	

	PaError status;
	if((status = Pa_Initialize()) != paNoError)
	{
		printf ("Could not initialize portaudio: %s", Pa_GetErrorText(status));
		return;
	}

	int cDevices = Pa_GetDeviceCount();

	PaStreamParameters params = { NULL };
	params.device = Pa_GetDefaultInputDevice();	
	PaDeviceInfo const* info = Pa_GetDeviceInfo (params.device);
	n_channels = info->maxInputChannels;
	params.channelCount = info->maxInputChannels;
	params.sampleFormat = paFloat32;
	params.suggestedLatency = info->defaultLowInputLatency;
	params.hostApiSpecificStreamInfo = NULL;

	wavfile = new File *[n_channels];
	char string[512];
	WAVEFORMATEX format = ssi_create_wfx (sample_rate, 1, sizeof (short));
	for (int i = 0; i < n_channels; i++) {
		sprintf (string, "check%d.wav", i);
		wavfile[i] = File::CreateAndOpen (File::BINARY, File::WRITE, string);		
		WavTools::WriteWavHeader (*wavfile[i], format, 0);
		WavTools::WriteWavChunk (*wavfile[i], format, 0);
	}
	counter = 0;

	PaStream *stream;
	status = Pa_OpenStream (&stream, &params, NULL, sample_rate, n_frames, paClipOff, &MyInputStreamCallback, 0);
	if (status != paNoError)
	{
		printf ("Could not open stream: %s", Pa_GetErrorText (status));
		return;
	}

	status = Pa_StartStream (stream);
	if (status != paNoError)
	{
		printf ("Could not start stream: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}

	printf ("\n\n\tpress enter to quit\n\n");
	getchar ();

	status = Pa_StopStream (stream);
	if(status != paNoError)
	{
		printf ("Disconnect failed: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}

	Pa_Terminate();

	for (int i = 0; i < n_channels; i++) {
		wavfile[i]->seek(0, File::BEGIN);
		WavTools::WriteWavHeader (*wavfile[i], format, counter);
		WavTools::WriteWavChunk (*wavfile[i], format, counter);
		delete wavfile[i];
	}	

	delete[] wavfile;
}

