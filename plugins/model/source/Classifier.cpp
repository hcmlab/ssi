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
#include "ssiml/include/Trainer.h"
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
	_loadedTrainerFromOptions(false),
	_merged_sample_dimension (0),
	_listener(0),	
	_n_select(0),
	_select(0),
	_winner_only(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) 
{	 

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) 
		{
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Classifier::~Classifier ()
{

	if (_file)
	{
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	release();
}

void Classifier::addTrainer(const ssi_char_t *name, Trainer *trainer)
{ 
	_helper.addTrainer(name, trainer);
};

bool Classifier::loadTrainerFromOptions() 
{	
	if (_loadedTrainerFromOptions || _options.trainer[0] == '\0')
	{
		return true;
	}

	_loadedTrainerFromOptions = true;
	return _helper.addTrainerFromString(_options.trainer);		
}

void Classifier::release()
{
	_helper.release();

	_n_select = 0;
	delete[] _select; _select = 0;
	_merged_sample_dimension = 0;

	if (_listener)
	{
		ssi_event_destroy(_event);
	}
	_listener = 0;

	_loadedTrainerFromOptions = false;
}

void Classifier::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[])
{

	if (!loadTrainerFromOptions()) 
	{
		ssi_wrn("could not load trainer '%s'", _options.trainer);
	}

	// in case of several streams
	// test if sample rates are equal
	if (_options.merge && stream_in_num > 0)
	{
		_merged_sample_dimension = stream_in[0].dim;
		if (stream_in_num > 0) {
			for (ssi_size_t i = 1; i < stream_in_num; i++)
			{
				if (stream_in[i].type != stream_in[0].type)
				{
					ssi_err("sample type must not differ (%s != %s)", SSI_TYPE_NAMES[stream_in[i].type], SSI_TYPE_NAMES[stream_in[0].type]);
				}
				if (stream_in[i].byte != stream_in[0].byte)
				{
					ssi_err("sample size must not differ (%u != %u)", stream_in[i].byte, stream_in[0].byte);
				}
				_merged_sample_dimension += stream_in[i].dim;
			}
		}
	}
}

void Classifier::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[])
{

	ssi_real_t *dataptr = ssi_pcast (ssi_real_t, stream_in[0].ptr);

	bool result = false;
	if (_options.merge && stream_in_num > 0)
	{
		ssi_stream_t tmp;
		ssi_stream_init (tmp, stream_in[0].num, _merged_sample_dimension, stream_in[0].byte, stream_in[0].type, stream_in[0].sr);
		ssi_byte_t *tmp_ptr = tmp.ptr;
		for (ssi_size_t i = 0; i < stream_in_num; i++) 
		{

			if (stream_in[i].num != stream_in[0].num) 
			{
				ssi_err ("#samples in merged streams must not differ (%u != %u)", stream_in[i].num, stream_in[0].num);
			}

			memcpy (tmp_ptr, stream_in[i].ptr, stream_in[i].tot);
			tmp_ptr += stream_in[i].tot;
		}
		result = predict(consume_info.time, consume_info.dur, 1, &tmp);
		ssi_stream_destroy (tmp);
	} 
	else if (_options.flat)
	{
		ssi_stream_t stream = stream_in[0];		
		stream.dim = stream.num * stream.dim;
		stream.num = 1;
		result = predict(consume_info.time, consume_info.dur, 1, &stream);
	}
	else
	{
		result = predict(consume_info.time, consume_info.dur, stream_in_num, stream_in);
	}
}

void Classifier::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[])
{

}

void Classifier::listen_enter() 
{
	if (!loadTrainerFromOptions()) 
	{
		ssi_wrn("could not load trainer '%s'", _options.trainer);
	}
}

