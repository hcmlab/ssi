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
#define IQ_TEST
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../jid.h"
#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

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
    protected:
      JID m_jid;
      Disco m_disco;
      StanzaExtensionFactory m_sef;
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
#include "../../jingleiceudp.h"
#include "../../jinglefiletransfer.h"
class TestInitiator : public ClientBase, public Jingle::SessionHandler
{
  public:
    TestInitiator() : m_sm( this, this ), m_result( false ), m_result2( false )
    {
      m_sef.registerExtension( new Jingle::Session::Jingle() );
      m_sm.registerPlugin( new Jingle::Content() );
      m_sm.registerPlugin( new Jingle::ICEUDP() );
      m_sm.registerPlugin( new Jingle::FileTransfer() );
    }
    virtual ~TestInitiator() {}
    void setTest( int test ) { m_test = test; }
    virtual void send( const IQ&  ) {}
    virtual void send( IQ& , IqHandler*, int ) {}
    virtual void send( Tag* tag );
    virtual void trackID( IqHandler *ih, const std::string& id, int context ) {}
    bool checkResult() { bool t = m_result; m_result = false; return t; }
    bool checkResult2() { bool t = m_result2; m_result2 = false; return t; }
    virtual void handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle );
    virtual void handleSessionActionError( Jingle::Action action, Jingle::Session* /*session*/, const Error* /*e*/ ) {}
    virtual void handleIncomingSession( Jingle::Session* session ) {}
private:
    Jingle::SessionManager m_sm;
    int m_test;
    bool m_result;
    bool m_result2;
};




void TestInitiator::handleSessionAction( Jingle::Action action, Jingle::Session* session, const Jingle::Session::Jingle* jingle )
{
  m_result = false;
  switch( m_test )
  {
    case 1:
      if( action != Jingle::InvalidAction || jingle->plugins().size() != 0 )
      {
        printf( "action: %d, plugins: %d\n", action, jingle->plugins().size() );
      }
      else
        m_result = true;
      break;
    case 2:
      if( action != Jingle::SessionInitiate || jingle->plugins().size() != 1 || static_cast<const Jingle::Content*>( jingle->plugins().front() )->name() != "stub" )
      {
        printf( "action: %d, plugins: %d\n", action, jingle->plugins().size() );
      }
      else
        m_result = true;
      break;
    case 3:
      if( action != Jingle::SessionInitiate || jingle->plugins().size() != 1 || static_cast<const Jingle::Content*>( jingle->plugins().front() )->name() != "iceudp"
        || static_cast<const Jingle::Content*>( jingle->plugins().front() )->plugins().size() != 1
        || static_cast<const Jingle::ICEUDP*>( static_cast<const Jingle::Content*>( jingle->plugins().front() )->plugins().front() )->pwd() != "pwd" )
      {
        printf( "action: %d, plugins: %d\n", action, jingle->plugins().size() );
      }
      else
        m_result = true;
      break;
    case 4:
      if( action != Jingle::SessionInitiate || jingle->plugins().size() != 1 || static_cast<const Jingle::Content*>( jingle->plugins().front() )->name() != "ft"
        || static_cast<const Jingle::Content*>( jingle->plugins().front() )->plugins().size() != 2
        || static_cast<const Jingle::ICEUDP*>( static_cast<const Jingle::Content*>( jingle->plugins().front() )->plugins().front() )->pwd() != "pwd"
        || static_cast<const Jingle::FileTransfer*>( static_cast<const Jingle::Content*>( jingle->plugins().front() )->plugins().back() )->type() != Jingle::FileTransfer::Offer )
      {
        printf( "action: %d, plugins: %d\n", action, jingle->plugins().size() );
      }
      else
        m_result = true;
      break;
  }

}

void TestInitiator::send( Tag* tag )
{
  IQ iq( tag );
  m_sef.addExtensions( iq, tag );
  m_sm.handleIq( iq );
}


// ------------------------------------------------------------------------------------------------------------



// ------------------------------------------------------------------------------------------------------------


int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  TestInitiator ini;
  Tag* i = new Tag( "iq" ); i->addAttribute( "from", "me@there" ); i->addAttribute( "to", "you@here" ); i->addAttribute( "type", "set" ); i->addAttribute( "id", "someid" );
  Tag* j = new Tag( i, "jingle", XMLNS, XMLNS_JINGLE );
  Tag* c = 0;

  // -------
  name = "invalid jingle";
  ini.setTest( 1 );
  ini.send( i );
  if( !ini.checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "jingle with empty content";
  ini.setTest( 2 );
  j->addAttribute( "action", "session-initiate" );
  c = new Tag( j, "content" ); c->addAttribute( "creator", "initor" ); c->addAttribute( "name", "stub" );
  ini.send( i );
  if( !ini.checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "jingle with content and transport";
  ini.setTest( 3 );
  c->addAttribute( "name", "iceudp" );
  Tag* t = new Tag( c, "transport", XMLNS, XMLNS_JINGLE_ICE_UDP ); t->addAttribute( "pwd", "pwd" ); t->addAttribute( "ufrag", "ufrag" );
  ini.send( i );
  if( !ini.checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "jingle with content and transport + description";
  ini.setTest( 4 );
  c->addAttribute( "name", "ft" );
  Tag* d = new Tag( c, "description", XMLNS, XMLNS_JINGLE_FILE_TRANSFER ); new Tag( new Tag( d, "offer" ), "file" );
  ini.send( i );
  if( !ini.checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }



  delete i;


  if( fail == 0 )
  {
    printf( "Jingle::SessionManager: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Jingle::SessionManager: %d test(s) failed\n", fail );
    return 1;
  }

}
