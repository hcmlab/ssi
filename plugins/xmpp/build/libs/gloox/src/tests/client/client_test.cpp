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

#define CLIENTBASE_TEST
#include "../../client.h"
#include "../../clientbase.cpp"
#include "../../jid.h"
#include "../../connectionbase.h"
// #include "../../logsink.h"
// #include "../../loghandler.h"
#include "../../connectionlistener.h"
#include "../../gloox.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

class ClientTest : public Client, LogHandler, ConnectionListener
{
  public:
    ClientTest( const JID& jid, const std::string& password, int port = -1 )
      : Client( jid, password, port ), m_connected( 0 ), m_disconnected( 0 ), m_log( false )
    {
      logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
      registerConnectionListener( this );
      jidCopy = jid.full();
    }
    virtual ~ClientTest() {}
    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      if( m_log )
        printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }
    virtual void onConnect()
    {
      ++m_connected;

      // magic jid
      if( jidCopy == "a@b/c" )
        disconnect();
    }
    virtual void onDisconnect( ConnectionError e )
    {
      ++m_disconnected;
      m_disconnect = e;
      m_streamerror = streamError();
//       printf( "reason: %d\n", e );
    }
    virtual void onResourceBindError( ResourceBindError /*error*/ )
      { /*printf( "res bind err: %d\n", error );*/ }
    virtual void onSessionCreateError( SessionCreateError /*error*/ )
      { /*printf( "ses err: %d\n", error );*/ }
    virtual bool onTLSConnect( const CertInfo& /*info*/ ) { return false; }
    int connected() const { return m_connected; }
    int disconnected() const { return m_disconnected; }
    ConnectionError disconnectReason() const { return m_disconnect; }
    StreamError streamErrorReason() const { return m_streamerror; }
    void setLog( bool log ) { m_log = log; }

  protected:

  private:
    int m_connected;
    int m_disconnected;
    ConnectionError m_disconnect;
    StreamError m_streamerror;
    bool m_log;
    std::string jidCopy;
};

class ConnectionImpl : public ConnectionBase
{
  public:
    ConnectionImpl( ConnectionDataHandler *cdh, int test )
      : ConnectionBase( cdh ), m_test( test ), m_pos( 0 ), m_run( true ) {}
    virtual ~ConnectionImpl() {}
    virtual ConnectionError connect()
    {
      m_run = true;
      m_state = StateConnected;
      m_handler->handleConnect( this );
      return ConnNoError;
    }
    virtual ConnectionError recv( int /*timeout = -1*/ )
    {
      if( m_msgs[m_test][m_pos] )
      {
        m_handler->handleReceivedData( this, m_msgs[m_test][m_pos++] );
        return ConnNoError;
      }
      else
      {
        m_handler->handleDisconnect( this, ConnIoError );
        return ConnIoError;
      }
    }
    virtual bool send( const std::string& /*data*/ ) { return true; }
    virtual ConnectionError receive()
    {
      ConnectionError ce = ConnNoError;
      while( m_run && ce == ConnNoError )
        ce = recv( 0 );
      return ce;
    }
    virtual void disconnect() { m_run = false; }
    virtual void cleanup()
    {
      m_state = StateDisconnected;
      m_pos = 0;
    }
    virtual void getStatistics( long int& /*totalIn*/, long int& /*totalOut*/ ) {}
    virtual ConnectionBase* newInstance() const { return 0; }

  private:
    int m_test;
    int m_pos;
    bool m_run;
    static const char* m_msgs[6][12];

};

