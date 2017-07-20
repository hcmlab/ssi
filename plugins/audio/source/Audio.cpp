// Audio.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#include "Audio.h"
#include "graphic/DialogLibGateway.h"
#include "VolumeInXXX.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *Audio::ssi_log_name = "audio_____";

Audio::Audio (const ssi_char_t *file)
	: _current_buffer (0),
	_buffer_pool (0),
	_bytes_per_buffer (0),
	_samples_per_buffer (0),
	_scale_buffer (0),
	_provider (0),
	_isStarted(false),
	_file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Audio::~Audio () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool Audio::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_AUDIO_PROVIDER_NAME) == 0) {
		setProvider (provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void Audio::setProvider (IProvider *provider) {

	if (_provider) {
		ssi_wrn ("already set");
	}

	_provider = provider;
	if (_provider) {

		// prepare format
		_format.wFormatTag = WAVE_FORMAT_PCM;
		_format.nChannels = _options.channels;
		_format.nSamplesPerSec = ssi_cast (DWORD, _options.sr);
		_format.nAvgBytesPerSec = ssi_cast (DWORD, _options.sr) * _options.channels * _options.bytes;
		_format.nBlockAlign = _options.channels * _options.bytes;
		_format.wBitsPerSample = _options.bytes*8;
		_format.cbSize = 0;

		ssi_stream_init (_audio_channel.stream, 0, _options.channels, _options.scale ? sizeof (float) : _options.bytes, _options.scale ? SSI_FLOAT : _options.bytes == 1 ? SSI_UCHAR : SSI_SHORT, _options.sr);
		_provider->init (&_audio_channel);

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "audio provider set");
	}
}

bool Audio::connect () {

	bool status;

	// select audio device
	if (_options.device[0] == '\0') {
		_device_id = selectDevice ();
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "connect 'id=%u'", _device_id);
	} else {
		_device_id = getDevice (_options.device);
		if (_device_id == -1) {
			_device_id = selectDevice ();
		}
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "connect 'name=%s'", _options.device);
	}

	// set volume
	#if __MINGW32__
	#else
    if (_options.volume >= 0) {

    ssi_msg (SSI_LOG_LEVEL_BASIC, "setting recording volume to %.2f", _options.volume);

    CVolumeInXXX vol (_device_id);
    DWORD vol_new = ssi_cast (DWORD, _options.volume * vol.GetMaximalVolume () + 0.5f);
    vol_new = vol_new > vol.GetMaximalVolume () ? vol.GetMaximalVolume () : vol_new;
    vol.SetCurrentVolume (vol_new);
	}
	#endif // __MINGW32__


	// prepare buffer
	if (_options.blockInSamples > 0) {
		_samples_per_buffer = _options.blockInSamples;
	} else {
		_samples_per_buffer = ssi_cast (ssi_size_t, _options.block * _options.sr + 0.5);
	}
	_bytes_per_buffer = _samples_per_buffer * _options.channels * _options.bytes,
	_buffer_pool = new char [_bytes_per_buffer * SSI_AUDIO_BLOCK_COUNT];
	if (_options.scale) {
		_scale_buffer = new float[_samples_per_buffer * _options.channels];
	}

	// block _event
	_event.block ();

	// start recording
	{
		Lock lock (_mutex);
		status = Start ();
	}

	if (!status) {
		status = false;
	}

	if (status) {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "connected");

		if (_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
             name\t= %s\n\
             rate\t= %.1f\n\
             dim\t= %u\n\
             bytes\t= %u\n\
             scale\t= %s\n",
					_options.device,
					_options.sr,
					_options.channels,
					_options.bytes,
					_options.scale ? "true" : "false");
		}

	} else {
		ssi_wrn ("could not connect sensor");
	}

	_first_call = true;

	// set thread name
	ssi_char_t thread_name[SSI_MAX_CHAR];
	if (_options.device[0] == '\0') {
		ssi_sprint (thread_name, "%s@%u", getName (), _device_id);
	} else {
		ssi_sprint (thread_name, "%s@%s", getName (), _options.device);
	}
	Thread::setName (thread_name);

	return status;
}

void Audio::run () {

	// wait until next buffer is ready
	_event.wait ();

	// lock _mutex
	Lock lock (_mutex);
	if (IsBufferDone ()) {

		if (_options.scale) {
			if (_options.bytes == 1) {
				unsigned char *srcptr = ssi_pcast (unsigned char, GetData ());
				float *dstptr = _scale_buffer;
				for (ssi_size_t i = 0; i < _samples_per_buffer * _options.channels; i++) {
					*dstptr++ = *srcptr++ / 128.0f - 1.0f;
				}
			} else {
				short *srcptr = ssi_pcast (short, GetData ());
				float *dstptr = _scale_buffer;
				for (ssi_size_t i = 0; i < _samples_per_buffer * _options.channels; i++) {
					*dstptr++ = *srcptr++ / 32768.0f;
				}
			}
			_provider->provide (ssi_pcast (ssi_byte_t, _scale_buffer), _samples_per_buffer);
		} else {
			_provider->provide (GetData (), _samples_per_buffer);
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "data provided");

		// release buffer
		BufferDone ();

	} else {
		if (_first_call) {
			_first_call = false;
		} else {
			ssi_wrn ("buffer not ready");
		}
	}
}

