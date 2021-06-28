// Shimmer3GSRPlus.h
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

#ifndef SSI_GERNERICSERIAL_H
#define	SSI_GERNERICSERIAL_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"

#include <map>
#include <sstream>
#include <memory>

#include <Windows.h>

#include "Shimmer3LogAndStreamDevice.h"

#define SSI_SHIMMER3_PPGRAW_PROVIDER_NAME "shimmer3ppgraw"
#define SSI_SHIMMER3_GSRRAW_PROVIDER_NAME "shimmer3gsrraw"
#define SSI_SHIMMER3_GSRCALIBRATEDRESISTANCE_PROVIDER_NAME "shimmer3gsrresistance"
#define SSI_SHIMMER3_GSRCALIBRATEDCONDUCTANCE_PROVIDER_NAME "shimmer3gsrconductance"

namespace ssi {

class Shimmer3GSRPlus : public ISensor, public Thread {

public:

	class PPGRawChannel : public IChannel {

		friend class Shimmer3GSRPlus;

	public:

		PPGRawChannel(ssi_size_t dim, ssi_time_t sr) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, sr);
		}
		~PPGRawChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t* getName() { return SSI_SHIMMER3_PPGRAW_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "ppg raw data from the shimmer gsr+ board."; };
		ssi_stream_t getStream() { return stream; };

	protected:
		ssi_stream_t stream;
	};

	class GSRRawChannel : public IChannel {

		friend class Shimmer3GSRPlus;

	public:

		GSRRawChannel(ssi_size_t dim, ssi_time_t sr) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, sr);
		}
		~GSRRawChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t* getName() { return SSI_SHIMMER3_GSRRAW_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "gsr raw data from the shimmer gsr+ board."; };
		ssi_stream_t getStream() { return stream; };

	protected:
		ssi_stream_t stream;
	};

	class GSRCalibratedResistanceChannel : public IChannel {

		friend class Shimmer3GSRPlus;

	public:

		GSRCalibratedResistanceChannel(ssi_size_t dim, ssi_time_t sr) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, sr);
		}
		~GSRCalibratedResistanceChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t* getName() { return SSI_SHIMMER3_GSRCALIBRATEDRESISTANCE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "calibrated gsr resistance data in kOhms from the shimmer gsr+ board."; };
		ssi_stream_t getStream() { return stream; };

	protected:
		ssi_stream_t stream;
	};

	class GSRCalibratedConductanceChannel : public IChannel {

		friend class Shimmer3GSRPlus;

	public:

		GSRCalibratedConductanceChannel(ssi_size_t dim, ssi_time_t sr) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, sr);
		}
		~GSRCalibratedConductanceChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t* getName() { return SSI_SHIMMER3_GSRCALIBRATEDCONDUCTANCE_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "calibrated gsr conductance data in microSiemens from the shimmer gsr+ board."; };
		ssi_stream_t getStream() { return stream; };

	protected:
		ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:
		Options ()
			: port (1), baud (9600), sr (50), dim(3), separator(','), size(0.2), skipLinesAfterStart(0), skipLinesAfterStartCMD(0), useId(false), showDebugSR(false) {

			addOption ("port", &port, 1, SSI_SIZE, "port number");
			addOption ("useId", &useId, 1, SSI_BOOL, "use device instance id instead of port number");
			addOption ("deviceInstanceId", &deviceInstanceId, SSI_MAX_CHAR, SSI_CHAR, "device instance id");


			addOption ("baud", &baud, 1, SSI_SIZE, "baud rate");									
			addOption ("separator", &separator, 1, SSI_CHAR, "separator used to split input data");
			addOption ("sr", &sr, 1, SSI_SIZE, "sampling rates (hz)");
			addOption ("dim", &dim, 1, SSI_SIZE, "value count");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");

			addOption ("startCMD", &startCMD, SSI_MAX_CHAR, SSI_CHAR, "command to be sent after first connection with controller (leave empty to send no command)");
			addOption ("skipLinesAfterStart", &skipLinesAfterStart, 1, SSI_INT, "skip lines from controller after connection" );
			addOption ("skipLinesAfterStartCMD", &skipLinesAfterStartCMD, 1, SSI_INT, "skip lines from controller after start command" );
			addOption ("stopCMD", &stopCMD, SSI_MAX_CHAR, SSI_CHAR, "command to be sent to controller during pipeline shutdown (leave empty to send no command)");

			addOption("showDebugSR", &showDebugSR, 1, SSI_BOOL, "show mean sample rate of input data");
		};

		void setStartCMD (const ssi_char_t *startCMD) {
			this->startCMD[0] = '\0';
			if (startCMD) {
				ssi_strcpy (this->startCMD, startCMD);
			}
		};
		
		void setStopCMD (const ssi_char_t *stopCMD) {
			this->stopCMD[0] = '\0';
			if (stopCMD) {
				ssi_strcpy (this->stopCMD, stopCMD);
			}
		};

		void setDeviceInstanceId (const ssi_char_t *deviceInstanceId) {
			this->deviceInstanceId[0] = '\0';
			if (deviceInstanceId) {
				ssi_strcpy (this->deviceInstanceId, deviceInstanceId);
			}
		};

		ssi_size_t port;
		ssi_size_t baud;
		ssi_char_t separator;
		ssi_size_t sr;
		ssi_size_t dim;
		ssi_time_t size;

		ssi_char_t startCMD[SSI_MAX_CHAR];
		ssi_char_t stopCMD[SSI_MAX_CHAR];
		ssi_size_t skipLinesAfterStart;
		ssi_size_t skipLinesAfterStartCMD;

		bool useId;
		ssi_char_t deviceInstanceId[SSI_MAX_CHAR];

		bool showDebugSR;

	};

