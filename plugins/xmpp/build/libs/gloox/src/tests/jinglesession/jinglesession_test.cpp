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

#define CLIENTBASE_H__
#define DISCO_H__
#define GLOOX_TESTS
#define JINGLE_TEST
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../jid.h"
#include "../../stanzaextension.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID remote_jid( "foo@bar" );

namespace gloox
{
  class Disco
  {
  public:
    Disco() {}
    void addFeature( const std::string& ) {}
  };

  class ClientBase
  {
    public:
      ClientBase() : m_jid( "self" ) {}
      virtual ~ClientBase() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( const IQ& ) = 0;
      virtual void send( IQ&, IqHandler*, int ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void removeIDHandler( IqHandler* ih );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      Disco* disco() { return &m_disco; }
    private:
      JID m_jid;
      Disco m_disco;
  };
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::removeIDHandler( IqHandler* ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define JINGLESESSION_TEST
#include "../../jinglesession.h"
#include "../../jinglesession.cpp"
#include "../../jinglesessionhandler.h"
#include "../../jinglesessionmanager.h"
#include "../../jinglesessionmanager.cpp"
#include "../../jinglecontent.h"
class TestInitiator : public ClientBase, public Jingle::SessionHandler
{
  public:
    TestInitiator() : m_js( new Jingle::Session( this, remote_jid, this ) ), m_result( false ), m_result2( false )
    { m_js->setInitiator( JID( "notself" ) ); }
    virtual ~TestInitiator() { delete m_js; }
    void setTest( int test ) { m_test = test; }
    virtual void send( const IQ& iq );
    virtual void send( IQ& iq, IqHandler*, int );
    virtual void trackID( IqHandler *ih, const std::string& id, int context );
    bool checkResult() { bool t = m_result; m_result = false; return t; }
    bool checkResult2() { bool t = m_result2; m_result2 = false; return t; }
    Jingle::Session* js() { return m_js; }
    virtual void handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle );
    virtual void handleSessionActionError( Jingle::Action action, Jingle::Session* /*session*/, const Error* /*e*/ ) {}
    virtual void handleIncomingSession( Jingle::Session* session ) {}
private:
    Jingle::Session* m_js;
    int m_test;
    bool m_result;
    bool m_result2;
};

class TestResponder : public ClientBase, public Jingle::SessionHandler
{
public:
  TestResponder() : m_js( 0 ), m_result( false ), m_result2( false ),
    m_manager( 0 )
    {}
  virtual ~TestResponder() { delete m_manager; }
  void setTest( int test ) { if( !m_manager ) m_manager = new Jingle::SessionManager( this, this ); m_test = test; }
  virtual void send( const IQ& iq );
  virtual void send( IQ& iq, IqHandler*, int );
  virtual void trackID( IqHandler *ih, const std::string& id, int context );
  bool checkResult() { bool t = m_result; m_result = false; return t; }
  bool checkResult2() { bool t = m_result2; m_result2 = false; return t; }
  Jingle::Session* js() { return m_js; }
  Jingle::SessionManager* sm() { return m_manager; }
  virtual void handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle );
  virtual void handleSessionActionError( Jingle::Action action, Jingle::Session* /*session*/, const Error* /*e*/ ) {}

  virtual void handleIncomingSession( Jingle::Session* session ) { m_js = session; }
private:
  Jingle::Session* m_js;
  int m_test;
  bool m_result;
  bool m_result2;
  Jingle::SessionManager* m_manager;
};

TestInitiator* ini;
TestResponder* res;


void TestInitiator::handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle )
{
//   m_result2 = false;

//   if( m_test == 1 && session->state() == Jingle::Session::Pending )
//     m_result2 = true;
}

void TestInitiator::send( const IQ& iq )
{
//   printf( "TestInitiator::senD(IQ): %s\n", iq.tag()->xml().c_str() );
  m_result2 = false;

  switch( m_test )
  {
    case 2:
    case 3:
      if( iq.subtype() == IQ::Result && iq.to().full() == "foo@bar" )
        m_result2 = true;
      break;
  }
}

