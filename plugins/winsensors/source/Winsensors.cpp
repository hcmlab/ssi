// MyWinsensors.cpp
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 25/3/2015
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

#include "Winsensors.h"

#include <windows.h>
#include <InitGuid.h>
#include <SensorsApi.h>
#include <Sensors.h>

#include <cguid.h>


#define MEAN_GRAVITY 9.80665


static ISensor* sAccelerometer;
static ISensor* sLight; //ambient light
static ISensor* sGyro;  //gyroscope
static ISensor* sIncl;  //inclinometer
static ISensor* sComp;  //compass

enum SensorType {
	SENSOR_UNKNOWN = -1,
	SENSOR_ORIENTATION,
	SENSOR_ACCELERATION,
	SENSOR_PROXIMITY,
	SENSOR_LINEAR_ACCELERATION,
	SENSOR_GYROSCOPE,
	SENSOR_LIGHT,
	SENSOR_INCLINOMETER,
	SENSOR_COMPASS,
	NUM_SENSOR_TYPE
};

class SensorEvent : public ISensorEvents{

	ssi::Winsensors *winSensors;


public:
	SensorEvent(ssi::Winsensors *winSensors) : mCount(0), winSensors(winSensors) {
	}

	// IUnknown interface

	STDMETHODIMP_(ULONG) AddRef() {
		return InterlockedIncrement(&mCount);
	}

	STDMETHODIMP_(ULONG) Release() {
		ULONG count = InterlockedDecrement(&mCount);
		if (!count) {
			delete this;
			return 0;
		}
		return count;
	}

	STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
		if (iid == IID_IUnknown) {
			*ppv = static_cast<IUnknown*>(this);
		}
		else if (iid == IID_ISensorEvents) {
			*ppv = static_cast<ISensorEvents*>(this);
		}
		else {
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
	}

	// ISensorEvents interface

	STDMETHODIMP OnEvent(ISensor *aSensor, REFGUID aId, IPortableDeviceValues *aData) {
		return S_OK;
	}

	STDMETHODIMP OnLeave(REFSENSOR_ID aId) {
		return S_OK;
	}

	STDMETHODIMP OnStateChanged(ISensor *aSensor, SensorState state) {
		return S_OK;
	}

	STDMETHODIMP OnDataUpdated(ISensor *aSensor, ISensorDataReport *aReport) {

		PROPVARIANT v;
		HRESULT hr;

		SENSOR_ID sensorID = GUID_NULL;
		aSensor->GetID(&sensorID);

		SENSOR_ID accID = GUID_NULL;
		if (sAccelerometer) sAccelerometer->GetID(&accID);

		SENSOR_ID gyroID = GUID_NULL;
		if (sGyro) sGyro->GetID(&gyroID);

		SENSOR_ID inclID = GUID_NULL;
		if (sIncl) sIncl->GetID(&inclID);

		SENSOR_ID compID = GUID_NULL;
		if (sComp) sComp->GetID(&compID);

		SENSOR_ID lightID = GUID_NULL;
		if (sLight) sLight->GetID(&lightID);

		if (accID == sensorID) {

			float x, y, z;

			// X-axis acceleration in g's
			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &v);
			if (FAILED(hr)) {
				return hr;
			}
			x = float(-v.dblVal * MEAN_GRAVITY);

			// Y-axis acceleration in g's
			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &v);
			if (FAILED(hr)) {
				return hr;
			}
			y = float(-v.dblVal * MEAN_GRAVITY);

