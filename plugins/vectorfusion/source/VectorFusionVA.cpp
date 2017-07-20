// VectorFusionVA.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/11/13
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

#include "../include/VectorFusionVA.h"
#include "ioput/file/FileTools.h"
#include "event/include/TheEventBoard.h"
#include "base/Factory.h"
#include "graphic/Window.h"
#include "graphic/Canvas.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *VectorFusionVA::ssi_log_name = "vecfus_va_";

VectorFusionVA::VectorFusionVA (const ssi_char_t *file)
	: _file (0),
	_dim (0),
	_update_ms (0),
	_update_counter(0),
	_decay_type (EVector::DECAY_TYPE_HYP),
	_fusion_point (0),
	_fusion_vector(0),
	_threshold(0),
	_baseline (0),
	_baseline_norm (0),
	_baseline_weight (0),
	_fusion_speed (0),
	_event_speed (0),
	_gradient(0),
	_framework_time (0),
	_last_call (0),
	_print(0),
	_paint(0),
	_window(0),
	_canvas(0),
	_listener (0),
	_plot (0),
	_modality_map (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);

	_baseline_id = Factory::AddString("baseline");

}


VectorFusionVA::~VectorFusionVA() {

}

bool VectorFusionVA::setEventListener (IEventListener *listener) {

	_listener = listener;

	_valence_id = Factory::AddString("Valence");
	_arousal_id = Factory::AddString("Arousal");
	
	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	}
	else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead");

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

void VectorFusionVA::listen_enter (){

	_dim = 2;
	_baseline = new ssi_real_t[_dim];
	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
		_baseline[ndim] = 0.0f;
	}
	_fusion_point = new ssi_real_t[_dim];
	for(ssi_size_t i = 0; i < _dim; i++){
		_fusion_point[i] = _baseline[i];
	}
	_baseline_weight = 1.0f;
	_update_ms = _options.update_ms;
	_update_counter = 0;
	_decay_type = _options.decay_type;
	_gradient = _options.gradient;
	_threshold = _options.threshold;		//minimum vector norm to be included in fusion 
	_fusion_speed = _options.fusionspeed;	//speed of fusion vector towards mass center
	_event_speed = _options.eventspeed;
	_print = _options.print;
	_paint = _options.paint;
	_board = Factory::GetEventBoard ();

	if (_dim <= 0) {
		ssi_wrn ("dimension of '%u' not permitted", _dim);
		return;
	}

	if (_update_ms < ssi_pcast (TheEventBoard, _board)->getOptions()->update) {
		ssi_wrn ("update rate of TheEventBoard too small: %d", ssi_pcast (TheEventBoard, _board)->getOptions()->update);
	}

	std::ifstream modality_file ( _options.path );
	if ( !modality_file.is_open() ){
		std::cout << "\nCould not open modality file.\n" <<  std::endl;
	}else{
		std::cout << "\nModality file openend.\n" <<  std::endl;
		int n_modalities = 0;
		std::string line;
		while(!modality_file.eof()){
            std::getline(modality_file, line);
			if(line != ""){
				n_modalities++;
			}
        }
		modality_file.close();
		_modality_map = new ModalitySpeedWeightMap();
		modality_file.open(_options.path);
		for(int i = 0; i < n_modalities; i++){

			MODALITY modality;
			
			std::string name;
			ssi_real_t speed = 0.0f;
			ssi_real_t weight = 0.0f;
		
			modality_file>> name;
			modality_file>> speed;
			modality_file>> weight;
			
			modality.name = name;
			modality.speed = speed;
			modality.weight = weight;

			_modality_map[0][name] = modality;

			std::cout<< "Modality " << _modality_map[0][name].name << "\tSpeed\t" << _modality_map[0][name].speed << "\tWeight\t" << _modality_map[0][name].weight << "\n" << std::endl;

		}
		modality_file.close();
	}

	if(_paint){
		_plot = new FusionPainterVA ();
		_canvas = new Canvas();
		_canvas->addClient(_plot);
		_window = new Window();		
		_window->setClient(_canvas);
		_window->setTitle(_options.wcaption);
		_window->setPosition(ssi_rect(_options.move[0], _options.move[1], _options.move[2], _options.move[3]));
		_window->create();
		_window->show();
	}

	if (_listener) {
		ssi_event_adjust (_event, _dim * sizeof (ssi_event_map_t));
	}
}

