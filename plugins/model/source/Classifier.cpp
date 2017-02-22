// Classifier.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#include "Classifier.h"
#include "base/Factory.h"
#include "Trainer.h"
#include "thread/RunAsThread.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t Classifier::ssi_log_name[] = "classifier";
ssi_char_t Classifier::ssi_log_name_static[] = "classifier";

Classifier::Classifier (const ssi_char_t *file) 
	: _file (0),
	_trainer (0),
	_is_loaded(false),
	_probs (0),
	_handler (0),
	_n_classes (0),
	_merged_sample_dimension (0),
	_consumer_sr (0),
	_consumer_dim (0),
	_consumer_byte (0),
	_consumer_num (0),
	_n_metas (0),
	_metas (0),
	_del_trainer(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {	 

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Classifier::~Classifier () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	releaseTrainer();
}

void Classifier::setTrainer (Trainer *trainer) { 
	loadTrainer(trainer); 
};

bool Classifier::loadTrainer(Trainer *trainer) {

	releaseTrainer();

	// load trainer
	if (trainer) {
		_trainer = trainer;
		_del_trainer = false;
	} else {
		if (_options.trainer[0] != '\0') {
			_trainer = new Trainer();
			if (!Trainer::Load(*_trainer, _options.trainer)) {
				_trainer = 0;
			} else {
				_del_trainer = true;
			}
		}
	} 

	if (!_trainer) {
		return false;
	}

	// prepare classification
	_n_classes = _trainer->getClassSize();
	_probs = new ssi_real_t[_n_classes];
	_merged_sample_dimension = 0;
	_consumer_sr = 0;
	_consumer_byte = sizeof(ssi_real_t);
	_consumer_dim = _trainer->getClassSize();
	_consumer_num = 1;
	_n_metas = _trainer->getMetaSize();
	if (_n_metas > 0) {
		_metas = new ssi_real_t[_n_metas];
	}

	_is_loaded = true;

	return true;
}

void Classifier::releaseTrainer() {

	if (_trainer) {

		delete _handler; _handler = 0;

		if (_del_trainer) {
			delete _trainer;			
		}
		_trainer = 0;

		delete[] _probs; _probs = 0;
		_n_classes = 0;
		_merged_sample_dimension = 0;
		_consumer_sr = 0;
		_consumer_dim = 0;
		_consumer_byte = 0;
		_consumer_num = 0;

		delete[] _metas; _metas = 0;
		_n_metas = 0;

		_is_loaded = false;
	}
}

void Classifier::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_is_loaded && !loadTrainer()) {
		ssi_wrn("could not load trainer '%s'", _options.trainer);
	}

	// in case of several streams
	// test if sample rates are equal
	if (_options.merge && stream_in_num > 0) {
		_merged_sample_dimension = stream_in[0].dim;
		if (stream_in_num > 0) {
			for (ssi_size_t i = 1; i < stream_in_num; i++) {
				if (stream_in[i].type != stream_in[0].type) {
					ssi_err("sample type must not differ (%s != %s)", SSI_TYPE_NAMES[stream_in[i].type], SSI_TYPE_NAMES[stream_in[0].type]);
				}
				if (stream_in[i].byte != stream_in[0].byte) {
					ssi_err("sample size must not differ (%u != %u)", stream_in[i].byte, stream_in[0].byte);
				}
				_merged_sample_dimension += stream_in[i].dim;
			}
		}
	}
}

bool Classifier::callTrainer(ssi_time_t time,
	ssi_time_t dur,
	ssi_size_t n_streams,
	ssi_stream_t stream_in[]) {

	if (!_is_loaded) {
		return false;
	}

	Lock lock(_mutex);

	bool result = false;

	if (n_streams == 1) {
		result = _trainer->forward_probs(stream_in[0], _n_classes, _probs);
	} else {
		ssi_stream_t **streams = new ssi_stream_t *[n_streams];
		for (ssi_size_t i = 0; i < n_streams; i++) {
			streams[i] = &stream_in[i];
		}
		result = _trainer->forward_probs(n_streams, streams, _n_classes, _probs);
		delete[] streams;
	}

	if (result && _options.pthres != 0) {
		bool exceeds_pthres = false;
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			if (_probs[i] > _options.pthres) {
				exceeds_pthres = true;
			}
		}
		result = exceeds_pthres;
	}

	if (result) {

		ssi_size_t max_ind = 0;
		ssi_real_t max_val = _probs[0];
		for (ssi_size_t i = 1; i < _n_classes; i++) {
			if (_probs[i] > max_val) {
				max_val = _probs[i];
				max_ind = i;
			}
		}

		bool has_meta = false;
		if (_n_metas > 0) {
			has_meta = _trainer->getMetaData(_n_metas, _metas);
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "recognized class %s", _trainer->getClassName(max_ind));


		if (_handler)
		{
			if (has_meta) {
				_handler->handle(time, dur, _n_classes, max_ind, _probs, _trainer->getClassNames(), _n_metas, _metas);
			} else {
				_handler->handle(time, dur, _n_classes, max_ind, _probs, _trainer->getClassNames(), 0, 0);
			}
		}

		if (_options.console) {
			ssi_print_off("");
			for (ssi_size_t i = 0; i < _n_classes; i++) {
				ssi_print("%s=%.2f ", _trainer->getClassName(i), _probs[i]);
			}
			ssi_print("\n");
		}		
	}

	return result;
}

