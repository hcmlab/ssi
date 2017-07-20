// GSRArousalEstimation.cpp
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/15 
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

#include "../include/GSRArousalEstimation.h"
#include "../include/GSRArousalCombination.h"
#include "../../signal/include/MvgAvgVar.h"
#include "../../signal/include/MvgNorm.h"
#include "../../signal/include/Butfilt.h"
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

GSRArousalEstimation::GSRArousalEstimation (const ssi_char_t *file) 
	:	_file (0),
		_detrend (0),
		_detrend_norm_minmax (0),
		_mvgavg_longterm (0),
		_butfilt_longterm (0),
		_mvgavg_shortterm (0),
		_combination (0),
		_mvgavg_combination (0)
	{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

GSRArousalEstimation::~GSRArousalEstimation () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void GSRArousalEstimation::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		MvgNorm *detrend = ssi_create(MvgNorm, 0, false);
		detrend->getOptions()->norm = MvgNorm::SUBMIN;
		detrend->getOptions()->method = MvgNorm::SLIDING;
		detrend->getOptions()->win = 60;
		_detrend = detrend;
		ssi_stream_init (_detrend_stream, 0, _detrend->getSampleDimensionOut (stream_in.dim), _detrend->getSampleBytesOut (stream_in.byte), _detrend->getSampleTypeOut (stream_in.type), stream_in.sr);
		_detrend->transform_enter (stream_in, _detrend_stream);
				
		MvgNorm *detrend_norm_minmax = ssi_create(MvgNorm, 0, false);
		detrend_norm_minmax->getOptions()->norm = MvgNorm::MINMAX;
		detrend_norm_minmax->getOptions()->rangea = 0.0f;
		detrend_norm_minmax->getOptions()->rangeb = 1.0f;
		detrend_norm_minmax->getOptions()->method = MvgNorm::SLIDING;
		detrend_norm_minmax->getOptions()->win = 60;
		_detrend_norm_minmax = detrend_norm_minmax;
		ssi_stream_init (_detrend_norm_minmax_stream, 0, _detrend_norm_minmax->getSampleDimensionOut (_detrend_stream.dim), _detrend_norm_minmax->getSampleBytesOut (_detrend_stream.byte), _detrend_norm_minmax->getSampleTypeOut (_detrend_stream.type), _detrend_stream.sr);
		ssi_stream_clone(_detrend_norm_minmax_stream, _detrend_norm_minmax_stream_copy);
		_detrend_norm_minmax->transform_enter (_detrend_stream, _detrend_norm_minmax_stream);

		MvgAvgVar *mvgavg_longterm = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
		mvgavg_longterm->getOptions ()->format = MvgAvgVar::AVG;
		mvgavg_longterm->getOptions ()->method = MvgAvgVar::SLIDING;
		mvgavg_longterm->getOptions ()->win = 90;
		_mvgavg_longterm = mvgavg_longterm;
		ssi_stream_init (_mvgavg_longterm_stream, 0, _mvgavg_longterm->getSampleDimensionOut (_detrend_norm_minmax_stream.dim), _mvgavg_longterm->getSampleBytesOut (_detrend_norm_minmax_stream.byte), _mvgavg_longterm->getSampleTypeOut (_detrend_norm_minmax_stream.type), _detrend_norm_minmax_stream.sr);
		_mvgavg_longterm->transform_enter (_detrend_norm_minmax_stream, _mvgavg_longterm_stream);
		
		Butfilt *butfilt_longterm = ssi_pcast (Butfilt, Factory::Create (Butfilt::GetCreateName (), 0, false));
		butfilt_longterm->getOptions()->zero = true;
		butfilt_longterm->getOptions()->norm = false;
		butfilt_longterm->getOptions ()->low = 0.1;
		butfilt_longterm->getOptions ()->order = 3;
		butfilt_longterm->getOptions ()->type = Butfilt::LOW;	
		_butfilt_longterm = butfilt_longterm;
		ssi_stream_init (_butfilt_longterm_stream, 0, _butfilt_longterm->getSampleDimensionOut (_mvgavg_longterm_stream.dim), _butfilt_longterm->getSampleBytesOut (_mvgavg_longterm_stream.byte), _butfilt_longterm->getSampleTypeOut (_mvgavg_longterm_stream.type), _mvgavg_longterm_stream.sr);
		_butfilt_longterm->transform_enter (_mvgavg_longterm_stream, _butfilt_longterm_stream);

		MvgAvgVar *mvgavg_shortterm = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
		mvgavg_shortterm->getOptions ()->format = MvgAvgVar::AVG;
		mvgavg_shortterm->getOptions ()->method = MvgAvgVar::SLIDING;
		mvgavg_shortterm->getOptions ()->win = 5;
		_mvgavg_shortterm = mvgavg_shortterm;
		ssi_stream_init (_mvgavg_shortterm_stream, 0, _mvgavg_shortterm->getSampleDimensionOut (_detrend_norm_minmax_stream.dim), _mvgavg_shortterm->getSampleBytesOut (_detrend_norm_minmax_stream.byte), _mvgavg_shortterm->getSampleTypeOut (_detrend_norm_minmax_stream.type), _detrend_norm_minmax_stream.sr);
		_mvgavg_shortterm->transform_enter (_detrend_norm_minmax_stream_copy, _mvgavg_shortterm_stream);
				
		GSRArousalCombination *combination = ssi_pcast(GSRArousalCombination, Factory::Create(GSRArousalCombination::GetCreateName(), 0, false));
		_combination = combination;
		ssi_stream_init (_combination_stream, 0, _combination->getSampleDimensionOut (_mvgavg_shortterm_stream.dim), _combination->getSampleBytesOut (_mvgavg_shortterm_stream.byte), _combination->getSampleTypeOut (_mvgavg_shortterm_stream.type), _mvgavg_shortterm_stream.sr);
		ssi_stream_t xtra_sources[1];
		xtra_sources[0] = _mvgavg_shortterm_stream;
		_combination->transform_enter(_butfilt_longterm_stream, _combination_stream, 1, xtra_sources);

		MvgAvgVar *mvgavg_combination = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
		mvgavg_combination->getOptions ()->format = MvgAvgVar::AVG;
		mvgavg_combination->getOptions ()->win = 30;
		mvgavg_combination->getOptions ()->method = MvgAvgVar::SLIDING;
		_mvgavg_combination = mvgavg_combination;
		ssi_stream_init (_mvgavg_combination_stream, 0, _mvgavg_combination->getSampleDimensionOut (_combination_stream.dim), _mvgavg_combination->getSampleBytesOut (_combination_stream.byte), _mvgavg_combination->getSampleTypeOut (_combination_stream.type), _combination_stream.sr);
		_mvgavg_combination->transform_enter(_combination_stream, _mvgavg_combination_stream);	
}

