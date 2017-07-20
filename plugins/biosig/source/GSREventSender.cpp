// GSREventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#include "GSREventSender.h"
#include "OverlapBuffer.h"
#include "../../signal/include/MvgAvgVar.h"
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

ssi_char_t *GSREventSender::ssi_log_name = "gsrevent__";

GSREventSender::GSREventSender (const ssi_char_t *file)
	: _file (0),
	_listener (0),
	_mvgvar (0),
	_findpeaks (0),
	_findslopes (0),
	_etuple_amplitude_id (0),
	_etuple_area_id (0),
	_send_etuple (false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

GSREventSender::~GSREventSender () {

	if (_listener) {
		ssi_event_destroy (_peak_event);
		ssi_event_destroy (_slope_event);
		ssi_event_destroy (_drop_event);
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool GSREventSender::setEventListener (IEventListener *listener) {

	_listener = listener;	

	_send_etuple = _options.tuple;
	if (_send_etuple) {
		_etuple_amplitude_id = Factory::AddString ("amplitude");
		_etuple_area_id = Factory::AddString ("area");

		ssi_event_init (_peak_event, SSI_ETYPE_MAP);		
		ssi_event_adjust (_peak_event, 2 * sizeof (ssi_event_map_t));
		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _peak_event.ptr);
		ptr[0].id = _etuple_amplitude_id;
		ptr[1].id = _etuple_area_id;

		ssi_event_init (_slope_event, SSI_ETYPE_MAP);		
		ssi_event_adjust (_slope_event, 1 * sizeof (ssi_event_map_t));
		ptr = ssi_pcast (ssi_event_map_t, _slope_event.ptr);
		ptr[0].id = _etuple_amplitude_id;

		ssi_event_init (_drop_event, SSI_ETYPE_MAP);
		ssi_event_adjust (_drop_event, 1 * sizeof (ssi_event_map_t));
		ptr = ssi_pcast (ssi_event_map_t, _drop_event.ptr);
		ptr[0].id = _etuple_amplitude_id;
	
	} else {
		ssi_event_init (_peak_event, SSI_ETYPE_TUPLE);
		ssi_event_init (_slope_event, SSI_ETYPE_TUPLE);
		ssi_event_init (_drop_event, SSI_ETYPE_TUPLE);
		ssi_event_adjust (_peak_event, 2 * sizeof (ssi_real_t));
		ssi_event_adjust (_slope_event, 1 * sizeof (ssi_real_t));
		ssi_event_adjust (_drop_event, 1 * sizeof (ssi_real_t));
	}

	_peak_event.sender_id = Factory::AddString (_options.sname);
	_slope_event.sender_id = Factory::AddString (_options.sname);
	_drop_event.sender_id = Factory::AddString (_options.sname);
	if (_peak_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_peak_event.event_id = Factory::AddString (_options.epeakname);
	if (_peak_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_slope_event.event_id = Factory::AddString (_options.eslopename);
	if (_slope_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_drop_event.event_id = Factory::AddString (_options.edropname);
	if (_drop_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.epeakname);
	_event_address.setEvents (_options.eslopename);
	_event_address.setEvents (_options.edropname);

	return true;
}

void GSREventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (stream_in[0].type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in[0].type]);
	}

	if (stream_in[0].dim != 1) {
		ssi_err ("dimension > 1 not supported");
	}

	MvgAvgVar *mvgvar = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
	mvgvar->getOptions ()->format = MvgAvgVar::ALL;
	mvgvar->getOptions ()->win = _options.winsize;
	mvgvar->getOptions ()->method = MvgAvgVar::SLIDING;
	_mvgvar = mvgvar;
	ssi_stream_init (_var_stream, 0, _mvgvar->getSampleDimensionOut (stream_in[0].dim), _mvgvar->getSampleBytesOut (stream_in[0].byte), _mvgvar->getSampleTypeOut (stream_in[0].type), stream_in[0].sr);
	_mvgvar->transform_enter (stream_in[0], _var_stream);

	GSRFindPeaks::Params peaks_params;
	peaks_params.sr = stream_in[0].sr;
	peaks_params.maxdur = _options.peakmaxd;
	peaks_params.mindur = _options.peakmind;
	peaks_params.nstd = _options.peaknstd;
	_findpeaks = new GSRFindPeaks (this, peaks_params);
	_findpeaks->setLogLevel (ssi_log_level);

	GSRFindSlopes::Params slope_params;
	slope_params.sr = stream_in[0].sr;
	slope_params.maxdur = _options.slopemaxd;
	slope_params.mindur = _options.slopemind;
	slope_params.nstd = _options.slopenstd;
	_findslopes = new GSRFindSlopes (this, slope_params);
	_findslopes->setLogLevel (ssi_log_level);	
}

void GSREventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	ssi_size_t n = stream_in[0].num;
	ssi_time_t sr = stream_in[0].sr;

	ITransformer::info tinfo;
	tinfo.delta_num = 0;
	tinfo.delta_num = n;
	tinfo.time = consume_info.time;

	ssi_stream_adjust (_var_stream, n);
	_mvgvar->transform (tinfo, stream_in[0], _var_stream);
	ssi_real_t *var = ssi_pcast (ssi_real_t, _var_stream.ptr);

	ssi_real_t *gsr = ssi_pcast (ssi_real_t, stream_in[0].ptr);
	_findpeaks->process (n, sr, gsr, var);

	gsr = ssi_pcast (ssi_real_t, stream_in[0].ptr);
	_findslopes->process (n, sr, gsr, var);
	
}

void GSREventSender::peak (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area) {

	_peak_event.time = ssi_cast (ssi_size_t, from * 1000 + 0.5);
	_peak_event.dur = ssi_cast (ssi_size_t, (to-from) * 1000 + 0.5);
	_peak_event.state = SSI_ESTATE_COMPLETED;
	
	if (_send_etuple) {
		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _peak_event.ptr);
		ptr[0].value = amplitude;
		ptr[1].value = area;
	} else {
		ssi_real_t *ptr = ssi_pcast (ssi_real_t, _peak_event.ptr);
		ptr[0] = amplitude;
		ptr[1] = area;
	}

	if (_listener) {
		_listener->update (_peak_event);
	} else {
		printf ("peak [%.2f %.2f]s\n", from, to);						
	}
}

