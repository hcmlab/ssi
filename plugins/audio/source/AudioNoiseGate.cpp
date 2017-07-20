// AudioNoiseGate.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2016/11/02
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

#include "AudioNoiseGate.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

AudioNoiseGate::AudioNoiseGate (const ssi_char_t *file)
	: _file (0) {

	if (file) {

		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	
	}

}

AudioNoiseGate::~AudioNoiseGate () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void AudioNoiseGate::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_state = STATE::CLOSED;
	_gate = 0;
	_holding = 0;
}

void AudioNoiseGate::updateOptions()
{
	_options.lock();

	_bypass = _options.bypass;
	_threshold = _options.threshold;
	_attack = _options.attack;
	_decay = _options.decay;
	_hold = _options.hold;
	_range = _options.range;

	_options.unlock();
}

void AudioNoiseGate::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *input = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *output = ssi_pcast (ssi_real_t, stream_out.ptr);
	ssi_size_t sample_count = stream_in.num;
	const float sample_rate = ssi_cast(float,stream_in.sr);

	updateOptions();

	if (!_bypass) 
	{
		const float threshold_value = pow(10, _threshold * 0.05);
		const float attack_coef = 1000 / (_attack * sample_rate);
		const int hold_samples = int(_hold * sample_rate * 0.001);
		const float decay_coef = 1000 / (_decay * sample_rate);
		const float range_coef = _range > -90 ? pow(10, _range * 0.05) : 0;

		for (uint32_t i = 0; i < sample_count; ++i) {

			// Counting input dB
			float sample = input[i];
			float abs_sample = fabs(sample);

			switch (_state) {
			case STATE::CLOSED:
			case STATE::DECAY:
				if (abs_sample >= threshold_value) { _state = STATE::ATTACK; }
				break;
			case STATE::ATTACK:
				break;
			case STATE::OPENED:
				if (abs_sample >= threshold_value) { _holding = hold_samples; }
				else if (_holding <= 0) { _state = STATE::DECAY; }
				else { _holding--; }
				break;
			default:
				// shouldn't happen
				_state = STATE::CLOSED;
			}

			// handle attack/decay in a second pass to avoid unnecessary one-sample delay
			switch (_state) {
			case STATE::CLOSED:
				output[i] = sample * range_coef;
				break;
			case STATE::DECAY:
				_gate -= decay_coef;
				if (_gate <= 0) {
					_gate = 0;
					_state = STATE::CLOSED;
				}
				output[i] = sample * (range_coef * (1 - _gate) + _gate);
				break;
			case STATE::ATTACK:
				_gate += attack_coef;
				if (_gate >= 1) {
					_gate = 1;
					_state = STATE::OPENED;
					_holding = hold_samples;
				}
				output[i] = sample * (range_coef * (1 - _gate) + _gate);
				break;
			case STATE::OPENED:
				output[i] = sample;
				break;
			}
		}
	}
	else
	{
		// Bypassing
		for (ssi_size_t i = 0; i < sample_count; ++i) { 
			output[i] = input[i]; 
		}		
	}	
}

void AudioNoiseGate::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

}


}