bool VectorFusionVA::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	Lock lock (_mutex);

	_last_call = _framework_time;
	_framework_time = time_ms;
	
	if(_print){
		ssi_print("\nEVENTS\n");
		_board->Print (events);
	}

	//transform (new) events to vectors
	ssi_event_t *e = 0;
	ssi_size_t i = 0;
	events.reset ();//! (because of printing of board)
	for(ssi_size_t i = 0; i < n_new_events; i++){
		e = events.next ();
		transformEventToVector(e);
		e = 0;
	}
		
	//update (decay) vectors
	for(ssi_size_t i = 0; i < VectorList.size(); i++){
		VectorList[i]->update(_framework_time, _baseline);
	}

	//combine event vectors to fusion vector
	if(!_fusion_vector){
		_fusion_vector = new EVector(_dim, 1.0f, _fusion_speed, _decay_type, _gradient, time_ms, false);
		_fusion_vector->set_values(_dim, _baseline);
		_fusion_vector->set_values_decay(_dim, _baseline);
	}else{
		//_fusion_vector->set_values_decay(_dim, _baseline);
		ssi_real_t delta_t	 = (_framework_time - _last_call)/1000.f;	//time elapsed between this and last update (current framework time minus framework time of last update)
		combineVectors(_fusion_speed, delta_t, _threshold);
	}

	if(_print && _fusion_vector){
		ssi_print("\nVECTORS\n");
		print();
	}

	if(_paint){
		_plot->setData(_dim, _baseline, _threshold, VectorList, _fusion_point, _fusion_vector, false, _options.paint_events);
		_window->update();
	}

	if (_listener) {
		if( _update_counter * _update_ms <= time_ms){
			_event.dur = time_ms - _event.time;
			_event.time = time_ms;		
			ssi_event_map_t *e = ssi_pcast (ssi_event_map_t, _event.ptr);
			for (ssi_size_t i = 0; i < _dim; i++) {
				if (i == 0) { e[i].id = _valence_id; }
				if (i == 1) { e[i].id = _arousal_id; }
				if (i >= 2) { e[i].id = Factory::AddString("Value"); }
				e[i].value = *(_fusion_vector->get_value_decay () + i);
			}
			_listener->update (_event);

			_update_counter++;
		}
	}

	return true;
}

