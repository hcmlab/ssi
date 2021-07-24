// LogAndStreamDevice.h
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
#include <vector>
#include <chrono>

#include "SSI_Cons.h"

#include "Serial.h"
#include "shimmer3datatypes.h"
#include "Shimmer3SensorConfiguration.h"

namespace ssi {

	namespace shimmer3 {

		/// Wrapper for the basic functionality of a Shimmer3 Device programmed with the LogAndStream Firmware.
		/// Can be used as the basis for all Shimmer3 based devices (GSR+, ECG, EMG, ...).
		/// (only supports LogAndStream version >= 0.6.0!!!! v 0.5.0 and older use 2 byte timestamps, newer versions 3 byte timestamps!)
		/// See: https://www.shimmersensing.com/images/uploads/docs/LogAndStream_for_Shimmer3_Firmware_User_Manual_rev0.11a.pdf
		class LogAndStreamDevice {
		public:

			/// A container for a single data packet. Depending on the current configuration of the LogAndStreamDevice it can hold different values.
			/// Use the "get" function to access values of individual sensors.
			/// Use the "LogAndStreamDevice::readNextPacket()" factory function to obtain instances.
			class DataPacket {
				friend LogAndStreamDevice; //because we want the device class to be able to access rawData directly to write it but not other classes
	
			public:
				DataPacket(size_t nrOfRawBytes, const LogAndStreamDevice& device) : rawData(nrOfRawBytes), device(device) {};
				~DataPacket() {};

				/// <summary>
				/// Returns the value of the requested sensor that's stored in this packet.
				/// THROWS if this packet does not contain a value for that sensor or on error.
				/// </summary>
				/// <param name="sensor">The SENSORID of the sensor which's value you want</param>
				/// <returns>the raw value (pure bytes) of the sensor (converted to long for convenience, to deal with varying endianness properly)</returns>
				long get(const SENSORID& sensor) const;

				long getRawTimestamp() const;

			private:
				unsigned char* data() { return rawData.data(); };

				std::vector<unsigned char> rawData;

				const LogAndStreamDevice& device;
			};


		private:
			enum class SHIMMER_STATE {
				DISCONNECTED,
				CONNECTED_NOTSTREAMING,
				CONNECTED_STREAMING
			};

			enum class COMMANDCODE {
				INQUIRY_COMMAND = 0x01,
				START_STREAMING = 0x07,
				STOP_STREAMING = 0x20
			};

			enum class RESPONSECODE {
				ACK = 0xFF,
				DATA_PACKET = 0x00,
				INQUIRY_RESPONSE = 0x02
			};

			#define TIMESTAMP_PACKET_MAX_VALUE 16777216 //variable TimeStampPacketRawMaxValue in ShimmerBluetooth.cs of the C# Shimmer API 

			// helper that keeps track of a sessions timestamps and converts them to ms since the power up of the shimmer
			struct TimestampConverter {
				double lastReceivedTimestamp = 0.0;
				double startTimestamp = 0.0;
				double currentTimestampCycle = 0;
				double firstSystemTimestamp = 0.0;
				bool firstTime = true;

				double convertToMs(long rawTimestampIn) {
					double rawTimestamp = static_cast<double>(rawTimestampIn);

					double calibratedTimeStamp = 0.0;

					//not exactly sure what this does. This is taken verbatim from the C# Shimmer API (ShimmerBluetooth.cs. method "CalibrateTimeStamp")
					if (lastReceivedTimestamp > (rawTimestamp + (TIMESTAMP_PACKET_MAX_VALUE * currentTimestampCycle)))
					{
						currentTimestampCycle++;
					}

					lastReceivedTimestamp = (rawTimestamp + (TIMESTAMP_PACKET_MAX_VALUE * currentTimestampCycle));
					calibratedTimeStamp = lastReceivedTimestamp / 32768 * 1000;   // to convert into mS

					if (firstTime)
					{
						firstTime = false;
						startTimestamp = calibratedTimeStamp;
						auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
						firstSystemTimestamp = std::chrono::duration<double>(now).count();
					}

					return firstSystemTimestamp + (calibratedTimeStamp - startTimestamp); //timestamp relative to session start
				}
			};

