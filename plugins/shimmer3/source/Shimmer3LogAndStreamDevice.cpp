#include <vector>
#include <type_traits>
#include <iostream>
#include <iomanip>

#include "SSI_Cons.h"

#include "Shimmer3LogAndStreamDevice.h"

#include "listComPorts.h"
#include "ScopeGuard.h"

namespace ssi {

	std::map<int, unsigned long> Shimmer3LogAndStreamDevice::supportedBaudRates = {
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
	std::map<Shimmer3LogAndStreamDevice::SENSORID, Shimmer3LogAndStreamDevice::SensorSampleInfo> Shimmer3LogAndStreamDevice::sensorSampleInfoMap = {
		{SENSORID::LOW_NOISE_ACCELEROMETER_X,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::LOW_NOISE_ACCELEROMETER_Y,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::LOW_NOISE_ACCELEROMETER_Z,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::V_SENSE_BATT,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::WIDE_RANGE_ACCELEROMETER_X,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::WIDE_RANGE_ACCELEROMETER_Y,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::WIDE_RANGE_ACCELEROMETER_Z,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::MAGNETOMETER_X,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::MAGNETOMETER_Y,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::MAGNETOMETER_Z,
			{2, SENSORDATATYPE::i16, true}},
		{SENSORID::GYROSCOPE_X,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::GYROSCOPE_Y,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::GYROSCOPE_Z,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::EXTERNAL_ADC_A7,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::EXTERNAL_ADC_A6,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::EXTERNAL_ADC_A15,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::INTERNAL_ADC_A1,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::INTERNAL_ADC_A12,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::INTERNAL_ADC_A13,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::INTERNAL_ADC_A14,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::TEMPERATURE,
			{2, SENSORDATATYPE::u16, false}},
		{SENSORID::PRESSURE,
			{3, SENSORDATATYPE::u24, false}},
		{SENSORID::GSR,
			{2, SENSORDATATYPE::u16, true}},
		{SENSORID::EXG1_STATUS,
			{1, SENSORDATATYPE::u8, true}},
		{SENSORID::EXG1_CH1,
			{3, SENSORDATATYPE::i24, false}},
		{SENSORID::EXG1_CH2,
			{3, SENSORDATATYPE::i24, false}},
		{SENSORID::EXG2_STATUS,
			{1, SENSORDATATYPE::u8, true}},
		{SENSORID::EXG2_CH1,
			{3, SENSORDATATYPE::i24, false}},
		{SENSORID::EXG2_CH2,
			{3, SENSORDATATYPE::i24, false}},
		{SENSORID::EXG1_CH1_16BIT,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::EXG1_CH2_16BIT,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::EXG2_CH1_16BIT,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::EXG2_CH2_16BIT,
			{2, SENSORDATATYPE::i16, false}},
		{SENSORID::BRIGE_AMPLIFIER_HIGH,
			{2, SENSORDATATYPE::u12, true}},
		{SENSORID::BRIGE_AMPLIFIER_LOW,
			{2, SENSORDATATYPE::u12, true}},
	};

	void Shimmer3LogAndStreamDevice::RawSensorConfigurationValues::parseValues(const uint8_t* rawBuffer) {
		wideRangeAccelHighResolutionMode = (*rawBuffer) & 0x01; //first bit -> bool
		wideRangeAccelLowPowerMode = ((*rawBuffer) >> 1) & 0x01; //second bit -> bool
		wideRangeAccelRange = ((*rawBuffer) >> 2) & 0x03; //third and fourth bit -> number
		wideRangeAccelDataRate = ((*rawBuffer) >> 4) & 0x0F; // fifth throuh eigth bit -> number
		MPU9X50DataRate = *(rawBuffer+1); //whole second byte -> number
		MPU9X50GyroscopeRange = (*(rawBuffer + 2)) & 0x03; //third byte, first and second bit -> number
		MagnetometerDataRate = ((*(rawBuffer + 2)) >> 2) & 0x07; //third byte, third through fifth bit -> number;
		MagnetometerRange = ((*(rawBuffer + 2)) >> 5) & 0x07; //third byte, sixth through eigth bit -> number;;
		internalExpansionPowerEnabled = (*(rawBuffer + 3)) & 0x01; //fourth byte, first bit -> bool
		GSRRange = ((*(rawBuffer + 3)) >> 1) & 0x07; //fourth byte, second through fourth bit -> number;
		BMPX80PresureResolution = ((*(rawBuffer + 3)) >> 4) & 0x03; //fourth byte, fifth and sixth bit -> number;
		MPU9X50AccelerometerRange = ((*(rawBuffer + 3)) >> 6) & 0x03; //fourth byte, seventh and eighth bit -> number;
	}

	template <typename T>
	T twos_complement(T val,
		// "allow this template to be instantiated only for unsigned types"
		typename std::enable_if<std::is_unsigned<T>::value>::type* = 0)
	{
		return -std::uintmax_t(val);
	}

	long Shimmer3LogAndStreamDevice::DataPacket::get(const SENSORID& sensor) const {
		auto& offsetIterator = device.m_sensorOffsetWithinPacketMap.find(sensor);
		auto& sampleInfoIt = Shimmer3LogAndStreamDevice::sensorSampleInfoMap.find(sensor);

		//check if sensor is actually available in the current configuration
		if (offsetIterator == device.m_sensorOffsetWithinPacketMap.end() || sampleInfoIt == Shimmer3LogAndStreamDevice::sensorSampleInfoMap.end()) {
			throw std::exception("this sensor is not supported");
		}

		const auto& offset = offsetIterator->second;
		const auto& sampleInfo = sampleInfoIt->second;

		if ((offset + sampleInfo.nrOfBytes) > rawData.size()) {
			throw std::exception("The value is represented by more bytes than are available in the raw data packet");
		}

		const unsigned char* dataPtr = rawData.data() + offset;

		long returnValue = 0;

		switch (sampleInfo.representedType)
		{
		case SENSORDATATYPE::u8:
			returnValue = static_cast<long>(*reinterpret_cast<const uint8_t*>(dataPtr));
			break;
		case SENSORDATATYPE::u12:
			if (sampleInfo.isLittleEndian) {
				//byte @dataPtr + ((byte @dataPtr+1) << 8) => [(dataPtr+1)(dataPtr)]
				uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
				uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8; //should the 4 most significant bits be masked out? Or do we assume they are always 0?
				returnValue = static_cast<long>(msb + lsb);
			}
			else {
				//byte @dataPtr+1 + ((byte @dataPtr) << 8) => [(dataPtr)(dataPtr+1)]
				uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8;
				uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
				returnValue = static_cast<long>(msb + lsb);
			}
			break;
		case SENSORDATATYPE::i16:
			{
				uint16_t combined;
				if (sampleInfo.isLittleEndian) {
					//byte @dataPtr + ((byte @dataPtr+1) << 8) => [(dataPtr+1)(dataPtr)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8;
					combined = msb + lsb;
				}
				else {
					//byte @dataPtr+1 + ((byte @dataPtr) << 8) => [(dataPtr)(dataPtr+1)]
					uint16_t lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8;
					uint16_t msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
					combined = msb + lsb;
				}
				//take care of the fact that i16 is a signed value
				bool inversionNeeded = (combined >> 15) == 1;
				returnValue = static_cast<long>((inversionNeeded ? twos_complement<uint16_t>(combined) : combined));
			}
			break;
		case SENSORDATATYPE::u16:
			{
				uint16_t lsb, msb;
				if (sampleInfo.isLittleEndian) {
					//byte @dataPtr + ((byte @dataPtr+1) << 8) => [(dataPtr+1)(dataPtr)]
					lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
					msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8;
				}
				else {
					//byte @dataPtr+1 + ((byte @dataPtr) << 8) => [(dataPtr)(dataPtr+1)]
					lsb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr + 1)) << 8;
					msb = static_cast<uint16_t>(*reinterpret_cast<const uint8_t*>(dataPtr));
				}
				returnValue = static_cast<long>(msb + lsb);
			}
			break;
		case SENSORDATATYPE::u24:
			{
				uint8_t firstByte = *reinterpret_cast<const uint8_t*>(dataPtr);
				uint8_t secondByte = *reinterpret_cast<const uint8_t*>(dataPtr + 1);
				uint8_t thirdByte = *reinterpret_cast<const uint8_t*>(dataPtr + 2);
				uint32_t xmsb, msb, lsb;
				if (sampleInfo.isLittleEndian) {
					xmsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF));
				}
				else {
					xmsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF));
				}
				returnValue = xmsb + msb + lsb;
			}
			break;
		case SENSORDATATYPE::i24:
			{
				uint8_t firstByte = *reinterpret_cast<const uint8_t*>(dataPtr);
				uint8_t secondByte = *reinterpret_cast<const uint8_t*>(dataPtr + 1);
				uint8_t thirdByte = *reinterpret_cast<const uint8_t*>(dataPtr + 2);
				uint32_t xmsb, msb, lsb;
				if (sampleInfo.isLittleEndian) {
					xmsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF));
				}
				else {
					xmsb = (static_cast<uint32_t>(firstByte) & uint32_t(0xFF)) << 16;
					msb = (static_cast<uint32_t>(secondByte) & uint32_t(0xFF)) << 8;
					lsb = (static_cast<uint32_t>(thirdByte) & uint32_t(0xFF));
				}
				//take care of the fact that i24 is a signed value
				uint32_t combined = xmsb + msb + lsb;
				bool inversionNeeded = (combined >> 23) == 1;
				returnValue = static_cast<long>((inversionNeeded ? twos_complement<uint32_t>(combined) : combined));
			}
			break;
		default:
			break;
		}

		return returnValue;
	}

	long Shimmer3LogAndStreamDevice::DataPacket::getTimestamp() const {
		//TODO: interpret the first 3 bytes of the rawData as a timestamp!
		return 0;
	}

	bool Shimmer3LogAndStreamDevice::checkComPort(const ssi_char_t* portNameStr) const {
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

	bool Shimmer3LogAndStreamDevice::isEnabled(const SENSORID& sensor)
	{
		auto& it = m_sensorOffsetWithinPacketMap.find(sensor);
		return it != m_sensorOffsetWithinPacketMap.end();
	}

	bool Shimmer3LogAndStreamDevice::connect() {
		if (initSerialConnection()) {
			m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

			inquireConfiguration();

			return true;
		}

		return false;		
	}

	bool Shimmer3LogAndStreamDevice::isConnected() {
		return m_serial && m_serial->IsConnected();
	}

	bool Shimmer3LogAndStreamDevice::isStreaming() {
		return m_serial && m_state == SHIMMER_STATE::CONNECTED_STREAMING;
	}

	bool Shimmer3LogAndStreamDevice::initSerialConnection() {
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

	void Shimmer3LogAndStreamDevice::inquireConfiguration() {
		if (!sendCommand(COMMANDCODE::INQUIRY_COMMAND)
			|| !waitForImmediateAck()) {
			ssi_wrn("Configuration request could not be sent to the Shimmer");
			return;
		}

		//wait for first byte (inquiry response code)
		unsigned char responseCode;
		if (m_serial->ReadData(&responseCode, 1) != 1) {
			ssi_wrn("Could not receive inquire command response code from the Shimmer");
			return;
		}

		std::cout << "Response to Inquiry: " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(responseCode) << std::endl;

		if (RESPONSECODE(responseCode) != RESPONSECODE::INQUIRY_RESPONSE) {
			ssi_wrn("Shimmer did not send correct response code after configuration request");
			//return;
		}

		//read header (next 8 bytes)
		unsigned char inquiryHeader[8];
		if (m_serial->ReadData(inquiryHeader, 8) != 8) {
			ssi_wrn("Configuration header could not be received from the Shimmer");
			return;
		}

		std::cout << "Shimmer3 inquiry header: \n";
		for (const auto& c : inquiryHeader) {
			std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c) << " ";
		}
		std::cout << std::endl;

		const size_t samplingRateIndex = 0;
		const size_t configSetupIndex = 2;
		const size_t nrOfChannelsIndex = 6;
		const size_t bufferSizeIndex = 7;

		//see how many channels there are
		uint8_t nrOfChannels = static_cast<uint8_t>(inquiryHeader[nrOfChannelsIndex]);
		std::vector<unsigned char> channelInfoBuffer(nrOfChannels);
		if (m_serial->ReadData(channelInfoBuffer.data(), channelInfoBuffer.size()) != channelInfoBuffer.size()) {
			ssi_wrn("Channel configuration information could not be received from the Shimmer");
			return;
		}

		std::cout << "Shimmer3 inquiry channelinfo: \n";
		for (const auto& c : channelInfoBuffer) {
			std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c) << " ";
		}
		std::cout << std::endl;

		//construct offsetmap and packetsize from the channelinfo
		size_t offset = 3; //offsets start at 3 because the shimmer always sends 3 bytes of timestamp info (at least for LogAndStream > v0.5.0)
		for (size_t i = 0; i < channelInfoBuffer.size(); i++) {
			auto sensorId = SENSORID(channelInfoBuffer[i]);
			m_sensorOffsetWithinPacketMap.emplace(std::make_pair(sensorId, offset));

			const auto& sensorInfo = sensorSampleInfoMap[sensorId];
			offset += sensorInfo.nrOfBytes;
		}
		m_packetSize = offset;

		//read out sampling rate
		uint16_t rawSamplingRate;
		uint16_t msb = static_cast<uint16_t>(inquiryHeader[samplingRateIndex + 1]) << 8;
		uint16_t lsb = static_cast<uint16_t>(inquiryHeader[samplingRateIndex]);
		rawSamplingRate = msb + lsb;
		m_samplingRate = 32768.0 / (rawSamplingRate * 1.0); //magic number taken from Shimmer3 C# API

		//read out how many datapackets will be sent as a batch
		m_nrOfPacketsSentAsBatch = static_cast<uint8_t>(inquiryHeader[bufferSizeIndex]);

		uint8_t* ptrToConfigBytes = reinterpret_cast<uint8_t*>(&inquiryHeader) + configSetupIndex;
		rawSensorConfigurationValues.parseValues(ptrToConfigBytes);
	}

	void Shimmer3LogAndStreamDevice::startStreaming() {
		if (m_state == SHIMMER_STATE::CONNECTED_NOTSTREAMING
			&& sendCommand(COMMANDCODE::START_STREAMING)
			&& waitForImmediateAck()) {
			m_state = SHIMMER_STATE::CONNECTED_STREAMING;
		}
	}

	void Shimmer3LogAndStreamDevice::stopStreaming() {
		if (m_state == SHIMMER_STATE::CONNECTED_STREAMING 
			&& sendCommand(COMMANDCODE::STOP_STREAMING)
			&& waitForImmediateAck()) {
			m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
		}
		else {
			ssi_wrn("Could not stop streaming for some reason o.O");
		}
	}

	std::unique_ptr<Shimmer3LogAndStreamDevice::DataPacket> Shimmer3LogAndStreamDevice::readNextPacket() {
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

	bool Shimmer3LogAndStreamDevice::sendCommand(const COMMANDCODE& cmd) const {
		if (m_serial) {
			unsigned char cmdBuffer[1] = { static_cast<unsigned char>(cmd) };
			return m_serial->WriteData(cmdBuffer, 1);
		}

		return false;
	}

	bool Shimmer3LogAndStreamDevice::waitForImmediateAck() const {
		return waitForResponseCode(RESPONSECODE::ACK);
	}

	bool Shimmer3LogAndStreamDevice::waitForNextDataPacket()
	{
		static const size_t connectivityCheckTimeout = 1000000000;
		size_t ctr = 0;
		while (true) {
			if (waitForResponseCode(RESPONSECODE::DATA_PACKET))
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

	bool Shimmer3LogAndStreamDevice::waitForResponseCode(const RESPONSECODE& code) const {
		unsigned char responseByte;
		if (m_serial->ReadData(&responseByte, 1) != 1) {
			ssi_wrn("Could not receive ack response from the Shimmer");
			return false;
		}

		if (RESPONSECODE(responseByte) != code) {
			ssi_wrn("Shimmer sent %d, when %d was expected", (int)responseByte, (int)code);
			return false;
		}

		return true;
	}
}
