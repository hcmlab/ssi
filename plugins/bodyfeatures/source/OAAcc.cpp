// KinectFeatures.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2012/08/10
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "OAAcc.h"
#include "signal/mathext.h"
#include <math.h>
#include "ssi.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	OAAcc::OAAcc(const ssi_char_t *file)
		: _file(0), _meta_in(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	OAAcc::~OAAcc() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OAAcc::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		//_prev_vel = new ssi_real_t[stream_in.dim];
		//for(ssi_size_t i=0; i<stream_in.dim; ++i)
		//	_prev_vel[i] = 0;
	}

	void OAAcc::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		ssi_real_t *dst_ptr = ssi_pcast(ssi_real_t, stream_out.ptr);
		ssi_real_t result = (_options.join == JOIN::MULT) ? 1.0f : 0.0f;

		//we expect 3 accelation values (3 axis) from 1 or more sensors
		for (ssi_size_t j = 0; j < stream_in.dim; ++j) //dimensions loop
		{
			//select a dimension
			ssi_stream_t stream;
			ssi_stream_select(stream_in, stream, 1, &j);

			//compute overall activation
			ssi_real_t oa = computeOA(info, stream, j);

			switch (_options.join)
			{
			case JOIN::SUM:
				result += oa;
				break;
			case JOIN::MULT:
				result *= oa;
				break;
			case JOIN::OFF:
				*dst_ptr++ = oa;
			}
		}

		if (_options.join != JOIN::OFF)
			*dst_ptr = result;
	}

	ssi_real_t OAAcc::computeOA(ITransformer::info info, ssi_stream_t &stream, ssi_size_t id)
	{
		ssi_real_t* src_ptr = ssi_pcast(ssi_real_t, stream.ptr);
		//ssi_real_t vel = _prev_vel[id];
		ssi_real_t vel = 0;

		/*
		 * compute new velocity
		 */
		switch (_options.input)
		{
		case INPUT::DYNACC:
		{
			//sum up velocity increments to get final velocity
			ssi_time_t t = 1.0 / stream.sr;
			for (ssi_size_t i = 0; i < stream.num; ++i)
			{
				ssi_real_t a = (abs(src_ptr[i]) < 0.1) ? 0 : src_ptr[i];
				vel += ssi_cast(ssi_real_t, a * t); //v_inc = a*t
			}
			break;
		}
		case INPUT::VELINC:
		{
			//sum up velocity increments to get final velocity
			for (ssi_size_t i = 0; i < stream.num; ++i)
				vel += src_ptr[i];
			break;
		}
		}

		/*
		 * compute displacement based on the previous velocity and the current one
		 */
		ssi_time_t t = (info.delta_num + info.frame_num) / stream.sr;
		//ssi_real_t displacement = (_prev_vel[id] + vel) * t * 0.5f; // x = ((u+v) * t) / 2
		ssi_real_t displacement = ssi_cast(ssi_real_t, vel * t * 0.5f); // x = ((0+v) * t) / 2

		//_prev_vel[id] = vel;
		return abs(displacement);
	}

	void OAAcc::transform_flush(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		//delete[] _prev_vel;
	}
}