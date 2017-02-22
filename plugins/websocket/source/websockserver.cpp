#include "websockserver.h"

#include "websocket.h"
#include <stdlib.h>
#include <sstream>

namespace ssi {

	static Websocket *_wsocket;

	std::string Websockserver::stream_info_JSON = "";


	static struct mg_serve_http_opts s_http_server_opts;

	struct mg_mgr mgr;
	struct mg_connection *nc;


	int Websockserver::is_websocket(const struct mg_connection *nc) {
		return nc->flags & MG_F_IS_WEBSOCKET;
	}

	void Websockserver::broadcast(struct mg_connection *nc, const char *msg, size_t len) {
		if (nc) {
			struct mg_connection *c;

			for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
				mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, msg, len);
			}
		}
	}

	void Websockserver::broadcastRaw(struct mg_connection *nc, const char *msg, size_t len) {
		if (nc) {

			struct mg_connection *c;

			for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
				mg_send_websocket_frame(c, WEBSOCKET_OP_BINARY, msg, len);
			}
		}
	}

	void Websockserver::ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
		struct http_message *hm = (struct http_message *) ev_data;
		struct websocket_message *wm = (struct websocket_message *) ev_data;
		
		//http://stackoverflow.com/questions/166132/maximum-length-of-the-textual-representation-of-an-ipv6-address
		char ip[45];

		switch (ev) {
		case MG_EV_HTTP_REQUEST: {
			/* Usual HTTP request - serve static files */
			mg_serve_http(nc, hm, s_http_server_opts);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			break;
		}
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {

			mg_sock_addr_to_str(&nc->sa, ip, 45, MG_SOCK_STRINGIFY_IP);

			ssi_msg(SSI_LOG_LEVEL_BASIC, "ip: %s id: %p joined.", ip, nc);

			/* New websocket connection. Send stream info. */
			if (stream_info_JSON.size() > 0)
				mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, stream_info_JSON.c_str(), stream_info_JSON.size());
			break;
		}
		case MG_EV_WEBSOCKET_FRAME: {
			/* New websocket message. Tell everybody. */
			//broadcast(nc, (char *)wm->data, wm->size);

			mg_sock_addr_to_str(&nc->sa, ip, 45, MG_SOCK_STRINGIFY_IP);

            _wsocket->sendSSIEvent((char *)wm->data, wm->size, ip, strlen(ip), (intptr_t)nc);

			break;
		}
		case MG_EV_CLOSE: {
			/* Disconnect. Tell everybody. */
			if (is_websocket(nc)) {
				//broadcast(nc, "left", 4);

				mg_sock_addr_to_str(&nc->sa, ip, 45, MG_SOCK_STRINGIFY_IP);

				ssi_msg(SSI_LOG_LEVEL_BASIC, "ip: %s id: %p left.", ip, nc);
			}
			break;
		}
		default:
			break;
		}
	}


	Websockserver::Websockserver(Websocket *socket)
	{
		this->_single_execution = true;
		_wsocket = socket;
		stop = false;
	}

	Websockserver::~Websockserver()
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "websockserver waiting for server to stop ...");
		
		stop = true;
		shutdownMutex.acquire();

		shutdownMutex.release();

		ssi_msg(SSI_LOG_LEVEL_BASIC, "websockserver ready to destroy.");
	}

	void Websockserver::enter()
    {

		mg_mgr_init(&mgr, NULL);

		char port[6];
#if _WIN32||_WIN64
		_itoa(_wsocket->getOptions()->http_port, port, 10);
#else
        snprintf(port, 5, "%i", _wsocket->getOptions()->http_port);
        port[5]='\0';
#endif

		nc = mg_bind(&mgr, port, ev_handler);

		s_http_server_opts.document_root = _wsocket->getOptions()->http_root;
		s_http_server_opts.enable_directory_listing = "no";

		mg_set_protocol_http_websocket(nc);

        ssi_msg(SSI_LOG_LEVEL_BASIC, "Started on port %s\n", port);

        //printf("Started on port %s\n", port);
	}

	void Websockserver::run()
	{
		shutdownMutex.acquire();
		if (!stop) {

			while (!stop) {
				mg_mgr_poll(&mgr, 200);
			}

			mg_mgr_free(&mgr);

			ssi_msg(SSI_LOG_LEVEL_BASIC, "server stopped.");
			shutdownMutex.release();
		}
	}

	void Websockserver::flush()
	{
	}

	void Websockserver::sendEvent(std::string msg)
	{
		broadcast(nc, msg.c_str(), msg.length());
	}

	void Websockserver::sendRawData(char* data, int len)
	{
		broadcastRaw(nc, data, len);
	}

}
