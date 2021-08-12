#pragma once
#include <cstdint>

#ifndef SSI_SHIMMER3_SENSORCONFIG_H
#define SSI_SHIMMER3_SENSORCONFIG_H

namespace ssi {
	namespace shimmer3 {

		/// Raw configuration values (parsed from their condensed 3 byte representation into a usable format)
		/// Intended to be used by specific classes that represent a type of Shimmer board (e.g. Shimmer3GSRplus.h)
		struct RawSensorConfigurationValues {
			bool wideRangeAccelHighResolutionMode;
			bool wideRangeAccelLowPowerMode;
			uint8_t wideRangeAccelRange;
			uint8_t wideRangeAccelDataRate;
			uint8_t MPU9X50DataRate;
			uint8_t MPU9X50GyroscopeRange;
			uint8_t MPU9X50AccelerometerRange;
			uint8_t MagnetometerRange;
			uint8_t MagnetometerDataRate;
			uint8_t BMPX80PresureResolution;
			uint8_t GSRRange;
			bool internalExpansionPowerEnabled;

			RawSensorConfigurationValues() :
				wideRangeAccelHighResolutionMode(false),
				wideRangeAccelLowPowerMode(false),
				wideRangeAccelRange(0),
				wideRangeAccelDataRate(0),
				MPU9X50DataRate(0),
				MPU9X50GyroscopeRange(0),
				MPU9X50AccelerometerRange(0),
				MagnetometerRange(0),
				MagnetometerDataRate(0),
				BMPX80PresureResolution(0),
				GSRRange(0),
				internalExpansionPowerEnabled(false) {};

			void parseValues(const uint8_t* rawBuffer) {
				wideRangeAccelHighResolutionMode = (*rawBuffer) & 0x01; //first bit -> bool
				wideRangeAccelLowPowerMode = ((*rawBuffer) >> 1) & 0x01; //second bit -> bool
				wideRangeAccelRange = ((*rawBuffer) >> 2) & 0x03; //third and fourth bit -> number
				wideRangeAccelDataRate = ((*rawBuffer) >> 4) & 0x0F; // fifth throuh eigth bit -> number
				MPU9X50DataRate = *(rawBuffer + 1); //whole second byte -> number
				MPU9X50GyroscopeRange = (*(rawBuffer + 2)) & 0x03; //third byte, first and second bit -> number
				MagnetometerDataRate = ((*(rawBuffer + 2)) >> 2) & 0x07; //third byte, third through fifth bit -> number;
				MagnetometerRange = ((*(rawBuffer + 2)) >> 5) & 0x07; //third byte, sixth through eigth bit -> number;;
				internalExpansionPowerEnabled = (*(rawBuffer + 3)) & 0x01; //fourth byte, first bit -> bool
				GSRRange = ((*(rawBuffer + 3)) >> 1) & 0x07; //fourth byte, second through fourth bit -> number;
				BMPX80PresureResolution = ((*(rawBuffer + 3)) >> 4) & 0x03; //fourth byte, fifth and sixth bit -> number;
				MPU9X50AccelerometerRange = ((*(rawBuffer + 3)) >> 6) & 0x03; //fourth byte, seventh and eighth bit -> number;
			}
		};
	} // namespace shimmer3
}//namespace ssi

#endif // !SSI_SHIMMER3_SENSORCONFIG_H