void GSREventSender::slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t gradient) {

	if (gradient > 0) {

		_slope_event.time = ssi_cast (ssi_size_t, from * 1000 + 0.5);
		_slope_event.dur = ssi_cast (ssi_size_t, (to-from) * 1000 + 0.5);
		_slope_event.state = SSI_ESTATE_COMPLETED;

		if (_send_etuple) {
			ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _slope_event.ptr);
			ptr[0].value = amplitude;			
		} else {
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, _slope_event.ptr);
			ptr[0] = amplitude;			
		}

		if (_listener) {
			_listener->update (_slope_event);
		} else {
			printf ("slope [%.2f %.2f]s\n", from, to);					
		}

	} else {

		_drop_event.time = ssi_cast (ssi_size_t, from * 1000 + 0.5);
		_drop_event.dur = ssi_cast (ssi_size_t, (to-from) * 1000 + 0.5);
		_drop_event.state = SSI_ESTATE_COMPLETED;
		
		if (_send_etuple) {
			ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _drop_event.ptr);
			ptr[0].value = amplitude;			
		} else {
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, _drop_event.ptr);
			ptr[0] = amplitude;			
		}

		if (_listener) {
			_listener->update (_drop_event);
		} else {
			printf ("drop [%.2f %.2f]s\n", from, to);					
		}
	}
}

void GSREventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	delete _findpeaks; _findpeaks = 0;
	delete _findslopes; _findslopes = 0;

	_mvgvar->transform_flush (stream_in[0], _var_stream);
	ssi_stream_destroy (_var_stream);
	delete _mvgvar; _mvgvar = 0;
}

}
