// MvgPeakGate.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/11
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "MvgPeakGate.h"
#include "MvgMinMax.h"
#include "MvgAvgVar.h"
#include "graphic/Monitor.h"
#include "graphic/Window.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *MvgPeakGate::ssi_log_name = "mvgpeakgat";

MvgPeakGate::MvgPeakGate (const ssi_char_t *file)
: _file (0),
	_threshold (0),
	_average (0),
	_stdeviation (0),
	_mvg (0),
	_mvg_strm (0),
	_mvg_upd_strm (0),
	_monitor (0),
	_window (0){

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MvgPeakGate::~MvgPeakGate () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}		
}

void MvgPeakGate::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_value = _options.value;
	_keep = _options.keep;
	_fix = _options.fix;
	_full = _options.full;
	_mstd = _options.mstd;
	_thres = _options.thres;

	switch (_thres) {
		case FIX: {	
			break; 
		}
		case FIXAVG: 
		case FIXAVGSTD: {

			_counter_tot = _counter = ssi_cast (int, _options.win * stream_in.sr);

			_mvg = new ITransformer *[stream_in.dim];
			_mvg_strm = new ssi_stream_t[stream_in.dim];				
			_mvg_upd_strm = new ssi_stream_t[stream_in.dim];

			for (ssi_size_t j = 0; j < stream_in.dim; j++) {
				MvgAvgVar *mvg = ssi_pcast (MvgAvgVar, MvgAvgVar::Create (0));
				mvg->getOptions ()->format = _thres == FIXAVG ? MvgAvgVar::AVG : MvgAvgVar::ALL; 	
				mvg->getOptions ()->method = _options.method == MOVING ? MvgAvgVar::MOVING : MvgAvgVar::SLIDING;
				mvg->getOptions ()->win = _options.win;
				_mvg[j] = mvg;
				ssi_stream_init (_mvg_upd_strm[j], 0, 1, stream_in.byte, stream_in.type, stream_in.sr);							
				ssi_stream_init (_mvg_strm[j], 0, _mvg[j]->getSampleDimensionOut (1), stream_in.byte, stream_in.type, stream_in.sr);
				_mvg[j]->transform_enter (_mvg_upd_strm[j], _mvg_strm[j]);	
			}

			_threshold = new ssi_real_t[stream_in.dim];
			_average = new ssi_real_t[stream_in.dim];
			_stdeviation = new ssi_real_t[stream_in.dim];

			break;
		}
	}

	if (_options.monitor) {
		ssi_rect_t rect;
		rect.left = _options.mpos[0];
		rect.top = _options.mpos[1];
		rect.width = _options.mpos[2];
		rect.height = _options.mpos[3];
		#if _WIN32||_WIN64		
		_monitor = new Monitor ();
		_window = new Window();
		_window->setPosition(rect);
		_window->setTitle("MvgPeakGate");
		_window->setClient(_monitor);
		_window->create();
		_window->show ();
		#endif
	}
}

void MvgPeakGate::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	switch (_thres) {

		case FIX: {

			ssi_real_t x, y;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

                    x = *srcptr;
                    srcptr++;
					y = x >= _fix ? (_keep ? x : x - _fix) : _value;
						
                    *dstptr = y;
                    dstptr++;
				}
			}

#if _WIN32||_WIN64
			if (_monitor) {		
				_monitor->clear ();		
				ssi_sprint (_string, "\r\nthres:\t%.5f", _fix);
				_monitor->print (_string);
				_monitor->update();
			}
