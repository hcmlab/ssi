// GSRFindDrops.cpp
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

#include "GSRFindPeaks.h"
#include "OverlapBuffer.h"

namespace ssi {

ssi_char_t *GSRFindPeaks::ssi_log_name = "gsrpeaks__";

GSRFindPeaks::GSRFindPeaks (ICallback *callback, Params params) 
: _callback (0),
_ob_gsr (0),
_ob_mvg (0) {
	
	_callback = callback;
	_params = params;

	_n_overlap = ssi_cast (ssi_size_t, _params.maxdur * _params.sr + 0.5);
	_ob_gsr = new OverlapBuffer (_n_overlap, 1);
	_ob_mvg = new OverlapBuffer (_n_overlap, 2);

}

GSRFindPeaks::~GSRFindPeaks () {

	delete _ob_gsr; _ob_gsr = 0;
	delete _ob_mvg; _ob_mvg = 0;
}

void GSRFindPeaks::process (ssi_size_t n, ssi_time_t sr, ssi_real_t *gsr, ssi_real_t *mvg) {

	/* store last peak_max_thres_2 samples for next call and add peak_max_thres_2 samples from previous call */

	_ob_gsr->push (n, gsr);
	_ob_mvg->push (n, mvg);

	find_peaks (sr, *_ob_gsr, *_ob_mvg);
}

void GSRFindPeaks::find_peaks (ssi_time_t sr, OverlapBuffer &gsr, OverlapBuffer &mvg) {

	/* 
	
	peak_n_std = params.n;
	peak_max_dur = params.maxdur;
	peak_max_thres_2 = round (peak_max_dur / 2 * sc_sr);
	peak_min_dur = params.mindur;
	peak_min_thres = round (peak_min_dur * sc_sr);

	*/

	ssi_real_t peak_n_std = _params.nstd;
	ssi_size_t peak_max_thres = ssi_cast (ssi_size_t, _params.maxdur * sr + 0.5);
	ssi_size_t peak_max_thres_2 = peak_max_thres / 2; // one sided
	ssi_size_t peak_min_thres = ssi_cast (ssi_size_t, _params.mindur * sr + 0.5);
	ssi_size_t from = peak_max_thres_2;
	ssi_size_t to = gsr.size () - (peak_max_thres - peak_max_thres_2);

	/*

	last_x = inf;
	inds = [];
	for i=1:length (sc)-1
    
		x = sc(i);
		next_x = sc(i+1);
   
		% look for a new maximum
    
		if last_x <= x && next_x < x
	
	*/

	ssi_real_t last_x = FLT_MAX;
	ssi_real_t x, next_x;
	ssi_size_t left_ind, right_ind;
	for (ssi_size_t i = from; i < to; i++) {

		x = gsr[i];
		next_x = gsr[i + 1];

		if (last_x <= x && next_x < x) {

			/*
        
			% look for a minimum on the left and right side within the interval
			% otherwise stick to the border of the interval
                
			left_ind = max(1,i-peak_max_thres_2);
			left = sc(left_ind:i);
			left_diff = diff (left);
			left_candidates = left_ind + find (left_diff < 0);        
			if ~isempty (left_candidates)
				left_ind = max (left_candidates);                    
			end

			*/

			left_ind = look_for_minimum_on_left (gsr, i, peak_max_thres_2);
	
			/*

			right_ind = min(length (sc),i+peak_max_thres_2);
			right = sc(i:right_ind);
			right_diff = diff (right);
			right_candidates = i + find (right_diff > 0);        
			if ~isempty (right_candidates)
				right_ind = min (right_candidates);                    
			end
			813
			*/
			
			right_ind = look_for_minimum_on_right (gsr, i, peak_max_thres_2);

			/*
        
			% keep the larger amplitude and shrink the interval on the other
			% side accordingly
        
			if sc(left_ind) < sc(right_ind)            
				while sc(left_ind) < sc(right_ind)        
					left_ind = left_ind + 1;
				end
            
			else        
				while sc(left_ind) > sc(right_ind)
					right_ind = right_ind - 1;
				end
			end

			*/

			if (gsr[left_ind] < gsr[right_ind]) {
				while (gsr[left_ind] < gsr[right_ind]) {
					++left_ind;
				}
			} else {
				while (gsr[left_ind] > gsr[right_ind]) {
					--right_ind;
				}
			}

			/*
        
			% store maximum amplitude of the peak
			% if > min duration and amplitude great than 
			% peak_n_std * mean of the standard deviation in that interval        
        
			if right_ind - left_ind >= peak_min_thres
            
				amplitude = max (sc(left_ind:right_ind) - sc(right_ind));
            
				if amplitude > peak_n_std * mean (sc_std(left_ind:right_ind))
        
					inds = [inds; left_ind right_ind];                
            
				end
            
			end
			*/

			ssi_time_t left_s = gsr.convertRelativeToAbsoluteSampleIndex (left_ind) / sr;
			ssi_time_t right_s = gsr.convertRelativeToAbsoluteSampleIndex (right_ind) / sr;
			if (right_ind - left_ind >= peak_min_thres) {
				
				ssi_time_t from = gsr.convertRelativeToAbsoluteSampleIndex (left_ind) / sr;
				ssi_time_t to = gsr.convertRelativeToAbsoluteSampleIndex (right_ind) / sr;
				ssi_real_t baseline = gsr[right_ind] < gsr[left_ind] ? gsr[right_ind] : gsr[left_ind];
				ssi_real_t amplitude = gsr[right_ind];
				ssi_real_t area = 0;
				ssi_real_t mean_std = 0;
				for (ssi_size_t j = left_ind; j < right_ind; j++) {
					if (gsr[j] > amplitude) {
						amplitude = gsr[j];
					}
					mean_std += sqrt (mvg[j*2+1]); 
					area += gsr[j] - baseline;
				}
				mean_std /= right_ind - left_ind;
				amplitude -= baseline;
				if (amplitude > peak_n_std * mean_std) {
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "found peak [%.2lf..%.2lf]s", left_s, right_s);
					_callback->peak (left_s, right_s, amplitude, area);
				} else {
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject peak [%.2lf..%.2lf]s because amplitude is below threshold (%.2f < %.2f)", left_s, right_s, amplitude, peak_n_std * mean_std);
				}
			} else {
				ssi_msg (SSI_LOG_LEVEL_DEBUG, "reject peak [%.2lf..%.2lf]s because duration is too short (%.2lfs < %.2fs)", left_s, right_s, right_s-left_s, _params.mindur);
			}

			/*
			% walk til end of drop
        
			i = min_ind;
			while i <= length (sc_detrend) && sc_detrend(i) < - n_sqrt * sc_std(i)
				i = i+1;
			end

			*/


			/*
		end
    
		last_x = x;
	end

	*/

		}

		last_x = x;
	}	

}

ssi_size_t GSRFindPeaks::look_for_minimum_on_left (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos <= max_walk ? 0 : pos - max_walk;

	while (pos > thres && gsr[pos] >= gsr[pos-1])
		--pos;

	return pos;
}

ssi_size_t GSRFindPeaks::look_for_minimum_on_right (OverlapBuffer &gsr, ssi_size_t pos, ssi_size_t max_walk) {

	ssi_size_t n = gsr.size ();
	ssi_size_t thres = pos + max_walk > n - 1 ? n - 1 : pos + max_walk;

	while (pos < thres && gsr[pos] >= gsr[pos+1])
		++pos;

	return pos;
}


}
