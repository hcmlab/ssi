// BVPBeatDetection.h
// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//         Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/02/24
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
// This class hold the algorithm for detecting beats in BVP signal streams. The algorithms runs in the findBeat() function.
//
// The detection algorithm consists of a state machine with two states: find minimum and find maximum. Switching between these two states 
// depends on the current signal value and the mean, minimum and maximum value of the last seen n samples. With these values, an upper 
// and a lower threshold value is calculated. The upper threshold is defined as: mean+(1/3*maximum) .The lower threshold is defined as: mean-(1/3*minimum).
// If the current value is below the lower threshold, the state machine is switched to the state “find minimum”.If the current value is 
// above the upper threshold, the state machine is switched to the state “find maximum”.
// 
// The found minimums and maximums are saved with their time and value.
// Once the state machine is in the state “find maximum” and is about to switch to the state “find minimum”, because the value went below 
// the lower threshold, a new beat is found.
// 
// 
// There are two buffers named energy_samples_buffer an mean_samples_buffer. The energy_sampels_buffer holds samples from the minimum of a beat to the maximum
// to be able to calculate the energy of a beat with these samples. The mean_samples_buffer holds samples of the last _mean_window_seconds seconds. It is used
// to calculate the mean value for the detection algorithm.
// 
//************************************************************************************************* 


#pragma once

#include "SSI_Cons.h"
#include "signal/SignalCons.h"


//#define  NDEBUG

#ifndef _BVP_BEATDETECTION_H
#define  _BVP_BEATDETECTION_H

namespace ssi{

	//Send thresholds for the beat detection's mean calculation in the events.
	//#define SSI_BVP_DETECTION_DEBUG_THRESHOLDS

#define SSI_BVP_EVENT_FEATURE_MINIMUM_INDEX 0
#define SSI_BVP_EVENT_FEATURE_MAXIMUM_INDEX 1
#define SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX 2
#define SSI_BVP_EVENT_FEATURE_ENERGY_INDEX 3
#define SSI_BVP_EVENT_FEATURE_POWER_INDEX 4

#ifdef SSI_BVP_DETECTION_DEBUG_THRESHOLDS
#define SSI_BVP_EVENT_FEATURE_DEBUG_MEAN_INDEX 5
#define SSI_BVP_EVENT_FEATURE_DEBUG_LOWER_INDEX 6
#define SSI_BVP_EVENT_FEATURE_DEBUG_HIGHER_INDEX 7

#define SSI_BVP_EVENT_FEATURE_N_VALUES 8
#else
#define SSI_BVP_EVENT_FEATURE_N_VALUES 5
#endif // SSI_BVP_DETECTION_DEBUG_THRESHOLDS


	struct bvp_beat_t
	{
		ssi_real_t minimum;
		ssi_real_t maximum;
		ssi_time_t start_time;
		ssi_time_t duration;
		ssi_real_t energy;
		ssi_real_t power;
#ifdef SSI_BVP_DETECTION_DEBUG_THRESHOLDS
		ssi_real_t mean;
		ssi_real_t lower;
		ssi_real_t higher;
#endif // SSI_BVP_DETECTION_DEBUG_THRESHOLDS


	};

	// States of the "detect beat" state machine
	enum bvp_beat_detection_state{
		bvp_beat_detection_find_minimum,
		bvp_beat_detection_find_maximum
	};

	enum bvp_statistical_function_t
	{
		BVP_SUM = 0,
		BVP_MEAN = 1,
		BVP_VARIANCE = 2,
		BVP_STANDARD_DEVIATION = 3
	};

	class BVPBeatDetection
	{
	public:
		class ICallback
		{
		public:

			virtual void handleBeat(bvp_beat_t beat) = 0;
		};
		BVPBeatDetection(ssi_size_t mean_window_sample_size, bool verbose);
		/// find beats in the bvp signal. When a beat is found, it calls the callback, see ICallBack subclass.
		void findBeat(ssi_byte_t * stream_in_ptr, ssi_time_t sample_rate, ssi_size_t index, ssi_time_t time, ICallback * callback);


	private:
		/// The Buffer for the energy and power calculation
		std::vector<ssi_real_t> energy_samples_buffer;
		ssi_size_t buffer_size_at_last_maximum;

		/// For the mean calculation
		bool wait_for_mean_buffer_to_fill;
		ssi_size_t _mean_window_seconds;
		// This buffer contains samples for the mean calculation. 
		std::vector<ssi_real_t > mean_samples_buffer;
		ssi_real_t mean_window_current; //mean in the window
		ssi_real_t mean_window_max; //minimum in the window
		ssi_real_t mean_window_min; //maximum in the window
		ssi_real_t threshold_range; //[0-1] how many percent of the range between mean and maximum/minimum are ignored


		/// The state of the state machine
		bvp_beat_detection_state current_state;
		/// Values for the state machine to find minimum and maximum
		ssi_real_t current_minimum_value, current_maximum_value;
		ssi_time_t current_minimum_time, current_maximum_time;

		ssi_real_t PowerFromBuffer(ssi_real_t minimum);
		ssi_real_t EnergyFromBuffer(ssi_real_t minimum);

		/// Print debug data?
		bool verbose;

	};
}


#endif // !_BVP_BEATDETECTION_H