public:

	static const ssi_char_t *GetCreateName () { return "Shimmer3GSR+"; };
	static IObject *Create (const ssi_char_t *file) { return new Shimmer3GSRPlus(file); };
	~Shimmer3GSRPlus();
	Shimmer3GSRPlus::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Shimmer3 GSR+ Sensor"; };

	ssi_size_t getChannelSize () { return 2; };

	IChannel *getChannel (ssi_size_t index) {
		switch (index)
		{
		case 0:
			if (!m_ppgraw_channel) m_ppgraw_channel = new PPGRawChannel(_options.dim, _options.sr);
			return m_ppgraw_channel;
		case 1:
			if (!m_gsrraw_channel) m_gsrraw_channel = new GSRRawChannel(_options.dim, _options.sr);
			return m_gsrraw_channel;
		case 2:
			if (!m_gsrcalibratedresistance_channel) m_gsrcalibratedresistance_channel = new GSRCalibratedResistanceChannel(_options.dim, _options.sr);
			return m_gsrcalibratedresistance_channel;
		case 3:
			if (!m_gsrcalibratedconductance_channel) m_gsrcalibratedconductance_channel = new GSRCalibratedConductanceChannel(_options.dim, _options.sr);
			return m_gsrcalibratedconductance_channel;
		default:
			ssi_wrn("Channel index %ud not supported", index);
			return 0;
		}
	};

	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

public:
	enum class GSRRange {
		OHMS_10K_TO_56K = 0,
		OHMS_56K_TO_220K = 1,
		OHMS_220K_TO_680K = 2,
		OHMS_680K_TO_4700K = 3,
		AUTO = 4
	};

	struct GSRCalibration {
		float referenceResistorKOhms;
		float resistanceMinKOhms;
		float resistanceMaxKOhms;
	};

private:
	float convertGSRRawBytesToKOhms(long rawGSRValue, GSRRange range);
	float calibrateGsrDataToResistanceFromAmplifierEq(float gsrUncalibratedData, GSRRange range);

	GSRRange extractRangeFromBytesWhenAutoRange(long rawGSRBytes);
	float extractGSRDataValueFromRawBytes(long rawGSRBytes);

	void processPPGValue(const std::unique_ptr<shimmer3::LogAndStreamDevice::DataPacket>& packet);
	void processGSRValue(const std::unique_ptr<shimmer3::LogAndStreamDevice::DataPacket>& packet);

protected:
	Shimmer3GSRPlus(const ssi_char_t *file = 0);
	Shimmer3GSRPlus::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;

	PPGRawChannel *m_ppgraw_channel;
	void setPPGRawProvider(IProvider *provider);
	IProvider *m_ppgraw_provider;

	GSRRawChannel* m_gsrraw_channel;
	void setGSRRawProvider(IProvider* provider);
	IProvider* m_gsrraw_provider;

	GSRCalibratedResistanceChannel* m_gsrcalibratedresistance_channel;
	void setGSRResistanceProvider(IProvider* provider);
	IProvider* m_gsrcalibratedresistance_provider;

	GSRCalibratedConductanceChannel* m_gsrcalibratedconductance_channel;
	void setGSRConductanceProvider(IProvider* provider);
	IProvider* m_gsrcalibratedconductance_provider;

	std::unique_ptr<shimmer3::LogAndStreamDevice> _device;

	static std::map<GSRRange, GSRCalibration> m_calibrationValues;

	static float lowerLimitCalibration680K;
};

}

#endif

