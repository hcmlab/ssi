// Shimmer3GSRPlus.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 9/3/2015
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

#include "Shimmer3GSRplus.h"
#include "Serial.h"
#include <iostream>
#include <algorithm>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

static char ssi_log_name[] = "shimmer3gsr+";

std::map<Shimmer3GSRPlus::GSRRange, Shimmer3GSRPlus::GSRCalibration> Shimmer3GSRPlus::m_calibrationValues {
	{Shimmer3GSRPlus::GSRRange::OHMS_10K_TO_56K, {40.2f, 8.0f, 63.0f} },
	{Shimmer3GSRPlus::GSRRange::OHMS_56K_TO_220K, {287.0f, 63.0f, 220.0f} },
	{Shimmer3GSRPlus::GSRRange::OHMS_220K_TO_680K, {1000.0f, 220.0f, 680.0f} },
	{Shimmer3GSRPlus::GSRRange::OHMS_680K_TO_4700K, {3300.0f, 680.0f, 4700.0f} }
};

float Shimmer3GSRPlus::lowerLimitCalibration680K = 683.0f;

Shimmer3GSRPlus::Shimmer3GSRPlus (const ssi_char_t *file) 
	: m_ppgraw_provider (0),
	m_gsrraw_provider(0),
	_file (0),
	m_ppgraw_channel (0),
	m_gsrraw_channel (0),
	m_gsrcalibratedresistance_channel(0),
	m_gsrcalibratedresistance_provider(0),
	m_gsrcalibratedconductance_channel(0),
	m_gsrcalibratedconductance_provider(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
	
	getOptions()->setStartCMD(0);
	getOptions()->setStopCMD(0);
	getOptions()->setDeviceInstanceId(0);

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Shimmer3GSRPlus::~Shimmer3GSRPlus () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete m_ppgraw_channel;
}

bool Shimmer3GSRPlus::setProvider (const ssi_char_t *name, IProvider *provider) {
	if (strcmp (name, SSI_SHIMMER3_PPGRAW_PROVIDER_NAME) == 0) {
		setPPGRawProvider (provider);
		return true;
	} else if (strcmp(name, SSI_SHIMMER3_GSRRAW_PROVIDER_NAME) == 0) {
		setGSRRawProvider(provider);
		return true;
	} else if (strcmp(name, SSI_SHIMMER3_GSRCALIBRATEDRESISTANCE_PROVIDER_NAME) == 0) {
		setGSRResistanceProvider(provider);
		return true;
	} else if (strcmp(name, SSI_SHIMMER3_GSRCALIBRATEDCONDUCTANCE_PROVIDER_NAME) == 0) {
		setGSRConductanceProvider(provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void Shimmer3GSRPlus::setPPGRawProvider (IProvider *provider) {

	if (m_ppgraw_provider) {
		ssi_wrn ("gsr raw was already set");
	}

	m_ppgraw_provider = provider;
	if (m_ppgraw_provider) {		
		m_ppgraw_provider->init (getChannel (0));
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "ppg raw provider set");
	}
}

void Shimmer3GSRPlus::setGSRRawProvider(IProvider* provider) {

	if (m_gsrraw_provider) {
		ssi_wrn("gsr raw was already set");
	}

	m_gsrraw_provider = provider;
	if (m_gsrraw_provider) {
		m_gsrraw_provider->init(getChannel(1));
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "gsr raw provider set");
	}
}

void Shimmer3GSRPlus::setGSRResistanceProvider(IProvider* provider) {
	if (m_gsrcalibratedresistance_provider) {
		ssi_wrn("gsr calibrated resistance was already set");
	}

	m_gsrcalibratedresistance_provider = provider;
	if (m_gsrcalibratedresistance_provider) {
		m_gsrcalibratedresistance_provider->init(getChannel(2));
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "gsr resistance provider set");
	}
}

void Shimmer3GSRPlus::setGSRConductanceProvider(IProvider* provider) {
	if (m_gsrcalibratedconductance_provider) {
		ssi_wrn("gsr calibrated conductance was already set");
	}

	m_gsrcalibratedconductance_provider = provider;
	if (m_gsrcalibratedconductance_provider) {
		m_gsrcalibratedconductance_provider->init(getChannel(3));
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "gsr conductance provider set");
	}
}

bool Shimmer3GSRPlus::connect () {

	_device = std::make_unique<shimmer3::LogAndStreamDevice>(_options.port, _options.baud);

	if (!_device->connect()) {
		return false;
	}

	// set thread name
	Thread::setName (getName ());

	return true;
}

void Shimmer3GSRPlus::run () {
	if (_device && _device->isConnected() && !_device->isStreaming()) {
		std::cout << "Start streaming!" << std::endl;
		_device->startStreaming();
	}

	auto packet = _device->readNextPacket();

	if (packet) {
		processPPGValue(packet);
		processGSRValue(packet);
	}
	else {
		ssi_wrn("Did not get a packet from the Shimmer Device");
	}
}