bool Audio::disconnect () {

	if (_options.device[0] == '\0') {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "disconnect 'id=%u'", _device_id);
	} else {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "disconnect 'name=%s'", _options.device);
	}

	// stop recording
	Lock lock (_mutex);
	Stop ();

    // clear buffer
    delete[] _buffer_pool;
	_buffer_pool = 0;
	delete _scale_buffer;
	_scale_buffer = 0;

	// release _event
	_event.release ();

	return true;
}

bool Audio::Start () {

	bool result;

	if (_options.device[0] == '\0') {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "open audio-in device 'id=%u'", _device_id);
	} else {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "open audio-in device 'name=%s'", _options.device);
	}

	result = _waveInDevice.Open (&_event, &_format, _device_id == -1 ? WAVE_MAPPER : _device_id);
	if (!result) {
		ssi_wrn ("could not open audio-device");
		return false;
	}
	result = _waveInDevice.Ok();
    if (!result) {
        char buf[164];
		result = _waveInDevice.IsInUse ();
        if (result) {
            ssi_strcpy (buf, "another application is recording audio");
        }
		else {
			_waveInDevice.GetErrorText (buf, sizeof (buf));
		}
		ssi_wrn (buf);
        return false;
    }

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "initialize buffer pool...");

    // Don't initialize the last buffer
    // It will be initialized in the
    // first call to BufferDone
    for ( int i = 0; i < SSI_AUDIO_BLOCK_COUNT - 1; i++ ) {
        _header_pool[i].lpData = & _buffer_pool [i * _bytes_per_buffer];
        _header_pool[i].dwBufferLength = _bytes_per_buffer;
        _header_pool[i].dwFlags = 0;
        _header_pool[i].dwLoops = 0;
        _waveInDevice.Prepare (& _header_pool[i]);
        _waveInDevice.SendBuffer (& _header_pool [i]);
    }
    _isStarted = TRUE;
    _current_buffer = 0;

	if (_options.device[0] == '\0') {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "start audio-in device 'id=%u'", _device_id);
	} else {
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "start audio-in device 'name=%s'", _options.device);
	}

    _waveInDevice.Start();

    return true;
}

bool Audio::BufferDone () {

	if (!IsBufferDone ()) {
		ssi_err ("buffer not ready");
		return false;
	}

    _waveInDevice.Unprepare (& _header_pool [_current_buffer]);
    int prevBuf = _current_buffer - 1;
    if (prevBuf < 0)
        prevBuf = SSI_AUDIO_BLOCK_COUNT - 1;

    // Next buffer to be filled
    _current_buffer++;
    if ( _current_buffer == SSI_AUDIO_BLOCK_COUNT )
        _current_buffer = 0;

    _header_pool[prevBuf].lpData = & _buffer_pool [prevBuf * _bytes_per_buffer];
    _header_pool[prevBuf].dwBufferLength = _bytes_per_buffer;
    _header_pool[prevBuf].dwFlags = 0;
    _header_pool[prevBuf].dwLoops = 0;
    _waveInDevice.Prepare (& _header_pool [prevBuf]);

    _waveInDevice.SendBuffer (& _header_pool [prevBuf]);

    return true;
}

void Audio::Stop ()
{
    _isStarted = FALSE;
    _waveInDevice.Reset ();
    _waveInDevice.Close ();
}


int Audio::selectDevice () {

	StringList devices;
	UINT n_devices = waveInGetNumDevs ();

	WAVEINCAPS waveInCaps;
	for (UINT i = 0; i < n_devices; i++) {
		if (waveInGetDevCaps (i, &waveInCaps, sizeof(WAVEINCAPS)) != MMSYSERR_NOERROR) {
			ssi_wrn ("could not determine capabilities for device #u", i);
		} else {
			devices.add (waveInCaps.szPname);
		}
	}

	int id = LetUserSelectDevice (devices);
	if (id >= 0) {
		if (_options.remember) {
			_options.setDevice (devices.get (id));
		}
	} else {
		ssi_wrn ("could not select a valid device");
	}

	return id;
}

int Audio::getDevice (const ssi_char_t *name) {

	UINT n_devices = waveInGetNumDevs ();

	WAVEINCAPS waveInCaps;
	for (UINT i = 0; i < n_devices; i++) {
		if (waveInGetDevCaps (i, &waveInCaps, sizeof(WAVEINCAPS)) != MMSYSERR_NOERROR) {
			ssi_wrn ("could not determine capabilities for device #u", i);
		} else {
			if (strcmp (waveInCaps.szPname, name) == 0) {
				return i;
			}
		}
	}

	ssi_wrn ("could not find device '%s'", name);
	return -1;
}


int Audio::LetUserSelectDevice (StringList &list) {

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

	int intHandle = dialogGateway.AlterExistingItem ("Caption", -1, "Select an audio device");

	for(ssi_size_t i = 0; i < list.size (); i++)
	{
		intHandle = dialogGateway.AppendItem("Item", list.get (i));
		if(intHandle < 0) {
			return LetUserSelectDeviceOnConsole (list);
		}
	}

	return dialogGateway.RunDialog();
}

int Audio::LetUserSelectDeviceOnConsole(StringList &list) {

	if(list.size () == 0)
	{
		ssi_wrn_static ("empty list");
		return -1;
	}

	ssi_print("Select an audio device:\n");
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
