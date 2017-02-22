// EVector.cpp
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

#include "../include/EVector.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

EVector::EVector ()
	: _dim (0),
	_type (EVector::DECAY_TYPE_LIN),
	_gradient (0),
	_creation_time (0),
	_lifetime (0),
	_speed (0),
	_value (0),
	_value_decay  (0),
	_norm (0),
	_norm_decay (0),
	_param (0),
	_starting_weight (0),
	_current_weight (0),
	_decay_weight(false){

}

EVector::EVector (ssi_size_t dim, ssi_real_t weight, ssi_real_t speed, EVector::DECAY_TYPE type, ssi_real_t gradient, ssi_size_t time, bool decay_weight)
	: _dim (0),
	_type (EVector::DECAY_TYPE_LIN),
	_gradient (0),
	_creation_time (0),
	_lifetime (0),
	_speed(0),
	_value (0),
	_value_decay  (0),
	_norm (0),
	_norm_decay(0),
	_param (0),
	_starting_weight (0),
	_current_weight (0),
	_decay_weight (false){

		if (type > 2) {
			ssi_wrn ("vector decay type '%u' not defined", type);
		}

		if (dim <= 0) {
			ssi_wrn ("vector dimension '%u' not permitted", dim);
		}

		_dim = dim;
		_starting_weight = weight;
		_current_weight = weight;
		_type = type;
		_gradient = gradient;
		_creation_time = time;
		_lifetime = 0;
		_speed = speed;
		_value = new ssi_real_t[_dim];
		_value_decay = new ssi_real_t[_dim];
		for(ssi_size_t i = 0; i < _dim; i++){
			_value[i] = 0.0f;
			_value_decay[i] = 0.0f;
		}
		_decay_weight = decay_weight;
		norm_values();

}

EVector::~EVector () {
	Release();
}

void EVector::Release() {

	_dim  = 0;
	_starting_weight = 0;
	_current_weight = 0;
	/*_type = 0;*/
	_gradient = 0;
	_creation_time = 0;
	_lifetime = 0;
	_norm = 0;
	_norm_decay = 0; 

	if (_value){
		delete [] _value;
		_value = 0;
	}

	if (_value_decay){
		delete [] _value_decay;
		_value_decay = 0;
	}

	if (_param){
		delete [] _param;
		_param = 0;
	}

}

bool EVector::update(ssi_size_t framework_time, ssi_real_t *baseline){
	
	//calculate lifetime
	set_lifetime(framework_time);
	
	subtract_baseline(baseline);

	//calculate norms
	norm_values();

	//decay
	decay();
	
	add_baseline(baseline);

	//calculate new norms
	norm_values();
	
	return true;

}

void EVector::subtract_baseline(ssi_real_t *baseline){

	for(ssi_size_t i = 0; i < _dim; i++){
		_value[i] -= baseline[i];
		_value_decay[i] -= baseline[i];
	}

}

void EVector::add_baseline(ssi_real_t *baseline){

	for(ssi_size_t i = 0; i < _dim; i++){
		_value[i] += baseline[i];
		_value_decay[i] += baseline[i];
	}

}

bool EVector::norm_values(){

	ssi_real_t norm = 0.0f;
	ssi_real_t norm_decay = 0.0f;

	for(ssi_size_t i = 0; i < _dim; i++){
		norm += pow(_value[i], 2.0f);
		norm_decay += pow(_value_decay[i], 2.0f);

	}
	norm = sqrt(norm);
	norm_decay = sqrt(norm_decay);

	_norm = norm;
	_norm_decay = norm_decay;

	return true;
}

