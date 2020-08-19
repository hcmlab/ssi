// CombinerVA.cpp
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

//TODO: Categorical Mapping

#include "../include/CombinerVA.h"
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

ssi_char_t *CombinerVA::ssi_log_name = "vecfus_va_";

CombinerVA::CombinerVA(const ssi_char_t *file)
	: _file (0),
	_dim (2),
	_update_ms (0),
	_update_counter(0),
	_fusion_point (0),
	_baseline(0),
	_fusion_vector(0),
	_framework_time (0),
	_last_call (0),
	_paint(0),
	_window(0),
	_canvas(0),
	_listener (0),
	_plot (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);
}


	CombinerVA::~CombinerVA() {

}

bool CombinerVA::setEventListener(IEventListener *listener) {

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

void CombinerVA::listen_enter(){

	_dim = 2;
	_fusion_point = new ssi_real_t[_dim];
	for(ssi_size_t i = 0; i < _dim; i++){
		_fusion_point[i] = 0.0f;
	}
	_baseline = new ssi_real_t[_dim];
	for (ssi_size_t i = 0; i < _dim; i++){
		_baseline[i] = 0.0f;
	}
	_update_ms = _options.update_ms;
	_update_counter = 0;
	_paint = _options.paint;
	_board = Factory::GetEventBoard ();

	if (_dim != 2) {
		ssi_wrn ("dimension of '%u' not permitted", _dim);
		return;
	}

	if (_update_ms < ssi_pcast (TheEventBoard, _board)->getOptions()->update) {
		ssi_wrn ("update rate of TheEventBoard too small: %d", ssi_pcast (TheEventBoard, _board)->getOptions()->update);
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

bool CombinerVA::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	Lock lock (_mutex);

	_last_call = _framework_time;
	_framework_time = time_ms;

	//transform (new) events to fvector
	ssi_event_t *e = 0;
	ssi_size_t i = 0;
	ssi_real_t* fusion_values = new ssi_real_t[_dim];
	events.reset ();//! (because of printing of board)
	for(ssi_size_t i = 0; i < n_new_events; i++){
		e = events.next ();
		ssi_event_map_t *em = ssi_pcast(ssi_event_map_t, e->ptr);
		if (strcmp(Factory::GetString(e->event_id), "valence") == 0)
		{
			//ssi_print("\nreceived valence event:\t%s:\t%.2f", Factory::GetString(e->event_id), em[0].value);
			fusion_values[0] = em[0].value;
		}
		else if (strcmp(Factory::GetString(e->event_id), "arousal") == 0)
		{
			//ssi_print("\nreceived arousal event:\t%s:\t%.2f", Factory::GetString(e->event_id), em[0].value);
			fusion_values[1] = em[0].value;
		}
		e = 0;
	}

	if (!_fusion_vector){
		_fusion_vector = new EVector(_dim, 1.0f, 0.01f, ssi::EVector::DECAY_TYPE_LIN, 0.5f, time_ms, false);
		_fusion_vector->set_values(_dim, _baseline);
		_fusion_vector->set_values_decay(_dim, _baseline);
	}
	else
	{
		_fusion_vector->set_values(_dim, fusion_values);
		_fusion_vector->set_values_decay(_dim, fusion_values);
	}
	
	if(_paint){
		_plot->setData(_dim, _baseline, 0.0f, VectorList, fusion_values, _fusion_vector, false, _options.paint_events);
		_window->update();
	}

	delete fusion_values;

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

void CombinerVA::listen_flush(){

	if (_fusion_point){
		delete [] _fusion_point;
		_fusion_point = 0;
	}

	if (_fusion_vector){
		delete _fusion_vector;
		_fusion_vector = 0;
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
	_paint = false;

}

bool CombinerVA::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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
