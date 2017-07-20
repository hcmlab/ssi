// TupleScale.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/11/25
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

#include "../include/TupleScale.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"

namespace ssi {

char TupleScale::ssi_log_name[] = "t_scale___";

TupleScale::TupleScale (const ssi_char_t *file)
	:	_file (0),
		_listener (0),
		_dim(0),
		_sum(0),
		_mean(0),
		_stddev_h(0),
		_stddev_h_sum(0),
		_stddev(0),
		_nevents(0),
		_sscore(0),
		_sscore_min(0),
		_sscore_max(0),
		_min(0),
		_max(0)
		{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);

}

TupleScale::~TupleScale () {
	
	ssi_event_destroy (_event);

}

bool TupleScale::setEventListener (IEventListener *listener) {

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

void TupleScale::listen_enter (){

	_nevents = 0;

	_dim = _options.dimension;

	_sum = new ssi_real_t[_dim];
	_mean = new ssi_real_t[_dim];
	_stddev_h = new ssi_real_t[_dim];
	_stddev_h_sum = new ssi_real_t[_dim];
	_stddev = new ssi_real_t[_dim];
	_sscore = new ssi_real_t[_dim];

	_min = new ssi_real_t[_dim];
	_max = new ssi_real_t[_dim];

	_sscore_min = new ssi_real_t[_dim];
	_sscore_max = new ssi_real_t[_dim];

	for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
		_sum[ndim] = 0.0f;
		_mean[ndim] = 0.0f;
		_stddev_h[ndim] = 0.0f;
		_stddev_h_sum[ndim] = 0.0f;
		_stddev[ndim] = 0.0f;
		_sscore[ndim] = 0.0f;

		_min[ndim] = FLT_MAX;
		_max[ndim] = -FLT_MAX;

		_sscore_min[ndim] = FLT_MAX;
		_sscore_max[ndim] = -FLT_MAX;

	}

	if (_listener) {
		ssi_event_adjust (_event, _dim * sizeof (ssi_event_map_t));
	}

}

bool TupleScale::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	ssi_event_t *e = 0;
	events.reset ();

	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){

		e = events.next();
		if (_listener) {

			if(e->type == SSI_ETYPE_MAP){
				
				ssi_size_t n_tuples = (e->tot / (sizeof(ssi_event_map_t)));

				if(n_tuples == _dim){

					_nevents++;
				
					ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, e->ptr);
					float *values = new ssi_real_t [n_tuples];
					for(ssi_size_t nval = 0; nval < n_tuples; nval++){
						values[nval] = ptr[nval].value;
					}
									
					//scaling over all dimensions
					//http://cvpr.uni-muenster.de/teaching/ws06/mustererkennungWS06/script/ME13-2.pdf

					for(ssi_size_t ntuple = 0; ntuple < n_tuples; ntuple++){

						_sum[ntuple] = _sum[ntuple] + values[ntuple];
						_mean[ntuple] = _sum[ntuple] / ssi_real_t(_nevents);
						
						if(_nevents > 2){

							//calc sscore
							_stddev_h[ntuple] = pow( (values[ntuple] - _mean[ntuple]), 2.0f);
							_stddev_h_sum[ntuple] = _stddev_h_sum[ntuple] + _stddev_h[ntuple];
							_stddev[ntuple] = sqrt( _stddev_h_sum[ntuple] / (_nevents - 1) );
							_sscore[ntuple] = (values[ntuple] - _mean[ntuple]) / _stddev[ntuple];		
			
							//norm sscore
							if(_sscore[ntuple] > _sscore_max[ntuple]){
								_sscore_max[ntuple] = _sscore[ntuple];
							}
							if(_sscore[ntuple] < _sscore_min[ntuple]){
								_sscore_min[ntuple] = _sscore[ntuple];
							}
							if(_sscore_max[ntuple] == _sscore_min[ntuple]){
								_sscore[ntuple] = 0.5f;
							}else{
								_sscore[ntuple] = (_sscore[ntuple] - _sscore_min[ntuple]) / (_sscore_max[ntuple] - _sscore_min[ntuple]);
							}

						}

					}
		
					//clean up
					if(values){
						delete values;
						values = 0;
					}

					//send event
					if(_nevents > 3){

						if (_listener) {
									
							_event.dur = e->dur;
							_event.time = e->time;		
									
							ssi_event_map_t *out_ptr = ssi_pcast (ssi_event_map_t, _event.ptr);
																		
							for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
										
								out_ptr[ndim].id = ptr[ndim].id;
								ssi_real_t value = _sscore[ndim];
								//where the magic happens
								{
									value -= 0.5f;
									value *= 2.0f;
								}
								out_ptr[ndim].value = value;
										
							}

							_listener->update (_event);

							//ssi_print("\nDEBUG::\t#%d", _nevents);
							
							//for(ssi_size_t ntuple = 0; ntuple < n_tuples; ntuple++){
								//ssi_print("\nDEBUG::\tDim:\t%d", ntuple);
								/*ssi_print("\nDEBUG::\tMin\t%.2f\n", _min[ntuple]);
								ssi_print("\nDEBUG::\tMax\t%.2f\n", _max[ntuple]);*/
								//ssi_print("\nDEBUG::\tMean\t%.2f\n", _mean[ntuple]);
								//ssi_print("\nDEBUG::\tStdDev\t%.2f\n", _stddev[ntuple]);
								//ssi_print("\nDEBUG::\tStandardScore\t%.2f\n", _sscore[ntuple]);

							//}
							
							//ssi_print("\n\n");
								
						}

					}

				}else{

					//(n_tuples != _dim)
					e = 0;
					return false;

				}

			}else{

				//(e->type != SSI_ETYPE_MAP)
				e = 0;
				return false;

			}

		}

		e = 0;

	}

	return true;
		
}

void TupleScale::listen_flush (){

	if(_sum){
		delete _sum;
		_sum = 0;
	}

	if(_mean){
		delete _mean;
		_mean = 0;
	}

	if(_stddev_h){
		delete _stddev_h;
		_stddev_h = 0;
	}

	if(_stddev_h_sum){
		delete _stddev_h_sum;
		_stddev_h_sum = 0;
	}

	if(_stddev){
		delete _stddev;
		_stddev = 0;
	}

	if(_sscore){
		delete _sscore;
		_sscore = 0;
	}

	if(_min){
		delete _min;
		_min = 0;
	}

	if(_max){
		delete _max;
		_max = 0;
	}

	if(_sscore_min){
		delete _sscore_min;
		_sscore_min = 0;
	}

	if(_sscore_max){
		delete _sscore_max;
		_sscore_max = 0;
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

}

}
