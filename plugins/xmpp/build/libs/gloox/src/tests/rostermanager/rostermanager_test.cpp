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

#define GLOOX_TESTS
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../presencehandler.h"
#include "../../subscriptionhandler.h"
#include "../../jid.h"
#include "../../stanzaextension.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;
  class Capabilities : public StanzaExtension
  {
    public:
      Capabilities() : StanzaExtension( ExtUser + 1 ) {}
      const std::string& ver() const { return EmptyString; }
      const std::string& node() const { return EmptyString; }
  };

  class ClientBase
  {
    public:
      ClientBase() : m_jid( "self" ) {}
      virtual ~ClientBase() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void rosterFilled() = 0;
      virtual void send( const Subscription& ) = 0;
      virtual void send( const IQ& ) {};
      virtual void send( const IQ&, IqHandler*, int ) {};
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void removeIDHandler( IqHandler* ih );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerPresenceHandler( PresenceHandler* /*ph*/ ) {}
      void removePresenceHandler( PresenceHandler* /*ph*/ ) {}
      void registerSubscriptionHandler( SubscriptionHandler* /*ph*/ ) {}
      void removeSubscriptionHandler( SubscriptionHandler* /*ph*/ ) {}
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
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
#define CAPABILITIES_H__
#define ROSTERMANAGER_TEST
#include "../../privatexml.h"
#include "../../privatexml.cpp"
#include "../../rostermanager.h"
#include "../../rostermanager.cpp"
#include "../../rosterlistener.h"
class RosterManagerTest : public ClientBase, public RosterListener
{
  public:
    RosterManagerTest() : m_result( false ), m_result2( false ) {}
    ~RosterManagerTest() {}
    void setTest( int test ) { m_test = test; }
    virtual void send( IQ& iq );
    virtual void send( const IQ& iq, IqHandler*, int );
    virtual void send( const Subscription& );
    virtual void rosterFilled() { if( m_test == 1 ) m_result = true; }
    virtual void trackID( IqHandler *ih, const std::string& id, int context );
    bool checkResult() { bool t = m_result; m_result = false; return t; }
    bool checkResult2() { bool t = m_result2; m_result2 = false; return t; }
    void setRM( RosterManager* rm ) { m_rm = rm; }
    virtual void handleItemAdded( const JID& jid )
    {
      if( m_test == 2 && jid.full() == "foo" )
        m_result = true;
    }
    virtual void handleItemSubscribed( const JID& /*jid*/ )
    {
      if( m_test == 5 )
        m_result = true;
    }
    virtual void handleItemRemoved( const JID& jid )
    {
      if( m_test == 4 && jid.full() == "foo" )
        m_result = true;
    }
    virtual void handleItemUpdated( const JID& jid )
    {
      if( m_test == 3 && jid.full() == "foo" )
        m_result = true;
    }
    virtual void handleItemUnsubscribed( const JID& /*jid*/ )
    {
      if( m_test == 6 )
        m_result = true;
    }
    virtual void handleRoster( const Roster& roster )
    {
      if( m_test == 1 && roster.size() == 3 )
        m_result2 = true;
      else
        printf("rostersize: %d\n", roster.size() );
    }
    virtual void handleRosterPresence( const RosterItem& /*item*/, const std::string& /*resource*/,
                                       Presence::PresenceType /*presence*/, const std::string& /*msg*/ ) {}
    virtual void handleSelfPresence( const RosterItem& /*item*/, const std::string& /*resource*/,
                                     Presence::PresenceType /*presence*/, const std::string& /*msg*/ ) {}
    virtual bool handleSubscriptionRequest( const JID& /*jid*/, const std::string& /*msg*/ )
    {
      if( m_test == 7 )
      {
        m_result = true;
        return true;
      }
      else if( m_test == 9 )
      {
        m_result = true;
        return false;
      }
      return false;
    }
    virtual bool handleUnsubscriptionRequest( const JID& /*jid*/, const std::string& /*msg*/ )
    {
      if( m_test == 8 )
      {
        m_result = true;
        return true;
      }
      return false;
    }
    virtual void handleNonrosterPresence( const Presence& /*presence*/ ) {}
    virtual void handleRosterError( const IQ& /*iq*/ ) {}
  private:
    RosterManager* m_rm;
    int m_test;
    bool m_result;
    bool m_result2;
};

void RosterManagerTest::send( IQ& /*iq*/ )
{
}

void RosterManagerTest::send( const IQ& iq, IqHandler*, int ctx )
{
  switch( m_test )
  {
    case 1: // fill()
    {
      IQ re( IQ::Result, JID(), iq.id() );
      Tag* r = new Tag( "iq" );
      Tag* q = new Tag( r, "query" );
      q->setXmlns( XMLNS_ROSTER );
      Tag* i = new Tag( q, "item", "jid", "foo@bar" ); i->addAttribute( "subscription", "both" );
      i = new Tag( q, "item", "jid", "bar@foo" ); i->addAttribute( "subscription", "both" );
      i = new Tag( q, "item", "jid", "foobar" ); i->addAttribute( "subscription", "both" );
      re.addExtension( new RosterManager::Query( q ) );
      delete r;
      m_rm->handleIqID( re, ctx );
      m_test = 0;
      break;
    }
    case 2: // add item
    case 3: // synchronize update
    case 4: // remove item
    case 5: // subscribe item
    {
      IQ re( IQ::Result, JID(), iq.id() );
      m_rm->handleIqID( re, ctx );

      IQ set( IQ::Set, JID(), getID() );
      const RosterManager::Query* q = iq.findExtension<RosterManager::Query>( ExtRoster );
      Tag* t = q->tag();
      set.addExtension( new RosterManager::Query( t ) );
      delete t;
      m_rm->handleIq( set );
      break;
    }
  }
}

void RosterManagerTest::send( const Subscription& s10n )
{
  if( m_test == 5 )
  {
    Subscription s( Subscription::Subscribed, s10n.from() );
    m_rm->handleSubscription( s );
  }
  else if( m_test == 6 )
  {
    Subscription s( Subscription::Unsubscribed, s10n.from() );
    m_rm->handleSubscription( s );

    IQ iq( IQ::Set, JID(), getID() );
    Tag* q = new Tag( "query" );
    q->setXmlns( XMLNS_ROSTER );
    Tag* i = new Tag( q, "item", "jid", "blah" );
    i->addAttribute( "subscription", "remove" );
    iq.addExtension( new RosterManager::Query( q ) );
    delete q;
    m_rm->handleIq( iq );
  }
  else if( m_test == 7 )
  {
    Tag* s = s10n.tag();
    if( s->xml() == "<presence to='foobar' type='subscribed'/>" )
      m_result2 = true;
    delete s;
  }
  else if( m_test == 8 )
  {
    Tag* s = s10n.tag();
    if( s->xml() == "<presence to='foobar' type='unsubscribed'/>" )
      m_result2 = true;
    delete s;
  }
  else if( m_test == 9 )
  {
    Tag* s = s10n.tag();
    if( s->xml() == "<presence to='foobar' type='unsubscribed'/>" )
      m_result2 = true;
    delete s;
  }
}

void RosterManagerTest::trackID( IqHandler*, const std::string&, int ) {}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  RosterManagerTest* rmt = new RosterManagerTest();
  RosterManager* rm = new RosterManager( rmt );
  rm->registerRosterListener( rmt );
  rmt->setRM( rm );


  // -------
  name = "request roster";
  rmt->setTest( 1 );
  rm->fill();
  if( !rmt->checkResult() || !rmt->checkResult2() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "add item";
  rmt->setTest( 2 );
  StringList gl;
  gl.push_back( "foogroup" );
  rm->add( JID( "foo" ), "fooname", gl );
  if( !rmt->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get item";
  RosterItem* ri = rm->getRosterItem( JID( "foo" ) );
  if( !ri || ri->name() != "fooname" || ri->groups().size() != 1 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  if( ri )
  {
    name = "update item";
    ri->setName( "foof" );
    gl.clear();
    gl.push_back( "f1" );
    gl.push_back( "f2" );
    gl.push_back( "f3" );
    ri->setGroups( gl );
  }
  if( !ri || ri->name() != "foof" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }


  // -------
  name = "synchronize item";
  rmt->setTest( 3 );
  rm->synchronize();
  if( !rmt->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "re-check item";
  ri = rm->getRosterItem( JID( "foo" ) );
  if( !ri || ri->name() != "foof" || ri->groups().size() != 3 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "remove item";
  rmt->setTest( 4 );
  rm->remove( JID( "foo" ) );
  if( !rmt->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "re-check removal";
  ri = rm->getRosterItem( JID( "foo" ) );
  if( ri )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), ri->jidJID().full().c_str() );
  }

  // -------
  name = "subscribe to contact";
  rmt->setTest( 5 );
  rm->subscribe( JID( "blah" ) );
  if( !rmt->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get subscribed item";
  ri = rm->getRosterItem( JID( "blah" ) );
  if( !ri || ri->name() != "" || ri->groups().size() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "unsubscribe from contact";
  rmt->setTest( 6 );
  rm->unsubscribe( JID( "blah" ) );
  if( !rmt->checkResult() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "re-check unsubscribe";
  ri = rm->getRosterItem( JID( "blah" ) );
  if( ri )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), ri->jidJID().full().c_str() );
  }

  // -------
  {
    name = "receive subscribe request (accept)";
    rmt->setTest( 7 );
    Subscription s( Subscription::Subscribe, JID(), EmptyString, EmptyString );
    s.setFrom( JID( "foobar" ) );
    rm->handleSubscription( s );
    if( !rmt->checkResult() || !rmt->checkResult2() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "receive unsubscribe request";
    rmt->setTest( 8 );
    Subscription s( Subscription::Unsubscribe, JID(), EmptyString, EmptyString );
    s.setFrom( JID( "foobar" ) );
    rm->handleSubscription( s );
    if( !rmt->checkResult() || !rmt->checkResult2() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "receive subscribe request (reject)";
    rmt->setTest( 9 );
    Subscription s( Subscription::Subscribe, JID(), EmptyString, EmptyString );
    s.setFrom( JID( "foobar" ) );
    rm->handleSubscription( s );
    if( !rmt->checkResult() || !rmt->checkResult2() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }







  delete rm;
  delete rmt;



  if( fail == 0 )
  {
    printf( "RosterManager: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "RosterManager: %d test(s) failed\n", fail );
    return 1;
  }

}
