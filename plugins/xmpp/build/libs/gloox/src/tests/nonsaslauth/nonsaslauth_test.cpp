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
      Client() : m_jid( "foo/resource" ) {}
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
      virtual void setAuthed( bool a ) = 0;
      virtual void connected() = 0;
      void disconnect( ConnectionError ) {}
      const std::string password() const { return "pwd"; }
      const std::string username() const { return "user"; }
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
    void setAuthed( bool a );
    void connected();
  private:
    int m_test;
    int m_result;
};
void NonSaslAuthTest::send( IQ& )
{
}
void NonSaslAuthTest::send( const IQ& iq, IqHandler* ih, int ctx )
{
  if( m_test == 1 )
  {
    const NonSaslAuth::Query* q = iq.findExtension<NonSaslAuth::Query>( ExtNonSaslAuth );
    if( q )
    {
      m_result++;
      m_test = 2;
      IQ re( IQ::Result, iq.from(), iq.id() );
      Tag* d = new Tag( "query" );
      d->setXmlns( XMLNS_AUTH );
      new Tag( d, "username" );
      new Tag( d, "password" );
      new Tag( d, "resource" );
      re.addExtension( new NonSaslAuth::Query( d ) );
      ih->handleIqID( re, ctx );
      delete d;
    }
  }
  else if( m_test == 2 )
  {
    const NonSaslAuth::Query* q = iq.findExtension<NonSaslAuth::Query>( ExtNonSaslAuth );
    if( q )
    {
      Tag* d = q->tag();
      if( d->xml() == "<query xmlns='" + XMLNS_AUTH + "'>"
                      "<username>user</username>"
                      "<password>pwd</password>"
                      "<resource>resource</resource>"
                      "</query>" )
      {
        m_result++;
        IQ re( IQ::Result, iq.from(), iq.id() );
        ih->handleIqID( re, ctx );
      }
      delete d;
    }
  }
  else if( m_test == 3 )
  {
    const NonSaslAuth::Query* q = iq.findExtension<NonSaslAuth::Query>( ExtNonSaslAuth );
    if( q )
    {
      m_result++;
      m_test = 4;
      IQ re( IQ::Result, iq.from(), iq.id() );
      Tag* d = new Tag( "query" );
      d->setXmlns( XMLNS_AUTH );
      new Tag( d, "username" );
      new Tag( d, "digest" );
      new Tag( d, "password" );
      new Tag( d, "resource" );
      re.addExtension( new NonSaslAuth::Query( d ) );
      ih->handleIqID( re, ctx );
      delete d;
    }
  }
  else if( m_test == 4 )
  {
    const NonSaslAuth::Query* q = iq.findExtension<NonSaslAuth::Query>( ExtNonSaslAuth );
    if( q )
    {
      Tag* d = q->tag();
      SHA sha;
      sha.feed( "sid2" );
      sha.feed( "pwd" );
      if( d->xml() == "<query xmlns='" + XMLNS_AUTH + "'>"
          "<username>user</username>"
          "<digest>" + sha.hex() + "</digest>"
          "<resource>resource</resource>"
          "</query>" )
      {
        m_result += 2;
        IQ re( IQ::Result, iq.from(), iq.id() );
        ih->handleIqID( re, ctx );
      }
      delete d;
    }
  }
}
void NonSaslAuthTest::setAuthed( bool a )
{
  if( a )
    m_result++;
}
void NonSaslAuthTest::connected()
{
  m_result++;
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  NonSaslAuthTest* nsat = new NonSaslAuthTest();
  NonSaslAuth n( nsat );

  // -------
  {
    name = "do auth w/ pwd";
    nsat->setTest( 1 );
    n.doAuth( "sid" );
    if( nsat->checkResult() != 4 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "do auth w/ digest";
    nsat->setTest( 3 );
    n.doAuth( "sid2" );
    if( nsat->checkResult() != 5 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }



  delete nsat;


  if( fail == 0 )
  {
    printf( "NonSaslAuth: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "NonSaslAuth: %d test(s) failed\n", fail );
    return 1;
  }

}
