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
#include "../../messagehandler.h"
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
  class Disco;

  class ClientBase
  {
    public:
      ClientBase() {}
      virtual ~ClientBase() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ&, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerMessageHandler( MessageHandler* ) {}
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void removeIDHandler( IqHandler* ) {}
      void removeMessageHandler( MessageHandler* ) {}
    private:
      JID m_jid;
  };
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define INBANDBYTESTREAM_TEST
#include "../../inbandbytestream.h"
#include "../../inbandbytestream.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;

  // -------
  {
    name = "empty tag() test";
    InBandBytestream::IBB ibb;
    t = ibb.tag();
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
    name = "open ibb";
    InBandBytestream::IBB ibb( "sid", 4096 );
    t = ibb.tag();
    if( !t || t->xml() != "<open xmlns='" + XMLNS_IBB + "' sid='sid' block-size='4096'/>"
        || ibb.sid() != "sid" || ibb.blocksize() != 4096 || ibb.seq() != 0 || !ibb.data().empty()
         || ibb.type() != InBandBytestream::IBBOpen)
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "data ibb";
    InBandBytestream::IBB ibb( "sid", 4, "data" );
    t = ibb.tag();
    if( !t || t->xml() != "<data xmlns='" + XMLNS_IBB + "' sid='sid' seq='4'>"
                           + Base64::encode64( "data" ) +
                          "</data>"
         || ibb.sid() != "sid" || ibb.seq() != 4 || ibb.data() != "data" || ibb.blocksize() != 0
         || ibb.type() != InBandBytestream::IBBData )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "close ibb";
    InBandBytestream::IBB ibb( "sid" );
    t = ibb.tag();
    if( !t || t->xml() != "<close xmlns='" + XMLNS_IBB + "' sid='sid'/>"
         || ibb.sid() != "sid" || ibb.seq() != 0 || !ibb.data().empty() || ibb.blocksize() != 0
         || ibb.type() != InBandBytestream::IBBClose )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "open ibb from tag";
    Tag* d = new Tag( "open" );
    d->setXmlns( XMLNS_IBB );
    d->addAttribute( "sid", "sid" );
    d->addAttribute( "block-size", 4096 );
    InBandBytestream::IBB ibb( d );
    t = ibb.tag();
    if( !t || *t != *d || ibb.sid() != "sid" || ibb.blocksize() != 4096 || ibb.seq() != 0
        || !ibb.data().empty() || ibb.type() != InBandBytestream::IBBOpen )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete d;
  }

  // -------
  {
    name = "data ibb from tag";
    Tag* d = new Tag( "data" );
    d->setXmlns( XMLNS_IBB );
    d->addAttribute( "sid", "sid" );
    d->addAttribute( "seq", 4 );
    d->setCData( Base64::encode64( "data" ) );
    InBandBytestream::IBB ibb( d );
    t = ibb.tag();
    if( !t || *t != *d || ibb.sid() != "sid" || ibb.seq() != 4 || ibb.data() != "data"
        || ibb.blocksize() != 0 || ibb.type() != InBandBytestream::IBBData )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete d;
  }

  // -------
  {
    name = "close ibb from tag";
    Tag* d = new Tag( "close" );
    d->setXmlns( XMLNS_IBB );
    d->addAttribute( "sid", "sid" );
    InBandBytestream::IBB ibb( d );
    t = ibb.tag();
    if( !t || t->xml() != "<close xmlns='" + XMLNS_IBB + "' sid='sid'/>"
         || ibb.sid() != "sid" || ibb.seq() != 0 || !ibb.data().empty() || ibb.blocksize() != 0
         || ibb.type() != InBandBytestream::IBBClose )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete d;
  }

  StanzaExtensionFactory sef;
  sef.registerExtension( new InBandBytestream::IBB() );
  // -------
  {
    name = "InBandBytestream::IBB/SEFactory test (open)";
    Tag* f = new Tag( "iq" );
    new Tag( f, "open", "xmlns", XMLNS_IBB );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const InBandBytestream::IBB* se = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  // -------
  {
    name = "InBandBytestream::IBB/SEFactory test (data)";
    Tag* f = new Tag( "iq" );
    new Tag( f, "data", "xmlns", XMLNS_IBB );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const InBandBytestream::IBB* se = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  // -------
  {
    name = "InBandBytestream::IBB/SEFactory test (close)";
    Tag* f = new Tag( "iq" );
    new Tag( f, "close", "xmlns", XMLNS_IBB );
    IQ iq( IQ::Set, JID(), "" );
    sef.addExtensions( iq, f );
    const InBandBytestream::IBB* se = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  if( fail == 0 )
  {
    printf( "InBandBytestream::IBB: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "InBandBytestream::IBB: %d test(s) failed\n", fail );
    return 1;
  }

}
