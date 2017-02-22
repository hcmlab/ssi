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

#include "../../tag.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;

  class Client
  {
    public:
      Client() {}
      virtual ~Client() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ&, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void removeIDHandler( IqHandler* ) {}
      void setAuthFailure( AuthenticationError ) {}
      void setAuthed( bool ) {}
      void connected() {}
      void disconnect( ConnectionError ) {}
      const std::string password() const { return EmptyString; }
      const std::string username() const { return EmptyString; }
    private:
      JID m_jid;
  };
  void Client::removeIqHandler( IqHandler*, int ) {}
  void Client::registerIqHandler( IqHandler*, int ) {}
  void Client::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void Client::removeStanzaExtension( int ) {}
  const std::string Client::getID() { return "id"; }
}
using namespace gloox;

#define CLIENT_H__
#define NONSASLAUTH_TEST
#include "../../nonsaslauth.h"
#include "../../nonsaslauth.cpp"

             class NonSaslAuthTest : public Client
{
  public:
    NonSaslAuthTest() : m_result( 0 ) {}
    virtual ~NonSaslAuthTest() {}
    void setTest( int test ) { m_test = test; }
    virtual void send( IQ& );
    virtual void send( const IQ&, IqHandler*, int );
    virtual void trackID( IqHandler*, const std::string&, int ) {}
    int checkResult() { int t = m_result; m_result = 0; return t; }
  private:
    int m_test;
    int m_result;
};
void NonSaslAuthTest::send( IQ& )
{
}
void NonSaslAuthTest::send( const IQ&, IqHandler*, int )
{
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag* t = 0;


  // -------
  {
    name = "field request";
    NonSaslAuth::Query q( "user" );
    t = q.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_AUTH + "'>"
                          "<username>user</username></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "reply w/ pwd";
    Tag* q = new Tag( "query" );
    q->setXmlns( XMLNS_AUTH );
    new Tag( q, "username" );
    new Tag( q, "password" );
    new Tag( q, "resource" );
    NonSaslAuth::Query n( q );
    NonSaslAuth::Query* nq = n.newInstance( "user", "sid", "pwd", "resource" );
    t = nq->tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_AUTH + "'>"
                          "<username>user</username>"
                          "<password>pwd</password>"
                          "<resource>resource</resource>"
                          "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete nq;
    delete q;
    delete t;
    t = 0;
  }

  // -------
  {
    name = "reply w/ digest";
    Tag* q = new Tag( "query" );
    q->setXmlns( XMLNS_AUTH );
    new Tag( q, "username" );
    new Tag( q, "password" );
    new Tag( q, "digest" );
    new Tag( q, "resource" );
    NonSaslAuth::Query n( q );
    NonSaslAuth::Query* nq = n.newInstance( "user", "sid", "pwd", "resource" );
    SHA sha;
    sha.feed( "sid" );
    sha.feed( "pwd" );
    t = nq->tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_AUTH + "'>"
         "<username>user</username>"
         "<digest>" + sha.hex() + "</digest>"
         "<resource>resource</resource>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete nq;
    delete q;
    delete t;
    t = 0;
  }

  // -------
  {
    StanzaExtensionFactory sef;
    sef.registerExtension( new NonSaslAuth::Query() );
    name = "NonSaslAuth::Query/SEFactory test";
    Tag* f = new Tag( "iq" );
    new Tag( f, "query", "xmlns", XMLNS_AUTH );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const NonSaslAuth::Query* se = iq.findExtension<NonSaslAuth::Query>( ExtNonSaslAuth );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  if( fail == 0 )
  {
    printf( "NonSaslAuth::Query: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "NonSaslAuth::Query: %d test(s) failed\n", fail );
    return 1;
  }

}
