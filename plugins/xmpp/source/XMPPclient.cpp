// XMPPclient.cpp
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


#include "XMPPclient.h"

bool XMPPclient::connect() {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "SSI XMPP Plugin using gloox %s on %s", GLOOX_VERSION.c_str(), SYSTEMNAME);

	JID jid(this->jidStr);
	j = new Client(jid, this->pw);
	j->registerConnectionListener(this);
	j->registerMessageSessionHandler(this, 0);

	j->registerStanzaExtension(new PubSub::Event(0));

	j->disco()->setVersion("SSI XMPP Plugin", GLOOX_VERSION, SYSTEMNAME);
	j->disco()->setIdentity("client", "bot", "SSI");
	j->disco()->addFeature(XMLNS_CHAT_STATES);

	if (useSaslMechPlain)
		j->setSASLMechanisms(gloox::SaslMechPlain);

	//StringList ca;
	//ca.push_back("andy.crt");
	//j->setCACerts(ca);

	j->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);

	//
	// this code connects to a jabber server through a SOCKS5 proxy
	//
	//       ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
	//                                   new ConnectionTCP( j->logInstance(),
	//                                                      "sockshost", 1080 ),
	//                                   j->logInstance(), "example.net" );
	//       conn->setProxyAuth( "socksuser", "sockspwd" );
	//       j->setConnectionImpl( conn );

	//
	// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
	//
	//       ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
	//       ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
	//       conn1->setProxyAuth( "socksuser", "sockspwd" );
	//       ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
	//       conn2->setProxyAuth( "httpuser", "httppwd" );
	//       j->setConnectionImpl( conn2 );


	//publish subscribe
	if (this->usePub || this->useSub)
		pubsub = new PubSub::Manager(j);

	return j->connect(false);
}


void XMPPclient::disconnect() {
	isConnected = false;

	pipelineShutdown = true;
	j->disconnect();

	//wait for thread loop to stop

	ssi_msg(SSI_LOG_LEVEL_BASIC, "XMPPclient::disconnect(): waiting for run() to stop ...");
	shutdownMutex.acquire();
	ssi_msg(SSI_LOG_LEVEL_BASIC, "XMPPclient::disconnect(): run() stopped.");

}


void XMPPclient::onConnect()
{
	ssi_msg(SSI_LOG_LEVEL_BASIC, "connected to server");


	if (this->usePub) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "using node publishing mode");
		pubsub->createNode(JID(this->pubNodeService), this->pubNodeName, 0, this);
	}

	if (this->useSub) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "using node subscription");
		//pubsub->unsubscribe(JID(this->subNodeService), this->subNodeName, "", this, gloox::JID());
		pubsub->subscribe(JID(this->subNodeService), this->subNodeName, this, gloox::JID(), gloox::PubSub::SubscriptionItems, 0, "");

		//get subscriptions
		//pubsub->getSubscriptions(JID(this->subNodeService), this);
	}

	isConnected = true;
}


void XMPPclient::onDisconnect(ConnectionError e) {
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "message_test: disconnected: %d\n", e);
	if (e == ConnAuthenticationFailed)
		ssi_wrn("auth failed. reason: %d\n", j->authError());

	isConnected = false;
}


bool XMPPclient::onTLSConnect(const CertInfo& info)
{
	time_t from(info.date_from);
	time_t to(info.date_to);

	ssi_msg(SSI_LOG_LEVEL_DEBUG, "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
		"from: %s\nto: %s\n",
		info.status, info.issuer.c_str(), info.server.c_str(),
		info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
		info.compression.c_str(), ctime(&from), ctime(&to));
	return true;
}