			#define INQUIRY_HEADER_BYTES 8

			struct InquiryResponseHeader {
				unsigned char rawData[INQUIRY_HEADER_BYTES];

				static const size_t samplingRateIndex = 0;
				static const size_t configSetupIndex = 2;
				static const size_t nrOfChannelsIndex = 6;
				static const size_t bufferSizeIndex = 7;

				InquiryResponseHeader() { };

				uint16_t samplingRateRawValue() const {
					uint16_t msb = static_cast<uint16_t>(rawData[samplingRateIndex + 1]) << 8;
					uint16_t lsb = static_cast<uint16_t>(rawData[samplingRateIndex]);
					return msb + lsb;
				}

				uint8_t nrOfChannels() const {
					return static_cast<uint8_t>(rawData[nrOfChannelsIndex]);
				}

				uint8_t bufferSize() const {
					return static_cast<uint8_t>(rawData[bufferSizeIndex]);
				}

				const uint8_t* ptrToConfigBytes() const {
					return reinterpret_cast<const uint8_t*>(&rawData) + configSetupIndex;
				}
			};

		public:
			/// map of baudrates, converts from int to windows COM required unsigned long
			static std::map<int, unsigned long> supportedBaudRates;

			///mapping of activated sensors to info how to interpret their bytes.
			static std::map<SENSORID, SensorSampleInfo> sensorSampleInfoMap;

			LogAndStreamDevice() : m_comPortNr(0), m_baudRate(0), m_state(SHIMMER_STATE::DISCONNECTED) {};
			LogAndStreamDevice(ssi_size_t comPortNr, ssi_size_t baudRate = 115200): m_comPortNr(comPortNr), m_baudRate(baudRate), m_state(SHIMMER_STATE::DISCONNECTED) {};
			~LogAndStreamDevice() {};

			bool connect();

			bool isConnected() const;
			bool isStreaming() const;

			void startStreaming();
			void stopStreaming();

			bool isEnabled(const SENSORID& sensor) const;

			float getCalibratedTimestamp(const std::unique_ptr<DataPacket>& packet) {
				return static_cast<float>(m_timestampConverter.convertToMs(packet->getRawTimestamp()));
			}

			/// <summary>
			/// Factory function that creates a new packet from the bytes received by the Shimmer.
			/// Blocks until all bytes of that next packet have arrived.
			/// 
			/// PRECONDITION: The Shimmer needs to be connected and the "startStreaming" command has to have been called.
			/// </summary>
			/// <returns>A unique_ptr to a Datapacket. I.e. ownership of the packet is given to the caller. The unique_ptr is empty if no packet could be received.</returns>
			std::unique_ptr<DataPacket> readNextPacket();

			/// Value ranges and data rate configuration values
			RawSensorConfigurationValues rawSensorConfigurationValues;

		private:
			bool initSerialConnection();

			/// Reads the configuration from the shimmer and sets up internal values of this class to reflect this configuration
			void setupConfigurationFromDeviceValues();
			void initOffsetMapAndPacketSize(const std::vector<unsigned char>& channelInfoBuffer);
			void initSampleRate(const InquiryResponseHeader& header);

			bool checkComPort(const ssi_char_t* portNameStr) const;

			bool sendCommand(const COMMANDCODE& cmd) const;

			/// Blocks until the next byte is received. Returns true if that byte is an ack
			bool waitForImmediateAck() const;

			/// Blocks until a data packet header has been received
			///	Checks for connectivity while waiting to ensure no deadlock if connection was lost
			/// Returns false if device is not streaming or connection was lost
			bool waitForNextAck();

			/// Blocks until a data packet header has been received
			///	Checks for connectivity while waiting to ensure no deadlock if connection was lost
			/// Returns false if device is not streaming or connection was lost
			bool waitForNextDataPacket();

			/// Blocks until the next byte is received. Returns true if that byte is equal to the given response code
			bool waitForImmediateResponseCode(const RESPONSECODE& code) const;

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

			TimestampConverter m_timestampConverter;
		}; //class LogAndStreamDevice

	} // namespace shimmer3
} // namespace ssi

#endif /* SSI_SHIMMER3_LOGANDSTREAMDEVICE_H */
