#pragma once


#ifndef SSI_SHIMMER3_DATATYPES_H
#define SSI_SHIMMER3_DATATYPES_H

namespace ssi {
	namespace shimmer3 {

		enum class SENSORID {
			LOW_NOISE_ACCELEROMETER_X = 0x00,
			LOW_NOISE_ACCELEROMETER_Y = 0x01,
			LOW_NOISE_ACCELEROMETER_Z = 0x02,
			V_SENSE_BATT = 0x03,
			WIDE_RANGE_ACCELEROMETER_X = 0x04,
			WIDE_RANGE_ACCELEROMETER_Y = 0x05,
			WIDE_RANGE_ACCELEROMETER_Z = 0x06,
			MAGNETOMETER_X = 0x07,
			MAGNETOMETER_Y = 0x08,
			MAGNETOMETER_Z = 0x09,
			GYROSCOPE_X = 0x0A,
			GYROSCOPE_Y = 0x0B,
			GYROSCOPE_Z = 0x0C,
			EXTERNAL_ADC_A7 = 0x0D,
			EXTERNAL_ADC_A6 = 0x0E,
			EXTERNAL_ADC_A15 = 0x0F,
			INTERNAL_ADC_A1 = 0x10,
			INTERNAL_ADC_A12 = 0x11,
			INTERNAL_ADC_A13 = 0x12,
			INTERNAL_ADC_A14 = 0x13,
			TEMPERATURE = 0x1A,
			PRESSURE = 0x1B,
			GSR = 0x1C,
			EXG1_STATUS = 0x1D,
			EXG1_CH1 = 0x1E,
			EXG1_CH2 = 0x1F,
			EXG2_STATUS = 0x20,
			EXG2_CH1 = 0x21,
			EXG2_CH2 = 0x22,
			EXG1_CH1_16BIT = 0x23,
			EXG1_CH2_16BIT = 0x24,
			EXG2_CH1_16BIT = 0x25,
			EXG2_CH2_16BIT = 0x26,
			BRIGE_AMPLIFIER_HIGH = 0x27,
			BRIGE_AMPLIFIER_LOW = 0x28,
		};

		enum class SENSORDATATYPE {
			u8,
			u12,
			i16,
			u16,
			u24,
			i24
		};

		typedef size_t SensorSampleSize_t;

		struct SensorSampleInfo {
			SensorSampleSize_t nrOfBytes;
			SENSORDATATYPE representedType;
			bool isLittleEndian;
		};

		typedef size_t SensorDataOffset_t;

	}
}



#endif // !SSI_SHIMMER3_DATATYPES_H
