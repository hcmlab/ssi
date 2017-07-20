// VectorFusionWriter.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/20
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

#include "../include/VectorFusionWriter.h"
#include "base/ITheEventBoard.h"
#include "base/ITheFramework.h"
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

ssi_char_t *VectorFusionWriter::ssi_log_name = "vecfuswr__";

VectorFusionWriter::VectorFusionWriter (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}


VectorFusionWriter::~VectorFusionWriter() {}

void VectorFusionWriter::listen_enter (){

	if(_options.f_update_ms != 0){
		ssi_time_t sr = 1000.0f / _options.f_update_ms;

		ssi_stream_init(_stream, 0, _options.dim, 4, SSI_REAL, sr);
		_out.open (_stream, _options.path, _options.type, _options.version);
		_out.getDataFile ()->setFormat (_options.delim, _options.flags);
	}else{
		ssi_err("Cannot calculate samplerate due to unset option (f_update_ms)");
		return;
	}
		
}

bool VectorFusionWriter::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
	
	ssi_event_t *e = 0;
	events.reset ();
	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		
		e = events.next ();
		
		if(e->type == SSI_ETYPE_MAP){
			
			ssi_size_t n_tuples = (e->tot / (sizeof(ssi_event_map_t)));
			if(n_tuples == _options.dim){
				ssi_stream_adjust(_stream, 1);
		
				ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, e->ptr);
		
				for(ssi_size_t ntuple = 0; ntuple < n_tuples; ntuple++){
					ssi_pcast(ssi_real_t, _stream.ptr)[ntuple] = ptr->value;
					ptr++;
				}

				_out.write (_stream, _options.stream);
			}else{
				ssi_wrn("specified dimension of writer does not fit number of tuples in fusion event");
			}

		}else if(e->type == SSI_ETYPE_TUPLE){
			
			ssi_size_t n_values = (e->tot / (sizeof(ssi_real_t)));
			if(n_values == _options.dim){
				ssi_stream_adjust(_stream, 1);
		
				ssi_real_t *ptr = ssi_pcast (ssi_real_t, e->ptr);
		
				for(ssi_size_t nvalue = 0; nvalue < n_values; nvalue++){
					ssi_pcast(ssi_real_t, _stream.ptr)[nvalue] = *ptr;
					ptr++;
				}

				_out.write (_stream, _options.stream);
			}else{
				ssi_wrn("specified dimension of writer does not fit number of tuples in fusion event");
			}
		}

	}

	return true;
}


bool VectorFusionWriter::update (ssi_event_t &e) {
	
	if(e.type == SSI_ETYPE_MAP){
			
		ssi_size_t n_tuples = (e.tot / (sizeof(ssi_event_map_t)));
		if(n_tuples == _options.dim){
			ssi_stream_adjust(_stream, 1);
		
			ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, e.ptr);
		
			for(ssi_size_t ntuple = 0; ntuple < n_tuples; ntuple++){
				ssi_pcast(ssi_real_t, _stream.ptr)[ntuple] = ptr->value;
				ptr++;
			}

			_out.write (_stream, _options.stream);
		}else{
			ssi_wrn("specified dimension of writer does not fit number of tuples in fusion event");
		}

	}else if(e.type == SSI_ETYPE_TUPLE){
			
		ssi_size_t n_values = (e.tot / (sizeof(ssi_real_t)));
		if(n_values == _options.dim){
			ssi_stream_adjust(_stream, 1);
		
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, e.ptr);
		
			for(ssi_size_t nvalue = 0; nvalue < n_values; nvalue++){
				ssi_pcast(ssi_real_t, _stream.ptr)[nvalue] = *ptr;
				ptr++;
			}

			_out.write (_stream, _options.stream);
		}else{
			ssi_wrn("specified dimension of writer does not fit number of tuples in fusion event");
		}
	}


	return true;
}

void VectorFusionWriter::listen_flush (){
	
	_out.close ();
	ssi_stream_reset(_stream);

}

}