bool EVector::decay(){

	ssi_real_t param = _gradient;

	ssi_real_t ground = 0.0f;
	ssi_real_t tmp = 0.0f;

	ssi_real_t decay_factor = 0.0f;

	ssi_real_t dt = ssi_cast(ssi_real_t, _lifetime)/1000;
	ssi_real_t dur = 1.0f/_speed;
	ssi_real_t fac = abs (_norm - ground);
	ssi_real_t lambda = (10.0f * param) / (fac * dur);
	

	//decay value berechnen aus _value, DECAY_TYPE und _lifetime
	switch (_type){
		case DECAY_TYPE_LIN:
			if(_norm > ground){
				tmp = _norm - dt * 1.0f/dur;
				if(tmp < ground){
					decay_factor = ground;
				}else{
					decay_factor = tmp;
				}
			}else{
				tmp = _norm + dt * 1.0f/dur;
				if(tmp > ground){
					decay_factor = ground;
				}else{
					decay_factor = tmp;
				}
			}
			break;
		case DECAY_TYPE_EXP:
			decay_factor = _norm * pow(2.71828183f, -lambda * dt);
			break;
		case DECAY_TYPE_HYP:
			dur = 0.5f * fac * dur;
			decay_factor = ground + ((_norm - ground) / 2.0f) * (1.0f - tanh(lambda * (dt - dur)));
			break;
		default:
			//DECAY_TYPE_LIN
			if(_norm > ground){
				tmp = _norm - dt * 1.0f/dur;
				if(tmp < ground){
					decay_factor = ground;
				}else{
					decay_factor = tmp;
				}
			}else{
				tmp = _norm + dt * 1.0f/dur;
				if(tmp > ground){
					decay_factor = ground;
				}else{
					decay_factor = tmp;
				}
			}
			break;
	}

	if(_norm != 0.0f){
		decay_factor = decay_factor / _norm;
	}else{
		decay_factor = 0.0f;
	}

	for(ssi_size_t i = 0; i < _dim; i++){
		_value_decay[i] = _value[i] * decay_factor /*+ baseline[i]*/;
	}

	if(_decay_weight){
		_current_weight = _starting_weight * decay_factor;
	}else{
		_current_weight = _starting_weight;
	}

	/*ssi_print("\n\nDEBUG::\t_starting_weight\t%.2f\nDEBUG::\t_current_weight\t\t%.2f", _starting_weight, _current_weight);*/
	return true;
}

bool EVector::set_values(ssi_size_t dim, ssi_event_map_t* tuples){
	
	if(dim != _dim){
		ssi_wrn ("new values dimension '%u' does not fit dimension '%u'", dim, _dim);
		return false;
	}
	
	for(ssi_size_t i = 0; i < dim; i++){
		_value[i] = tuples[i].value;
	}

	return true;
}

bool EVector::set_values(ssi_size_t dim, ssi_real_t *value){
	
	if(dim != _dim){
		ssi_wrn ("new values dimension '%u' does not fit dimension '%u'", dim, _dim);
		return false;
	}

	for(ssi_size_t i = 0; i < dim; i++){
		_value[i] = value[i];
	}

	return true;
}

bool EVector::set_values_decay(ssi_size_t dim, const ssi_real_t *value){
	
	if(dim != _dim){
		ssi_wrn ("new values dimension '%u' does not fit dimension '%u'", dim, _dim);
		return false;
	}

	for(ssi_size_t i = 0; i < dim; i++){
		_value_decay[i] = value[i];
	}

	return true;
}

bool EVector::set_lifetime(ssi_size_t framework_time){
	
	_lifetime = framework_time - _creation_time;
	
	return true;
}

const ssi_real_t *EVector::get_value_decay(){
	return _value_decay;
}

const ssi_real_t* EVector::get_value(){
	return _value;
}

ssi_real_t EVector::get_weight(){
	return _current_weight;
}

ssi_real_t EVector::get_speed(){
	return _speed;
}

ssi_real_t EVector::get_gradient(){
	return _gradient;
}

EVector::DECAY_TYPE EVector::get_type(){
	return _type;
}

ssi_size_t EVector::get_time(){
	return _creation_time;
}

ssi_real_t EVector::get_norm(){

	return _norm;
}

ssi_real_t EVector::get_norm_decay(){

	return _norm_decay;
}

bool EVector::get_does_decay_weight(){

	return _decay_weight;
}

void EVector::print(){

	ssi_print("'%d'\t\t'%d'\t\t'%0.2f'\t", _creation_time, _lifetime, _norm);
	for(ssi_size_t i = 0; i < _dim; i++){
		ssi_print("%0.2f ", _value[i]);
	}ssi_print("\n\t\t\t\t\t'%0.2f'\t", _norm_decay);
	for(ssi_size_t i = 0; i < _dim; i++){
		ssi_print("%0.2f ", _value_decay[i]);
	}ssi_print("\n");

}

}
