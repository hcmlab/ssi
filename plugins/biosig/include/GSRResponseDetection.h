// GSRResponse.h
// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//         Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/01/16
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

#pragma once

#ifndef GSR_RESPONSE_H
#define GSR_RESPONSE_H

#include <vector>
#include "SSI_Cons.h"
#include "signal\SignalCons.h"


namespace ssi{
	// defines for event sending
#define SSI_GSR_EVENT_FEATURE_MIN_VAL_INDEX 0
#define SSI_GSR_EVENT_FEATURE_MAX_VAL_INDEX 1
#define SSI_GSR_EVENT_FEATURE_AMP_INDEX 2
#define SSI_GSR_EVENT_FEATURE_ENERGY_INDEX 3
#define SSI_GSR_EVENT_FEATURE_POWER_INDEX 4
#define SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX 5

	// response item of gsr 
	struct gsr_response_t {
		ssi_real_t min_value;
		ssi_real_t max_value;
		ssi_time_t start_time;
		ssi_time_t duration;
		ssi_real_t energy;
		ssi_real_t power;
	};

	// States of the "find Response" state machine
	enum state{
		find_minimum,
		find_maximum
	};

	//Features of a response
	enum gsr_response_feature_t{
		GSR_AMPLITUDE,
		GSR_RISING_TIME,
		GSR_ENERGY,
		GSR_NUMBER_OF_RESPONSES,
		GSR_POWER
	};

	// Possible statistical functions
	enum gsr_response_feature_statistical_function_t{
		GSR_SUM = 0,
		GSR_MEAN = 1,
		GSR_VARIANCE = 2,
		GSR_STANDARD_DEVIATION = 3
	};

	// Class to find Responses in a GSR signal 
	class GSRResponse{

	public:
		//handleResponse is called every time a new response is detected
		class ICallback{

		public:
			/// @param response the new detected response
			virtual void handleResponse(gsr_response_t response) = 0;
		};

		GSRResponse(ssi_real_t minAllowedRegression, ssi_real_t minAmplitude, ssi_time_t minRisingTime, bool verbose);

		/// find the responses in a gsr signal. When a response is found, it calls the callback, see ICallBack subclass.
		void findResponse(ssi_byte_t * stream_in_ptr, ssi_time_t sample_rate, ssi_size_t index, ssi_time_t time, ICallback * callback);

		/// When a response minimum is found, all values to the maximum are saved to a buffer. This function calculates the energy of these values
		ssi_real_t EnergyFromBuffer(ssi_real_t minimum);
		/// When a response minimum is found, all values to the maximum are saved to a buffer. This function calculates the power of these values
		ssi_real_t PowerFromBuffer(ssi_real_t minimum);

	private:
		/// The Buffer for the energy and power calculation
		std::vector<ssi_real_t> buffer;
		/// A threshold. A response must have at least this rising time to be detected
		ssi_time_t minRisingTime;
		/// A threshold. A response signal may descend by that value without being detected as the responses maximum.
		ssi_real_t minAllowedRegression,
			/// A threshold. A response must have at least this amplitude to be detected
			minAmplitude;

		/// The state of the "find response" state machine
		state current_state;
		/// Values for the state machine to find minimum and maximum
		ssi_real_t current_minimum_value, current_maximum_value;
		ssi_time_t current_minimum_time, current_maximum_time;

		/// Print debug data?
		bool verbose;
	};
}
#endif
