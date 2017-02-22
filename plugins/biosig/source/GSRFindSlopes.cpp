// GSRFindSlopes.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/09/28 
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

#include "GSRFindSlopes.h"

#include "OverlapBuffer.h"

namespace ssi {

ssi_char_t *GSRFindSlopes::ssi_log_name = "gsrslopes_";

GSRFindSlopes::GSRFindSlopes (ICallback *callback, Params params) 
: _callback (0),
_ob_gsr (0),
_ob_mvg (0) {
	
	_callback = callback;
	_params = params;

	_n_overlap = ssi_cast (ssi_size_t, _params.maxdur * _params.sr + 0.5);
	_ob_gsr = new OverlapBuffer (_n_overlap, 1);
	_ob_mvg = new OverlapBuffer (_n_overlap, 2);

}

GSRFindSlopes::~GSRFindSlopes () {

	delete _ob_gsr; _ob_gsr = 0;
	delete _ob_mvg; _ob_mvg = 0;
}

void GSRFindSlopes::process (ssi_size_t n, ssi_time_t sr, ssi_real_t *gsr, ssi_real_t *mvg) {

	/* store last slope_max_thres_2 samples for next call and add slope_max_thres_2 samples from previous call */

	_ob_gsr->push (n, gsr);
	_ob_mvg->push (n, mvg);

	find_slopes (sr, *_ob_gsr, *_ob_mvg);
}

void GSRFindSlopes::find_slopes (ssi_time_t sr, OverlapBuffer &gsr, OverlapBuffer &mvg) {

	/* 
	
	slope_n_std = params.n;
	slope_max_dur = params.maxdur;
	slope_max_thres_2 = round (slope_max_dur / 2 * sc_sr);
	slope_min_dur = params.mindur;
	slope_min_thres = round (slope_min_dur * sc_sr);

	*/

	ssi_real_t slope_n_std = _params.nstd;
	ssi_size_t slope_max_thres = ssi_cast (ssi_size_t, _params.maxdur * sr + 0.5);
	ssi_size_t slope_max_thres_2 = slope_max_thres / 2; // one sided
	ssi_size_t slope_min_thres = ssi_cast (ssi_size_t, _params.mindur * sr + 0.5);
	ssi_size_t from = slope_max_thres_2;
	ssi_size_t to = gsr.size () - (slope_max_thres - slope_max_thres_2);

	/*

	i = 1;
	while i <= length (sc_detrend)
   
		% look for a slope
		if sc(i) - sc_avg(i) > n_sqrt * sc_std(i)
	
	*/

	ssi_size_t left_ind, right_ind;
	for (ssi_size_t i = from; i < to; i++) {

		if (gsr[i] - mvg[2*i+0] > slope_n_std * sqrt (mvg[2*i+1])) {

			/*
        
			% now look for a minimum on left        
        
			slope_min = sc(i);        
			j = i;
			thres = max (1, j - max_thres);
			while j >= thres && sc(j) <= slope_min           
				slope_min = sc(j);            
				j = j-1;
			end
			min_ind = j+1;

			*/

			left_ind = look_for_minimum_on_left (gsr, i, slope_max_thres_2);
	
			/*

			slope_max = sc(i);        
			j = i;  
			thres = min (j + max_thres, length (sc));
			while j <= thres && sc(j) >= slope_max
				slope_max = sc(j);
				j = j+1;
			end
			max_ind = j-1; 

			*/
			
			right_ind = look_for_maximum_on_right (gsr, i, slope_max_thres_2);

			/*
        
			% store indices
       
			if max_ind - min_ind > min_thres
				inds = [inds; min_ind max_ind];        
			end

			*/

			ssi_time_t left_s = gsr.convertRelativeToAbsoluteSampleIndex (left_ind) / sr;
			ssi_time_t right_s = gsr.convertRelativeToAbsoluteSampleIndex (right_ind) / sr;
			if (right_ind - left_ind >= slope_min_thres) {				
				
				ssi_real_t baseline = gsr[right_ind] < gsr[left_ind] ? gsr[right_ind] : gsr[left_ind];
				ssi_real_t amplitude = gsr[right_ind];
				ssi_real_t slope = +1.0f;
				ssi_real_t area = 0;
				ssi_real_t mean_std = 0;
				for (ssi_size_t j = left_ind; j < right_ind; j++) {
					if (gsr[j] > amplitude) {
						amplitude = gsr[j];
					}
					mean_std += sqrt (mvg[j]);
					area += gsr[j] - baseline;
				}
				mean_std /= right_ind - left_ind;
				amplitude -= baseline;
				//if (amplitude > slope_n_std * mean_std) {
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "found up slope [%.2lf..%.2lf]s", left_s, right_s);
					_callback->slope (left_s, right_s, amplitude, area, slope);
				//} else {
				//	ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject up slope [%.2lf..%.2lf]s because amplitude is below threshold (%.2f < %.2f)", left_s, right_s, amplitude, slope_n_std * mean_std);
				//}
			} else {
				ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject up slope [%.2lf..%.2lf]s because duration is too short (%.2lfs < %.2fs)", left_s, right_s, right_s-left_s, _params.mindur);
			}

			/*         
			
			% walk til end of drop
        
			i = max_ind;
			while i <= length (sc_detrend) && sc_detrend(i) > n_sqrt * sc_std(i)
				i = i+1;
			end

			*/

			i = right_ind;
			while (i < to && gsr[i] - mvg[2*i+0] > slope_n_std * sqrt (mvg[2*i+1])) {
				++i;
			}
			
			/*

			% look for a drop
			elseif sc_detrend(i) < - n_sqrt * sc_std(i)

			*/

		} else if (gsr[i] - mvg[2*i+0] < - slope_n_std * sqrt (mvg[2*i+1])) {
        
			/*

			% look for a maximum on left              
        
			drop_max = sc(i);        
			j = i;
			thres = max (1, j - max_thres);         
			while j >= thres && sc(j) >= drop_max
				drop_max = sc(j);
				j = j-1;
			end
			max_ind = j+1;

			*/

			left_ind = look_for_maximum_on_left (gsr, i, slope_max_thres_2);

			/*
        
			% now look for a minimum on right        
        
			drop_min = sc(i);        
			j = i;
			thres = min (j + max_thres, length (sc));       
			while j <= thres && sc(j) <= drop_min
				drop_min = sc(j);            
				j = j+1;
			end
			min_ind = j-1;

			*/
    
			right_ind = look_for_minimum_on_right (gsr, i, slope_max_thres_2);

			/*

			% store indices
        
			if min_ind - max_ind > min_thres
				inds = [inds; max_ind min_ind];        
			end      

			*/

			ssi_time_t left_s = gsr.convertRelativeToAbsoluteSampleIndex (left_ind) / sr;
			ssi_time_t right_s = gsr.convertRelativeToAbsoluteSampleIndex (right_ind) / sr;
			if (right_ind - left_ind >= slope_min_thres) {				
				
				ssi_real_t baseline = gsr[right_ind] < gsr[left_ind] ? gsr[right_ind] : gsr[left_ind];
				ssi_real_t amplitude = gsr[right_ind];
				ssi_real_t slope = -1.0f;
				ssi_real_t area = 0;
				ssi_real_t mean_std = 0;
				for (ssi_size_t j = left_ind; j < right_ind; j++) {
					if (gsr[j] > amplitude) {
						amplitude = gsr[j];
					}
					mean_std += sqrt (mvg[j]);
					area += gsr[j] - baseline;
				}
				mean_std /= right_ind - left_ind;
				amplitude -= baseline;
				//if (amplitude > slope_n_std * mean_std) {
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "found down slope [%.2lf..%.2lf]s", left_s, right_s);
					_callback->slope (left_s, right_s, amplitude, area, slope);
				//} else {
				//	ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject down slope [%.2lf..%.2lf]s because amplitude is below threshold (%.2f < %.2f)", left_s, right_s, amplitude, slope_n_std * mean_std);
				//}
			} else {
				ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject down slope [%.2lf..%.2lf]s because duration is too short (%.2lfs < %.2fs)", left_s, right_s, right_s-left_s, _params.mindur);
			}

			/*
        
			% walk til end of drop
        
			i = min_ind;
			while i <= length (sc_detrend) && sc_detrend(i) < - n_sqrt * sc_std(i)
				i = i+1;
			end

			*/

			i = right_ind;
			while (i < to && gsr[i] - mvg[2*i+0] < - slope_n_std * sqrt (mvg[2*i+1])) {
				++i;
			}

			/*

			else
			i = i+1;
		end   

	*/

		} else {
		
			i = i+1;		
		}

	}	

}

ssi_size_t GSRFindSlopes::look_for_minimum_on_left (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos <= max_walk ? 0 : pos - max_walk;

	while (pos > thres && gsr[pos] >= gsr[pos-1])
		--pos;

	return pos;
}

ssi_size_t GSRFindSlopes::look_for_minimum_on_right (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos + max_walk > n - 1 ? n - 1 : pos + max_walk;

	while (pos < thres && gsr[pos] >= gsr[pos+1])
		++pos;

	ssi_real_t value = gsr[pos];
	ssi_real_t value2 = gsr[pos+1];
	return pos;
}

ssi_size_t GSRFindSlopes::look_for_maximum_on_left (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos <= max_walk ? 0 : pos - max_walk;

	while (pos > thres && gsr[pos] <= gsr[pos-1])
		--pos;

	return pos;
}

ssi_size_t GSRFindSlopes::look_for_maximum_on_right (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos + max_walk > n - 1 ? n - 1 : pos + max_walk;

	while (pos < thres && gsr[pos] <= gsr[pos+1])
		++pos;

	ssi_real_t value = gsr[pos];
	ssi_real_t value2 = gsr[pos+1];
	return pos;
}

}
