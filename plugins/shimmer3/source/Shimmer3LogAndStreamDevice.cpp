#include <vector>

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

	bool Shimmer3LogAndStreamDevice::checkComPort(const ssi_char_t* portNameStr) {
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

	bool Shimmer3LogAndStreamDevice::connect() {

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
		std::map<int, unsigned long>::iterator it;
		it = supportedBaudRates.find(m_baudRate);
		if (it == supportedBaudRates.end()) {
			ssi_wrn("baud rate %u not supported, using default baud rate", m_baudRate);
			m_serial = std::make_unique<Serial>(comPortName);
		}
		else {
			m_serial = std::make_unique<Serial>(comPortName, it->second);
		}

		if (!m_serial->IsConnected()) {
			ssi_err("could not connect serial sensor at port=%s", comPortName);
			return false;
		}

		m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

		return true;
	}

	bool Shimmer3LogAndStreamDevice::isConnected() {
		return m_serial && m_serial->IsConnected();
	}

	void Shimmer3LogAndStreamDevice::startStreaming() {
		if (m_state == SHIMMER_STATE::CONNECTED_NOTSTREAMING && sendCommand(COMMANDCODE::START_SDBT)) {
			m_state = SHIMMER_STATE::CONNECTED_STREAMING;
		}
	}

	void Shimmer3LogAndStreamDevice::stopStreaming() {
		if (m_state == SHIMMER_STATE::CONNECTED_STREAMING && sendCommand(COMMANDCODE::STOP_SDBT)) {
			m_state = SHIMMER_STATE::CONNECTED_NOTSTREAMING;
		}
	}

	bool Shimmer3LogAndStreamDevice::sendCommand(const COMMANDCODE& cmd) {
		if (m_serial) {
			char cmdBuffer[1] = { static_cast<char>(cmd) };
			return m_serial->WriteData(cmdBuffer, 1);
		}

		return false;
	}
}