void VectorFusionVA::listen_flush (){

	if (_fusion_point){
		delete [] _fusion_point;
		_fusion_point = 0;
	}

	if (_fusion_vector){
		delete _fusion_vector;
		_fusion_vector = 0;
	}

	if (_modality_map){
		delete _modality_map;
		_modality_map = 0;
	}

	if(_baseline){
		delete _baseline;
		_baseline = 0;
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	for(ssi_size_t i = 0; i < VectorList.size(); i++){		
		delete VectorList[i];
	}
	VectorList.clear();

	if(_paint){
		_window->close();
		delete _plot;
		_plot = 0;
		delete _window;
		_window = 0;
		delete _canvas;
		_canvas = 0;
	}
	
	if (_listener) {
		ssi_event_reset (_event);
	}

	_dim = 0;
	_framework_time = 0;
	_last_call = 0;
	_threshold = 0;
	_fusion_speed = 0;
	_event_speed = 0;
	_gradient = 0;
	_print = 0;
	_paint = false;

}

bool VectorFusionVA::transformEventToVector(ssi_event_t *Event){

	/*ssi_size_t dim = (Event->tot / (sizeof(ssi_event_map_t)));

	if(dim != _dim){
		ssi_wrn ("events values from id#%u dimension '%u' does not fit dimension '%u'", Event->sender_id, dim, _dim);
		return false;
	}

	if(Event->event_id == _baseline_id){
		ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
		for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
			_baseline[ndim] = tuples[ndim].value;
		}
		if(_modality_map){
		
			bool mFound = _modality_map[0].count(Factory::GetString(Event->sender_id)) != 0;
		
			if(mFound){
			
				ssi_real_t mWeight = _modality_map[0][Factory::GetString(Event->sender_id)].weight;
				_baseline_weight = mWeight;

			}

		}
		return true;

	}else{

		if(_modality_map){
		
			bool mFound = _modality_map[0].count(Factory::GetString(Event->sender_id)) != 0;
		
			if(mFound){
			
				ssi_real_t mWeight = _modality_map[0][Factory::GetString(Event->sender_id)].weight;
				ssi_real_t mSpeed = _modality_map[0][Factory::GetString(Event->sender_id)].speed;

				ssi_size_t time = Event->time;
				EVector *v = new EVector( dim, mWeight, mSpeed, _decay_type, _gradient, time, true);
				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
				v->set_values(dim, tuples);
				tuples = 0;
				VectorList.push_back(v);
				return true;

			}else{

				ssi_wrn ("modality %s values not specified, using default values", Factory::GetString(Event->sender_id));
				ssi_size_t time = Event->time;
				EVector *v = new EVector( dim, 1.0f, _event_speed, _decay_type, _gradient, time, true);
				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
				v->set_values(dim, tuples);
				tuples = 0;
				VectorList.push_back(v);
				return true;

			}	

		}else{

			ssi_size_t time = Event->time;
			EVector *v = new EVector( dim, 1.0f, _event_speed, _decay_type, _gradient, time, true);
			ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
			v->set_values(dim, tuples);
			tuples = 0;
			VectorList.push_back(v);
			return true;

		}

	}*/

	ssi_size_t dim = (Event->tot / (sizeof(ssi_event_map_t)));

	EventAddress ea;
	ea.setEvents(Factory::GetString(Event->event_id));
	ea.setSender(Factory::GetString(Event->sender_id));	

	if(dim != _dim){
		ssi_wrn("events values from '%s' dimension '%u' does not fit dimension '%u'", ea.getAddress(), dim, _dim);
		return false;
	}

	if(Event->event_id == _baseline_id){
		ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
		for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
			_baseline[ndim] = tuples[ndim].value;
		}
		return true;

	}else{

		if(_modality_map){
		
			bool mFound = _modality_map[0].count(ea.getAddress()) != 0;
		
			if(mFound){
			
				ssi_real_t mWeight = _modality_map[0][ea.getAddress()].weight;
				ssi_real_t mSpeed = _modality_map[0][ea.getAddress()].speed;

				ssi_size_t time = Event->time;
				EVector *v = new EVector( dim, mWeight, mSpeed, _decay_type, _gradient, time, true);
				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
				v->set_values(dim, tuples);
				tuples = 0;
				VectorList.push_back(v);
				return true;

			}else{

				ssi_wrn("modality '%s' values not specified, using default values", ea.getAddress());
				ssi_size_t time = Event->time;
				EVector *v = new EVector( dim, 1.0f, _event_speed, _decay_type, _gradient, time, true);
				ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
				v->set_values(dim, tuples);
				tuples = 0;
				VectorList.push_back(v);
				return true;

			}	

		}else{

			ssi_size_t time = Event->time;
			EVector *v = new EVector( dim, 1.0f, _event_speed, _decay_type, _gradient, time, true);
			ssi_event_map_t *tuples = ssi_pcast (ssi_event_map_t, Event->ptr);
			v->set_values(dim, tuples);
			tuples = 0;
			VectorList.push_back(v);
			return true;

		}

	}

}

