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

#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../iqhandler.h"
#include "../../privacylisthandler.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

const std::string& g_dir = "test.dir";
const std::string& g_inst = "the instructions";
const std::string& g_profile = "test-prof";

gloox::Tag* t1 = 0;
gloox::Tag* t2 = 0;
const gloox::JID to( "abc@def.gh/ijk" );

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

  class ClientBase : public PrivacyListHandler
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

      virtual void handlePrivacyListNames( const std::string& /*active*/, const std::string& /*def*/,
                                           const StringList& /*lists*/ ) {}
      virtual void handlePrivacyList( const std::string& /*name*/, const PrivacyList& /*items*/ ) {}
      virtual void handlePrivacyListChanged( const std::string& /*name*/ ) {}
      virtual void handlePrivacyListResult( const std::string& /*id*/, PrivacyListResult /*plResult*/ ) {}

    private:
      Disco* m_disco;
      int m_test;
      bool m_ok;
  };
  ClientBase::ClientBase() : m_disco( new Disco() ), m_test( 0 ), m_ok( false ) {}
  ClientBase::~ClientBase() { delete m_disco; }
  const std::string ClientBase::getID() { return "id"; }
  Disco* ClientBase::disco() { return m_disco; }
  void ClientBase::send( IQ& iq, IqHandler*, int )
  {
    Tag* tag = iq.tag();
    switch( m_test )
    {
      case 1:
      {
        Tag* si = tag->findChild( "si", "xmlns", XMLNS_SI );
        if( tag->findAttribute( "to" ) == to.full() && si && *(si->findChild( "file" )) == *t1
            && *(si->findChild( "feature" )) == *t2 && si->findAttribute( "mime-type" ) == "binary/octet-stream"
            && si->findAttribute( "profile" ) == g_profile )
          m_ok = true;
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
}

#define CLIENTBASE_H__
#define DISCO_H__
#include "../../privacymanager.h"
#include "../../privacymanager.cpp"
int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  gloox::ClientBase* cb = new gloox::ClientBase();


  // -------
  name = "dummy test";
  if( 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }







  delete cb;

  if( fail == 0 )
  {
    printf( "PrivacyManager: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "PrivacyManager: %d test(s) failed\n", fail );
    return 1;
  }

}
