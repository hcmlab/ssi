// Shimmer3LogAndStreamDevice.h
// author: Fabian Wildgrube <fabian.widlgrube@student.uni-augsburg.de>
// created: 05/18/2021
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


#ifndef SSI_SHIMMER3_LOGANDSTREAMDEVICE_H
#define	SSI_SHIMMER3_LOGANDSTREAMDEVICE_H

#include <memory>
#include <map>

#include "Serial.h"

namespace ssi {
	/// <summary>
	/// Wrapper for the basic functionality of a Shimmer3 Device programmed with the LogAndStream Firmware.
	/// Can be used as the basis for all Shimmer3 based devices (GSR+, ECG, EMG, ...).
	/// (only supports LogAndStream version >= 0.6.0!!!! v 0.5.0 and older use 2 byte timestamps, newer versions 3 byte timestamps!)
	/// See: https://www.shimmersensing.com/images/uploads/docs/LogAndStream_for_Shimmer3_Firmware_User_Manual_rev0.11a.pdf
	/// </summary>
	class Shimmer3LogAndStreamDevice {
	public:
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

			void parseValues(const uint8_t* rawBuffer);
		};

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

		/// <summary>
		/// A container for a single data packet. Depending on the current configuration of the Shimmer3LogAndStreamDevice it can hold different values.
		/// Use the "get" function to access values of individual sensors.
		/// Do NOT construct an instance of this class anywhere. Use the "Shimmer3LogAndStreamDevice::readNextPacket()" factory function to obtain instances.
		/// </summary>
		class DataPacket {
			friend Shimmer3LogAndStreamDevice; //because we want the device class to be able to access rawData directly to write it but not other classes
	
		public:
			DataPacket(size_t nrOfRawBytes, const Shimmer3LogAndStreamDevice& device) : rawData(nrOfRawBytes), device(device)  {};
			~DataPacket() {};

			/// <summary>
			/// Returns the value of the requested sensor that's stored in this packet.
			/// THROWS if this packet does not contain a value for that sensor or on error.
			/// </summary>
			/// <param name="sensor">The SENSORID of the sensor which's value you want</param>
			/// <returns>the value of the sensor (converted to long for convenience)</returns>
			long get(const SENSORID& sensor) const;

			long getTimestamp() const;

		private:
			unsigned char* data() { return rawData.data(); };

			std::map<SENSORID, SensorDataOffset_t> offsetMap;

			std::vector<unsigned char> rawData;

			const Shimmer3LogAndStreamDevice& device;
		};


	private:
		enum class SHIMMER_STATE {
			DISCONNECTED,
			CONNECTED_NOTSTREAMING,
			CONNECTED_STREAMING
		};

		enum class COMMANDCODE {
			INQUIRY_COMMAND = 0x01,
			START_SDBT = 0x70,
			STOP_SDBT = 0x97,
			GET_STATUS = 0x72
		};

		enum class RESPONSECODE {
			ACK = 0xFF,
			DATA_PACKET = 0x00,
			INQUIRY_RESPONSE = 0x02,
			STATUS_RESPONSE = 0x71
		};

	public:
		Shimmer3LogAndStreamDevice() : m_comPortNr(0), m_baudRate(0), m_state(SHIMMER_STATE::DISCONNECTED) {};
		Shimmer3LogAndStreamDevice(ssi_size_t comPortNr, ssi_size_t baudRate = 115200): m_comPortNr(comPortNr), m_baudRate(baudRate), m_state(SHIMMER_STATE::DISCONNECTED) {};
		~Shimmer3LogAndStreamDevice() {};

		/// <summary>
		/// Attempts to connect to the Shimmer device at the comPortNr of this instance.
		/// </summary>
		/// <returns>true if connection could be established. false otherwise</returns>
		bool connect();

		bool isConnected();
		bool isStreaming();

		void startStreaming();
		void stopStreaming();

		/// <summary>
		/// Factory function that creates a new packet from the bytes received by the Shimmer.
		/// Blocks until all bytes of that next packet have arrived.
		/// 
		/// PRECONDITION: The Shimmer needs to be connected and the "startStreaming" command has to have been called.
		/// </summary>
		/// <returns>A unique_ptr to a Datapacket. I.e. ownership of the packet is given to the caller. The unique_ptr is null if no packet could be received.</returns>
		std::unique_ptr<DataPacket> readNextPacket();

		static std::map<int, unsigned long> supportedBaudRates;

		///mapping of activated sensors to info how to interpret their bytes.
		static std::map<SENSORID, SensorSampleInfo> sensorSampleInfoMap;

		/// Value ranges and data rate configuration values
		RawSensorConfigurationValues rawSensorConfigurationValues;

	private:
		bool initSerialConnection();

		/// Reads the configuration from the shimmer and sets up internal values of this class to reflect this configuration
		void inquireConfiguration();

		bool checkComPort(const ssi_char_t* portNameStr);

		bool isEnabled(const SENSORID& sensor);

        bool sendCommand(const COMMANDCODE& cmd);

		bool waitForAck();
		bool waitForNextDataPacket();
		bool waitForResponseCode(const RESPONSECODE& code);

		ssi_size_t m_comPortNr;
		ssi_size_t m_baudRate;

		size_t m_packetSize;
		uint8_t m_nrOfPacketsSentAsBatch;
		double m_samplingRate;

		/// Each sensor that is currently activated on the shimmer is mapped to the offset its data will have in the raw bytes of a data packet from the shimmer
		/// If a sensorId is not in this map, the sensor is not active
		std::map<SENSORID, SensorDataOffset_t> m_sensorOffsetWithinPacketMap;

		SHIMMER_STATE m_state;

		std::unique_ptr<Serial> m_serial;
	};
}

#endif /* SSI_SHIMMER3_LOGANDSTREAMDEVICE_H */
