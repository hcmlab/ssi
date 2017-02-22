// XMPPclient.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 17/3/2015
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


#include "gloox/client.h"
#include "gloox/messagesessionhandler.h"
#include "gloox/messageeventhandler.h"
#include "gloox/messageeventfilter.h"
#include "gloox/chatstatehandler.h"
#include "gloox/chatstatefilter.h"
#include "gloox/connectionlistener.h"
#include "gloox/disco.h"
#include "gloox/message.h"
#include "gloox/gloox.h"
#include "gloox/lastactivity.h"
#include "gloox/loghandler.h"
#include "gloox/logsink.h"
#include "gloox/connectiontcpclient.h"
#include "gloox/connectionsocks5proxy.h"
#include "gloox/connectionhttpproxy.h"
#include "gloox/messagehandler.h"

#include "gloox/pubsubmanager.h"
#include "gloox/pubsubresulthandler.h"
#include "gloox/pubsubitem.h"
#include "gloox/pubsubevent.h"

using namespace gloox;

#include "base/IObject.h"
#include "thread/Thread.h"
#include "thread/Lock.h"

#include "base/Factory.h"



#if defined( WIN32 ) || defined( _WIN32 )
#define SYSTEMNAME "windows"
#else
#define SYSTEMNAME "linux"
#endif


#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]


class XMPPclient : public MessageSessionHandler, ConnectionListener, LogHandler,
	MessageEventHandler, MessageHandler, ChatStateHandler, PubSub::ResultHandler,
	public ssi::Thread
{
public:
	XMPPclient(std::string jidStr, std::string pw, bool usePub, std::string pubNodeName, std::string pubNodeService, ssi::IEventListener *_listener, ssi_event_t *_event, ssi_event_t *_event_sub, ssi::ITheFramework *_frame, bool useSub, std::string subNodeName, std::string subNodeService, bool useSaslMechPlain) :
		m_session(0),
		m_messageEventFilter(0),
		m_chatStateFilter(0), 
		Thread(true), 
		pipelineShutdown(false), 
		_listener(_listener), 
		_event(_event),
		_event_sub(_event_sub),
		_frame(_frame),
		usePub(usePub),
		useSub(useSub),
		isConnected(false),
		useSaslMechPlain(useSaslMechPlain),
		j(0)
	{
		this->jidStr = jidStr;
		this->pw = pw;

		this->pubNodeName = pubNodeName;
		this->pubNodeService = pubNodeService;

		this->subNodeName = subNodeName;
		this->subNodeService = subNodeService;
	}

	virtual ~XMPPclient() {

		//delete client
		if (j)
			delete j;
	}

	//Thread
	virtual void run();
	virtual void enter();
	virtual void flush();


	bool connect();
	void disconnect();

	virtual void onConnect();
	virtual void onDisconnect(ConnectionError e);
	virtual bool onTLSConnect(const CertInfo& info);

	virtual void handleMessage(const Message& msg, MessageSession * /*session*/);
	virtual void handleMessageEvent(const JID& from, MessageEventType event);
	virtual void handleChatState(const JID& from, ChatStateType state);
	virtual void handleMessageSession(MessageSession *session);

	virtual void handleLog(LogLevel level, LogArea area, const std::string& message);

	virtual void sendMessage(std::string toJID, std::string msg);
	virtual void publishToNode(std::string msg);


	virtual void handleItem(const JID& service,
		const std::string& node,
		const Tag* entry);


	virtual void handleItems(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::ItemList& itemList,
		const Error* error);


	virtual void handleItemPublication(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::ItemList& itemList,
		const Error* error);


	virtual void handleItemDeletion(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::ItemList& itemList,
		const Error* error);


	virtual void handleSubscriptionResult(const std::string& id,
		const JID& service,
		const std::string& node,
		const std::string& sid,
		const JID& jid,
		const PubSub::SubscriptionType subType,
		const Error* error);


	virtual void handleUnsubscriptionResult(const std::string& id,
		const JID& service,
		const Error* error);


	virtual void handleSubscriptionOptions(const std::string& id,
		const JID& service,
		const JID& jid,
		const std::string& node,
		const DataForm* options,
		const std::string& sid,
		const Error* error);


	virtual void handleSubscriptionOptionsResult(const std::string& id,
		const JID& service,
		const JID& jid,
		const std::string& node,
		const std::string& sid,
		const Error* error);


	virtual void handleSubscribers(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::SubscriberList* list,
		const Error* error);


	virtual void handleSubscribersResult(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::SubscriberList* list,
		const Error* error);


	virtual void handleAffiliates(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::AffiliateList* list,
		const Error* error);


	virtual void handleAffiliatesResult(const std::string& id,
		const JID& service,
		const std::string& node,
		const PubSub::AffiliateList* list,
		const Error* error);


	virtual void handleNodeConfig(const std::string& id,
		const JID& service,
		const std::string& node,
		const DataForm* config,
		const Error* error);


	virtual void handleNodeConfigResult(const std::string& id,
		const JID& service,
		const std::string& node,
		const Error* error);


	virtual void handleNodeCreation(const std::string& id,
		const JID& service,
		const std::string& node,
		const Error* error);


	virtual void handleNodeDeletion(const std::string& id,
		const JID& service,
		const std::string& node,
		const Error* error);


	virtual void handleNodePurge(const std::string& id,
		const JID& service,
		const std::string& node,
		const Error* error);


	virtual void handleSubscriptions(const std::string& id,
		const JID& service,
		const PubSub::SubscriptionMap& subMap,
		const Error* error);


	virtual void handleAffiliations(const std::string& id,
		const JID& service,
		const PubSub::AffiliationMap& affMap,
		const Error* error);


	virtual void handleDefaultNodeConfig(const std::string& id,
		const JID& service,
		const DataForm* config,
		const Error* error);

private:
	Client *j;
	MessageSession *m_session;
	MessageEventFilter *m_messageEventFilter;
	ChatStateFilter *m_chatStateFilter;

	PubSub::Manager *pubsub;

	std::string jidStr, pw, pubNodeName, pubNodeService, subNodeName, subNodeService;
	bool usePub, useSub;

	bool pipelineShutdown;

	bool useSaslMechPlain;

	ssi::Mutex shutdownMutex;


	std::string subscriptionId;

	ssi::IEventListener *_listener;
	ssi_event_t *_event, *_event_sub;

	ssi::ITheFramework *_frame;

	bool isConnected;
};