void XMPPclient::handleMessage(const Message& msg, MessageSession * /*session*/)
{
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
		msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str());

	const PubSub::Event* pse =  msg.findExtension<PubSub::Event>(StanzaExtensionType::ExtPubSubEvent);
	if (pse) {
			ssi_msg(SSI_LOG_LEVEL_BASIC, "found node event");

			if (pse->type() == PubSub::EventItems) {

				std::string xml = "";

				if (pse->items().size() >= 1) {

					//getting one item
					PubSub::Event::ItemOperationList::const_iterator iter = pse->items().begin();
					PubSub::Event::ItemOperation* item = *iter;

					//getting payload as xml
					xml = item->payload->xml();

				}
				//ssi event sender
				ssi_event_adjust(*_event_sub, xml.size() + 1);
				ssi_strcpy(_event_sub->ptr, xml.c_str());
				_event_sub->time = _frame->GetElapsedTimeMs();

				_listener->update(*_event_sub);
			}
	}
	else 

	if (msg.body().size() > 0) {

		//ssi event sender
		ssi_event_adjust(*_event, msg.body().size() + 1);
		ssi_strcpy(_event->ptr, msg.body().c_str());
		_event->time = _frame->GetElapsedTimeMs();

		_listener->update(*_event);


		/*std::string re = "You said:\n> " + msg.body() + "\nI like that statement.";
		std::string sub;
		if (!msg.subject().empty())
		sub = "Re: " + msg.subject();

		m_messageEventFilter->raiseMessageEvent(MessageEventDisplayed);
		m_messageEventFilter->raiseMessageEvent(MessageEventComposing);
		m_chatStateFilter->setChatState(ChatStateComposing);
		m_session->send(re, sub);*/

		/*if (msg.body() == "quit")
			j->disconnect();*/
	}
}


void XMPPclient::handleMessageEvent(const JID& from, MessageEventType event)
{
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "received event: %d from: %s\n", event, from.full().c_str());
}


void XMPPclient::handleChatState(const JID& from, ChatStateType state)
{
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "received state: %d from: %s\n", state, from.full().c_str());
}


void XMPPclient::handleMessageSession(MessageSession *session)
{
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "got new session\n");
	// this example can handle only one session. so we get rid of the old session
	j->disposeMessageSession(m_session);
	m_session = session;
	m_session->registerMessageHandler(this);
	//m_messageEventFilter = new MessageEventFilter(m_session);
	//m_messageEventFilter->registerMessageEventHandler(this);
	//m_chatStateFilter = new ChatStateFilter(m_session);
	//m_chatStateFilter->registerChatStateHandler(this);
}


void XMPPclient::handleLog(LogLevel level, LogArea area, const std::string& message)
{
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "log: level: %d, area: %d, %s\n", level, area, message.c_str());
}


//Thread
void XMPPclient::run() {

	shutdownMutex.acquire();

	//automatically reconnect on disconnect (forever)
	while (!pipelineShutdown) {

		ConnectionError ce = ConnNoError;
		while (ce == ConnNoError && !pipelineShutdown)
		{
			ce = j->recv(100000);
		}

		if (!pipelineShutdown) {
			ssi_wrn("disconnect reason: %d; -> reconnecting ...", ce);
			j->connect(false);
		}

	}

	shutdownMutex.release();
}


void XMPPclient::enter() {
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "Connecting ...");
	connect();
}


void XMPPclient::flush() {
	ssi_msg(SSI_LOG_LEVEL_DEBUG, "Disconnecting  ...");
	//disconnect();

	//if (j)
		//delete j;
	//j = NULL;
}


/*
	create session; send message; delete session
*/
void XMPPclient::sendMessage(std::string toJID, std::string msg) {
	if (isConnected) {
		MessageSession* session = new MessageSession(j, JID(toJID));
		session->send(msg);
		j->disposeMessageSession(session);
	}
}



void XMPPclient::publishToNode(std::string msg) {
	//remove all old items from node
	//pubsub->purgeNode(JID(this->pubNodeService), this->pubNodeName, this);

	PubSub::ItemList itemlist;
	gloox::Tag * tag = new gloox::Tag("item");
	gloox::Tag * dev = new gloox::Tag("event");
	dev->addAttribute("value", msg);
	tag->addChild(dev);
	PubSub::Item *i = new PubSub::Item(tag);
	itemlist.push_back(i);

	pubsub->publishItem(JID(this->pubNodeService), this->pubNodeName, itemlist, NULL, this);
}




