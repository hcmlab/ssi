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
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID g_jid( "foof" );

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

  class ClientBase
  {
    public:
      ClientBase() {}
      virtual ~ClientBase() {}
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
#define DISCO_H__
#define SIMANAGER_TEST
#include "../../simanager.h"
#include "../../simanager.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;

  // -------
  {
    name = "empty tag() test";
    SIManager::SI si;
    t = si.tag();
    if( t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "full ctor";
    Tag* t1 = new Tag( "foo" );
    Tag* t2 = new Tag( "bar" );
    SIManager::SI si( t1, t2, "id", "mime", "prof" );
    t = si.tag();
    if( !t || t->name() != "si" || t->xmlns() != XMLNS_SI
        || !t->hasAttribute( "id", "id" )
        || !t->hasAttribute( "mime-type", "mime" )
        || !t->hasAttribute( "profile", "prof" )
        || !t->hasChild( "foo" )
        || !t->hasChild( "bar" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "tag ctor";
    Tag* s = new Tag( "si" );
    s->setXmlns( XMLNS_SI );
    s->addAttribute( "id", "id" );
    s->addAttribute( "mime-type", "mime" );
    s->addAttribute( "profile", "prof" );
    Tag* f1 = new Tag( s, "file" );
    f1->setXmlns( XMLNS_SI_FT );
    Tag* f2 = new Tag( s, "feature" );
    f2->setXmlns( XMLNS_FEATURE_NEG );

    SIManager::SI si( s );
    t = si.tag();
    if( !t || *t != *s )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:\n%s\n%s\n", name.c_str(), t->xml().c_str(), s->xml().c_str() );
    }
    delete s;
    delete t;
    t = 0;
  }

  StanzaExtensionFactory sef;
  sef.registerExtension( new SIManager::SI() );
  // -------
  {
    name = "SIManager::SI/SEFactory test";
    Tag* f = new Tag( "iq" );
    new Tag( f, "si", "xmlns", XMLNS_SI );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const SIManager::SI* se = iq.findExtension<SIManager::SI>( ExtSI );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "SIManager::SI: " );
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
