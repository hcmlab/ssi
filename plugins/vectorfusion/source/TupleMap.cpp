// TupleMap.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/10/15
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

#include "../include/TupleMap.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "SSI_Tools.h"

namespace ssi {

char TupleMap::ssi_log_name[] = "t_map_____";

TupleMap::TupleMap (const ssi_char_t *file)
	:	_file (0),
		_listener (0),
		_dim(0),
		_mapped (0),
		_mapping(0)
		{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	

}

TupleMap::~TupleMap () {
	
	ssi_event_destroy (_event);

}

bool TupleMap::setEventListener (IEventListener *listener) {

	_listener = listener;

	if (_options.address[0] != '\0') {

		_event_address.setAddress(_options.address);
		_event.sender_id = Factory::AddString(_event_address.getSender(0));
		_event.event_id = Factory::AddString(_event_address.getEvent(0));

	}
	else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

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
	}

	return true;

}

void TupleMap::listen_enter (){

	_dim = _options.dimension;
	_mapped = _options.mapped;
	ssi_size_t nmaps = ssi_string2array_count(_options.mapping, ',');
	if(_mapped && nmaps != _dim){
		ssi_wrn ("#mapping indices does not fit #dims");
		return;
	}	

	if (!_mapping && _mapped) {
		this->createMapping(_dim);
		ssi_string2array(nmaps, _mapping, _options.mapping, ',');
	}

	if (_listener) {
		ssi_event_adjust (_event, _dim * sizeof (ssi_event_map_t));
	}

}

bool TupleMap::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (!_mapping && _mapped) {
		ssi_wrn ("no mapping set yet");
		return false;
	}

	ssi_event_t *e = 0;
	events.reset ();

	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){

		e = events.next();
		
		if (_listener) {

			if(e->type == SSI_ETYPE_MAP){

				ssi_size_t n_tuples = (e->tot / (sizeof(ssi_event_map_t)));

				if(n_tuples == 1){

					//n_tuples == 1

					_event.dur = e->dur;
					_event.time = e->time;
								
					ssi_event_map_t *out_ptr = ssi_pcast (ssi_event_map_t, _event.ptr);
					ssi_event_map_t *in_ptr = ssi_pcast (ssi_event_map_t, e->ptr);
							
					for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
						
						out_ptr[ndim].id = in_ptr->id;

						if(_options.mapped){

							out_ptr[ndim].value = _mapping[ndim]*in_ptr->value;

							if(out_ptr[ndim].value > 1.0f){
								out_ptr[ndim].value = 1.0f;
							}
							if(out_ptr[ndim].value < -1.0f){
								out_ptr[ndim].value = -1.0f;
							}
							if(!_options.negative && out_ptr[ndim].value < 0.0f){
								out_ptr[ndim].value = 0.0f;
							}

						}else{

							if(in_ptr != NULL && in_ptr->value >= -1.0f && in_ptr->value <= 1.0f){

								if(!_options.negative && in_ptr[ndim].value < 0.0f){
									out_ptr[ndim].value = 0.0f;
								}else{
									out_ptr[ndim].value = in_ptr->value;
								}

							}else{

								out_ptr[ndim].value = 0.0f;
								ssi_wrn ("no usable data (value[-1.0f .. 1.0f]) in event; please use mapping");

							}
						}

					}

					_listener->update (_event);

				}else if(n_tuples == _dim){
			
					//n_tuples == _dim

					_event.dur = e->dur;
					_event.time = e->time;

					ssi_event_map_t *out_ptr = ssi_pcast (ssi_event_map_t, _event.ptr);
					ssi_event_map_t *in_ptr = ssi_pcast (ssi_event_map_t, e->ptr);

					for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
						
						out_ptr[ndim].id = in_ptr[ndim].id;

						if(_options.mapped){

							out_ptr[ndim].value = _mapping[ndim]*in_ptr[ndim].value;

							if(out_ptr[ndim].value > 1.0f){
								out_ptr[ndim].value = 1.0f;
							}
							if(out_ptr[ndim].value < -1.0f){
								out_ptr[ndim].value = -1.0f;
							}
							if(!_options.negative && out_ptr[ndim].value < 0.0f){
								out_ptr[ndim].value = 0.0f;
							}

						}else{

							if(in_ptr != NULL && in_ptr->value >= -1.0f && in_ptr[ndim].value <= 1.0f){
								
								if(!_options.negative && in_ptr[ndim].value < 0.0f){
									out_ptr[ndim].value = 0.0f;
								}else{
									out_ptr[ndim].value = in_ptr[ndim].value;
								}

							}else{

								out_ptr[ndim].value = 0.0f;
								ssi_wrn ("no usable data (value[-1.0f .. 1.0f]) in event; please use mapping");

							}

						}

					}

					_listener->update (_event);
					
				}else{

					//(n_tuples != 1) && (n_tuples != _dim)

					ssi_wrn ("#tuples neither 1 nor dim, please change #tuples (%d)", n_tuples);
					e = 0;
					return false;
				}

			}else{

				ssi_wrn ("event type not supported");
				e = 0;
				return false;

			}
					
			e = 0;
		}

	}

	return true;
}

void TupleMap::listen_flush (){

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

	releaseMapping();

}

void TupleMap::createMapping (ssi_size_t ndims){

	if(_dim == 0){
		_dim = _options.dimension;
	}

	if (ndims != _dim) {
		ssi_wrn ("number of dimensions does not fit specified dimensions");
		return;
	}

	releaseMapping();

	_mapping = new ssi_real_t[_dim];
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
		_mapping[ndim] = 0.0f;
	}
				
}

void TupleMap::setMapping (ssi_size_t nvalues, ssi_real_t* values){

	if (!_mapping) {
		ssi_wrn ("no mapping created yet");
		return;
	}

	if (nvalues != _dim) {
		ssi_wrn ("number of values exceeds mapped dimensions");
		return;
	}

	for(ssi_size_t ndim = 0; ndim < nvalues; ndim++){
		_mapping[ndim] = values[ndim];
	}

}

void TupleMap::releaseMapping(){
	
	if (_mapping){
		delete _mapping;
		_mapping = 0;
	}

}

}
