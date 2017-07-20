// PraatScriptBase.cpp
// author: Andreas Seiderer
// created: 2013/09/16
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
// version 3 of the License, or any later version.
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

#include "PraatScriptBase.h"
#include "PraatTools.h"
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

	char *PraatScriptBase::ssi_log_name = "praat_____";

	PraatScriptBase::PraatScriptBase (const ssi_char_t *file)
		: ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
		_ready (false),
		_parser (0),
		_elistener (0),
		_file (0){

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy (file);
		}

		//events
		ssi_event_init (_event, SSI_ETYPE_MAP);	
		_n_values = 0;
	}

	PraatScriptBase::~PraatScriptBase () {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy (_event);
		_n_values = 0;
	}

	bool PraatScriptBase::setEventListener (IEventListener *listener) {

		_elistener = listener;
		_event.sender_id = Factory::AddString (_options.sname);
		_event.event_id = Factory::AddString (_options.ename);
		_event_address.setSender (_options.sname);
		_event_address.setEvents (_options.ename);
	
		return true;
	}

	void PraatScriptBase::consume_enter (ssi_size_t stream_in_num, ssi_stream_t stream_in[]) {		

		_ready = false;

		if (!(_parser = getParser ())) {
			ssi_wrn ("could not initialize parser");	
			return;
		}
		if (!ssi_exists (_options.script)) {
			ssi_wrn ("script not found '%s'", _options.script);
			return;
		}
		if (!ssi_exists (_options.exe)) {
			ssi_wrn ("exe not found '%s'", _options.exe);
			return;
		}

		_ready = true;
	}

	void PraatScriptBase::consume (IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[]) {

		if (!_ready) {
			return;
		}

		std::string praatOutput = PraatTools::RunPraat (stream_in[0], _options.exe, _options.script, _options.script_args, _options.tmpwav);

		if (_elistener && _parser->parseValues (praatOutput)) {
			
			ssi_size_t n_values = _parser->getNumberOfValues ();

			if (_n_values == 0) { 
				_n_values = n_values;
				ssi_event_adjust (_event, _n_values * sizeof (ssi_event_map_t));		
				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, _event.ptr);
				for (ssi_size_t i = 0; i < _n_values; i++) { 
					tuples[i].value = 0;
					tuples[i].id = Factory::AddString (_parser->getValueName (i));
				}
			}	

			if (_n_values != n_values) {
				ssi_wrn ("number of events must not change");
			} else {

				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, _event.ptr);
				for (ssi_size_t i = 0; i < _n_values; i++) { 
					tuples[i].value = _parser->getValue (i);			
				}

				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sucessfully parsed \"praat\" result");		
			}

			_event.time = ssi_cast (ssi_size_t, 1000 * consume_info.time + 0.5);
			_event.dur = ssi_cast (ssi_size_t, 1000 * consume_info.dur + 0.5);
			_elistener->update (_event);
			

		} else {
			ssi_wrn ("could not parse praat result");
		}
		
	}

	void PraatScriptBase::consume_flush (ssi_size_t stream_in_num, ssi_stream_t stream_in[]) {

		delete _parser; _parser = 0;
		_ready = false;
	}

}
