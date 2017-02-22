/*
 *  Copyright (c) 2004-2015 by Jakob Schröter <js@camaya.net>
 *  This file is part of the gloox library. http://camaya.net/gloox
 *
 *  This software is distributed under a license. The full license
 *  agreement can be found in the file LICENSE in this distribution.
 *  This software may not be copied, modified, sold or distributed
 *  other than expressed in the named license agreement.
 *
 *  This software is distributed without any warranty.
 */

#include "../config.h"
#include "../linklocalmanager.h"
#include "../linklocalhandler.h"
#include "../logsink.h"
#include "../loghandler.h"
#include "../connectionhandler.h"
#include "../connectionlistener.h"
#include "../messagehandler.h"
#include "../message.h"
#include "../linklocalclient.h"
#include "../linklocal.h"
#include "../jid.h"
using namespace gloox;

#include <stdio.h>

#ifdef HAVE_MDNS

#ifndef _WIN32
# include <unistd.h>
#endif

#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class LinkLocalExample : public LinkLocal::Handler, LogHandler, ConnectionHandler, MessageHandler, ConnectionListener
{
  public:
    LinkLocalExample() : m_mdns( "js", this, m_log ), m_client( 0 ), m_fClient( 0 ), m_disconnect( false ), m_ok( false ) {}

    virtual ~LinkLocalExample()
    {
      delete m_client;
      delete m_fClient;
    }

    void start()
    {
      m_mdns.registerLinkLocalHandler( this );
      m_mdns.addTXTData("nick","foo");
      m_mdns.addTXTData("1st","Firstname");
      m_mdns.addTXTData("last","Lastname");
      m_mdns.addTXTData("msg","Hanging out");
      m_mdns.addTXTData("jid","me@there.com");
      m_mdns.addTXTData("status","avail");
      m_mdns.registerService();
      if( !m_mdns.startBrowsing() )
      {
        printf( "startBrowsing() failed\n" );
        return;
      }
      int count = 0;
      int fCount = 0;
      while( true )
      {
        m_mdns.recv( 1000 );
        if ( m_disconnect )
        {
          delete m_client;
          m_client = 0;
          delete m_fClient;
          m_fClient = 0;
          m_disconnect = false;
        }
        if( m_client )
        {
          m_client->recv( 1000 );
          if( m_ok && count == 1000 )
          {
            count = 0;
            Message m( Message::Chat, JID( "user@Nokia-N900" ), "hey again!" );
            m_client->send( m );
          }
          ++count;
        }
        if( m_fClient )
        {
          m_fClient->recv( 1000 );
          if( fCount == 1000 )
          {
            fCount = 0;
            Message m( Message::Chat, JID( "user@Nokia-N900" ), "hey there!" );
            m_fClient->send( m );
          }
          ++fCount;
        }
        //         printf( "ok\n" );
      }
    }

    virtual void handleBrowseReply( const LinkLocal::ServiceList& services )
    {
      LinkLocal::ServiceList::const_iterator it = services.begin();
      for( ; it != services.end(); ++it )
      {
        printf( "%s:\t%s.%s%s on interface %d\n", (*it).flag == LinkLocal::AddService ? "Added" : "Removed", (*it).service.c_str(), (*it).regtype.c_str(),
                (*it).domain.c_str(), (*it).interface );
        if( !m_fClient /*&& (*it)->flag == LinkLocal::AddService && (*it)->service != "js@pitufo"*/ )
        {
          printf( "setting up new local client and connecting to %s.%s%s on interface %d\n",
                  (*it).service.c_str(), (*it).regtype.c_str(), (*it).domain.c_str(), (*it).interface );
          m_fClient = new LinkLocal::Client( JID( "js@pitufo" ) );
          m_fClient->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
          m_fClient->registerConnectionListener( this );
          if( !m_fClient->connect( (*it).service, (*it).regtype, (*it).domain, (*it).interface ) )
          {
            printf( "m_fClient->connect() failed\n" );
          }
        }
      }
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleIncomingConnection( ConnectionBase* /*server*/, ConnectionBase* connection )
    {
      printf( "incoming connection on port %d!\n", connection->localPort() );
      m_client = new LinkLocal::Client( JID( "js@pitufo" ) );
      m_client->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
      connection->registerConnectionDataHandler( m_client );
      m_client->setConnectionImpl( connection );
      m_client->registerMessageHandler( this );
      m_client->registerConnectionListener( this );
      m_client->connect();
    }

    virtual void handleMessage( const Message& msg, MessageSession* /*session*/ = 0 )
    {
      m_ok = true;
      printf( "Message received: %s\n", msg.body().c_str() );
      Message m( Message::Chat, msg.from(), "hey!" );
      m_client->send( m );
    }

    virtual void onConnect() {}

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "onDisconnect\n" );
      m_disconnect = true;
    }

//     virtual void handleClient( LinkLocal::Client* client )
//     {
//       if( !client)
//         return;
//
//       printf( "received LinkLocalClient, connect() successful!\n" );
//       client->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
//       client->registerMessageHandler( this );
//       client->registerConnectionListener( this );
//       Message msg( Message::Chat, JID( "user@Nokia-N900" ), "hex" );
//       client->send( msg );
//       if( m_client )
//         delete m_client;
//       m_client = client;
//     }

    virtual void onResourceBind( const std::string& resource ) { (void)resource; }

    virtual void onResourceBindError( const Error* error ) { (void) (error); }

    virtual void onSessionCreateError( const Error* error ) { (void) (error); }

    virtual bool onTLSConnect( const CertInfo& info ) { return true; }

    virtual void onStreamEvent( StreamEvent event ) { (void) (event); }
private:
    LogSink m_log;
    LinkLocal::Manager m_mdns;
    LinkLocal::Client* m_client;
    LinkLocal::Client* m_fClient;
    bool m_disconnect;
    bool m_ok;

};

int main( int /*argc*/, char** /*argv*/ )
{
  LinkLocalExample *r = new LinkLocalExample();
  r->start();
  while( true )
    sleep(15);
  delete( r );
  return 0;
}

#else

int main( int /*argc*/, char** /*argv*/ )
{
  printf( "Link-local support not compiled in\n" );
  return 0;
}


#endif // HAVE_MDNS
