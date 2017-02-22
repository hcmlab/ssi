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
#include "../../base64.h"
#include "../../stanzaextensionfactory.h"
#include "../../disco.h"
#include "../../lastactivityhandler.h"
using namespace gloox;

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
      ClientBase() : m_disco( new Disco( this ) ) {}
      virtual ~ClientBase() { delete m_disco; }
      Disco* disco();
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
    private:
      Disco* m_disco;
      JID m_jid;
  };
  Disco* ClientBase::disco() { return m_disco; }
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define LASTACTIVITY_TEST
#include "../../disco.cpp"
#include "../../lastactivity.h"
#include "../../lastactivity.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  {
    name = "empty tag() test";
    LastActivity::Query laq;
    if( false )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "parse test";
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_LAST );
    t->addAttribute( "seconds", "123" );
    t->setCData( "foo" );
    LastActivity::Query laq( t );
    if( laq.seconds() != 123 || laq.status() != "foo" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }

  // -------
    name = "tag() test"; // uses t from previous test
    Tag* s = laq.tag();
    if( *t != *s )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete s;
  }




  StanzaExtensionFactory sef;
  sef.registerExtension( new LastActivity::Query() );
  // -------
  {
    name = "LastActivity::Query/SEFactory test";
    Tag* f = new Tag( "iq" );
    new Tag( f, "query", "xmlns", XMLNS_LAST );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const LastActivity::Query* se = iq.findExtension<LastActivity::Query>( ExtLastActivity );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "LastActivity::Query: " );
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