bool Classifier::update(IEvents &events,
	ssi_size_t n_new_events,
	ssi_size_t time_ms)
{

	if (n_new_events > 0) 
	{

		if (!_helper.hasTrainer()) 
		{
			return false;
		}

		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++) 
		{
			es[n_new_events - 1 - i] = events.next();
		}

		for (ssi_size_t i = 0; i < n_new_events; i++)
		{
			
			ssi_event_t *e = es[i];

			switch (e->type)
			{

			case SSI_ETYPE_STRING:
			{				
				_helper.switchTrainer(e);
				break;
			}
			
			case SSI_ETYPE_TUPLE:
			{

				ssi_size_t n = e->tot / sizeof(ssi_real_t);

				ssi_stream_t stream;
				ssi_stream_init(stream, 0, n, sizeof(ssi_real_t), SSI_REAL, 0);
				stream.ptr = e->ptr;
				stream.tot = stream.tot_real = e->tot;
				stream.num = stream.num_real = 1;

				predict(e->time / 1000.0, e->dur / 1000.0, 1, &stream);
				
				break;
			}

			case SSI_ETYPE_MAP:
			{
				_helper.switchTrainer(e);
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

void Classifier::listen_flush()
{
}

bool Classifier::setEventListener (IEventListener *listener) 
{

	if (!listener) 
	{
		return false;
	}

	_listener = listener;

	ssi_size_t eid, sid;

	if (_options.address[0] != '\0')
	{
		_event_address.setAddress(_options.address);
		sid = Factory::AddString(_event_address.getSender(0));
		eid = Factory::AddString(_event_address.getEvent(0));
	}
	else 
	{

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

	if (_options.select && _options.select[0] != '\0')
	{
		_select = ssi_parse_indices(_options.select, _n_select);
	}

	ssi_event_init(_event, SSI_ETYPE_MAP, sid, eid);

	return true;
}

bool Classifier::predict(ssi_time_t time,
	ssi_time_t dur,
	ssi_size_t n_streams,
	ssi_stream_t stream_in[]) 
{	
	if (!_helper.hasTrainer())
	{
		return false;
	}

	ClassifierHelper::Prediction prediction;
	bool result = _helper.predict(prediction, time, dur, n_streams, stream_in, _options.pthres);

	if (result)
	{
		_winner_only = _options.winner;

		if (prediction.n_probabilites == 1)
		{
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "%g", prediction.probabilites[0]);
		}
		else
		{
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "%s", prediction.trainer->getClassName(prediction.winner));
		}

		if (_listener)
		{

			ssi_size_t n_classes = prediction.n_probabilites;
			ssi_size_t *class_ids = new ssi_size_t[n_classes];
			ssi_char_t *const *class_names = prediction.trainer->getClassNames();
			for (ssi_size_t i = 0; i < n_classes; i++)
			{
				class_ids[i] = Factory::AddString(class_names[i]);
			}

			if (_winner_only)
			{
				ssi_event_adjust(_event, sizeof(ssi_event_map_t));
			}
			else if (_select)
			{
				ssi_event_adjust(_event, _n_select * sizeof(ssi_event_map_t));

				// check if indices are valid
				for (ssi_size_t i = 0; i < _n_select; i++) {
					if (_select[i] < 0) {
						ssi_wrn_static("index '%d' is negative and will be replaced by 0", _select[i]);
						_select[i] = 0;
					}
					else if (_select[i] >= (int)n_classes) {
						ssi_wrn_static("index '%d' out of range and will be replaced by 0", _select[i]);
						_select[i] = 0;
					}
				}
			}
			else
			{
				ssi_event_adjust(_event, n_classes * sizeof(ssi_event_map_t));
			}

			ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
			if (_winner_only)
			{
				ptr[0].id = class_ids[prediction.winner];
				ptr[0].value = prediction.probabilites[prediction.winner];
			}
			else if (_select)
			{
				for (ssi_size_t i = 0; i < _n_select; i++) {
					ptr[i].value = prediction.probabilites[_select[i]];
					ptr[i].id = class_ids[_select[i]];
				}
			}
			else
			{
				for (ssi_size_t i = 0; i < n_classes; i++) {
					ptr[i].value = prediction.probabilites[i];
					ptr[i].id = class_ids[i];
				}
			}

			_event.time = ssi_cast(ssi_size_t, time * 1000 + 0.5);
			_event.dur = ssi_cast(ssi_size_t, dur * 1000 + 0.5);
			_listener->update(_event);

		}

		if (_options.console) 
		{
			ssi_print_off("");
			for (ssi_size_t i = 0; i < prediction.n_probabilites; i++)
			{
				ssi_print("%s=%.2f ", prediction.trainer->getClassName(i), prediction.probabilites[i]);
			}
			ssi_print("\n");
		}
	}

	return result;
}

bool Classifier::notify(INotify::COMMAND::List command, const ssi_char_t *message)
{
	switch (command) {

	case INotify::COMMAND::MESSAGE:
	{
		return _helper.switchTrainer(message);
	}
	}

	return false;
}

}
