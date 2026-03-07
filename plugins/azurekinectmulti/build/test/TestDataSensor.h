/**
* Sensor that provides a configurable amount of meaningless data. Used to stress test pipelines (and specifically networking)
*/

#pragma once

#ifndef SSI_GARBAGEDATA_SENSOR_H
#define	SSI_GARBAGEDATA_SENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "ioput/option/OptionList.h"

#include <ssiocv.h>

#include <tuple>
#include <iostream>
#include <chrono>

#define SSI_GARBAGEDATA_PROVIDER "garbagedata"

namespace ssi {

	class TestDataSensor : public ISensor, public Thread {
	public:
		class DataChannel : public IChannel {

			friend class TestDataSensor;

		public:
			DataChannel() {
				ssi_stream_init(stream, 0, 0, sizeof(float), SSI_FLOAT, 0);
			}
			~DataChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t* getName() { return SSI_GARBAGEDATA_PROVIDER; };
			const ssi_char_t* getInfo() { return "Sends garbage data for testing purposes"; };
			ssi_stream_t getStream() { return stream; };
		protected:
			ssi_stream_t stream;
		};

	public:
		class Options : public OptionList {

		public:
			Options() : sr(30.0), dim(1), sendMonotonicallyRisingData(true), printRisingData(true)
			{
				addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz");
				addOption("dim", &dim, 1, SSI_SIZE, "number of float values per sample");
				addOption("sendMonotonicallyRisingData", &sendMonotonicallyRisingData, 1, SSI_BOOL, "if enabled, the first dimension will hold the current frame number");
				addOption("printRisingData", &printRisingData, 1, SSI_BOOL, "if enabled, the data provided will be written to std::out as well");
			};

			ssi_time_t sr;
			ssi_size_t dim;
			bool sendMonotonicallyRisingData;
			bool printRisingData;
		};


	public:

		static const ssi_char_t* GetCreateName() { return "TestPipelineWithRandomDataSensor"; };
		static IObject* Create(const ssi_char_t* file) { return new TestDataSensor(file); };
		~TestDataSensor() {
			{
				if (_file) {
					OptionList::SaveXML(_file, &_options);
					delete[] _file;
				}
			}
		}
		Options* getOptions() { return &_options; };
		const ssi_char_t* getName() { return GetCreateName(); };
		const ssi_char_t* getInfo() { return "A dummy sensor that provides a configurable nr of float dimensions with meaningless values at a configurable sample rate."; };

		ssi_size_t getChannelSize() { return 1; };
		IChannel* getChannel(ssi_size_t index) {
			return &m_data_channel;
		};
		bool setProvider(const ssi_char_t* name, IProvider* provider) {
			m_data_provider = provider;

			if (m_data_provider) {
				m_data_channel.stream.dim = _options.dim;
				m_data_channel.stream.sr = _options.sr;
				m_data_provider->init(&m_data_channel);
			}
			return true;
		}
		bool connect() { 
			m_garbageDataBuffer = new float[_options.dim];
			return true;
		};
		bool start() { return Thread::start(); };
		bool stop() { return Thread::stop(); };
		void run() {
			if (!m_timer) {
				m_timer = new Timer(1 / _options.sr); //fixed sample rate
			}

			if (_options.sendMonotonicallyRisingData) {
				m_garbageDataBuffer[0] = m_currentFrNr;
			}

			if (m_data_provider) {
				m_data_provider->provide(ssi_pcast(ssi_byte_t, m_garbageDataBuffer), 1);
			}

			if (_options.printRisingData) {
				std::cout << "Test Data: " << m_currentFrNr << std::endl;
			}

			m_currentFrNr++;

			m_timer->wait();
		};
		bool disconnect() {
			delete[] m_garbageDataBuffer;
			m_data_provider = 0;
			return true;
		}

		void setLogLevel(int level) {
			ssi_log_level = level;
		}

	protected:
		TestDataSensor(const ssi_char_t* file = 0):
			_file(0),
			m_timer(0),
			ssi_log_level(SSI_LOG_LEVEL_DEFAULT),
			m_garbageDataBuffer(0),
			m_data_provider(0),
			m_currentFrNr(0.0f)
		{
			if (file) {
				if (!OptionList::LoadXML(file, &_options)) {
					OptionList::SaveXML(file, &_options);
				}
				_file = ssi_strcpy(file);
			}
		}

		Options _options;
		ssi_char_t* _file;

		const ssi_char_t* ssi_log_name = "randomtestdata";
		int ssi_log_level;

		Timer* m_timer;

		float m_currentFrNr;

		float* m_garbageDataBuffer;

		DataChannel m_data_channel;
		IProvider* m_data_provider;
	};


}

#endif