void XMPPclient::handleItem(const JID& service,
	const std::string& node,
	const Tag* entry) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "item to node '%s' on '%s'", node.c_str(), service.bare().c_str());
}


void XMPPclient::handleItems(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::ItemList& itemList,
	const Error* error) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "items from node '%s' on '%s'", node.c_str(), service.bare().c_str());

}


void XMPPclient::handleItemPublication(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::ItemList& itemList,
	const Error* error) {
}


void XMPPclient::handleItemDeletion(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::ItemList& itemList,
	const Error* error){
}


void XMPPclient::handleSubscriptionResult(const std::string& id,
	const JID& service,
	const std::string& node,
	const std::string& sid,
	const JID& jid,
	const PubSub::SubscriptionType subType,
	const Error* error){

	subscriptionId = sid;
	ssi_msg(SSI_LOG_LEVEL_BASIC, "subscribed to node '%s' on '%s'", node.c_str(), service.bare().c_str());
}


void XMPPclient::handleUnsubscriptionResult(const std::string& id,
	const JID& service,
	const Error* error){

	ssi_msg(SSI_LOG_LEVEL_BASIC, "unsubscribed from node");
}


void XMPPclient::handleSubscriptionOptions(const std::string& id,
	const JID& service,
	const JID& jid,
	const std::string& node,
	const DataForm* options,
	const std::string& sid,
	const Error* error){
}


void XMPPclient::handleSubscriptionOptionsResult(const std::string& id,
	const JID& service,
	const JID& jid,
	const std::string& node,
	const std::string& sid,
	const Error* error){
}


void XMPPclient::handleSubscribers(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::SubscriberList* list,
	const Error* error){
}


void XMPPclient::handleSubscribersResult(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::SubscriberList* list,
	const Error* error){
}


void XMPPclient::handleAffiliates(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::AffiliateList* list,
	const Error* error){
}


void XMPPclient::handleAffiliatesResult(const std::string& id,
	const JID& service,
	const std::string& node,
	const PubSub::AffiliateList* list,
	const Error* error){
}


void XMPPclient::handleNodeConfig(const std::string& id,
	const JID& service,
	const std::string& node,
	const DataForm* config,
	const Error* error){
}


void XMPPclient::handleNodeConfigResult(const std::string& id,
	const JID& service,
	const std::string& node,
	const Error* error){
}


void XMPPclient::handleNodeCreation(const std::string& id,
	const JID& service,
	const std::string& node,
	const Error* error){

	ssi_msg(SSI_LOG_LEVEL_BASIC, "created node '%s' on '%s'", node.c_str(), service.bare().c_str());
}


void XMPPclient::handleNodeDeletion(const std::string& id,
	const JID& service,
	const std::string& node,
	const Error* error){
}


void XMPPclient::handleNodePurge(const std::string& id,
	const JID& service,
	const std::string& node,
	const Error* error){
}


void XMPPclient::handleSubscriptions(const std::string& id,
	const JID& service,
	const PubSub::SubscriptionMap& subMap,
	const Error* error){



	for (PubSub::SubscriptionMap::const_iterator iter = subMap.begin(); iter != subMap.end(); iter++)
	{
		std::string node = iter->first;
		// Get the list of subscriptions for this node
		const std::list<PubSub::SubscriptionInfo>& lst = iter->second;
		ssi_msg(SSI_LOG_LEVEL_BASIC, "subscription : node=%s cnt=%i", node.c_str(), lst.size());

		// Iterate through each subscription
		//for (std::list<PubSub::SubscriptionInfo>::const_iterator it2 = lst.begin(); it2 != lst.end(); it2++)
		//{
		//
		//}
	}

	
}


void XMPPclient::handleAffiliations(const std::string& id,
	const JID& service,
	const PubSub::AffiliationMap& affMap,
	const Error* error){
}


void XMPPclient::handleDefaultNodeConfig(const std::string& id,
	const JID& service,
	const DataForm* config,
	const Error* error){
}