const char* ConnectionImpl::m_msgs[6][12] =
  {
    { // connection/auth goes ok.
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
        "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
        "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
          "<mechanism>PLAIN</mechanism>"
          "<mechanism>DIGEST-MD5</mechanism>"
        "</mechanisms>"
      "</stream:features>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
        "bm9uY2U9ImhvS1I2VkZDSGFibUVYY01weFhlL0QrcVZjWEdyMUdFNzQ0MVFzM2MxY2M9IixyZWFsbT0iamFiYmV"
        "yLmNjIixxb3A9ImF1dGgsYXV0aC1pbnQsYXV0aC1jb25mIixjaXBoZXI9InJjNC00MCxyYzQtNTYscmM0LGRlcyw"
        "zZGVzIixtYXhidWY9MTAyNCxjaGFyc2V0PXV0Zi04LGFsZ29yaXRobT1tZDUtc2Vzcw=="
      "</challenge>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
        "cnNwYXV0aD1mNGFhZTM0YWY0N2I1MmM0MmQ2NWQzY2NjMGNjN2YyNA=="
      "</challenge>",
      "<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>",
      "<stream:stream from='jabber.cc' id='1o4p1gz2h0m1wvqutohs24d439nbv9zxx4nykm11' version='1.0' "
        "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
        "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>"
        "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
      "</stream:features>",
      "<iq id='uid1' type='result' xmlns='jabber:client'>"
        "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
        "<jid>hurkhurk@jabber.cc/gloox</jid></bind></iq>",
      "<iq id='uid2' type='result' xmlns='jabber:client'/>",
      "<iq id='uid3' type='result' xmlns='jabber:client'><query xmlns='jabber:iq:private'>"
        "<roster xmlns='roster:delimiter'>::</roster></query></iq>"
      "<iq id='uid4' type='result' xmlns='jabber:client'><query xmlns='jabber:iq:roster'/></iq>",
      0
    },
    { // auth failure
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
        "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
        "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
          "<mechanism>PLAIN</mechanism>"
          "<mechanism>DIGEST-MD5</mechanism>"
        "</mechanisms>"
      "</stream:features>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
        "bm9uY2U9ImhvS1I2VkZDSGFibUVYY01weFhlL0QrcVZjWEdyMUdFNzQ0MVFzM2MxY2M9IixyZWFsbT0iamFiYmV"
        "yLmNjIixxb3A9ImF1dGgsYXV0aC1pbnQsYXV0aC1jb25mIixjaXBoZXI9InJjNC00MCxyYzQtNTYscmM0LGRlcyw"
        "zZGVzIixtYXhidWY9MTAyNCxjaGFyc2V0PXV0Zi04LGFsZ29yaXRobT1tZDUtc2Vzcw=="
      "</challenge>",
      "<failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><temporary-auth-failure/></failure>",
      0,
    },
    { // chokes in the middle
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
        "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
        "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
          "<mechanism>PLAIN</mechanism>"
          "<mechanism>DIGEST-MD5</mechanism>"
        "</mechanisms>"
      "</stream:features>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
        "bm9uY2U9ImhvS1I2VkZDSGFibUVYY01weFhlL0QrcVZjWEdyMUdFNzQ0MVFzM2MxY2M9IixyZWFsbT0iamFiYmV"
        "yLmNjIixxb3A9ImF1dGgsYXV0aC1pbnQsYXV0aC1jb25mIixjaXBoZXI9InJjNC00MCxyYzQtNTYscmM0LGRlcyw"
        "zZGVzIixtYXhidWY9MTAyNCxjaGFyc2V0PXV0Zi04LGFsZ29yaXRobT1tZDUtc2Vzcw=="
      "</challenge>",
      0,
    },
    { // chokes on the xml
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
        "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
        "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
          "<mechanism>PLAIN</mechanism>"
          "<mechanism>DIGEST-MD5</mechanism>"
        "</mechanisms>"
      "</stream:features>",
      "<stream:error><xml-not-well-formed xmlns='urn:ietf:params:xml:ns:xmpp-streams'/></stream:error>",
      0,
    },
    { // connection/auth goes ok. basic xep-0198 (stream management) ack'ing
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
      "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
      "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "<mechanism>PLAIN</mechanism>"
      "<mechanism>DIGEST-MD5</mechanism>"
      "</mechanisms>"
      "</stream:features>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "bm9uY2U9ImhvS1I2VkZDSGFibUVYY01weFhlL0QrcVZjWEdyMUdFNzQ0MVFzM2MxY2M9IixyZWFsbT0iamFiYmV"
      "yLmNjIixxb3A9ImF1dGgsYXV0aC1pbnQsYXV0aC1jb25mIixjaXBoZXI9InJjNC00MCxyYzQtNTYscmM0LGRlcyw"
      "zZGVzIixtYXhidWY9MTAyNCxjaGFyc2V0PXV0Zi04LGFsZ29yaXRobT1tZDUtc2Vzcw=="
      "</challenge>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "cnNwYXV0aD1mNGFhZTM0YWY0N2I1MmM0MmQ2NWQzY2NjMGNjN2YyNA=="
      "</challenge>",
      "<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>",
      "<stream:stream from='jabber.cc' id='1o4p1gz2h0m1wvqutohs24d439nbv9zxx4nykm11' version='1.0' "
      "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
      "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>"
      "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
      "<sm xmlns='urn:xmpp:sm:3'/>"
      "</stream:features>",
      "<iq id='uid1' type='result' xmlns='jabber:client'>"
      "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
      "<jid>hurkhurk@jabber.cc/gloox</jid></bind></iq>",
      "<enabled xmlns='urn:xmpp:sm:3' resume='true' id='some-long-id'/>",
      "<iq id='uid2' type='result' xmlns='jabber:client'/>",
      "<iq id='uid3' type='result' xmlns='jabber:client'><query xmlns='jabber:iq:private'>"
      "<roster xmlns='roster:delimiter'>::</roster></query></iq>"
      "<iq id='uid4' type='result' xmlns='jabber:client'><query xmlns='jabber:iq:roster'/></iq>",
      0
    },
    { // connection/auth goes ok.
      "<stream:stream from='jabber.cc' id='6kpid3u736sqjwd65n25wm57mzz10wz7hopvsj2w' version='1.0' "
      "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
      "<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "<mechanism>PLAIN</mechanism>"
      "<mechanism>DIGEST-MD5</mechanism>"
      "</mechanisms>"
      "</stream:features>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "bm9uY2U9ImhvS1I2VkZDSGFibUVYY01weFhlL0QrcVZjWEdyMUdFNzQ0MVFzM2MxY2M9IixyZWFsbT0iamFiYmV"
      "yLmNjIixxb3A9ImF1dGgsYXV0aC1pbnQsYXV0aC1jb25mIixjaXBoZXI9InJjNC00MCxyYzQtNTYscmM0LGRlcyw"
      "zZGVzIixtYXhidWY9MTAyNCxjaGFyc2V0PXV0Zi04LGFsZ29yaXRobT1tZDUtc2Vzcw=="
      "</challenge>",
      "<challenge xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>"
      "cnNwYXV0aD1mNGFhZTM0YWY0N2I1MmM0MmQ2NWQzY2NjMGNjN2YyNA=="
      "</challenge>",
      "<success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>",
      "<stream:stream from='jabber.cc' id='1o4p1gz2h0m1wvqutohs24d439nbv9zxx4nykm11' version='1.0' "
      "xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>"
      "<stream:features xmlns:stream='http://etherx.jabber.org/streams'>"
      "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>"
      "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
      "<sm xmlns='urn:xmpp:sm:3'/>"
      "</stream:features>",
      "<resumed xmlns='urn:xmpp:sm:3' h='3' previd='some-long-id' />",
      "<r xmlns='urn:xmpp:sm:3'/>",
      "<message from='someone' to='someother'><body>something</body></message>",
      0
    },
  };

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ClientTest *c = 0;
  ConnectionImpl *conn = 0;
  JID j( "a@b/c" );


  // -------