			// Z-axis acceleration in g's
			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &v);
			if (FAILED(hr)) {
				return hr;
			}
			z = float(-v.dblVal * MEAN_GRAVITY);

			ssi_real_t *ptr = ssi_pcast(ssi_real_t, winSensors->_event_acc.ptr);
			ptr[0] = x;
			ptr[1] = y;
			ptr[2] = z;

			winSensors->listenerUpdate( winSensors->_event_acc);


			//winSensors->accelerometerUpdate(x, y, z);
		}
		else if (gyroID == sensorID) {
			float x, y, z;

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &v);
			if (FAILED(hr)) {
				return hr;
			}
			x = float(v.dblVal);

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &v);
			if (FAILED(hr)) {
				return hr;
			}
			y = float(v.dblVal);;

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &v);
			if (FAILED(hr)) {
				return hr;
			}
			z = float(v.dblVal);


			ssi_real_t *ptr = ssi_pcast(ssi_real_t, winSensors->_event_gyr.ptr);
			ptr[0] = x;
			ptr[1] = y;
			ptr[2] = z;

			winSensors->listenerUpdate(winSensors->_event_gyr);

		}
		else if (inclID == sensorID) {
			float x, y, z;

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &v);
			if (FAILED(hr)) {
				return hr;
			}
			x = (float)V_R4(&v);

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &v);
			if (FAILED(hr)) {
				return hr;
			}
			y = (float)V_R4(&v);

			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &v);
			if (FAILED(hr)) {
				return hr;
			}
			z = (float)V_R4(&v);


			ssi_real_t *ptr = ssi_pcast(ssi_real_t, winSensors->_event_incl.ptr);
			ptr[0] = x;
			ptr[1] = y;
			ptr[2] = z;

			winSensors->listenerUpdate(winSensors->_event_incl);
		}
		else if (compID == sensorID) {
			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_MAGNETIC_HEADING_COMPENSATED_MAGNETIC_NORTH_DEGREES, &v);
			if (FAILED(hr)) {
				return hr;
			}
			float val = (float)V_R8(&v);

			ssi_real_t *ptr = ssi_pcast(ssi_real_t, winSensors->_event_comp.ptr);
			ptr[0] = val;

			winSensors->listenerUpdate(winSensors->_event_comp);
		}
		else if (lightID == sensorID) {
			hr = aReport->GetSensorValue(SENSOR_DATA_TYPE_LIGHT_LEVEL_LUX, &v);
			if (FAILED(hr)) {
				return hr;
			}

			ssi_real_t *ptr = ssi_pcast(ssi_real_t, winSensors->_event_light.ptr);
			ptr[0] = V_R4(&v);

			winSensors->listenerUpdate(winSensors->_event_light);
		}

		return S_OK;
	}

private:
	ULONG mCount;
};

void
EnableSensorNotifications(SensorType aSensor, ssi::Winsensors *winSensors)
{
	ISensorManager* manager;
	if (FAILED(CoCreateInstance(CLSID_SensorManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager)))) {
		return;
	}

	ISensorCollection* collection;

	if (aSensor == SENSOR_ACCELERATION) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "enabling accelerometer ...");

		if (FAILED(manager->GetSensorsByType(SENSOR_TYPE_ACCELEROMETER_3D, &collection))) {
			ssi_wrn("enabling accelerometer: failed!");
			return;
		}
	}
	else if (aSensor == SENSOR_GYROSCOPE) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "enabling gyroscope ...");

		if (FAILED(manager->GetSensorsByType(SENSOR_TYPE_GYROMETER_3D, &collection))) {
			ssi_wrn("enabling gyroscope: failed!");
			return;
		}
	}
	else if (aSensor == SENSOR_LIGHT) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "enabling light sensor ...");

		if (FAILED(manager->GetSensorsByType(SENSOR_TYPE_AMBIENT_LIGHT, &collection))) {
			ssi_wrn("enabling light sensor: failed!");
			return;
		}
	}
	else if (aSensor == SENSOR_INCLINOMETER) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "enabling inclinometer ...");
		if (FAILED(manager->GetSensorsByType(SENSOR_TYPE_INCLINOMETER_3D, &collection))) {
			ssi_wrn("enabling inclinometer: failed!");
			return;
		}
	}
	else if (aSensor == SENSOR_COMPASS) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "enabling compass ...");
		if (FAILED(manager->GetSensorsByType(SENSOR_TYPE_COMPASS_3D, &collection))) {
			ssi_wrn("enabling compass: failed!");
			return;
		}
	}

	ULONG count = 0;
	collection->GetCount(&count);
	if (!count) {
		return;
	}

	ISensor* sensor;
	collection->GetAt(0, &sensor);
	if (!sensor) {
		return;
	}

	// Set report interval to 100ms if possible.
	// Default value depends on drivers.
	IPortableDeviceValues* values;
	if (SUCCEEDED(CoCreateInstance(CLSID_PortableDeviceValues, nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&values)))) {
		if (SUCCEEDED(values->SetUnsignedIntegerValue(
			SENSOR_PROPERTY_CURRENT_REPORT_INTERVAL,
			winSensors->getOptions()->pollMs))) {
			IPortableDeviceValues* returns;
			sensor->SetProperties(values, &returns);
		}
	}

	SensorEvent* event = new SensorEvent(winSensors);
	ISensorEvents* sensorEvents;
	if (FAILED(event->QueryInterface(IID_ISensorEvents, ((void**)&sensorEvents)))) {
		return;
	}

	if (FAILED(sensor->SetEventSink(sensorEvents))) {
		return;
	}

	if		(aSensor == SENSOR_ACCELERATION)  sAccelerometer = sensor;
	else if (aSensor == SENSOR_GYROSCOPE)     sGyro = sensor;
	else if (aSensor == SENSOR_LIGHT)         sLight = sensor;
	else if (aSensor == SENSOR_INCLINOMETER)  sIncl = sensor;
	else if (aSensor == SENSOR_COMPASS)       sComp = sensor;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "sensor enabled.");
}

