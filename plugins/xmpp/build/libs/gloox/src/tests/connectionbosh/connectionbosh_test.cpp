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

#include "../../gloox.h"
#include "../../connectionbase.h"
#include "../../connectionbosh.h"
#include "../../connectiondatahandler.h"
#include "../../logsink.h"
#include "../../loghandler.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int g_test = 0;

namespace gloox
{
  class FakeConnection : public ConnectionBase
  {
    public:
      FakeConnection() : ConnectionBase( 0 ) {}
      virtual ~FakeConnection() {}
      virtual ConnectionError connect();
      virtual ConnectionError recv( int timeout = -1 );
      virtual bool send( const std::string& data );
      virtual ConnectionError receive();
      virtual void disconnect();
      virtual void cleanup() {}
      virtual ConnectionBase* newInstance() const
      {
//         printf( "FakeConnection::newInstance(): %d\n", g_test );
        return new FakeConnection();
      }
      virtual void getStatistics( long int& /*totalIn*/, long int& /*totalOut*/ ) {}
      void setTest( int test ) { g_test = test; }
  };

  ConnectionError FakeConnection::connect()
  {
//     printf( "FakeConnection::connect(): %d\n", g_test );
    m_state = StateConnecting;
    return ConnNoError;
  }
  ConnectionError FakeConnection::recv( int )
  {
//     printf( "FakeConnection::recv(): %d\n", g_test );
    if( g_test == 1 )
    {
      m_state = StateConnected;
      m_handler->handleConnect( this );
      ++g_test;
    }
    else if( g_test == 2 )
    {
      ++g_test;
      m_handler->handleReceivedData( this, "HTTP/1.0 200 OK\r\n"
          "Date: Sun, 09 Dec 2007 17:49:11 GMT\r\n"
              "Content-length: 592\r\n"
              "Content-type: text/xml\r\n"
              "Server: TwistedWeb/2.5.0\r\n\r\n"
              "<body xmlns='http://jabber.org/protocol/httpbind' inactivity='30' "
              "xmpp:version='1.0' sid='a6b1b3867e408aaa64fbdacde8f25af2139a74a6' requests='2' hold='1' "
              "xmlns:xmpp='urn:xmpp:xbosh'><stream:features xmlns:stream='http://etherx.jabber.org/streams' "
              "xmlns='jabber:client'>"
              "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
              "<mechanism>DIGEST-MD5</mechanism>"
              "<mechanism>PLAIN</mechanism>"
              "<mechanism>ANONYMOUS</mechanism>"
              "<mechanism>CRAM-MD5</mechanism>"
              "</mechanisms>"
              "<auth xmlns='http://jabber.org/features/iq-auth'/>"
              "<register xmlns='http://jabber.org/features/iq-register'/></stream:features></body>" );
    }

    return ConnNoError;
  }
  bool FakeConnection::send( const std::string& /*data*/ )
  {
//     printf( "FakeConnection::send(): %d\n", g_test );
    return true;
  }
  ConnectionError FakeConnection::receive()
  {
//     printf( "FakeConnection::receive(): %d\n", g_test );
    return ConnNoError;
  }
  void FakeConnection::disconnect()
  {
//     printf( "FakeConnection::disconnect(): %d\n", g_test );
  }

  class FakeClientBase : public ConnectionDataHandler, public LogHandler
  {
    public:
      FakeClientBase( ConnectionBOSH* cb ) : m_bosh( cb ) { m_bosh->registerConnectionDataHandler( this ); }
      virtual ~FakeClientBase() {}
      virtual void handleReceivedData( const ConnectionBase* connection, const std::string& data );
      virtual void handleConnect( const ConnectionBase* connection );
      virtual void handleDisconnect( const ConnectionBase* connection, ConnectionError reason );
      virtual void handleLog( LogLevel /*level*/, LogArea area, const std::string& /*message*/ )
      {
//         printf("%d: ", int( time( 0 ) ) );
        switch(area)
        {
          case LogAreaXmlIncoming:
//             printf("Received XML: ");
            break;
          case LogAreaXmlOutgoing:
//             printf("Sent XML: ");
            break;
          case LogAreaClassConnectionBOSH:
//             printf("BOSH: ");
            break;
          default:
//             printf("log: level: %d, area: %d, ", level, area);
            break;
        }
//         printf("%s\n", message.c_str() );
      }
      void doLoop();
    private:
      ConnectionBOSH* m_bosh;
      bool m_stopLoop;
  };

  void FakeClientBase::handleConnect( const ConnectionBase* /*connection*/ )
  {
//     printf( "FakeClientBase::handleConnect(): %d\n", g_test );
    m_stopLoop = true;
    if( g_test == 3 ) // send outgoing stream opener
    {
      m_bosh->send( "<?xml version='1.0' ?><stream:stream to='example.net' xmlns='jabber:client' "
          "xmlns:stream='http://etherx.jabber.org/streams'  xml:lang='en' version='1.0'>" );
      ++g_test;
    }
  }
  void FakeClientBase::handleReceivedData( const ConnectionBase* /*connection*/, const std::string& /*data*/ )
  {
//     printf( "FakeClientBase::handleReceivedData(): %d\n", g_test );
    m_stopLoop = true;
    if( g_test == 4 ) //
    {
      // stream opener, ignored, we'll wait for the stream features
      ++g_test;
    }
    else if( g_test == 5 ) // stream features
    {
      m_bosh->send( "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='DIGEST-MD5'/>" );
      ++g_test;
    }
//     else
//       printf( "RECEIVED UNHANDLED: %s\n", data.c_str() );
  }
  void FakeClientBase::handleDisconnect( const ConnectionBase* /*connection*/, ConnectionError /*reason*/ )
  {
//     printf( "FakeClientBase::handleDisconnect(): %d\n", g_test );
    m_stopLoop = true;
  }
  void FakeClientBase::doLoop()
  {
    m_stopLoop = false;
    while( !m_stopLoop )
      m_bosh->recv();
  }
}

using namespace gloox;

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  LogSink ls;
  FakeConnection* fc = new FakeConnection();
  ConnectionBOSH* cb = new ConnectionBOSH( fc, ls, "example.net" ,"example.net" );
  FakeClientBase* fcb = new FakeClientBase( cb );
  ls.registerLogHandler( LogLevelDebug, LogAreaAll, fcb );



  // -------
  name = "connect";
  fc->setTest( 1 );
  cb->connect();
  fcb->doLoop();
  if( 1 )
  {
    ++fail;
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }


  delete cb;
  delete fcb;


  if( fail == 0 )
  {
    printf( "ConnectionBOSH: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "ConnectionBOSH: %d test(s) failed\n", fail );
    return 1;
  }


}