//   printf( "-----------------------------\n" );
  name = "connect test: ok";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 0 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  c->connect();
//   printf( "connected: %d, disconnected: %d, reason: %d\n", c->connected(), c->disconnected(), c->disconnectReason() );
  if( c->connected() != 1 || c->disconnected() != 1 || c->disconnectReason() != ConnUserDisconnected )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  // -------
//   printf( "-----------------------------\n" );
  name = "connect test: auth failure";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 1 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  c->connect();
  if( c->connected() != 0 || c->disconnected() != 1 || c->disconnectReason() != ConnAuthenticationFailed )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  // -------
//   printf( "-----------------------------\n" );
  name = "connect test: io error";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 2 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  c->connect();
  if( c->connected() != 0 || c->disconnected() != 1 || c->disconnectReason() != ConnIoError )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

//   printf( "-----------------------------\n" );
  // -------
  name = "connect test: xml error";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 3 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  c->connect();
  if( c->connected() != 0 || c->disconnected() != 1 || c->disconnectReason() != ConnStreamError
      || c->streamErrorReason() != StreamErrorXmlNotWellFormed )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %d, %d\n", name.c_str(), c->disconnectReason(), c->streamErrorReason() );
  }
  delete c;
  c = 0;

  // -------
  name = "re-connect test 1";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 2 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  for( int i = 1; i <= 100; ++i )
  {
    c->connect();
    if( c->connected() != 0 || c->disconnected() != i || c->disconnectReason() != ConnIoError )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed, %d, %d, %d\n", name.c_str(),
              c->connected(), c->disconnected(),
              c->disconnectReason() );
      break;
    }
  }
  delete c;
  c = 0;

  // -------
  name = "re-connect test 2";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 0 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  //   c->setLog( true );
  for( int i = 1; i <= 100; ++i )
  {
    c->connect();
    if( c->connected() != i || c->disconnected() != i || c->disconnectReason() != ConnUserDisconnected )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed, %d, %d, %d\n", name.c_str(),
              c->connected(), c->disconnected(),
              c->disconnectReason() );
      break;
    }
  }
  delete c;
  c = 0;



  // -------
  name = "stream management test 1: basic ack";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 4 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  c->setStreamManagement( true, false );
