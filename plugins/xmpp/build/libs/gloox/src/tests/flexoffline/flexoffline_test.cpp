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
#include "../../disco.h"
#include "../../flexoffhandler.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

const std::string& g_dir = "test.dir";
const std::string& g_inst = "the instructions";
const std::string& g_profile = "test-prof";

gloox::Tag* t1 = 0;
gloox::Tag* t2 = 0;
const gloox::JID g_jid( "abc@def.gh/ijk" );

namespace gloox
{

//   class Disco
//   {
//     public:
//       Disco();
//       ~Disco();
//       void addFeature( const std::string& feature );
//       void removeFeature( const std::string& feature );
//   };
//   Disco::Disco() {}
//   Disco::~Disco() {}
//   void Disco::addFeature( const std::string& /*feature*/ ) {}
//   void Disco::removeFeature( const std::string& /*feature*/ ) {}

  class ClientBase : public FlexibleOfflineHandler
  {
    public:
      ClientBase();
      ~ClientBase();
      const std::string getID();
      Disco* disco();
      void send( IQ& iq, IqHandler* = 0 , int = 0 );
      void trackID( IqHandler *ih, const std::string& id, int context );
      void registerIqHandler( IqHandler *ih, const int ext );
      void removeIqHandler( IqHandler* ih, const int ext );
      void removeIDHandler( IqHandler* ) {}
      void registerStanzaExtension( StanzaExtension* se );
      void removeStanzaExtension( int /*ext*/ ) {}
      virtual void handleFlexibleOfflineSupport( bool /*support*/ ) {}
      virtual void handleFlexibleOfflineMsgNum( int /*num*/ ) {}
      virtual void handleFlexibleOfflineMessageHeaders( const Disco::ItemList& /*headers*/ ) {}
      virtual void handleFlexibleOfflineResult( FlexibleOfflineResult /*foResult*/ ) {}
      const JID& jid() const { return g_jid; }
      void setTest( int test );
      bool ok();
    private:
      Disco* m_disco;
      int m_test;
      bool m_ok;
  };
  ClientBase::ClientBase() : m_disco( new Disco( this ) ), m_test( 0 ), m_ok( false ) {}
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
        break;
      }
    }
    delete tag;
  }
  void ClientBase::trackID( IqHandler* /*ih*/, const std::string& /*id*/, int /*context*/ ) {}
  void ClientBase::registerIqHandler( IqHandler* /*ih*/, const int /*ext*/ ) {}
  void ClientBase::removeIqHandler( IqHandler* /*ih*/, const int /*ext*/ ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* /*se*/ ) {}
  void ClientBase::setTest( int test ) { m_test = test; }
  bool ClientBase::ok() { bool t = m_ok; m_ok = false; return t; }
}

#define CLIENTBASE_H__
#define DISCO_H__
#include "../../disco.cpp"
#include "../../flexoff.h"
#include "../../flexoff.cpp"
int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  gloox::FlexibleOffline* fo = 0;

  gloox::ClientBase* cb = new gloox::ClientBase();
  fo = new gloox::FlexibleOffline( cb );


  // -------
  name = "request si";
  // ...
  if( false )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }









  delete fo;
  delete cb;

  printf( "FlexOffline: " );
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
