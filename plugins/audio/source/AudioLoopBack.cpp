// AudioLoopBack.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 15/6/2018
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "AudioLoopBack.h"
#include "AudioLoopBackHelper.h"

namespace ssi {

	char AudioLoopBack::ssi_log_name[] = "audioloopb";

	AudioLoopBack::AudioLoopBack(const ssi_char_t *file)
		: _provider(0), 
		_listener(0),
		_file(0) 
	{

		if (file)
		{
			if (!OptionList::LoadXML(file, &_options))
			{
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	AudioLoopBack::~AudioLoopBack()
	{

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool AudioLoopBack::setProvider(const ssi_char_t *name, IProvider *provider)
	{

		if (strcmp(name, SSI_AUDIOLOOPBACK_PROVIDER_NAME) == 0) {
			setProvider(provider);
			return true;
		}

		ssi_wrn("unkown provider name '%s'", name);

		return false;
	}

	void AudioLoopBack::setProvider(IProvider *provider) 
	{

		if (_provider) {
			ssi_wrn("already set");
		}

		_provider = provider;
		if (_provider) 
		{
			if (!_listener)
			{
				_listener = new AudioLoopBackHelper(this);
				if (!_listener->Init(_format, _options.scale))
				{
					ssi_wrn("could not init audio client")
					return;
				}
			}
			
			ssi_stream_init(_audio_channel.stream, 0, _format.nChannels, _format.wBitsPerSample / 8, _options.scale ? SSI_FLOAT : SSI_SHORT, _format.nSamplesPerSec);
			_provider->init(&_audio_channel);

			ssi_msg(SSI_LOG_LEVEL_DETAIL, "audio provider set");
		}
	}

	bool AudioLoopBack::connect() 
	{
		if (!_listener)
		{
			return false;
		}

		if (!_listener->Start())
		{
			ssi_wrn("could not start audio client")
			return false;
		}
		
		return true;
	}

	void AudioLoopBack::run() 
	{
		if (_listener)
		{
			_listener->Read();
		}
	}

	bool AudioLoopBack::disconnect() 
	{
		if (!_listener)
		{
			return false;
		}

		if (!_listener->Stop())
		{
			ssi_wrn("could not stop audio client");
			return false;
		}
		delete _listener; _listener = 0;
		
		return true;
	}

	int AudioLoopBack::CopyData(const BYTE* Data, const int NumFramesAvailable)
	{
		
		if (Data)
		{
			_provider->provide((ssi_byte_t*)Data, NumFramesAvailable);
		}		
		/* we let ssi take care of syncronisation */
		//else 
		//{		
		//	ssi_size_t n_bytes = NumFramesAvailable * _audio_channel.getStream().byte * _audio_channel.getStream().dim;
		//	ssi_byte_t *silence = new ssi_byte_t[n_bytes];
		//	memset(silence, 0, n_bytes);
		//	_provider->provide(silence, NumFramesAvailable);
		//	delete[] silence;
		//}

		return 0;
	}

}