void
DisableSensorNotifications(SensorType aSensor)
{
	if (aSensor == SENSOR_ACCELERATION && sAccelerometer) {
		sAccelerometer->SetEventSink(nullptr);
		sAccelerometer = nullptr;
	}
	else if (aSensor == SENSOR_LIGHT && sLight) {
		sLight->SetEventSink(nullptr);
		sLight = nullptr;
	}
	else if (aSensor == SENSOR_GYROSCOPE && sGyro) {
		sGyro->SetEventSink(nullptr);
		sGyro = nullptr;
	}
	else if (aSensor == SENSOR_INCLINOMETER && sIncl) {
		sIncl->SetEventSink(nullptr);
		sIncl = nullptr;
	}
	else if (aSensor == SENSOR_COMPASS && sComp) {
		sComp->SetEventSink(nullptr);
		sComp = nullptr;
	}
}



namespace ssi {

	char Winsensors::ssi_log_name[] = "winsensors";

	Winsensors::Winsensors(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
		
		if (_options.activeSensors[SENSOR_INDEX_ACC]) {
			ssi_event_init(_event_acc, SSI_ETYPE_TUPLE);
			ssi_event_adjust(_event_acc, 3 * sizeof(ssi_real_t));
		}

		if (_options.activeSensors[SENSOR_INDEX_GYR]) { 
			ssi_event_init(_event_gyr, SSI_ETYPE_TUPLE); 
			ssi_event_adjust(_event_gyr, 3 * sizeof(ssi_real_t));
		}

		if (_options.activeSensors[SENSOR_INDEX_LIG]) { 
			ssi_event_init(_event_light, SSI_ETYPE_TUPLE); 
			ssi_event_adjust(_event_light, 1 * sizeof(ssi_real_t));
		}

		if (_options.activeSensors[SENSOR_INDEX_INC]) {
			ssi_event_init(_event_incl, SSI_ETYPE_TUPLE); 
			ssi_event_adjust(_event_incl, 3 * sizeof(ssi_real_t));
		}

		if (_options.activeSensors[SENSOR_INDEX_COM]) { 
			ssi_event_init(_event_comp, SSI_ETYPE_TUPLE); 
			ssi_event_adjust(_event_comp, 1 * sizeof(ssi_real_t));
		}

		::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	}