//   c->setLog( true );
  c->connect();
  //   printf( "connected: %d, disconnected: %d, reason: %d\n", c->connected(), c->disconnected(), c->disconnectReason() );
  if( c->connected() != 1 || c->disconnected() != 1 || c->disconnectReason() != ConnUserDisconnected )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed, %d, %d, %d\n", name.c_str(),
            c->connected(), c->disconnected(),
            c->disconnectReason() );
  }
  delete c;
  c = 0;


  j.setServer( "d" );
  // -------
  name = "stream management test 2: resume";
  c = new ClientTest( j, "b" );
  conn = new ConnectionImpl( c, 4 );
  c->setConnectionImpl( conn );
  c->setTls( TLSDisabled );
  c->setCompression( false );
  c->setStreamManagement( true, true );
//   c->setLog( true );
  c->connect();
  //   printf( "connected: %d, disconnected: %d, reason: %d\n", c->connected(), c->disconnected(), c->disconnectReason() );
  if( c->connected() != 1 || c->disconnected() != 1 || c->disconnectReason() != ConnIoError )
  {
    ++fail;
    fprintf( stderr, "test '%s' part 1 failed, %d, %d, %d\n", name.c_str(),
            c->connected(), c->disconnected(),
            c->disconnectReason() );
  }
  else
  {
    conn = new ConnectionImpl( c, 5 );
    c->setConnectionImpl( conn );
    c->connect();
    //   printf( "connected: %d, disconnected: %d, reason: %d\n", c->connected(), c->disconnected(), c->disconnectReason() );
    if( c->connected() != 2 || c->disconnected() != 2 || c->disconnectReason() != ConnIoError )
    {
      ++fail;
      fprintf( stderr, "test '%s' part 2 failed, %d, %d, %d\n", name.c_str(),
              c->connected(), c->disconnected(),
              c->disconnectReason() );
    }
  }
  delete c;
  c = 0;
















  if( fail == 0 )
  {
    printf( "Client: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Client: %d test(s) failed\n", fail );
    return 1;
  }

}
