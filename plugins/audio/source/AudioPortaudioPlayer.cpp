// AudioAsioPlayer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/19
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

#include "AudioPortaudioPlayer.h"
#include "graphic/DialogLibGateway.h"
#if _WIN32||_WIN64
#include "PortAudio.h"
#else
#include "portaudio.h"
#endif

#include <sstream>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *AudioPortaudioPlayer::ssi_log_name = "audioportp";

AudioPortaudioPlayer::AudioPortaudioPlayer (const ssi_char_t *file)
	:  _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

AudioPortaudioPlayer::~AudioPortaudioPlayer () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void AudioPortaudioPlayer::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// init portaudio

	PaError status;
	if((status = Pa_Initialize()) != paNoError)
	{
		ssi_err ("could not initialize portaudio: %s", Pa_GetErrorText(status));		
	}

	// select audio device	

	int device = -1;
	if (_options.defdevice)
		device = Pa_GetDefaultOutputDevice ();
	else
	{
		if(_options.device == -1)
			device = selectDevice ();
		else
			device = _options.device;
	}

	if (stream_in[0].type != SSI_FLOAT && stream_in[0].type != SSI_SHORT) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	_scaled = stream_in[0].type == SSI_FLOAT;	

    _format = ssi_create_wfx (ssi_cast (unsigned int, stream_in[0].sr),
        ssi_cast (unsigned int16_t, stream_in[0].dim),
        ssi_cast (unsigned int16_t, (_scaled ? sizeof (int16_t) : sizeof (float))));

	// init output stream

	PaStreamParameters params = { NULL };
	params.device = device;		
	PaDeviceInfo const* info = Pa_GetDeviceInfo (params.device);	
	params.channelCount = 1;//_format.nChannels;	
	params.sampleFormat = _scaled ? paFloat32 : paInt16;
	params.suggestedLatency = info->defaultLowInputLatency;
	params.hostApiSpecificStreamInfo = NULL;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "connect '%s (id=%u)'", info->name, device);
		
	status = Pa_OpenStream (&_stream, NULL, &params, _format.nSamplesPerSec, 0, paClipOff, 0, 0);
	if (status != paNoError)
	{
		printf ("could not open stream: %s", Pa_GetErrorText (status));
		return;
	}

	status = Pa_StartStream (_stream);
	if (status != paNoError)
	{
		printf ("could not start stream: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}
	
}

void AudioPortaudioPlayer::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	Pa_WriteStream (_stream, stream_in[0].ptr, stream_in[0].num);
}

void AudioPortaudioPlayer::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	PaError status;

	status = Pa_StopStream (_stream);
	if(status != paNoError)
	{
		printf ("could not stop stream: %s", Pa_GetErrorText (status));
		Pa_Terminate();
		return;
	}

	Pa_Terminate();
}

int AudioPortaudioPlayer::selectDevice () {

	StringList devices;
	PaDeviceIndex n_devices = Pa_GetDeviceCount();
	int *map = new int[n_devices];
	int map_count = 0;

	//check if we have to restrict api choice
	bool usePreferredApi = false;
	if (_options.api > 0)
	{
		usePreferredApi = numDevicesForApi(_options.api) > 0;
		if (!usePreferredApi)
			ssi_wrn("found no devices for preferred api");
	}

	const PaDeviceInfo *device;
	const PaHostApiInfo *host;
	for (PaDeviceIndex i = 0; i < n_devices; i++) {

		device = Pa_GetDeviceInfo(i);
		host = Pa_GetHostApiInfo(device->hostApi);

		if (device->maxOutputChannels > 0 && (!usePreferredApi || _options.api == host->type)) {

			std::stringstream name;
			name << host->name << " - " << device->name;

			devices.add(name.str().c_str());
			map[map_count++] = i;
		}
	}

	int id = LetUserSelectDevice (devices);
	int result = 0;
	if (id >= 0) {
		if (_options.remember) {
			_options.device = map[id];
		}
		result = map[id];
	} else {
		ssi_wrn ("could not select a valid device");		
	}

	delete[] map;

	return result;
}

int AudioPortaudioPlayer::getDevice (const ssi_char_t *name) {

	PaDeviceIndex n_devices = Pa_GetDeviceCount();

	const PaDeviceInfo *device;
	const PaHostApiInfo *host;
	for (PaDeviceIndex i = 0; i < n_devices; i++) {

		device = Pa_GetDeviceInfo(i);
		host = Pa_GetHostApiInfo(device->hostApi);

		std::stringstream name_dev;
		name_dev << host->name << " - " << device->name;

		if (strcmp(name, name_dev.str().c_str()) == 0) {
			return i;
		}
	}

	ssi_wrn("could not find device '%s'", name);
	return -1;
}

unsigned int AudioPortaudioPlayer::numDevicesForApi(int api_type)
{
	PaDeviceIndex n_devices = Pa_GetDeviceCount();
	const PaDeviceInfo *device;
	unsigned int count = 0;

	for (PaDeviceIndex i = 0; i < n_devices; i++) {

		device = Pa_GetDeviceInfo(i);
		if (Pa_GetHostApiInfo(device->hostApi)->type == api_type && device->maxInputChannels > 0) {
			count++;
		}
	}

	return count;
}

int AudioPortaudioPlayer::LetUserSelectDevice (StringList &list) {
/*
	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	DialogLibGateway dialogGateway;

	if(!dialogGateway.didInitWork())
	{
		return LetUserSelectDeviceOnConsole (list);
	}

	if(!dialogGateway.SetNewDialogType("SimpleSelectionDialog"))
	{
		return LetUserSelectDeviceOnConsole (list);
	}

	int intHandle = dialogGateway.AlterExistingItem ("Caption", -1, "Select an audio device (output)");

	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		intHandle = dialogGateway.AppendItem("Item", list.get (i));
		if(intHandle < 0) {
			return LetUserSelectDeviceOnConsole (list);
		}
	}

	return dialogGateway.RunDialog();
    */
    return 0;
}

int AudioPortaudioPlayer::LetUserSelectDeviceOnConsole(StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	ssi_print("Select an audio device:\n");
	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		ssi_print("%d: %s\n", i, list.get (i));
	}

	int selection = -1;
	ssi_print("Your selection: ");
	scanf("%d", &selection);
	if(selection == EOF || selection < 0 || selection > ssi_cast (int, list.size ())) {
		ssi_wrn_static ("invalid selection");
		return -1;
	}
	
	return (selection - 1);
}


}
