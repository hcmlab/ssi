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

#define SSI_GARBAGEDATA_PROVIDER "garbagedata"

namespace ssi {

	class GarbageDataSensor : public ISensor, public Thread {
	public:
		class DataChannel : public IChannel {

			friend class GarbageDataSensor;

		public:
			DataChannel() {
				ssi_stream_init(stream, 0, 0, sizeof(unsigned char), SSI_UCHAR, 0);
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
			Options() : sr(30.0), bytesPerSample(10000)
			{
				addOption("sr", &sr, 1, SSI_TIME, "sample rate in hz");
				addOption("bytesPerSample", &sr, 1, SSI_SIZE, "nr of garbage bytes to provide for each sample");
			};

			ssi_time_t sr;
			ssi_size_t bytesPerSample;
		};


	public:

		static const ssi_char_t* GetCreateName() { return "GarbageDataSensor"; };
		static IObject* Create(const ssi_char_t* file) { return new GarbageDataSensor(file); };
		~GarbageDataSensor() {
			{
				if (_file) {
					OptionList::SaveXML(_file, &_options);
					delete[] _file;
				}
			}
		}
		Options* getOptions() { return &_options; };
		const ssi_char_t* getName() { return GetCreateName(); };
		const ssi_char_t* getInfo() { return "A dummy sensor that provides a configurable amount of meaningless bytes at a configurable sample rate."; };

		ssi_size_t getChannelSize() { return 1; };
		IChannel* getChannel(ssi_size_t index) {
			return &m_data_channel;
		};
		bool setProvider(const ssi_char_t* name, IProvider* provider) {
			m_data_provider = provider;

			if (m_data_provider) {
				m_data_channel.stream.dim = _options.bytesPerSample;
				m_data_channel.stream.sr = _options.sr;
				m_data_provider->init(&m_data_channel);
			}
			return true;
		}
		bool connect() { 
			m_garbageDataBuffer = new unsigned char[_options.bytesPerSample];
			return true;
		};
		bool start() { return Thread::start(); };
		bool stop() { return Thread::stop(); };
		void run() {
			if (!m_timer) {
				m_timer = new Timer(1 / _options.sr); //fixed sample rate
			}

			if (m_data_provider) {
				m_data_provider->provide(ssi_pcast(ssi_byte_t, m_garbageDataBuffer), 1);
			}

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
		GarbageDataSensor(const ssi_char_t* file = 0):
			_file(0),
			m_timer(0),
			ssi_log_level(SSI_LOG_LEVEL_DEFAULT),
			m_garbageDataBuffer(0),
			m_data_provider(0)
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

		const ssi_char_t* ssi_log_name = "garbagedata";
		int ssi_log_level;

		Timer* m_timer;

		unsigned char* m_garbageDataBuffer;

		DataChannel m_data_channel;
		IProvider* m_data_provider;
	};


}

#endif