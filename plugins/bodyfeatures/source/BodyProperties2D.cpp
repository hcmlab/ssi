// BodyProperties.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2013/02/25
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
// version 3 of the License, or any later version.
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

#include "../include/BodyProperties2D.h"
#include "base/Factory.h"
#include <Math.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	char* BodyProperties2D::ssi_log_name = "bodyproper2D";

	BodyProperties2D::BodyProperties2D(const ssi_char_t* file)
		: ssi_log_level(SSI_LOG_LEVEL_DEFAULT),
		_elistener(0),
		_update_ms(0),
		_file(0),
		framecount(0),
		nodcount(0),
		lasthnf(0),
		m_timer(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		PropertyNames = GetPropertyNames(NPropertyNames);
		values = new ssi_real_t[NPropertyNames];

		ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	BodyProperties2D::~BodyProperties2D() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		delete[] values;
		ssi_event_destroy(_event);
	}

	bool BodyProperties2D::setEventListener(IEventListener* listener) {
		_elistener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
		_event.prob = 1.0;

		ssi_event_adjust(_event, NPropertyNames * sizeof(ssi_event_map_t));
		ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
		for (ssi_size_t i = 0; i < NPropertyNames; i++) {
			e[i].id = Factory::AddString(PropertyNames[i]);;
			e[i].value = 0.0f;
		}

		return true;
	}

	void BodyProperties2D::transform_enter(
		ssi_stream_t& stream_in,
		ssi_stream_t& stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		_update_counter = 0;
	}

	void BodyProperties2D::transform(ITransformer::info info,
		ssi_stream_t& stream_in,
		ssi_stream_t& stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_time_t time = info.time;
		ssi_real_t* ptr = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;
		ssi_real_t* out = ssi_pcast(ssi_real_t, stream_out.ptr);
		SSI_SKELETON* ss = 0;


		for (int i = 0; i < NPropertyNames; i++) {
			*(out++) = values[i];
		}

		_event.time = ssi_cast(ssi_size_t, 1000 * info.time + 0.5);
		_event.dur = 0;

		if (_elistener)
		{
			ssi_event_map_t* e = ssi_pcast(ssi_event_map_t, _event.ptr);
			for (ssi_size_t i = 0; i < NPropertyNames; i++) {
				e[i].value = ssi_cast(ssi_real_t, values[i]);
			}

			_elistener->update(_event);
		}
	}

	ssi_real_t BodyProperties2D::normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max) {
		ssi_real_t result = (ssi_real_t)((value - min) * (1 / (max - min)));
		if (result < 0.0) return 0.0;
		else if (result > 1.0) return 1.0;
		else return result;
	}

}