bool VectorFusionVA::combineVectors(ssi_real_t fusion_speed, ssi_real_t delta_t, ssi_real_t threshold){

	//set up
	ssi_size_t n_vectors = ssi_size_t(VectorList.size());
	ssi_size_t *indices = new ssi_size_t[n_vectors];
	ssi_real_t sum_of_weights = 0.0f;
	ssi_real_t baselinenorm = 0.0f;
	
	for(ssi_size_t i = 0; i < _dim; i++){
		baselinenorm += pow(_baseline[i], 2.0f);
	}
	baselinenorm = sqrt(baselinenorm);
	_baseline_norm = baselinenorm;
	
	//find contributing values and calculate sum of weights for mass center calculation
	for(ssi_size_t i = 0; i < n_vectors; i++){
		if(VectorList[i]->get_weight() > threshold){
			indices[i] = 1;
			sum_of_weights += VectorList[i]->get_weight();
		}else{
			indices[i] = 0;
		}
	}
	
	//are there contributing values?
	ssi_size_t sum = 0;
	for(ssi_size_t i = 0; i < n_vectors; i++){
		if(indices[i] == 1){
			sum = sum++;
		}
	}

	//combine
	ssi_real_t *mod_vector = new ssi_real_t[_dim];
	ssi_real_t *fusion_values = new ssi_real_t[_dim];
	for (ssi_size_t i = 0; i < _dim; i++) {
		fusion_values[i] = _fusion_vector->get_value_decay()[i];
	}
	for(ssi_size_t i = 0; i < _dim; i++){
		_fusion_point[i] = 0.0f;
		mod_vector[i] = 0.0f;
	}

	if(sum_of_weights == 0){
		//if no contributing value walk in direction of origin
		//determine modification vector
		for(ssi_size_t n_dim = 0; n_dim < _dim; n_dim++){
			_fusion_point[n_dim] = _baseline[n_dim];
			mod_vector[n_dim] = _fusion_point[n_dim] - _fusion_vector->get_value_decay()[n_dim];
		}
	}else{
		//determine fusion point, depending on fusion type
		for(ssi_size_t n_dim = 0; n_dim < _dim; n_dim++){
			for(ssi_size_t n_vec = 0; n_vec < n_vectors; n_vec++){
				if(indices[n_vec] == 1){
					_fusion_point[n_dim] = _fusion_point[n_dim] + (VectorList[n_vec]->get_value()[n_dim] * VectorList[n_vec]->get_weight());
				}
			}
			_fusion_point[n_dim] = _fusion_point[n_dim] + (_baseline[n_dim] * _baseline_weight);
			sum_of_weights = sum_of_weights + _baseline_weight;
			_fusion_point[n_dim] = _fusion_point[n_dim] / sum_of_weights;
		}
		//determine modification vector
		for(ssi_size_t n_dim = 0; n_dim < _dim; n_dim++){
			mod_vector[n_dim] = _fusion_point[n_dim] - _fusion_vector->get_value_decay()[n_dim];
		}
			
	}

	//determine modification vector norm
	ssi_real_t mod_vector_norm = 0.0f;
	for(ssi_size_t i = 0; i < _dim; i++){
		mod_vector_norm += pow(mod_vector[i], 2.0f);
	}
	mod_vector_norm = sqrt(mod_vector_norm);

	//how far did we move since last call?
	ssi_real_t movement = delta_t * fusion_speed;
	if(_options.accelerate){
		ssi_real_t acceleration = ssi_real_t(sum);
		if(acceleration > 0.0f){
			movement = movement * acceleration;
		}
	}

	if(mod_vector_norm > 0.0f){

		if(mod_vector_norm < movement){
			movement = mod_vector_norm;
		}

		for(ssi_size_t n_dim = 0; n_dim < _dim; n_dim++){
			mod_vector[n_dim] =  mod_vector[n_dim] * (movement / mod_vector_norm);
			fusion_values[n_dim] = fusion_values[n_dim] + mod_vector[n_dim];
		}

	}

	//update fusion vector
	_fusion_vector->set_values(_dim, _fusion_point);
	_fusion_vector->set_values_decay(_dim, fusion_values);
	_fusion_vector->set_lifetime(_framework_time);

	//erase vectors not contributing anymore
	ssi_size_t iterator = 0;
	for(ssi_size_t i = 0; i < n_vectors; i++){
		if(indices[i] == 0){
			delete (*(VectorList.begin() + iterator));
			VectorList.erase(VectorList.begin() + iterator);
		}else{
			iterator++;
		}
	}

	if(indices){
		delete [] indices;
		indices = 0;
	}

	if(mod_vector){
		delete [] mod_vector;
		mod_vector = 0;
	}

	if(fusion_values){
		delete [] fusion_values;
		fusion_values = 0;
	}

	return true;
}

EVector* VectorFusionVA::getFusionVector(){
	return _fusion_vector;
}

ssi_real_t VectorFusionVA::getThreshold(){
	return _threshold;
}

void VectorFusionVA::print(){

	ssi_print ("#\tcreation time\tlifetime\tnorm\tvalue\n------------------------------------------------------------------------\n");
	for(ssi_size_t i = 0; i < VectorList.size(); i++){
		ssi_print("%03d\t", i); VectorList[i]->print();
	}ssi_print("\nFUS\t");
	_fusion_vector->print();
	ssi_print("\n");

}

bool VectorFusionVA::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_window) {
			_window->hide();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_window) {
			_window->show();
			return true;
		}
		break;
	}
	/*case INotify::COMMAND::RESET:
	{
	// reset vector fusion?
	return true;
	}*/
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_window) {
			return _window->setPosition(message);
		}
		break;
	}

	}

	return false;
}

}
