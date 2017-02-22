#pragma once

#ifndef SSI_WEBSO_H
#define SSI_WEBSO_H

#include "mongoose.h"
#include <thread/Thread.h>

//class Websocket;

namespace ssi{
	class Websocket;


	class Websockserver : public ssi::Thread {
	public:
		Websockserver(Websocket *socket);
		~Websockserver();
		void enter();
		void run();
		void flush();


		void sendEvent(std::string msg);
		void sendRawData(char* data, int len);

		static std::string stream_info_JSON;

	private:

		static int is_websocket(const struct mg_connection *nc);
		static void broadcast(struct mg_connection *nc, const char *msg, size_t len);
		static void broadcastRaw(struct mg_connection *nc, const char *msg, size_t len);
		static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);


		bool stop;

		ssi::Mutex shutdownMutex;
	};

}
#endif