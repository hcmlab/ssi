// VectorSenderContinuous.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#include "../include/VectorSenderContinuous.h"
#include "../include/VectorFusionTools.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "dataanalysis.h"
#include "ap.h"

namespace ssi {

char VectorSenderContinuous::ssi_log_name[] = "vector_s_c";

VectorSenderContinuous::VectorSenderContinuous (const ssi_char_t *file)
	:	_file (0),
		_listener (0),
		_dim(0),
		_data_index(0),
		_anno_index(0),
		_trained(false),
		_lm(0),
		_mapping(0)
		{

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	
}

VectorSenderContinuous::~VectorSenderContinuous () {
	
	ssi_event_destroy (_event);
	
	if(_lm){
		delete _lm;
	}
}

bool VectorSenderContinuous::setEventListener (IEventListener *listener) {

	_listener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;
}

void VectorSenderContinuous::listen_enter (){

	_dim = _options.dimension;
	if (_listener) {
		ssi_event_adjust (_event, _dim * sizeof (ssi_event_map_t));
	}
	
}

bool VectorSenderContinuous::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if(!_trained || !_lm){
		ssi_wrn("Linear Model not trained");
		return false;
	}

	if (!_mapping) {
		ssi_wrn ("no mapping set yet");
		return false;
	}

	ssi_event_t *e = 0;
	events.reset ();
	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		ssi_char_t string[SSI_MAX_CHAR];
		e = events.next ();
		ssi_size_t nVars = e->tot / sizeof(ssi_event_map_t);
		alglib::real_1d_array X;
		X.setlength(nVars);
		ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, e->ptr);
		for(ssi_size_t nvar = 0; nvar < nVars; nvar++){
			double datavar = (double)ptr->value;
			ptr++;
			X(nvar) = datavar;
		}
		ssi_real_t result = (ssi_real_t)lrprocess(*_lm, X);
		e = 0;

		if (_listener) {
			_event.dur = time_ms - _event.time;
			_event.time = time_ms;		
			ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, _event.ptr);

			for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
				ssi_sprint(string, "Dimension%u", ndim);
				e[ndim].id = Factory::AddString(string); // TODO: einmalig in listen_enter im _event variable registrieren
				e[ndim].value = result*_mapping[ndim];
			}

			_listener->update (_event);
		}
	}

	return true;
}

void VectorSenderContinuous::listen_flush (){

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

	releaseMapping();

}

bool VectorSenderContinuous::train(ISamples &samples, ssi_size_t data_index, ssi_size_t anno_index){

	if(_trained && _lm){
		ssi_wrn("Linear Model already trained");
	}

	VectorFusionTools *tools = ssi_pcast ( VectorFusionTools, Factory::Create (VectorFusionTools::GetCreateName()));
	_lm = tools->samples2regfunc(&samples, data_index, anno_index);

	if(_lm){
		_data_index = data_index;
		_anno_index = anno_index;
		_trained = true;
	}else{
		_data_index = 0;
		_anno_index = 0;
		_trained = false;
		ssi_wrn("Linear Model not trained");
	}

	return _trained;

}

void VectorSenderContinuous::save_lm(const ssi_char_t *filepath){

	if(_trained && _lm){
		VectorFusionTools *tools = ssi_pcast ( VectorFusionTools, Factory::Create (VectorFusionTools::GetCreateName()));
		tools->save_lm(_lm, filepath);
	}else{
		ssi_wrn("No Linear Model to save");
		return;
	}

}

void VectorSenderContinuous::load_lm(const ssi_char_t *filepath){

	if(!_trained && !_lm){
		VectorFusionTools *tools = ssi_pcast ( VectorFusionTools, Factory::Create (VectorFusionTools::GetCreateName()));
		_lm = new alglib::linearmodel();
		tools->load_lm(_lm, filepath);
	}else{
		ssi_wrn("Linear Model already trained");
		return;
	}
	if(_lm){
		_trained = true;
	}else{
		ssi_wrn("Linear Model not loaded");
	}

}

void VectorSenderContinuous::createMapping (ssi_size_t ndims){

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

void VectorSenderContinuous::setMapping (ssi_size_t nvalues, ssi_real_t* values){

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

void VectorSenderContinuous::releaseMapping(){
	
	if (_mapping){
		delete _mapping;
		_mapping = 0;
	}

}

}