void Classifier::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_real_t *dataptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	bool result = false;
	if (_options.merge && stream_in_num > 0) {
		ssi_stream_t tmp;
		ssi_stream_init (tmp, stream_in[0].num, _merged_sample_dimension, stream_in[0].byte, stream_in[0].type, stream_in[0].sr);
		ssi_byte_t *tmp_ptr = tmp.ptr;
		for (ssi_size_t i = 0; i < stream_in_num; i++) {

			if (stream_in[i].num != stream_in[0].num) {
				ssi_err ("#samples in merged streams must not differ (%u != %u)", stream_in[i].num, stream_in[0].num);
			}

			memcpy (tmp_ptr, stream_in[i].ptr, stream_in[i].tot);
			tmp_ptr += stream_in[i].tot;
		}
		result = callTrainer(consume_info.time, consume_info.dur, 1, &tmp);
		ssi_stream_destroy (tmp);
	} else {
		result = callTrainer(consume_info.time, consume_info.dur, stream_in_num, stream_in);
	}
}

void Classifier::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

}

void Classifier::listen_enter() {

	if (!_is_loaded && !loadTrainer()) {
		ssi_wrn("could not load trainer '%s'", _options.trainer);
	}
}

bool Classifier::update(IEvents &events,
	ssi_size_t n_new_events,
	ssi_size_t time_ms) {

	if (n_new_events > 0) {

		if (!_is_loaded) {
			return false;
		}

		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++) {
			es[n_new_events - 1 - i] = events.next();
		}

		for (ssi_size_t i = 0; i < n_new_events; i++) {
			
			ssi_event_t *e = es[i];

			switch (e->type) {
			
			case SSI_ETYPE_TUPLE: {

				ssi_size_t n = e->tot / sizeof(ssi_real_t);

				ssi_stream_t stream;
				ssi_stream_init(stream, 0, n, sizeof(ssi_real_t), SSI_REAL, 0);
				stream.ptr = e->ptr;
				stream.tot = stream.tot_real = e->tot;
				stream.num = stream.num_real = 1;

				callTrainer(e->time / 1000.0, e->dur / 1000.0, 1, &stream);
				
				break;
			}

			default:
				ssi_wrn("event type not supported '%s'", SSI_ETYPE_NAMES[e->type]);
			}
		}

		delete[] es;
	}

	return true;
}

void Classifier::listen_flush() {

	//releaseTrainer();
}

bool Classifier::setEventListener (IEventListener *listener) {

	if (!listener) {
		return false;
	}

	ssi_size_t eid, sid;

	if (_options.address[0] != '\0') {

		_event_address.setAddress(_options.address);
		sid = Factory::AddString(_event_address.getSender(0));
		eid = Factory::AddString(_event_address.getEvent(0));

	}
	else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

		sid = Factory::AddString(_options.sname);
		if (sid == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		eid = Factory::AddString(_options.ename);
		if (eid == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
	}

	_handler = new EventHandler (listener, sid, eid, _options.winner, _options.select);	

	return true;
}

// Class EventHandler
Classifier::EventHandler::EventHandler(IEventListener *listener,
	ssi_size_t sid,
	ssi_size_t eid,
	bool winner_only,
	ssi_char_t *select)
	: _listener(listener),
	_winner_only(winner_only),
	_class_ids(0),
	_n_select(0),
	_select(0) {

	if (select && select[0] != '\0') {
		_select = ssi_parse_indices(select, _n_select);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP, sid, eid);
};

Classifier::EventHandler::~EventHandler () {

	_n_select = 0;
	delete[] _select; _select = 0;

	ssi_event_destroy (_event);	
	delete[] _class_ids; _class_ids = 0;
}

void Classifier::EventHandler::handle (ssi_time_t time,
	ssi_time_t duration,
	ssi_size_t n_classes,
	ssi_size_t class_index,
	const ssi_real_t *probs,
	ssi_char_t *const*class_names,
	ssi_size_t n_metas,
	ssi_real_t *metas) { 

	if (_event.tot == 0) {
		_class_ids = new ssi_size_t[n_classes];
		for (ssi_size_t i = 0; i < n_classes; i++) {
			_class_ids[i] = Factory::AddString(class_names[i]);
		}
		if (_winner_only) {
			ssi_event_adjust(_event, sizeof(ssi_event_map_t));									
		} else if (_select) {
			ssi_event_adjust(_event, _n_select * sizeof(ssi_event_map_t));

			// check if indices are valid
			for (ssi_size_t i = 0; i < _n_select; i++) {
				if (_select[i] < 0) {
					ssi_wrn_static("index '%d' is negative and will be replaced by 0", _select[i]);
					_select[i] = 0;
				} else if (_select[i] >= n_classes) {
					ssi_wrn_static("index '%d' out of range and will be replaced by 0", _select[i]);
					_select[i] = 0;
				}
			}

		} else {
			ssi_event_adjust(_event, n_classes * sizeof(ssi_event_map_t));			
		}
	} 

	ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
	if (_winner_only) {
		ssi_real_t max_val = probs[0];
		ssi_size_t max_ind = 0;
		for (ssi_size_t i = 1; i < n_classes; i++) {
			if (probs[i] > max_val) {
				max_val = probs[i];
				max_ind = i;
			}
		}		
		ptr[0].id = _class_ids[max_ind];
		ptr[0].value = probs[max_ind];
	} else if (_select) {
		for (ssi_size_t i = 0; i < _n_select; i++) {
			ptr[i].value = probs[_select[i]];
			ptr[i].id = _class_ids[_select[i]];
		}
	} else {
		for (ssi_size_t i = 0; i < n_classes; i++) {			
			ptr[i].value = probs[i];	
			ptr[i].id = _class_ids[i];
		}	
	}

	_event.time = ssi_cast (ssi_size_t, time * 1000 + 0.5);
	_event.dur = ssi_cast (ssi_size_t, duration * 1000 + 0.5);
	_listener->update (_event);
};

}