#endif
			break;
		}

		case FIXAVG:
		case FIXAVGSTD: {
				
			// prepare streams to collect non-peak values
			for (ssi_size_t j = 0; j < stream_in.dim; j++) {
				ssi_stream_adjust (_mvg_upd_strm[j], sample_number);
				_mvg_upd_strm[j].num = 0;
			}
					
			ssi_real_t x, y;				
			bool peak;

			for (ssi_size_t i = 0; i < sample_number; ++i) {
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {

                    x = *srcptr;
                    srcptr++;

					peak = _counter <= 0 && x >= _threshold[j];
					if (peak) { // in case of a peak do not block value
						y = _keep ? x : x - _threshold[j];							
					} else { // otherwise assign block value and add to non-peak values
						*(ssi_pcast (ssi_real_t, _mvg_upd_strm[j].ptr) + _mvg_upd_strm[j].num++) = x;
						y =  _value;
					}			

                    *dstptr = y;
                    dstptr++;
				}
			}

			// update moving threshold using non-peak values
			for (ssi_size_t j = 0; j < sample_dimension; ++j) {
				if (_mvg_upd_strm[j].num > 0) {
					ssi_stream_adjust (_mvg_strm[j], _mvg_upd_strm[j].num);
					_mvg[j]->transform (info, _mvg_upd_strm[j], _mvg_strm[j]);
					// take last value as new threshold
					if (_thres == FIXAVG) {
						_average[j] = (ssi_pcast (ssi_real_t, _mvg_strm[j].ptr))[_mvg_strm[j].num - 1]; // pick latest avg
						_threshold[j] = _fix + _average[j];
					} else {
						_average[j] = (ssi_pcast (ssi_real_t, _mvg_strm[j].ptr))[2 * _mvg_strm[j].num - 2]; // pick latest avg
						_stdeviation[j] = sqrt ((ssi_pcast (ssi_real_t, _mvg_strm[j].ptr))[2 * _mvg_strm[j].num - 1]); // pick latest std
						_threshold[j] = _fix + _average[j] + _mstd * _stdeviation[j];
					}					
				}	
			}
#if _WIN32||_WIN64
			if (_monitor) {		
				_monitor->clear ();		
				_monitor->print ("thres:\t");
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {
					ssi_sprint (_string, "%.5f ", _threshold[j]);					
					_monitor->print (_string);
				}
				ssi_sprint (_string, "\r\nfix:\t%.5f", _fix);
				_monitor->print (_string);
				_monitor->print ("\r\navg:\t");
				for (ssi_size_t j = 0; j < sample_dimension; ++j) {						
					ssi_sprint (_string, "%.5f ", _average[j]);					
					_monitor->print (_string);
				}
				if (_thres == FIXAVGSTD) {
					_monitor->print ("\r\nstd:\t");
					for (ssi_size_t j = 0; j < sample_dimension; ++j) {						
						ssi_sprint (_string, "%.5f ", _stdeviation[j]);					
						_monitor->print (_string);
					}	
				}
				_monitor->update ();
			}
#endif

            if (_counter > 0) {
                #if _WIN32||_WIN64
				if (_monitor) {
					ssi_sprint (_string, "\r\ninitialization..%.1f%%", 100.0f * (1.0f - ((1.0f * _counter) / _counter_tot)));
					_monitor->print (_string);
					_monitor->update ();
				}
                #endif
				// decrement counter until moving window size is reached
				_counter -= sample_number;	
				if (_counter <= 0) {										
					ssi_msg (SSI_LOG_LEVEL_BASIC, "initialization done");
				}
			}

			break;
		}
	}
}

void MvgPeakGate::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	if (_mvg) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			_mvg[j]->transform_flush (_mvg_upd_strm[j], _mvg_strm[j]);
			ssi_stream_destroy (_mvg_strm[j]);			
			ssi_stream_destroy (_mvg_upd_strm[j]);
			delete _mvg[j];
		}
	}
	delete[] _mvg; _mvg = 0;
	delete[] _mvg_upd_strm; _mvg_upd_strm = 0;
	delete[] _threshold; _threshold = 0;
	delete[] _average; _average = 0;
	delete[] _stdeviation; _stdeviation = 0;


	if (_options.monitor) {		
		_window->close ();		
		delete _monitor; _monitor = 0;
		delete _window; _window = 0;
	}
}

bool MvgPeakGate::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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
