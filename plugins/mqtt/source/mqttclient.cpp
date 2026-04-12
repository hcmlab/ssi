#include "mqttclient.h"

#include "mqtt.h"
#include <stdlib.h>
#include <sstream>
#include <thread/Lock.h>

namespace ssi {

	static Mqtt *_msocket;



	struct mg_mgr mgr;
	struct mg_connection *nc_stat;

	char address[128];


	struct mg_mqtt_topic_expression topic_expressions[1];


	void MQTTclient::ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
		struct mg_mqtt_message *msg = (struct mg_mqtt_message *)ev_data;
		(void)nc;

		switch (ev) {
		case MG_EV_CONNECT:
			mg_set_protocol_mqtt(nc);

			mg_send_mqtt_handshake(nc, "ssi");
			break;
		case MG_EV_MQTT_CONNACK:
			if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "Got mqtt connection error: %d\n", msg->connack_ret_code);
				break;
			}
			ssi_msg(SSI_LOG_LEVEL_BASIC, "Subscribing ...\n");
			mg_mqtt_subscribe(nc, topic_expressions, sizeof(topic_expressions) / sizeof(*topic_expressions), 42);
			break;
		case MG_EV_MQTT_PUBACK:
			ssi_msg(SSI_LOG_LEVEL_BASIC, "Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
			break;
		case MG_EV_MQTT_SUBACK:
			ssi_msg(SSI_LOG_LEVEL_BASIC, "Subscription acknowledged.");
			break;
		case MG_EV_MQTT_PUBLISH:
		{

			printf("Got incoming message %s: %.*s\n", msg->topic, (int)msg->payload.len, msg->payload.p);
			_msocket->sendSSIEvent((char *)msg->payload.p, (int)msg->payload.len, msg->topic, strlen(msg->topic), (intptr_t)nc);

			//printf("Forwarding to /test\n");
			//mg_mqtt_publish(nc_stat, "/test", 65, MG_MQTT_QOS(0), msg->payload.p, msg->payload.len);
		}
		break;
		case MG_EV_CLOSE: {

			ssi_wrn("Connection closed.");
			
			nc_stat = nullptr;

			ssi_wrn("Reconnect ...");
			nc_stat = mg_connect(&mgr, address, ev_handler);

			break;
		}
		default:
			break;
		}
	}


	MQTTclient::MQTTclient(Mqtt *socket)
	{
		nc_stat = 0;

		this->_single_execution = true;
		_msocket = socket;
		stop = false;
	}

	MQTTclient::~MQTTclient()
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "mqttclient waiting for client to stop ...");
		
		stop = true;
		shutdownMutex.acquire();

		shutdownMutex.release();

		ssi_msg(SSI_LOG_LEVEL_BASIC, "mqttclient ready to destroy.");
	}

	void MQTTclient::enter()
    {

		mg_mgr_init(&mgr, NULL);

		snprintf(address, 128, "%s:%i", _msocket->getOptions()->mqtt_client_server, _msocket->getOptions()->mqtt_client_port);

		topic_expressions[0] = { _msocket->getOptions()->mqtt_sub_topic, 0};

		//nc_stat = mg_connect(&mgr, "127.0.0.1:1883", ev_handler);
		nc_stat = mg_connect(&mgr, address, ev_handler);



		if (nc_stat == NULL) {
			fprintf(stderr, "mg_connect(%s) failed\n", address);

			ssi_err("Could not connect to mqtt server!");
		}

		//static struct mg_send_mqtt_handshake_opts opts;
		//opts.keep_alive = 3600000;
		//mg_send_mqtt_handshake_opt(nc_stat, "ssi", opts);

        ssi_msg(SSI_LOG_LEVEL_BASIC, "Client started on port %s\n", address);

        //printf("Started on port %s\n", port);
	}

	void MQTTclient::run()
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

	void MQTTclient::flush()
	{
	}

	void MQTTclient::sendEvent(std::string msg)
	{
		Lock lock(publishMutex);

		if (nc_stat) {
			mg_mqtt_publish(nc_stat, _msocket->getOptions()->mqtt_pub_topic, 65, MG_MQTT_QOS(0), msg.c_str(), msg.length());
		} 
		else
		{
			ssi_wrn("Failed to publish message.");
		}
	}

}
