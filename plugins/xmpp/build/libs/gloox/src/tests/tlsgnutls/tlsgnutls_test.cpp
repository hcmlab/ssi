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
#include "../../tlshandler.h"
#include "../../tlsgnutlsclientanon.h"
#include "../../tlsgnutlsserveranon.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

#include "../../config.h"

#ifdef HAVE_GNUTLS

class GnuTLSTest : TLSHandler
{
  public:
    GnuTLSTest();
    ~GnuTLSTest();
    virtual void handleEncryptedData( const TLSBase* base, const std::string& data );
    virtual void handleDecryptedData( const TLSBase* base, const std::string& data );
    virtual void handleHandshakeResult( const TLSBase* base, bool success, CertInfo &certinfo );

    bool handshake();
    std::string send( const std::string& txt );
  private:
    void printfCert( CertInfo &certinfo );
    void loop();
    GnuTLSClientAnon *m_client;
    GnuTLSServerAnon *m_server;
    std::string m_clientToServer;
    std::string m_serverToClient;
    std::string m_clientDecrypted;
    std::string m_serverDecrypted;
    bool m_clientHandshake;
    bool m_clientHandshakeResult;
    bool m_serverHandshake;
    bool m_serverHandshakeResult;
};

GnuTLSTest::GnuTLSTest()
 : m_clientHandshake( false ), m_clientHandshakeResult( false ),
   m_serverHandshake( false ), m_serverHandshakeResult( false )
{
  m_client = new GnuTLSClientAnon( this );
  m_client->init();
  m_server = new GnuTLSServerAnon( this );
  m_server->init();
}

GnuTLSTest::~GnuTLSTest()
{
  delete m_client;
  delete m_server;
}

bool GnuTLSTest::handshake()
{
  m_client->handshake();
  while( !m_clientHandshakeResult && !m_serverHandshakeResult )
    loop();
  return m_clientHandshake && m_serverHandshake;
}

void GnuTLSTest::loop()
{
  while( !m_clientToServer.empty() )
  {
//     printf( "we have %d bytes for the server\n", m_clientToServer.length() );
    m_server->decrypt( m_clientToServer );
    m_clientToServer = "";
//     printf( "we have %d bytes left for the server\n", m_clientToServer.length() );
  }
  while( !m_serverToClient.empty() )
  {
//     printf( "we have %d bytes for the client\n", m_serverToClient.length() );
    m_client->decrypt( m_serverToClient );
    m_serverToClient = "";
//     printf( "we have %d bytes left for the client\n", m_serverToClient.length() );
  }
  while( !m_serverDecrypted.empty() )
  {
//     printf( "we have %d bytes for the server to encrypt\n", m_serverDecrypted.length() );
    m_server->encrypt( m_serverDecrypted );
    m_serverDecrypted = "";
//     printf( "we have %d bytes left for the server to encrypt\n", m_serverDecrypted.length() );
  }
}

void GnuTLSTest::handleEncryptedData( const TLSBase* base, const std::string& data )
{
//   const GnuTLSClientAnon *c = dynamic_cast<const GnuTLSClientAnon*>( base );
//   if( c )
  if( base == m_client )
  {
//     printf( "recv encrypted data from client: %d\n", data.length() );
    m_clientToServer += data;
//     printf( "m_clientToServer: %d\n", m_clientToServer.length() );
    return;
  }

//   const GnuTLSServerAnon *s = dynamic_cast<const GnuTLSServerAnon*>( base );
//   if( s )
  if( base == m_server )
  {
//     printf( "recv encrypted data from server: %d\n", data.length() );
    m_serverToClient += data;
//     printf( "m_serverToClient: %d\n", m_serverToClient.length() );
  }
}

void GnuTLSTest::handleDecryptedData( const TLSBase* base, const std::string& data )
{
//   const GnuTLSClientAnon *c = dynamic_cast<const GnuTLSClientAnon*>( base );
//   if( c )
  if( base == m_client )
  {
//     printf( "recv decrypted data from client: %d\n", data.length() );
    m_clientDecrypted += data;
//     printf( "m_clientDecrypted: %d\n", m_clientDecrypted.length() );
    return;
  }

//   const GnuTLSServerAnon *s = dynamic_cast<const GnuTLSServerAnon*>( base );
//   if( s )
  if( base == m_server )
  {
//     printf( "recv decrypted data from server: %d\n", data.length() );
    m_serverDecrypted += data;
//     printf( "m_serverDecrypted: %d\n", m_serverDecrypted.length() );
  }
}

void GnuTLSTest::handleHandshakeResult( const TLSBase* base, bool success, CertInfo& /*certinfo*/ )
{
//   printfCert( certinfo );
//   const GnuTLSClientAnon *c = dynamic_cast<const GnuTLSClientAnon*>( base );
//   if( c )
  if( base == m_client )
  {
//     printf( "recv handshake result from client: %d\n", success );
    m_clientHandshakeResult = true;
    m_clientHandshake = success;
    return;
  }

//   const GnuTLSServerAnon *s = dynamic_cast<const GnuTLSServerAnon*>( base );
//   if( s )
  if( base == m_server )
  {
//     printf( "recv handshake result from server: %d\n", success );
    m_serverHandshakeResult = true;
    m_serverHandshake = success;
  }
}

void GnuTLSTest::printfCert( CertInfo &certinfo )
{
  printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
          certinfo.status, certinfo.issuer.c_str(), certinfo.server.c_str(),
          certinfo.protocol.c_str(), certinfo.mac.c_str(), certinfo.cipher.c_str(),
          certinfo.compression.c_str() );
}

std::string GnuTLSTest::send( const std::string& txt )
{
//   printf( "sending %d\n", txt.length() );

  m_client->encrypt( txt );
  while( m_clientDecrypted.empty() )
    loop();

//   printf( "recv'ed %d\n", m_clientDecrypted.length() );
  const std::string t = m_clientDecrypted;
  m_clientDecrypted = "";
  return t;
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  name = "anon client/server handshake test";
  GnuTLSTest *t = new GnuTLSTest();
  if( !t->handshake() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "simple send";
  std::string text( "text" );
  if( t->send( text ) != text )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "subseququent send";
  text = std::string( "txt"/*17000, 'x'*/ );
  if( t->send( text ) != text )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "large send";
  text = std::string( 17000, 'x' );
  if( t->send( text ) != text )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "larger send";
  text = std::string( 170000, 'x' );
  if( t->send( text ) != text )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }





  delete t;













  if( fail == 0 )
  {
    printf( "TLSGnuTLS: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "TLSGnuTLS: %d test(s) failed\n", fail );
    return 1;
  }
}
#else
int main( int /*argc*/, char** /*argv*/ )
{
  printf( "GnuTLS not enabled. Skipped tests.\n" );
}
#endif // HAVE_GNUTLS

