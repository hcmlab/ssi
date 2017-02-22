/*
  Copyright (c) 2004-2015 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#define GLOOX_TESTS
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../jid.h"
#include "../../mutex.h"
#include "../../mutexguard.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;

  class ClientBase
  {
    public:
      ClientBase() : m_disco( 0 )  {}
      virtual ~ClientBase() {}
      Disco* disco() { return m_disco; }
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ&, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void removeIDHandler( IqHandler* ih );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
    protected:
      Disco* m_disco;
    private:
      JID m_jid;
  };
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::removeIDHandler( IqHandler* ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define DISCO_TEST
#define DISCO_INFO_TEST
#define ADHOC_TEST
#include "../../disco.h"
#include "../../disco.cpp"
#include "../../adhoc.h"
#include "../../adhoc.cpp"
#include "../../adhochandler.h"
#include "../../adhoccommandprovider.h"
class AdhocTest : public ClientBase, public AdhocCommandProvider, public AdhocHandler
{
  public:
    AdhocTest() : m_result( false ) { m_disco = new Disco( this ); }
    ~AdhocTest() { delete m_disco; }
    void setTest( int test ) { m_test = test; }
    void setAdhoc( Adhoc* adhoc ) { m_adhoc = adhoc; }
    virtual void send( IQ& iq );
    virtual void send( const IQ& iq, IqHandler*, int );
    virtual void trackID( IqHandler *ih, const std::string& id, int context );
    virtual void handleAdhocCommand( const JID& /*from*/, const Adhoc::Command& command,
                                     const std::string& /*sess*/ )
    {
      if( m_test == 5 && command.node() == "foocmd" )
        m_result = true;
    }
    virtual void handleAdhocSupport( const JID& /*remote*/, bool support, int /*context*/ )
    {
      if( m_test == 1 || m_test == 2 )
        m_result = support;
    }
    virtual void handleAdhocCommands( const JID& /*remote*/, const StringMap& commands, int /*context*/ )
    {
      if( m_test == 3 && commands.find( "node" ) != commands.end()
          && (*(commands.find( "node" ))).second == "name" )
        m_result = true;
    }
    virtual void handleAdhocError( const JID& /*remote*/, const Error* /*error*/, int /*context*/ ) {}
    virtual void handleAdhocExecutionResult( const JID& /*remote*/, const Adhoc::Command& /*command*/, int /*context*/ )
    {
      if( m_test == 4 )
        m_result = true;
    }
    bool checkResult() { bool t = m_result; m_result = false; return t; }
  private:
    Adhoc* m_adhoc;
    int m_test;
    bool m_result;
};

void AdhocTest::send( IQ& /*iq*/ )
{
}

void AdhocTest::send( const IQ& iq, IqHandler*, int ctx )
{
  switch( m_test )
  {
    case 1: // getSupport()
    {
      Disco::Info i;
      i.m_features.push_back( XMLNS_ADHOC_COMMANDS );
      m_adhoc->handleDiscoInfo( g_jid, i, Adhoc::CheckAdhocSupport );
      break;
    }
    case 2: // getSupport() fails
    {
      Disco::Info i;
      m_adhoc->handleDiscoInfo( g_jid, i, Adhoc::CheckAdhocSupport );
      break;
    }
    case 3: // getCommands()
    {
      Disco::ItemList il;
      il.push_back( new Disco::Item( g_jid, "node", "name" ) );
      Disco::Items i;
      i.setItems( il );
      m_adhoc->handleDiscoItems( g_jid, i, Adhoc::FetchAdhocCommands );
      break;
    }
    case 4: // execute single stage command
    {
      IQ re( IQ::Result, iq.from(), iq.id() );
      re.setFrom( g_jid );
      re.addExtension( new Adhoc::Command( "foocmd", "somesess", Adhoc::Command::Completed, 0 ) );
      m_adhoc->handleIqID( re, ctx );
      break;
    }
  }
}
void AdhocTest::trackID( IqHandler*, const std::string&, int ) {}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  AdhocTest* at = new AdhocTest();
  Adhoc* ah = new Adhoc( at );
  at->setAdhoc( ah );


  // -------
  name = "check support";
  at->setTest( 1 );
  ah->checkSupport( g_jid, at );
  if( !at->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "check support fails";
  at->setTest( 2 );
  ah->checkSupport( g_jid, at );
  if( at->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get commands";
  at->setTest( 3 );
  ah->getCommands( g_jid, at );
  if( !at->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "execute command";
  at->setTest( 4 );
  Adhoc::Command* cmd = new Adhoc::Command( "foocmd", Adhoc::Command::Execute );
  ah->execute( g_jid, cmd, at );
  if( !at->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  ah->registerAdhocCommandProvider( at, "foocmd", "fooname" );

  // -------
  {
    name = "execute local command";
    at->setTest( 5 );
    IQ iq( IQ::Set, g_jid, at->getID() );
    iq.setFrom( g_jid );
    iq.addExtension( new Adhoc::Command( "foocmd", Adhoc::Command::Execute ) );
    ah->handleIq( iq );
    if( !at->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  delete ah;
  delete at;



  if( fail == 0 )
  {
    printf( "Adhoc: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Adhoc: %d test(s) failed\n", fail );
    return 1;
  }

}