void TestInitiator::send( IQ& iq, IqHandler*, int ctx )
{
  m_result = false;
  iq.setFrom( JID( "self" ) );
  Tag* t = iq.tag();
  std::string expected;

//   printf( "TestInitiator: test %d: %s\n", m_test, t->xml().c_str() );

  switch( m_test )
  {
    case 1:
    {
      expected = "<iq to='foo@bar' from='self' id='id' type='set'><jingle xmlns='" + XMLNS_JINGLE + "' action='session-initiate' initiator='notself' sid='somesid'/></iq>";
      if( t->xml() == expected )
        m_result = true;
      else
        fprintf( stderr, "Jingle::Session test %d\nHave:     %s\nExpected: %s\n", m_test, t->xml().c_str(), expected.c_str() );
      res->sm()->handleIq( iq );
      break;
    }
  }

  delete t;
}

void TestInitiator::trackID( IqHandler*, const std::string&, int ) {}

// ------------------------------------------------------------------------------------------------------------

void TestResponder::handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle )
{
  m_result2 = false;

  if( m_test == 1 && session->state() == Jingle::Session::Pending )
    m_result2 = true;
}

void TestResponder::send( const IQ& iq )
{
  m_result2 = false;
//   printf( "TestResponder::senD(IQ): %s\n", iq.tag()->xml().c_str() );

  switch( m_test )
  {
    case 1:
      if( iq.subtype() == IQ::Result && iq.to().full() == "self" )
      {
//         printf( "m_result2 = true;\n" );
        m_result2 = true;
      }
      break;
  }
}

void TestResponder::send( IQ& iq, IqHandler*, int ctx )
{
  m_result = false;
  iq.setFrom( remote_jid );
  Tag* t = iq.tag();
  std::string expected;

//   printf( "TestResponder: test %d: %s\n", m_test, t->xml().c_str() );
  switch( m_test )
  {
    case 1:
      break;
    case 2:
      expected = "<iq to='self' from='foo@bar' id='id' type='set'><jingle xmlns='" + XMLNS_JINGLE + "' action='session-accept' responder='self' sid='somesid'/></iq>";
      if( t->xml() == expected )
        m_result = true;
      else
        fprintf( stderr, "Jingle::Session test %d\nHave:     %s\nExpected: %s\n", m_test, t->xml().c_str(), expected.c_str() );
      ini->js()->handleIq( iq );
      break;
    case 3:
      expected = "<iq to='self' from='foo@bar' id='id' type='set'><jingle xmlns='" + XMLNS_JINGLE + "' action='session-terminate' sid='somesid'><reason><success/></reason></jingle></iq>";
      if( t->xml() == expected )
        m_result = true;
      else
        fprintf( stderr, "Jingle::Session test %d\nHave:     %s\nExpected: %s\n", m_test, t->xml().c_str(), expected.c_str() );
      ini->js()->handleIq( iq );
      break;
    case 4:
    case 5:
    {
      break;
    }
  }

  delete t;
}

void TestResponder::trackID( IqHandler*, const std::string&, int ) {}

// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------


int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ini = new TestInitiator();
  ini->js()->setSID( "somesid" );

  res = new TestResponder();

  // -------
  name = "Initiator: ended (initial) state";
  if( ini->js()->state() != Jingle::Session::Ended )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Initiator: session initiate";
  ini->setTest( 1 );
  res->setTest( 1 );
  Jingle::PluginList pl;
  pl.push_back( new Jingle::Content() );
  if( !ini->js()->sessionInitiate( pl ) || !ini->checkResult() || !res->checkResult2() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Initiator: pending state";
  if( ini->js()->state() != Jingle::Session::Pending )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Responder: pending state";
  if( res->js()->state() != Jingle::Session::Pending )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Responder: session accept";
  ini->setTest( 2 );
  res->setTest( 2 );
  if( !res->js()->sessionAccept( new Jingle::Content() ) || !res->checkResult() || !ini->checkResult2() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Initiator: active state";
  if( ini->js()->state() != Jingle::Session::Active )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Responder: active state";
  if( res->js()->state() != Jingle::Session::Active )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Responder: session terminate";
  ini->setTest( 3 );
  res->setTest( 3 );
  if( !res->js()->sessionTerminate( new Jingle::Session::Reason( Jingle::Session::Reason::Success ) ) || !res->checkResult() || !ini->checkResult2() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Initiator: ended state";
  if( ini->js()->state() != Jingle::Session::Ended )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "Responder: ended state";
  if( res->js()->state() != Jingle::Session::Ended )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }





  delete ini;
  delete res;

  if( fail == 0 )
  {
    printf( "Jingle::Session: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Jingle::Session: %d test(s) failed\n", fail );
    return 1;
  }

}
