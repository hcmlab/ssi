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

#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../jid.h"
#include "../../error.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

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
      virtual void send( const IQ& iq ) = 0;
      virtual void send( const IQ& iq, IqHandler*, int ) = 0;
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
  void ClientBase::registerStanzaExtension( StanzaExtension* ext ) { delete ext; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define DISCO_TEST
#define DISCO_INFO_TEST
#define DISCO_ITEMS_TEST
#include "../../disco.h"
#include "../../disco.cpp"
#include "../../discohandler.h"
#include "../../disconodehandler.h"
class DiscoTest : public ClientBase, public DiscoHandler, public DiscoNodeHandler
{
  public:
    DiscoTest() : m_result( false ) {}
    ~DiscoTest() {}
    void setTest( int test ) { m_test = test; }
    void setDisco( Disco* disco ) { m_disco = disco; }
    virtual void send( const IQ& iq );
    virtual void send( const IQ& iq, IqHandler*, int );
    virtual void trackID( IqHandler *ih, const std::string& id, int context );
    virtual StringList handleDiscoNodeFeatures( const JID&, const std::string& )
    {
      StringList sl;
      if( m_test == 6 )
        sl.push_back( "test6-feature" );
      return sl;
    }
    virtual Disco::IdentityList handleDiscoNodeIdentities( const JID&,
        const std::string& )
    {
      Disco::IdentityList dil;
      if( m_test == 6 )
        dil.push_back( new Disco::Identity( "6cat", "6type", "6name" ) );
      return dil;
    }
    virtual Disco::ItemList handleDiscoNodeItems( const JID&,
        const JID&, const std::string& = EmptyString )
    {
      Disco::ItemList dil;
      if( m_test == 7 )
        dil.push_back( new Disco::Item( JID( "6jid" ), "6node", "6name" ) );
      return dil;
    }

    virtual void handleDiscoInfo( const JID& /*from*/, const Disco::Info& info, int /*context*/ )
    {
      if( m_test == 8 && info.hasFeature( XMLNS_DISCO_INFO ) && info.hasFeature( "foofeature" )
          && info.hasFeature( "foofeature2" ) )
        m_result = true;
    }
    virtual void handleDiscoInfoResult( IQ*, int ) {} // FIXME remove for 1.1
    virtual void handleDiscoItems( const JID& /*from*/, const Disco::Items& items, int /*context*/ )
    {
      if( m_test == 9 && items.node() == "foonode" && items.items().size() == 2 )
        m_result = true;
    }
    virtual void handleDiscoItemsResult( IQ*, int ) {} // FIXME remove for 1.1
    virtual void handleDiscoError( const JID& /*from*/, const Error* error, int /*context*/ )
    {
      if( m_test == 10 && error && error->error() == StanzaErrorItemNotFound )
        m_result = true;
    }
    virtual void handleDiscoError( IQ*, int ) {} // FIXME remove for 1.1
    virtual bool handleDiscoSet( IQ* iq ) { (void)iq; return false; }
    bool checkResult() { bool t = m_result; m_result = false; return t; }
  private:
    Disco* m_disco;
    int m_test;
    bool m_result;
};

void DiscoTest::send( const IQ& iq )
{
//   printf( "send: %s\n", iq.tag()->xml().c_str() );
  if( m_test == 1 )
  {
    const Disco::Info* se = iq.findExtension<Disco::Info>( ExtDiscoInfo );
    if( se && se->hasFeature( "foofeature" ) && se->hasFeature( "foofeature2" )
        && se->identities().size() == 2 )
    {
      m_result = true;
    }
  }
  else if( m_test == 2 )
  {
    const Disco::Info* se = iq.findExtension<Disco::Info>( ExtDiscoInfo );
    if( se && se->hasFeature( "foofeature" ) && !se->hasFeature( "foofeature2" )
        && se->identities().size() == 1 )
    {
      m_result = true;
    }
  }
  else if( m_test == 3 )
  {
    const Disco::Items* se = iq.findExtension<Disco::Items>( ExtDiscoItems );
    if( se && se->node().empty() && se->items().size() == 0 )
    {
      m_result = true;
    }
  }
  else if( m_test == 4 )
  {
    const Error* e = iq.findExtension<Error>( ExtError );
    if( e && e->error() == StanzaErrorItemNotFound )
    {
      m_result = true;
    }
  }
  else if( m_test == 5 )
  {
    const Error* e = iq.findExtension<Error>( ExtError );
    if( e && e->error() == StanzaErrorItemNotFound )
    {
      m_result = true;
    }
  }
  else if( m_test == 6 )
  {
    const Disco::Info* se = iq.findExtension<Disco::Info>( ExtDiscoInfo );
    if( se && se->node() == "foonode" && se->hasFeature( XMLNS_DISCO_INFO )
        && se->hasFeature( XMLNS_DISCO_ITEMS )
        && se->hasFeature( "test6-feature" ) && se->features().size() == 3 )
    {
      m_result = true;
    }
  }
  else if( m_test == 7 )
  {
    const Disco::Items* se = iq.findExtension<Disco::Items>( ExtDiscoItems );
    if( se && se->node() == "foonode" && se->items().size() == 1 )
    {
      m_result = true;
    }
  }
}
void DiscoTest::send( const IQ& iq, IqHandler*, int ctx )
{
  Tag* q = 0;
  if( m_test == 8 )
  {
    IQ re( IQ::Result, iq.from(), iq.id() );
    q = new Tag( "query" );
    q->setXmlns( XMLNS_DISCO_INFO );
    new Tag( q, "feature", "var", XMLNS_DISCO_INFO );
    new Tag( q, "feature", "var", "foofeature" );
    new Tag( q, "feature", "var", "foofeature2" );
    re.addExtension( new Disco::Info( q ) );
    m_disco->handleIqID( re, ctx );
  }
  else if( m_test == 9 )
  {
    IQ re( IQ::Result, iq.from(), iq.id() );
    q = new Tag( "query" );
    q->setXmlns( XMLNS_DISCO_ITEMS );
    q->addAttribute( "node", "foonode" );
    Tag* i = new Tag( q, "item", "jid", "jid1" );
    i->addAttribute( "node", "node1" );
    i->addAttribute( "name", "name1" );
    i = new Tag( q, "item", "jid", "jid2" );
    i->addAttribute( "node", "node2" );
    i->addAttribute( "name", "name2" );
    re.addExtension( new Disco::Items( q ) );
    m_disco->handleIqID( re, ctx );
  }
  else if( m_test == 10 )
  {
    IQ re( IQ::Error, iq.from(), iq.id() );
    re.addExtension( new Error( StanzaErrorTypeCancel, StanzaErrorItemNotFound ) );
    m_disco->handleIqID( re, ctx );
  }

  delete q;
}
void DiscoTest::trackID( IqHandler*, const std::string&, int ) {}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  DiscoTest* dt = new DiscoTest();
  Disco* d = new Disco( dt );
  dt->setDisco( d );


  // -------
  {
    name = "disco info";
    dt->setTest( 1 );
    d->addFeature( "foofeature" );
    d->addFeature( "foofeature2" );
    d->setIdentity( "foocat", "footype", "fooname" );
    d->addIdentity( "foocat2", "footype2", "fooname2" );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Info() );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "disco info";
    dt->setTest( 2 );
    d->removeFeature( "foofeature2" );
    d->setIdentity( "foocat", "footype", "fooname" );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Info() );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "disco items";
    dt->setTest( 3 );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Items() );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "info for non-existant node";
    dt->setTest( 4 );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Info( "foonode" ) );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "items for non-existant node";
    dt->setTest( 5 );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Items( "foonode" ) );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "info for a specific node";
    dt->setTest( 6 );
    d->registerNodeHandler( dt, "foonode" );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Info( "foonode" ) );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "items for a specific node";
    dt->setTest( 7 );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Items( "foonode" ) );
    d->handleIq( iq );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "getDiscoInfo()";
    dt->setTest( 8 );
    d->getDiscoInfo( JID( "foof" ), EmptyString, dt, 0 );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "getDiscoItems()";
    dt->setTest( 9 );
    d->getDiscoItems( JID( "foof" ), "foonode", dt, 0 );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "getDiscoInfo() error";
    dt->setTest( 10 );
    d->getDiscoInfo( JID( "foof" ), "fooof", dt, 0 );
    if( !dt->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "remove node handlers";
    dt->setTest( 7 ); // intentional
    d->removeNodeHandlers( dt );
    IQ iq( IQ::Get, JID(), dt->getID() );
    iq.addExtension( new Disco::Items( "foonode" ) );
    d->handleIq( iq );
    if( dt->checkResult() ) // should fail, as there's no handler for foonode anymore
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }





  delete d;
  delete dt;


  if( fail == 0 )
  {
    printf( "Disco: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Disco: %d test(s) failed\n", fail );
    return 1;
  }

}
