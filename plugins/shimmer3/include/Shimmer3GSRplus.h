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
#include "ShimmerClosedLibraryAlgoFactory.h"

#define SSI_SHIMMER3_PPGRAW_PROVIDER_NAME "shimmer3ppgraw"
#define SSI_SHIMMER3_PPGCALIBRATED_PROVIDER_NAME "shimmer3ppgcalibrated"
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

	class PPGCalibratedChannel: public IChannel {

		friend class Shimmer3GSRPlus;

	public:

		PPGCalibratedChannel(ssi_size_t dim, ssi_time_t sr) {
			ssi_stream_init(stream, 0, dim, sizeof(ssi_real_t), SSI_REAL, sr);
		}
		~PPGCalibratedChannel() {
			ssi_stream_destroy(stream);
		}

		const ssi_char_t* getName() { return SSI_SHIMMER3_PPGCALIBRATED_PROVIDER_NAME; };
		const ssi_char_t* getInfo() { return "ppg value in mV"; };
		ssi_stream_t getStream() { return stream; };

	protected:
		ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:
		Options ()
			: port(1), baud(9600), sr(50), size(0.2) {

			addOption ("port", &port, 1, SSI_SIZE, "port number");
			addOption ("baud", &baud, 1, SSI_SIZE, "baud rate");									
			addOption ("sr", &sr, 1, SSI_SIZE, "sampling rates (hz)");
			addOption ("size", &size, 1, SSI_DOUBLE, "block size in seconds");
		};

		ssi_size_t port;
		ssi_size_t baud;
		ssi_size_t sr;
		ssi_time_t size;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Shimmer3GSR+"; };
	static IObject *Create (const ssi_char_t *file) { return new Shimmer3GSRPlus(file); };
	~Shimmer3GSRPlus();
	Shimmer3GSRPlus::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Shimmer3 GSR+ Sensor"; };

	ssi_size_t getChannelSize () { return 5; };

	IChannel *getChannel (ssi_size_t index) {
		switch (index)
		{
		case 0:
			if (!m_ppgraw_channel) m_ppgraw_channel = new PPGRawChannel(1, _options.sr);
			return m_ppgraw_channel;
		case 1:
			if (!m_gsrraw_channel) m_gsrraw_channel = new GSRRawChannel(1, _options.sr);
			return m_gsrraw_channel;
		case 2:
			if (!m_gsrcalibratedresistance_channel) m_gsrcalibratedresistance_channel = new GSRCalibratedResistanceChannel(1, _options.sr);
			return m_gsrcalibratedresistance_channel;
		case 3:
			if (!m_gsrcalibratedconductance_channel) m_gsrcalibratedconductance_channel = new GSRCalibratedConductanceChannel(1, _options.sr);
			return m_gsrcalibratedconductance_channel;
		case 4:
			if (!m_ppgcalibrated_channel) m_ppgcalibrated_channel = new PPGCalibratedChannel(1, _options.sr);
			return m_ppgcalibrated_channel;
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
	float calibratedPPGValue(long rawPPGValue);

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

	PPGCalibratedChannel* m_ppgcalibrated_channel;
	void setPPGCalibratedProvider(IProvider* provider);
	IProvider* m_ppgcalibrated_provider;

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

