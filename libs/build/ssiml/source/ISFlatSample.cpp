// ISFlatSample.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/01
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

#include "ISFlatSample.h"
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

	ISFlatSample::ISFlatSample(ISamples *samples)
		: _samples(samples),
		_n_streams(0),
		_streams(0) {

		_n_streams = _samples->getStreamSize();
		_streams = new ssi_stream_t[_n_streams];
		ssi_sample_create(_sample_out, _n_streams, 0, 0, 0, 0);

		for (ssi_size_t i = 0; i < _n_streams; i++) {

			ssi_sample_t *sample = samples->get(0);
			ssi_size_t num = sample->streams[i]->num;
			samples->reset();
			bool found_smaller = false;
			while (sample = samples->next())
			{
				if (sample->streams[i]->num < num)
				{
					found_smaller = true;
					num = sample->streams[i]->num;
				}
			}

			if (found_smaller)
			{
				ssi_wrn("#samples differ in stream#%u", i);
			}

			sample = samples->get(0);
			ssi_stream_init(_streams[i], 0, sample->streams[i]->dim * num, samples->getStream(i).byte, samples->getStream(i).type, samples->getStream(i).sr, 0);
			_streams[i].num = _streams[i].num_real = 1;
			_streams[i].tot = _streams[i].tot_real = _streams[i].dim * _streams[i].byte;
			_sample_out.streams[i] = _streams + i;
		}
	}

	ISFlatSample::~ISFlatSample() {

		release();
	}

	void ISFlatSample::release() {

		delete[] _streams; _streams = 0;
		delete[] _sample_out.streams; _sample_out.streams = 0;
	}

	ssi_sample_t *ISFlatSample::get(ssi_size_t index) {

		ssi_sample_t *tmp = _samples->get(index);

		if (tmp == 0) {
			return 0;
		}

		for (ssi_size_t i = 0; i < _n_streams; i++)
		{
			_streams[i].ptr = tmp->streams[i]->ptr;
			if (_streams[i].dim > tmp->streams[i]->dim * tmp->streams[i]->num)
			{
				ssi_err("dimension mismatch '%u > %u'", _streams[i].dim, tmp->streams[i]->dim * tmp->streams[i]->num);
			}
		}

		_sample_out.user_id = tmp->user_id;
		_sample_out.class_id = tmp->class_id;
		_sample_out.score = tmp->score;
		_sample_out.time = tmp->time;

		return &_sample_out;
	}

	ssi_sample_t *ISFlatSample::next() {

		ssi_sample_t *tmp = _samples->next();
		if (!tmp) {
			return 0;
		}

		for (ssi_size_t i = 0; i < _n_streams; i++)
		{
			_streams[i].ptr = tmp->streams[i]->ptr;
			if (_streams[i].dim > tmp->streams[i]->dim * tmp->streams[i]->num)
			{
				ssi_err("dimension mismatch '%u > %u'", _streams[i].dim, tmp->streams[i]->dim * tmp->streams[i]->num);
			}
		}

		_sample_out.user_id = tmp->user_id;
		_sample_out.class_id = tmp->class_id;
		_sample_out.score = tmp->score;
		_sample_out.time = tmp->time;

		return &_sample_out;
	}




}
