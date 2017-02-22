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
#include "../../sihandler.h"
#include "../../siprofilehandler.h"

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

  class ClientBase : public SIHandler, public SIProfileHandler
  {
    public:
      ClientBase();
      ~ClientBase();
      const std::string getID();
      Disco* disco();
      void send( IQ& iq, IqHandler* = 0 , int = 0 );
      void trackID( IqHandler *ih, const std::string& id, int context );
      void registerIqHandler( IqHandler* ih, int exttype );
      void removeIqHandler( IqHandler* ih, int exttype );
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void removeIDHandler( IqHandler* ) {}
      virtual void handleSIRequestResult( const JID& from, const JID& to, const std::string& sid,
                                          const SIManager::SI& si );
      virtual void handleSIRequestError( const IQ& iq, const std::string& /*sid*/ );
      virtual void handleSIRequest( const JID& from, const JID& to, const std::string& id, const SIManager::SI& si );
      void setTest( int test );
      bool ok();
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
  void ClientBase::handleSIRequestResult( const JID& /*from*/, const JID& /*to*/, const std::string& /*sid*/,
                                          const SIManager::SI& /*si*/ ) {}
  void ClientBase::handleSIRequestError( const IQ& /*iq*/, const std::string& /*sid*/ ) {}
  void ClientBase::handleSIRequest( const JID& /*from*/, const JID& /*to*/, const std::string& /*id*/,
                                    const SIManager::SI& /*si*/ ) {}
  void ClientBase::setTest( int test ) { m_test = test; }
  bool ClientBase::ok() { bool t = m_ok; m_ok = false; return t; }
}

#define CLIENTBASE_H__
#define DISCO_H__
#include "../../simanager.h"
#include "../../simanager.cpp"
int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  t1 = new gloox::Tag( "file", "xmlns", gloox::XMLNS_SI_FT );
  t1->addAttribute( "name", "filename" );
  t1->addAttribute( "size", "1022" );

  t2 = new gloox::Tag( "feature", "xmlns", gloox::XMLNS_FEATURE_NEG );
  t2->addAttribute( "abc", "def" );
  t2->addAttribute( "ghi", "jkl" );

  gloox::SIManager* sim = 0;

  gloox::ClientBase* cb = new gloox::ClientBase();
  sim = new gloox::SIManager( cb, true );


  // -------
  name = "request si";
  cb->setTest( 1 );
  sim->requestSI( cb, to, g_profile, t1->clone(), t2->clone() );
  if( !cb->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }









  delete t1;
  delete t2;
  delete sim;
  delete cb;

  if( fail == 0 )
  {
    printf( "SIManager: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "SIManager: %d test(s) failed\n", fail );
    return 1;
  }

}
