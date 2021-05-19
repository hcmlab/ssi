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

	enum class COMMANDCODE {
        START_SDBT = 0x70,
        STOP_SDBT = 0x97,
        GET_STATUS = 0x72
	};

	enum class RESPONSECODE {
        STATUS_RESPONSE = 0x71
	};

	/// <summary>
	/// Wrapper for the basic functionality of a Shimmer3 Device programmed with the LogAndStream Firmware
	/// See: https://www.shimmersensing.com/images/uploads/docs/LogAndStream_for_Shimmer3_Firmware_User_Manual_rev0.11a.pdf
	/// </summary>
	class Shimmer3LogAndStreamDevice {
	private:
		enum class SHIMMER_STATE {
			DISCONNECTED,
			CONNECTED_NOTSTREAMING,
			CONNECTED_STREAMING
		};

	public:
		Shimmer3LogAndStreamDevice() : m_comPortNr(0), m_baudRate(0), m_state(SHIMMER_STATE::DISCONNECTED) {};
		Shimmer3LogAndStreamDevice(ssi_size_t comPortNr, ssi_size_t baudRate = 115200): m_comPortNr(comPortNr), m_baudRate(baudRate), m_state(SHIMMER_STATE::DISCONNECTED) {};
		~Shimmer3LogAndStreamDevice() {};

		bool connect();
		bool isConnected();

		void startStreaming();
		void stopStreaming();

	private:
		bool checkComPort(const ssi_char_t* portNameStr);

        bool sendCommand(const COMMANDCODE& cmd);

		ssi_size_t m_comPortNr;
		ssi_size_t m_baudRate;

		SHIMMER_STATE m_state;

		std::unique_ptr<Serial> m_serial;

		static std::map<int, unsigned long> supportedBaudRates;
	};
}

#endif /* SSI_SHIMMER3_LOGANDSTREAMDEVICE_H */
