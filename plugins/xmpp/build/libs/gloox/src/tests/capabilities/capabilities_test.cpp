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
#include "../../jid.h"

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
      virtual void send( IQ& ) {};
      virtual void send( const IQ&, IqHandler*, int ) {};
      virtual void trackID( IqHandler *, const std::string&, int ) {};
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
#include "../../capabilities.h"
#include "../../capabilities.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ClientBase cb;
  Disco d( &cb );
  Capabilities c( &d );


  // -------
  name = "simple ver check";
  d.setIdentity( "client", "pc", "Exodus 0.9.1");
  d.removeFeature( "jabber:iq:version" );
  d.addFeature( "http://jabber.org/protocol/muc");
  d.addFeature( "http://jabber.org/protocol/caps" );
  if( c.ver() != "QgayPKawpkPSDYmwT/WM94uAlu0=" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }




  printf( "Capabilities: " );
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
