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
#include "../../gloox.h"
#include "../../jid.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../privatexmlhandler.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

Tag* t1 = 0;
Tag* t2 = 0;
const JID to( "abc@def.gh/ijk" );

namespace gloox
{

  class Disco
  {
    public:
      Disco();
      ~Disco();
      void addFeature( const std::string& feature );
      void removeFeature( const std::string& feature );
  };
  Disco::Disco() {}
  Disco::~Disco() {}
  void Disco::addFeature( const std::string& /*feature*/ ) {}
  void Disco::removeFeature( const std::string& /*feature*/ ) {}

  class ClientBase : public PrivateXMLHandler
  {
    public:
      ClientBase();
      ~ClientBase();
      const std::string getID();
      Disco* disco();
      void send( IQ& iq, IqHandler* = 0 , int = 0 );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void trackID( IqHandler *ih, const std::string& id, int context );
      void removeIqHandler( IqHandler* ih, int exttype );
      void removeIDHandler( IqHandler* ) {}
      void setTest( int test );
      bool ok();

      virtual void handlePrivateXML( const Tag* xml );
      virtual void handlePrivateXMLResult( const std::string& uid, PrivateXMLResult pxResult );

    private:
      Disco* m_disco;
      int m_test;
      bool m_ok;
  };

}

#define CLIENTBASE_H__
#define DISCO_H__
#define PRIVATEXML_TEST
#include "../../privatexml.h"
#include "../../privatexml.cpp"

namespace gloox
{
  ClientBase::ClientBase() : m_disco( new Disco() ), m_test( 0 ), m_ok( false ) {}
  ClientBase::~ClientBase() { delete m_disco; }
  const std::string ClientBase::getID() { return "id"; }
  Disco* ClientBase::disco() { return m_disco; }
  void ClientBase::send( IQ& iq, IqHandler* ih, int ctx )
  {
    const PrivateXML::Query* q = iq.findExtension<PrivateXML::Query>( ExtPrivateXML );
    if( !q )
      return;

    Tag* tag = q->tag();
    switch( m_test )
    {
      case 1:
      {
        if( iq.subtype() == IQ::Set
            && tag && *(tag->findChild( "foo", "xmlns", "test" )) == *t1 )
        {
          IQ re( IQ::Result, iq.from(), iq.id() );
          re.addExtension( new PrivateXML::Query() );
          ih->handleIqID( re, ctx );
        }
        break;
      }
      case 2:
      {
        if( iq.subtype() == IQ::Get
            && tag && tag->hasChild( "foo", "xmlns", "test" ) )
        {
          IQ re( IQ::Result, iq.from(), iq.id() );
          re.addExtension( new PrivateXML::Query( t1->clone() ) );
          ih->handleIqID( re, ctx );
        }
        break;
      }
    }
    delete tag;
  }
  void ClientBase::trackID( IqHandler* /*ih*/, const std::string& /*id*/, int /*context*/ ) {}
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  void ClientBase::setTest( int test ) { m_test = test; }
  bool ClientBase::ok() { bool t = m_ok; m_ok = false; return t; }
  void ClientBase::handlePrivateXML( const Tag* xml )
  {
    switch( m_test )
    {
      case 1:
        m_ok = false;
        break;
      case 2:
        if( *xml == *t1 )
          m_ok = true;
        break;
    }
  }
  void ClientBase::handlePrivateXMLResult( const std::string& /*uid*/, PrivateXMLResult pxResult )
  {
    switch( m_test )
    {
      case 1:
        if( pxResult == PrivateXMLHandler::PxmlStoreOk )
          m_ok = true;
        break;
      case 2:
        m_ok = false;
        break;
    }
  }

}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  t1 = new Tag( "foo" );
  t1->setXmlns( "test" );
  new Tag( t1, "foobar" );
  ClientBase* cb = new ClientBase();
  PrivateXML px( cb );

  // -------
  {
    name = "store xml";
    cb->setTest( 1 );
    px.storeXML( t1->clone(), cb );
    if( !cb->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

    // -------
  {
    name = "request xml";
    cb->setTest( 2 );
    px.requestXML( "foo", "test", cb );
    if( !cb->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  StanzaExtensionFactory sef;
  sef.registerExtension( new PrivateXML::Query() );
  // -------
  {
    name = "Query/SEFactory test";
    Tag* f = new Tag( "iq" );
    new Tag( f, "query", "xmlns", XMLNS_PRIVATE_XML );
    IQ iq( IQ::Get, JID(), "" );
    sef.addExtensions( iq, f );
    const PrivateXML::Query* se = iq.findExtension<PrivateXML::Query>( ExtPrivateXML );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  delete t1;
  delete cb;

  printf( "PrivateXML: " );
  if( fail == 0 )
  {
    printf( "OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "%d test(s) failed\n", fail );
    return 1;
  }

}
