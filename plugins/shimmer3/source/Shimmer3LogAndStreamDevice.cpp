#include <vector>
#include <type_traits>
#include <iostream>
#include <iomanip>

#include "Shimmer3LogAndStreamDevice.h"

#include "listComPorts.h"
#include "ScopeGuard.h"

namespace ssi {
	namespace shimmer3 {
		std::map<int, unsigned long> LogAndStreamDevice::supportedBaudRates = {
			{ 1200, 1200UL },
			{ 2400, 2400UL },
			{ 4800, 4800UL },
			{ 9600, 9600UL },
			{ 19200, 19200UL },
			{ 38400, 38400UL },
			{ 57600, 57600UL },
			{ 115200, 115200UL }
			//shimmer theoretically can also do:  230400, 460800, 921600 but windows might not
		};

		//Taken from the "LogAndStream_for_Shimmer3_Firmware_User_Manual_rev0.11a"
		std::map<SENSORID, SensorSampleInfo> LogAndStreamDevice::sensorSampleInfoMap = {
			{SENSORID::LOW_NOISE_ACCELEROMETER_X,	{2, SENSORDATATYPE::u12, true}},
			{SENSORID::LOW_NOISE_ACCELEROMETER_Y,	{2, SENSORDATATYPE::u12, true}},
			{SENSORID::LOW_NOISE_ACCELEROMETER_Z,	{2, SENSORDATATYPE::u12, true}},
			{SENSORID::V_SENSE_BATT,				{2, SENSORDATATYPE::u12, true}},
			{SENSORID::WIDE_RANGE_ACCELEROMETER_X,	{2, SENSORDATATYPE::i16, true}},
			{SENSORID::WIDE_RANGE_ACCELEROMETER_Y,	{2, SENSORDATATYPE::i16, true}},
			{SENSORID::WIDE_RANGE_ACCELEROMETER_Z,	{2, SENSORDATATYPE::i16, true}},
			{SENSORID::MAGNETOMETER_X,				{2, SENSORDATATYPE::i16, true}},
			{SENSORID::MAGNETOMETER_Y,				{2, SENSORDATATYPE::i16, true}},
			{SENSORID::MAGNETOMETER_Z,				{2, SENSORDATATYPE::i16, true}},
			{SENSORID::GYROSCOPE_X,					{2, SENSORDATATYPE::i16, false}},
			{SENSORID::GYROSCOPE_Y,					{2, SENSORDATATYPE::i16, false}},
			{SENSORID::GYROSCOPE_Z,					{2, SENSORDATATYPE::i16, false}},
			{SENSORID::EXTERNAL_ADC_A7,				{2, SENSORDATATYPE::u12, true}},
			{SENSORID::EXTERNAL_ADC_A6,				{2, SENSORDATATYPE::u12, true}},
			{SENSORID::EXTERNAL_ADC_A15,			{2, SENSORDATATYPE::u12, true}},
			{SENSORID::INTERNAL_ADC_A1,				{2, SENSORDATATYPE::u12, true}},
			{SENSORID::INTERNAL_ADC_A12,			{2, SENSORDATATYPE::u12, true}},
			{SENSORID::INTERNAL_ADC_A13,			{2, SENSORDATATYPE::u12, true}},
			{SENSORID::INTERNAL_ADC_A14,			{2, SENSORDATATYPE::u12, true}},
			{SENSORID::TEMPERATURE,					{2, SENSORDATATYPE::u16, false}},
			{SENSORID::PRESSURE,					{3, SENSORDATATYPE::u24, false}},
			{SENSORID::GSR,							{2, SENSORDATATYPE::u16, true}},
			{SENSORID::EXG1_STATUS,					{1, SENSORDATATYPE::u8, true}},
			{SENSORID::EXG1_CH1,					{3, SENSORDATATYPE::i24, false}},
			{SENSORID::EXG1_CH2,					{3, SENSORDATATYPE::i24, false}},
			{SENSORID::EXG2_STATUS,					{1, SENSORDATATYPE::u8, true}},
			{SENSORID::EXG2_CH1,					{3, SENSORDATATYPE::i24, false}},
			{SENSORID::EXG2_CH2,					{3, SENSORDATATYPE::i24, false}},
			{SENSORID::EXG1_CH1_16BIT,				{2, SENSORDATATYPE::i16, false}},
			{SENSORID::EXG1_CH2_16BIT,				{2, SENSORDATATYPE::i16, false}},
			{SENSORID::EXG2_CH1_16BIT,				{2, SENSORDATATYPE::i16, false}},
			{SENSORID::EXG2_CH2_16BIT,				{2, SENSORDATATYPE::i16, false}},
			{SENSORID::BRIGE_AMPLIFIER_HIGH,		{2, SENSORDATATYPE::u12, true}},
			{SENSORID::BRIGE_AMPLIFIER_LOW,			{2, SENSORDATATYPE::u12, true}},
		};