void Shimmer3GSRPlus::processPPGValue(const std::unique_ptr<shimmer3::LogAndStreamDevice::DataPacket>& packet) {
	try
	{
		float rawPPGValue = packet->get(shimmer3::SENSORID::INTERNAL_ADC_A13) * 1.0f;

		if (m_ppgraw_provider) {
			m_ppgraw_provider->provide(ssi_pcast(char, &rawPPGValue), 1);
		}
	}
	catch (const std::exception&)
	{
		ssi_wrn("The shimmer does not seem to be configured to provide ppg values! Please make sure you are using the correct shimmer board and configuration!");
	}
}

void Shimmer3GSRPlus::processGSRValue(const std::unique_ptr<shimmer3::LogAndStreamDevice::DataPacket>& packet) {
	try
	{
		long gsrDataBytes = packet->get(shimmer3::SENSORID::GSR);

		float uncalibratedGSRValue = extractGSRDataValueFromRawBytes(gsrDataBytes);

		float calibratedGSRResistanceKOhms = convertGSRRawBytesToKOhms(gsrDataBytes, GSRRange(_device->rawSensorConfigurationValues.GSRRange));
		float calibratedGSRConductanceValue = (1.0f / calibratedGSRResistanceKOhms) * 1000.0f;

		if (m_gsrraw_provider) {
			m_gsrraw_provider->provide(ssi_pcast(char, &uncalibratedGSRValue), 1);
		}

		if (m_gsrcalibratedresistance_provider) {
			m_gsrcalibratedresistance_provider->provide(ssi_pcast(char, &calibratedGSRResistanceKOhms), 1);
		}

		if (m_gsrcalibratedconductance_provider) {
			m_gsrcalibratedconductance_provider->provide(ssi_pcast(char, &calibratedGSRConductanceValue), 1);
		}
	}
	catch (const std::exception& e)
	{
		ssi_wrn("Exception: %s", e.what());
		ssi_wrn("The shimmer does not seem to be configured to provide gsr values! Please make sure you are using the correct shimmer board and configuration!");
	}
}

bool Shimmer3GSRPlus::disconnect()
{
	if (!_device) {
		ssi_wrn("sensor not connected");
		return false;
	}

	if (!_device->isConnected()) {
		ssi_wrn("sensor not connected");
		_device.reset(nullptr);
		return false;
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "sending stop message ...");
	_device->stopStreaming();

	ssi_msg(SSI_LOG_LEVEL_BASIC, "try to disconnect sensor ...");

	_device.reset(nullptr);

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

	return true;
}

//Converts the uncalibrated gsr value to a value in KOhms; clamped to the min/max of the passed range
//NOTE: replica of the function with the same name from the C# Shimmer API (ShimmerBluetooth.cs, starting at line 6220 @Commit 58992aea) 
float Shimmer3GSRPlus::calibrateGsrDataToResistanceFromAmplifierEq(float gsrUncalibratedData, GSRRange range)
{
	auto& calibration = m_calibrationValues[range];

	float vRefP = 3.0f;
	float gain = 1.0f;
	float volts = (gsrUncalibratedData * (((vRefP * 1000) / gain) / 4095)) / 1000.0f;
	float calibratedGSRResistance = calibration.referenceResistorKOhms / ((volts / 0.5f) - 1.0f);
	
	return std::max<float>(calibration.resistanceMinKOhms, std::min<float>(calibration.resistanceMaxKOhms, calibratedGSRResistance)); //clamp
}

Shimmer3GSRPlus::GSRRange Shimmer3GSRPlus::extractRangeFromBytesWhenAutoRange(long rawGSRBytes) {
	GSRRange range = GSRRange((49152 & (int)rawGSRBytes) >> 14); //bit mask and shift to only get the two bits at position 14 and 15

	if (!(range == GSRRange::OHMS_10K_TO_56K || range == GSRRange::OHMS_56K_TO_220K || range == GSRRange::OHMS_220K_TO_680K || range == GSRRange::OHMS_680K_TO_4700K)) {
		throw std::exception{ "GSRPlus Auto range did not resolve to an allowed value!" };
	}

	return range;
}

float Shimmer3GSRPlus::extractGSRDataValueFromRawBytes(long rawGSRBytes) {
	return static_cast<float>(rawGSRBytes & 4095); //only use lower 12 bits!
}

//replica of the relevant section from the "BuildMsg" function in the C# Shimmer API
//(ShimmerBluetooth.cs, lines 3096 - 3158, @Commit 58992aea)
float Shimmer3GSRPlus::convertGSRRawBytesToKOhms(long rawGSRBytes, GSRRange range)
{
	//handle auto range
	if (range == GSRRange::AUTO) {
		range = extractRangeFromBytesWhenAutoRange(rawGSRBytes);
	}

	float rawGSRValue = extractGSRDataValueFromRawBytes(rawGSRBytes);

	//handle special lower limit for gsrrange 680K
	if (range == GSRRange::OHMS_680K_TO_4700K && rawGSRValue < lowerLimitCalibration680K) {
		rawGSRValue = lowerLimitCalibration680K;
	}

	return calibrateGsrDataToResistanceFromAmplifierEq(rawGSRValue, range);
}

}
