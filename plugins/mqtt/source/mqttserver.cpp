#include "mqttserver.h"

#include "mqtt.h"
#include <stdlib.h>
#include <sstream>

namespace ssi {

	static Mqtt *_msocket;



	struct mg_mgr mgr_serv;
	struct mg_connection *nc_serv;

	struct mg_mqtt_broker brk;



	MQTTserver::MQTTserver(Mqtt*socket)
	{
		nc_serv = 0;

		this->_single_execution = true;
		_msocket = socket;
		stop = false;
	}

	MQTTserver::~MQTTserver()
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "mqttclient waiting for client to stop ...");
		
		stop = true;
		shutdownMutex.acquire();

		shutdownMutex.release();

		ssi_msg(SSI_LOG_LEVEL_BASIC, "mqttclient ready to destroy.");
	}

	void MQTTserver::enter()
    {

		mg_mgr_init(&mgr_serv, NULL);

		char address[128];
		snprintf(address, 128, "%s:%i", _msocket->getOptions()->mqtt_server_bind, _msocket->getOptions()->mqtt_server_port);


		//nc_stat = mg_connect(&mgr, "127.0.0.1:1883", ev_handler);
		mg_mgr_init(&mgr_serv, NULL);
		mg_mqtt_broker_init(&brk, NULL);

		if ((nc_serv = mg_bind(&mgr_serv, address, mg_mqtt_broker)) == NULL) {
			fprintf(stderr, "Server: mg_bind(%s) failed\n", address);
			exit(EXIT_FAILURE);
		}
		nc_serv->user_data = &brk;

        ssi_msg(SSI_LOG_LEVEL_BASIC, "Server started on %s\n", address);

        //printf("Started on port %s\n", port);
	}

	void MQTTserver::run()
	{
		shutdownMutex.acquire();
		if (!stop) {

			while (!stop) {
				mg_mgr_poll(&mgr_serv, 200);
			}

			mg_mgr_free(&mgr_serv);

			ssi_msg(SSI_LOG_LEVEL_BASIC, "server stopped.");
			shutdownMutex.release();
		}
	}

	void MQTTserver::flush()
	{
	}

}