		bool LogAndStreamDevice::checkComPort(const ssi_char_t* portNameStr) const {
			std::vector<USBserialDevice*> serialDevices;
			ScopeGuard cleanupDevices([&serialDevices]() {
				for (int i = 0; i < serialDevices.size(); i++)
					delete serialDevices.at(i);

				serialDevices.clear();
			});

			listComPorts(&serialDevices);

			bool foundSerial = false;

			for (int i = 0; i < serialDevices.size(); i++) {
				if (serialDevices.at(i)->comName == portNameStr) {
					foundSerial = true;
					break;
				}
			}

			if (!foundSerial) {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "Available devices: ");
				for (int i = 0; i < serialDevices.size(); i++)
					std::cout << serialDevices.at(i)->comName << " - " << serialDevices.at(i)->manufacturer << " - " << serialDevices.at(i)->pnpId << std::endl;

				ssi_err("Serial port (%s) not found!", portNameStr);
				return false;
			}

			return true;
		}

		bool LogAndStreamDevice::isEnabled(const SENSORID& sensor) const
		{
			auto& it = m_sensorOffsetWithinPacketMap.find(sensor);
			return it != m_sensorOffsetWithinPacketMap.end();
		}

		bool LogAndStreamDevice::connect() {
			if (initSerialConnection()) {
				m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
				ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

				setupConfigurationFromDeviceValues();

				return true;
			}

			return false;		
		}

		bool LogAndStreamDevice::isConnected() const {
			return m_serial && m_serial->IsConnected();
		}

		bool LogAndStreamDevice::isStreaming() const {
			return m_serial && m_state == SHIMMER_STATE::CONNECTED_STREAMING;
		}

		bool LogAndStreamDevice::initSerialConnection() {
			if (m_comPortNr == 0) {
				ssi_err("can not connect to Shimmer if no comport is provided ");
				return false;
			}

			ssi_char_t comPortName[16];
			ssi_sprint(comPortName, "COM%u", m_comPortNr);

			if (!checkComPort(comPortName)) {
				return false;
			}

			//check if baud rate is supported by system
			auto& it = supportedBaudRates.find(m_baudRate);
			if (it == supportedBaudRates.end()) {
				ssi_wrn("baud rate %u not supported. Using default baud rate", m_baudRate);
				m_serial = std::make_unique<Serial>(comPortName);
			}
			else {
				m_serial = std::make_unique<Serial>(comPortName, it->second);
			}

			if (!m_serial->IsConnected()) {
				ssi_err("could not connect serial sensor at port=%s", comPortName);
				return false;
			}
		}

		void LogAndStreamDevice::setupConfigurationFromDeviceValues() {
			if (!sendCommand(COMMANDCODE::INQUIRY_COMMAND)
				|| !waitForImmediateAck()) {
				ssi_wrn("Configuration request could not be sent to the Shimmer");
				return;
			}

			if (!waitForImmediateResponseCode(RESPONSECODE::INQUIRY_RESPONSE)) {
				ssi_wrn("Shimmer did not send correct response code after configuration request");
				return;
			}

			InquiryResponseHeader header;

			//read header
			if (m_serial->ReadData(header.rawData, INQUIRY_HEADER_BYTES) != INQUIRY_HEADER_BYTES) {
				ssi_wrn("Configuration header could not be received from the Shimmer");
				return;
			}

			std::cout << "Shimmer3 inquiry header: \n";
			for (const auto& c : header.rawData) {
				std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c) << " ";
			}
			std::cout << std::endl;

			//get the bytes describing the channels
			std::vector<unsigned char> channelInfoBuffer(header.nrOfChannels());
			if (m_serial->ReadData(channelInfoBuffer.data(), channelInfoBuffer.size()) != channelInfoBuffer.size()) {
				ssi_wrn("Channel configuration information could not be received from the Shimmer");
				return;
			}

