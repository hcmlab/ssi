#pragma once

#ifndef SSI_MQTTC_H
#define SSI_MQTTC_H

#include "mongoose.h"
#include <thread/Thread.h>


namespace ssi{
	class Mqtt;


	class MQTTclient : public ssi::Thread {
	public:
		MQTTclient(Mqtt *socket);
		~MQTTclient();
		void enter() override;
		void run() override;
		void flush() override;


		void sendEvent(std::string msg);

	private:

		static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);


		bool stop;

		ssi::Mutex shutdownMutex;
		ssi::Mutex publishMutex;
	};

}
#endif