void GSRArousalEstimation::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		ssi_size_t n = stream_in.num;
		ssi_time_t sr = stream_in.sr;

		ssi_stream_adjust (_detrend_stream, n);
		ssi_stream_adjust (_detrend_norm_minmax_stream, n);
		ssi_stream_adjust (_detrend_norm_minmax_stream_copy, n);
		ssi_stream_adjust (_mvgavg_longterm_stream, n);
		ssi_stream_adjust (_butfilt_longterm_stream, n);
		ssi_stream_adjust (_mvgavg_shortterm_stream, n);
		ssi_stream_adjust (_combination_stream, n);
		ssi_stream_adjust (_mvgavg_combination_stream, n);
		
		_detrend->transform (info, stream_in, _detrend_stream);
		_detrend_norm_minmax->transform (info, _detrend_stream, _detrend_norm_minmax_stream);
		ssi_stream_reset(_detrend_norm_minmax_stream_copy);
		ssi_stream_clone(_detrend_norm_minmax_stream, _detrend_norm_minmax_stream_copy);
		_mvgavg_longterm->transform (info, _detrend_norm_minmax_stream, _mvgavg_longterm_stream);
		_butfilt_longterm->transform (info, _mvgavg_longterm_stream, _butfilt_longterm_stream);
		_mvgavg_shortterm->transform (info, _detrend_norm_minmax_stream_copy, _mvgavg_shortterm_stream);
		
		ssi_stream_t xtra_sources[1];
		xtra_sources[0] = _mvgavg_shortterm_stream;
		_combination->transform(info, _butfilt_longterm_stream, _combination_stream, 1, xtra_sources);
		_mvgavg_combination->transform (info, _combination_stream, _mvgavg_combination_stream);
		
		ssi_real_t *ptr_result = ssi_pcast (ssi_real_t, _mvgavg_combination_stream.ptr);
		ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
		for(ssi_size_t nsamp = 0; nsamp < stream_in.num; nsamp++){
			*ptr_out = *ptr_result;
			ptr_result++;
			ptr_out++;
		}
}

void GSRArousalEstimation::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[] ) {

		_mvgavg_combination->transform_flush (_combination_stream, _mvgavg_combination_stream);
		ssi_stream_reset (_mvgavg_combination_stream);
		delete _mvgavg_combination; _mvgavg_combination = 0;

		ssi_stream_t xtra_sources[1];
		xtra_sources[0] = _mvgavg_shortterm_stream;
		_combination->transform_flush (_mvgavg_longterm_stream, _combination_stream, 1, xtra_sources);
		ssi_stream_reset (_combination_stream);
		delete _combination; _combination = 0;

		_mvgavg_shortterm->transform_flush (_detrend_norm_minmax_stream_copy, _mvgavg_shortterm_stream);
		ssi_stream_reset (_mvgavg_shortterm_stream);
		delete _mvgavg_shortterm; _mvgavg_shortterm = 0;

		_butfilt_longterm->transform_flush (_detrend_norm_minmax_stream, _butfilt_longterm_stream);
		ssi_stream_reset (_butfilt_longterm_stream);
		delete _butfilt_longterm; _butfilt_longterm = 0;

		_mvgavg_longterm->transform_flush (_detrend_norm_minmax_stream, _mvgavg_longterm_stream);
		ssi_stream_reset (_mvgavg_longterm_stream);
		delete _mvgavg_longterm; _mvgavg_longterm = 0;

		_detrend_norm_minmax->transform_flush (_detrend_stream, _detrend_norm_minmax_stream);
		ssi_stream_reset (_detrend_norm_minmax_stream);
		ssi_stream_reset (_detrend_norm_minmax_stream_copy);
		delete _detrend_norm_minmax; _detrend_norm_minmax = 0;

		_detrend->transform_flush (stream_in, _detrend_stream);
		ssi_stream_reset (_detrend_stream);
		delete _detrend; _detrend = 0;
}

}