			std::cout << "Shimmer3 inquiry channelinfo: \n";
			for (const auto& c : channelInfoBuffer) {
				std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c) << " ";
			}
			std::cout << std::endl;

			//synchronize with received device configuration
			initOffsetMapAndPacketSize(channelInfoBuffer);
			initSampleRate(header);
			m_nrOfPacketsSentAsBatch = header.bufferSize();
			rawSensorConfigurationValues.parseValues(header.ptrToConfigBytes());
		}

		void LogAndStreamDevice::initOffsetMapAndPacketSize(const std::vector<unsigned char>& channelInfoBuffer)
		{
			size_t offset = 3; //offsets start at 3 because the shimmer always sends 3 bytes of timestamp info (at least for LogAndStream > v0.5.0)
			for (size_t i = 0; i < channelInfoBuffer.size(); i++) {
				auto sensorId = SENSORID(channelInfoBuffer[i]);
				m_sensorOffsetWithinPacketMap.emplace(std::make_pair(sensorId, offset));

				const auto& sensorInfo = sensorSampleInfoMap[sensorId];
				offset += sensorInfo.nrOfBytes;
			}
			m_packetSize = offset;
		}

		void LogAndStreamDevice::initSampleRate(const InquiryResponseHeader& header)
		{
			m_samplingRate = 32768.0 / (header.samplingRateRawValue() * 1.0); //magic number taken from Shimmer3 C# API
		}

		void LogAndStreamDevice::startStreaming() {
			if (m_state == SHIMMER_STATE::CONNECTED_NOTSTREAMING
				&& sendCommand(COMMANDCODE::START_STREAMING)
				&& waitForImmediateAck()) {
				m_state = SHIMMER_STATE::CONNECTED_STREAMING;
			}
		}

		void LogAndStreamDevice::stopStreaming() {
			if (m_state == SHIMMER_STATE::CONNECTED_STREAMING 
				&& sendCommand(COMMANDCODE::STOP_STREAMING)
				&& waitForNextAck()) {
				m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
			}
			else {
				ssi_wrn("Could not stop streaming for some reason o.O");
			}
		}

		std::unique_ptr<LogAndStreamDevice::DataPacket> LogAndStreamDevice::readNextPacket() {
			std::unique_ptr<DataPacket> invalidPacket;

			if (!m_serial || m_state != SHIMMER_STATE::CONNECTED_STREAMING) {
				ssi_wrn("Shimmer3 must be connected and streaming to be able to read packets!");
				return invalidPacket;
			}

			if (m_packetSize > 0 && m_sensorOffsetWithinPacketMap.size() > 0) {
				auto packet = std::make_unique<DataPacket>(m_packetSize, *this);

				if (!waitForNextDataPacket()) return invalidPacket;

				m_serial->ReadData(packet->data(), m_packetSize);

				return packet;
			}
			else {
				ssi_wrn("Packet from Shimmer3 could not be read because internal configuration is wrong!");
				return invalidPacket;
			}		
		}

		bool LogAndStreamDevice::sendCommand(const COMMANDCODE& cmd) const {
			if (isConnected()) {
				unsigned char cmdBuffer[1] = { static_cast<unsigned char>(cmd) };
				return m_serial->WriteData(cmdBuffer, 1);
			}

			return false;
		}

		bool LogAndStreamDevice::waitForImmediateAck() const {
			return waitForImmediateResponseCode(RESPONSECODE::ACK);
		}

		bool LogAndStreamDevice::waitForNextAck()
		{
			if (m_state != SHIMMER_STATE::CONNECTED_STREAMING) return false;

			static const size_t connectivityCheckTimeout = 1000000000;
			size_t ctr = 0;
			while (true) {
				if (waitForImmediateResponseCode(RESPONSECODE::ACK))
					return true;

				ctr++;
				if (ctr > connectivityCheckTimeout) {
					if (m_serial->IsConnected()) {
						ctr = 0;
					}
					else {
						ssi_wrn("Shimmer Serial Connection seems to be lost!");
						m_state = SHIMMER_STATE::DISCONNECTED;
						break;
					}
				}
			}

			return false;
		}

		bool LogAndStreamDevice::waitForNextDataPacket()
		{
			if (m_state != SHIMMER_STATE::CONNECTED_STREAMING) return false;

			static const size_t connectivityCheckTimeout = 1000000000;
			size_t ctr = 0;
			while (true) {
				if (waitForImmediateResponseCode(RESPONSECODE::DATA_PACKET))
					return true;

				ctr++;
				if (ctr > connectivityCheckTimeout) {
					if (m_serial->IsConnected()) {
						ctr = 0;
					}
					else {
						ssi_wrn("Shimmer Serial Connection seems to be lost!");
						m_state = SHIMMER_STATE::DISCONNECTED;
						break;
					}
				}
			}

			return false;
		}

		bool LogAndStreamDevice::waitForImmediateResponseCode(const RESPONSECODE& code) const {
			unsigned char responseByte;
			if (m_serial->ReadData(&responseByte, 1) != 1) {
				ssi_wrn("Could not receive response byte from the Shimmer");
				return false;
			}

			if (RESPONSECODE(responseByte) != code) {
				ssi_wrn("Shimmer sent %d, when %d was expected", (int)responseByte, (int)code);
				return false;
			}

			return true;
		}

	} // namespace shimmer3
} //namespace ssi
