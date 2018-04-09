// ISUnfoldSample.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2018/02/25
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

#include "ISUnfoldSample.h"
#include "signal/SignalTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	ISUnfoldSample::ISUnfoldSample(ISamples *samples)
		: _samples(samples),
		_n_samples(0),
		_n_samples_per_class(0),
		_stream_index(0), 
		_n_context(0),
		_map(0)
	{
		ssi_sample_create(_sample_out, 1, 0, 0, 0, 0);
		_sample_out.streams[0] = new ssi_stream_t;
	}

	bool ISUnfoldSample::set(ssi_size_t stream_index, ssi_size_t n_context)
	{
		SSI_ASSERT(stream_index < _samples->getStreamSize());

		release();

		_stream_index = stream_index;
		_n_context = n_context;

		ssi_size_t dim = _samples->getStream(stream_index).dim * (1 + _n_context);
		ssi_stream_init(*_sample_out.streams[0], 0, dim, _samples->getStream(stream_index).byte, _samples->getStream(stream_index).type, _samples->getStream(stream_index).sr, 0);
		_sample_out.streams[0]->num = _sample_out.streams[0]->num_real = 1;
		_sample_out.streams[0]->tot = _sample_out.streams[0]->tot_real = dim * _sample_out.streams[0]->byte;
		_n_samples_per_class = new ssi_size_t[_samples->getClassSize()];
		for (ssi_size_t i = 0; i < _samples->getClassSize(); i++)
		{
			_n_samples_per_class[i] = 0;
		}

		_samples->reset();
		ssi_sample_t *sample = 0;
		ssi_stream_t *stream = 0;
		_n_samples = 0;
		while (sample = _samples->next())
		{
			stream = sample->streams[_stream_index];
			if (stream->num >= _n_context)
			{
				ssi_size_t n = stream->num - _n_context;
				_n_samples += n;
				_n_samples_per_class[sample->class_id] += n;
			}
		}

		_map = new entry[_n_samples];
		_samples->reset();
		ssi_size_t count = 0;
		while (sample = _samples->next())
		{
			stream = sample->streams[_stream_index];
			if (stream->num >= _n_context)
			{
				ssi_size_t n = stream->num - _n_context;
				for (ssi_size_t i = 0; i < n; i++, count++)
				{
					_map[count].class_id = sample->class_id;
					_map[count].user_id = sample->user_id;
					_map[count].score = sample->score;
					_map[count].time = sample->time + (stream->sr > 0 ? i/stream->sr : 0);
					_map[count].ptr = stream->ptr + i * stream->dim * stream->byte;
				}
			}
		}

		return true;
	}

	ISUnfoldSample::~ISUnfoldSample() {

		//delete _sample_out.streams[0];
		_sample_out.streams[0]->ptr = 0;
		ssi_sample_destroy(_sample_out);

		release();
	}

	void ISUnfoldSample::release() {

		_n_samples = 0;
		_stream_index = 0;
		delete[] _n_samples_per_class; _n_samples_per_class = 0;
		delete[] _map; _map = 0;
	}

	ssi_sample_t *ISUnfoldSample::get(ssi_size_t index) {

		_sample_out.user_id = _map[index].user_id;
		_sample_out.class_id = _map[index].class_id;
		_sample_out.score = _map[index].score;
		_sample_out.time = _map[index].time;
		_sample_out.streams[0]->ptr = _map[index].ptr;

		return &_sample_out;
	}

	ssi_sample_t *ISUnfoldSample::next() {
		if (_next_counter == _n_samples)
		{
			return false;
		}
		return get(_next_counter++);
	}




}