	Winsensors::~Winsensors() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}

		if (_options.activeSensors[SENSOR_INDEX_ACC])	DisableSensorNotifications(SENSOR_ACCELERATION);
		if (_options.activeSensors[SENSOR_INDEX_GYR])	DisableSensorNotifications(SENSOR_GYROSCOPE);
		if (_options.activeSensors[SENSOR_INDEX_LIG])	DisableSensorNotifications(SENSOR_LIGHT);
		if (_options.activeSensors[SENSOR_INDEX_INC])	DisableSensorNotifications(SENSOR_INCLINOMETER);
		if (_options.activeSensors[SENSOR_INDEX_COM])	DisableSensorNotifications(SENSOR_COMPASS);

		if (_options.activeSensors[SENSOR_INDEX_ACC])	ssi_event_destroy(_event_acc);
		if (_options.activeSensors[SENSOR_INDEX_GYR])	ssi_event_destroy(_event_gyr);
		if (_options.activeSensors[SENSOR_INDEX_LIG])	ssi_event_destroy(_event_light);
		if (_options.activeSensors[SENSOR_INDEX_INC])	ssi_event_destroy(_event_incl);
		if (_options.activeSensors[SENSOR_INDEX_COM])	ssi_event_destroy(_event_comp);

		::CoUninitialize(); // Uninitializes COM
	}


	bool Winsensors::setEventListener(IEventListener *listener) {
		_elistener = listener;

		ssi_size_t sender_id = Factory::AddString(_options.sname);

		std::stringstream str;

		if (sender_id == SSI_FACTORY_STRINGS_INVALID_ID)
			return false;


		if (_options.activeSensors[SENSOR_INDEX_ACC]) {
			_event_acc.sender_id = sender_id;
			_event_acc.event_id = Factory::AddString("accelerometer");

			if (_event_acc.event_id == SSI_FACTORY_STRINGS_INVALID_ID)
				return false;

			str << "accelerometer" << ",";
			_event_acc.prob = 1.0;
		}

		if (_options.activeSensors[SENSOR_INDEX_GYR]) {
			_event_gyr.sender_id = sender_id;
			_event_gyr.event_id = Factory::AddString("gyroscope");

			if (_event_gyr.event_id == SSI_FACTORY_STRINGS_INVALID_ID)
				return false;

			str << "gyroscope" << ",";
			_event_gyr.prob = 1.0;
		}

		if (_options.activeSensors[SENSOR_INDEX_LIG]) {
			_event_light.sender_id = sender_id;
			_event_light.event_id = Factory::AddString("light");

			if (_event_light.event_id == SSI_FACTORY_STRINGS_INVALID_ID)
				return false;

			str << "light" << ",";
			_event_light.prob = 1.0;
		}

		if (_options.activeSensors[SENSOR_INDEX_INC]) {
			_event_incl.sender_id = sender_id;
			_event_incl.event_id = Factory::AddString("inclinometer");

			if (_event_incl.event_id == SSI_FACTORY_STRINGS_INVALID_ID)
				return false;

			str << "inclinometer" << ",";
			_event_incl.prob = 1.0;
		}

		if (_options.activeSensors[SENSOR_INDEX_COM]) {
			_event_comp.sender_id = sender_id;
			_event_comp.event_id = Factory::AddString("compass");

			if (_event_comp.event_id == SSI_FACTORY_STRINGS_INVALID_ID)
				return false;

			str << "compass" << ",";
			_event_comp.prob = 1.0;
		}


		_event_address.setSender(_options.sname);
		_event_address.setEvents(str.str().c_str());


		if (_options.activeSensors[SENSOR_INDEX_ACC])	EnableSensorNotifications(SENSOR_ACCELERATION, this);
		if (_options.activeSensors[SENSOR_INDEX_GYR])	EnableSensorNotifications(SENSOR_GYROSCOPE, this);
		if (_options.activeSensors[SENSOR_INDEX_LIG])	EnableSensorNotifications(SENSOR_LIGHT, this);
		if (_options.activeSensors[SENSOR_INDEX_INC])	EnableSensorNotifications(SENSOR_INCLINOMETER, this);
		if (_options.activeSensors[SENSOR_INDEX_COM])	EnableSensorNotifications(SENSOR_COMPASS, this);

		return true;
	}


	void Winsensors::listenerUpdate(ssi_event_t _event) {
			Lock lock(listenerMutex);

			_elistener->update(_event);
	}

}
