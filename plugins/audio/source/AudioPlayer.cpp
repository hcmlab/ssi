// AudioPlayer.cpp
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

#include "AudioPlayer.h"
#include "graphic/DialogLibGateway.h"

#include "VolumeOutMaster.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *AudioPlayer::ssi_log_name = "audioplay_";

AudioPlayer::AudioPlayer (const ssi_char_t *file)
	:  _file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

AudioPlayer::~AudioPlayer () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void AudioPlayer::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// select audio device
	if (_options.device == -1) {
		_options.device = selectDevice ();
	}
	ssi_char_t name[SSI_MAX_CHAR];
	if (getDevice(_options.device, SSI_MAX_CHAR, name)) {
		ssi_msg (SSI_LOG_LEVEL_BASIC, "connect '%s (id=%u)'", name, _options.device);
	} else {
		_options.device = selectDevice ();
	}

	if (stream_in[0].type != SSI_FLOAT && stream_in[0].type != SSI_SHORT) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	_scale = stream_in[0].type == SSI_FLOAT;
	if (_scale) {
		ssi_stream_init (_stream_scale, 0, stream_in[0].dim, sizeof (short), SSI_SHORT, stream_in[0].sr);
	}

    audio_format = ssi_create_wfx (ssi_cast (unsigned int, stream_in[0].sr),
		ssi_cast (unsigned short, stream_in[0].dim), 
		ssi_cast (unsigned short, 2));

	// check format
	MMRESULT result = ::waveOutOpen (0, _options.device, &audio_format, 0, 0, WAVE_FORMAT_QUERY);
	if (result != MMSYSERR_NOERROR) {
		ssi_err ("audio-out device '%u' does not support the requested format", _options.device);
	}

	if(_options.bufferSizeSamples > 0)
		_options.bufferSize = _options.bufferSizeSamples * (1.0 / stream_in[0].sr);
	else
		_options.bufferSizeSamples = ssi_cast (ssi_size_t, _options.bufferSize * stream_in[0].sr + 0.5);

	hWaveOut = _waveOutDevice.init (_options.device, audio_format, _options.bufferSizeSamples * sizeof (short), _options.numBuffers);
}

void AudioPlayer::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_scale) {
		ssi_stream_adjust (_stream_scale, stream_in[0].num);
		float *src = ssi_pcast (float, stream_in[0].ptr);
		short *dst = ssi_pcast (short, _stream_scale.ptr);
		for (ssi_size_t i = 0; i < stream_in[0].num * stream_in[0].dim; i++) {
			*dst++ = ssi_cast (short, *src++ * 32768.0f);
		}
		_waveOutDevice.writeAudio (hWaveOut, _stream_scale.ptr, _stream_scale.tot);  
	} else {
		_waveOutDevice.writeAudio (hWaveOut, stream_in[0].ptr, stream_in[0].tot);  
	}
}

void AudioPlayer::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_scale) {
		ssi_stream_destroy (_stream_scale);
	}
	_waveOutDevice.clean (hWaveOut);
}

int AudioPlayer::selectDevice () {

	StringList devices;
	UINT n_devices = waveOutGetNumDevs ();

	WAVEOUTCAPS waveOutCaps;
	for (UINT i = 0; i < n_devices; i++) {		
		if (waveOutGetDevCaps (i, &waveOutCaps, sizeof(WAVEOUTCAPS)) != MMSYSERR_NOERROR) {
			ssi_wrn ("could not determine capabilities for device #u", i);
		} else {
			devices.add (waveOutCaps.szPname);		
		}
	}

	int id = LetUserSelectDevice (devices);
	if (id >= 0) {
		if (_options.remember) {
			_options.device = id;
		}
	} else {
		ssi_wrn ("could not select a valid device");
	}

	return id;
}

int AudioPlayer::getDevice (const ssi_char_t *name) {

	UINT n_devices = waveOutGetNumDevs ();

	WAVEOUTCAPS waveOutCaps;
	for (UINT i = 0; i < n_devices; i++) {		
		if (waveOutGetDevCaps (i, &waveOutCaps, sizeof(WAVEOUTCAPS)) != MMSYSERR_NOERROR) {
			ssi_wrn ("could not determine capabilities for device #u", i);
		} else {
			if (strcmp (waveOutCaps.szPname, name) == 0) {
				return i;
			}
		}
	}

	ssi_wrn ("could not find device '%s'", name);
	return -1;
}

bool AudioPlayer::getDevice (ssi_size_t id, ssi_size_t n_name, char *name) {
	
	WAVEOUTCAPS waveOutCaps;	
	if (waveOutGetDevCaps (id, &waveOutCaps, sizeof(WAVEOUTCAPS)) != MMSYSERR_NOERROR) {
		ssi_wrn ("could not determine capabilities for device #u", id);
		return false;
	} else {
		if (n_name < MAXPNAMELEN) {			
			ssi_wrn ("name field too small");
			return false;
		}		
	}
	
	memcpy (name, waveOutCaps.szPname, MAXPNAMELEN);
	return true;
}


int AudioPlayer::LetUserSelectDevice (StringList &list) {

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
}

int AudioPlayer::LetUserSelectDeviceOnConsole(StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	ssi_print("Select an audio device (output):\n");
	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		ssi_print("%d: %s\n", i+1, list.get (i));
	}

	int selection = -1;
	ssi_print("Your selection: ");
	scanf("%d", &selection);
	getchar ();
	if(selection == EOF || selection < 0 || selection > ssi_cast (int, list.size ())) {
		ssi_wrn_static ("invalid selection");
		return -1;
	}	
	
	return (selection - 1);
}
	
}
