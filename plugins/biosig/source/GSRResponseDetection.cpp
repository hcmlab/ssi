
#include "GSRResponseDetection.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi{

	GSRResponse::GSRResponse(ssi_real_t minAllowedRegression, ssi_real_t minAmplitude, ssi_time_t minRisingTime, bool verbose) :
		minAllowedRegression(minAllowedRegression),
		minAmplitude(minAmplitude),
		minRisingTime(minRisingTime),
		verbose(verbose)
	{
		current_state = find_minimum;
		current_maximum_value = REAL_MIN;
		current_minimum_value = REAL_MAX;
	}



	void GSRResponse::findResponse(ssi_byte_t * stream_in_ptr, ssi_time_t sample_rate, ssi_size_t index, ssi_time_t time, ICallback * callback){
		ssi_time_t current_time;

		//make a copy of the stream pointers
		ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in_ptr);

		current_time = time + index / sample_rate;

		//add element to buffer for energy calculation
		buffer.push_back(ptr_in[index]);

		if (current_state == find_minimum){
			//check if there is a new minimum
			if (ptr_in[index] <= current_minimum_value){
				current_minimum_value = ptr_in[index];
				//time of the current sample
				current_minimum_time = current_time;
				//clear buffer for the energy calculation
				buffer.clear();
			}
			//check if the thresholds to go to the state find_maximum are exceeded
			if (ptr_in[index] - current_minimum_value > minAmplitude
				&& current_time - current_minimum_time > minRisingTime){
				current_state = find_maximum;
				current_maximum_value = ptr_in[index];
				current_maximum_time = current_time;
			}
		}
		else{
			//check if there is a new maximum
			if (ptr_in[index] >= current_maximum_value){
				current_maximum_value = ptr_in[index];
				//time of the current sample
				current_maximum_time = current_time;
			}
			//check if the thresholds to go to the state find_minumum are exceeded
			if (current_maximum_value - ptr_in[index] > minAllowedRegression){
				//response found, create Response object and call callback
				gsr_response_t response;
				response.duration = current_maximum_time - current_minimum_time;
				response.max_value = current_maximum_value;
				response.min_value = current_minimum_value;
				response.start_time = current_minimum_time;
				response.energy = EnergyFromBuffer(current_minimum_value);
				response.power = PowerFromBuffer(current_minimum_value);

				// use callback for sending events
				if (callback){
					callback->handleResponse(response);
				}

				buffer.clear();

				if (verbose){
					ssi_print("[min_value: %5.5f,\nmax_value: %5.5f,\nstart_time:  %5.5f,\nend_time:  %5.5f,\nduration: %5.5f,\nenergy: %5.5f,\npower: %5.5f]\n\n", response.min_value, response.max_value, response.start_time, current_maximum_time, response.duration, response.energy, response.power);
				}
				current_state = find_minimum;
				current_minimum_value = ptr_in[index];
				current_minimum_time = current_time;
			}
		}
	}


	ssi_real_t GSRResponse::EnergyFromBuffer(ssi_real_t minimum){
		/// Buffer contains the found response
		ssi_real_t sum = 0.0;

		for (ssi_size_t i = 0; i < buffer.size(); i++){
			sum += (buffer[i] - minimum) * (buffer[i] - minimum);
		}

		return sum;
	}

	ssi_real_t GSRResponse::PowerFromBuffer(ssi_real_t minimum){
		/// Buffer contains the found response
		if (buffer.size() == 0){
			return 0.0;
		}
		return EnergyFromBuffer(minimum) / buffer.size();
	}

}