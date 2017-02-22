#pragma once

#ifndef SSI_MQTTS_H
#define SSI_MQTTS_H

#include "mongoose.h"
#include <thread/Thread.h>


namespace ssi{
	class Mqtt;


	class MQTTserver : public ssi::Thread {
	public:
		MQTTserver(Mqtt *socket);
		~MQTTserver();
		void enter() override;
		void run() override;
		void flush() override;

	private:

		bool stop;

		ssi::Mutex shutdownMutex;
	};

}
#endif