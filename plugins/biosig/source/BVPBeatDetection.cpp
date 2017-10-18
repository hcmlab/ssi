#include "BVPBeatDetection.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi{


	BVPBeatDetection::BVPBeatDetection(ssi_size_t mean_window_number_of_beats, bool verbose) : _mean_window_seconds(mean_window_number_of_beats), verbose(verbose)
	{
		wait_for_mean_buffer_to_fill = true;

		current_state = bvp_beat_detection_find_minimum;
		current_maximum_value = REAL_MIN;
		current_minimum_value = REAL_MAX;
		buffer_size_at_last_maximum = 0;

		mean_window_current = 0;
		mean_window_max = REAL_MIN;
		mean_window_min = REAL_MAX;
		threshold_range = 2 / 3.0f;


	}

	void BVPBeatDetection::findBeat(ssi_byte_t * stream_in_ptr, ssi_time_t sample_rate, ssi_size_t index, ssi_time_t time, ICallback * callback)
	{
		ssi_time_t current_time;

		//make a copy of the stream pointers
		ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in_ptr);

		current_time = time + index / sample_rate;
		ssi_real_t current_value = ptr_in[index];

		//add element to energy_samples_buffer for energy calculation
		energy_samples_buffer.push_back(current_value);

		//add element for mean calculation
		mean_samples_buffer.push_back(current_value);

		//recalculate the mean of the last _mean_window_seconds seconds
		//remove samples outside of the window
		while (mean_samples_buffer.size() / sample_rate > _mean_window_seconds){
			mean_samples_buffer.erase(mean_samples_buffer.begin());
			wait_for_mean_buffer_to_fill = false;
		}
		//calculate mean, min, max inside the window
		if (!mean_samples_buffer.empty()){
			mean_window_current = 0;
			mean_window_max = REAL_MIN;
			mean_window_min = REAL_MAX;
            for (auto val : mean_samples_buffer){
				mean_window_current += val;
				mean_window_max = val > mean_window_max ? val : mean_window_max;
				mean_window_min = val < mean_window_min ? val : mean_window_min;
			}
			mean_window_current = mean_window_current / mean_samples_buffer.size();
		}

		//calculate lower an upper thresholds. Above and below these thresholds the algorithm searches for minimums and maximums to detect beats.
		ssi_real_t lower_threshold = mean_window_current - ((1 - threshold_range) * (mean_window_current - mean_window_min));
		ssi_real_t upper_threshold = mean_window_current + ((1 - threshold_range) * (mean_window_max - mean_window_current));

		/// find minimum or maximum
		if (current_state == bvp_beat_detection_find_minimum){
			if (current_value < current_minimum_value){
				//new minimum
				current_minimum_value = current_value;
				current_minimum_time = current_time;
				//clear energy_samples_buffer for the energy calculation
				energy_samples_buffer.clear();
			}
		}
		else if (current_state == bvp_beat_detection_find_maximum){
			if (current_value > current_maximum_value){
				//new maximum
				current_maximum_value = current_value;
				current_maximum_time = current_time;

				buffer_size_at_last_maximum = ssi_size_t (energy_samples_buffer.size());

			}
		}

		//switch the state based on the mean value and the current value and handle found beats
		if (current_value > upper_threshold && current_state == bvp_beat_detection_find_minimum){
			current_state = bvp_beat_detection_find_maximum;
			// reset the maximum values.
			current_maximum_value = current_value;
			current_maximum_time = current_time;
		}
		else if (current_value < lower_threshold){
			//If the state was find_maximum a new beat is found
			if (current_state == bvp_beat_detection_find_maximum){
				//Energy and power may only be calculated from min to max, energy_samples_buffer has values from min to state switch.
				// These values must be erased.
				if (buffer_size_at_last_maximum <= energy_samples_buffer.size()){
					energy_samples_buffer.erase(energy_samples_buffer.begin() + buffer_size_at_last_maximum, energy_samples_buffer.end());
				}

				if (!wait_for_mean_buffer_to_fill){
					//Create beat object.
					bvp_beat_t beat;
					beat.duration = current_maximum_time - current_minimum_time;
					beat.maximum = current_maximum_value;
					beat.minimum = current_minimum_value;
					beat.start_time = current_minimum_time;
					beat.energy = EnergyFromBuffer(current_minimum_value);
					beat.power = PowerFromBuffer(current_minimum_value);

#ifdef SSI_BVP_DETECTION_DEBUG_THRESHOLDS
					beat.mean = mean_window_current;
					beat.higher = upper_threshold;
					beat.lower = lower_threshold;
#endif // SSI_BVP_DETECTION_DEBUG_THRESHOLDS

					if (verbose){
						ssi_print("[lower: %5.5f, mean: %5.5f, higher: %5.5f,\nmin_value: %5.5f,\nmax_value: %5.5f,\nstart_time:  %5.5f,\nend_time:  %5.5f,\nduration: %5.5f,\nenergy: %5.5f,\npower: %5.5f]\n\n", lower_threshold, mean_window_current, upper_threshold, beat.minimum, beat.maximum, beat.start_time, current_maximum_time, beat.duration, beat.energy, beat.power);
					}

					// use callback for sending events
					if (callback){
						callback->handleBeat(beat);
					}
				}
				//reset the value
				current_minimum_value = current_value;
				current_minimum_time = current_time;
			}
			current_state = bvp_beat_detection_find_minimum;
		}
	}

	ssi_real_t BVPBeatDetection::EnergyFromBuffer(ssi_real_t minimum){
		/// Buffer contains the found beat
		ssi_real_t sum = 0.0;

		for (ssi_size_t i = 0; i < energy_samples_buffer.size(); i++){
			sum += (energy_samples_buffer[i] - minimum) * (energy_samples_buffer[i] - minimum);
		}

		return sum;
	}

	ssi_real_t BVPBeatDetection::PowerFromBuffer(ssi_real_t minimum){
		/// Buffer contains the found beat
		if (energy_samples_buffer.size() == 0){
			return 0.0;
		}
		return EnergyFromBuffer(minimum) / energy_samples_buffer.size();
	}
}
