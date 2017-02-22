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

#define JINGLE_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

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
      ClientBase() : m_jid( "fooqbar/foobar" ) {}
      virtual ~ClientBase() {}
      const std::string getID() { return "id"; }
      virtual void send( IQ& iq, IqHandler*, int ) = 0;
      virtual void send( IQ& iq ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIDHandler( IqHandler* ) {}
      void registerIqHandler( IqHandler*, int ) {}
      void removeIqHandler( IqHandler*, int ) {}
      void registerStanzaExtension( StanzaExtension* ext ) { delete ext; }
      void removeStanzaExtension( int ) {}
      ConnectionState state() const { return StateConnected; }
      bool authed() { return false; }
      const JID& jid() const { return m_jid; }
    private:
      JID m_jid;
  };
}

#define CLIENTBASE_H__
#include "../../jinglesession.h"
#include "../../jinglesession.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  JID jid( "foo@bar/barfoo" );

  // -------
  {
    name = "invalid Jingle 1";
    Jingle::Session::Jingle js;
    Tag* t = js.tag();
    if( t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "invalid Jingle 2";
    Jingle::Session::Jingle js( Jingle::SessionAccept, JID(), JID(), 0, "" );
    Tag* t = js.tag();
    if( t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-accept";
    Jingle::Session::Jingle js( Jingle::ContentAccept, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='content-accept' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-add";
    Jingle::Session::Jingle js( Jingle::ContentAdd, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='content-add' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-modify";
    Jingle::Session::Jingle js( Jingle::ContentModify, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='content-modify' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-reject";
    Jingle::Session::Jingle js( Jingle::ContentReject, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='content-reject' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, content-remove";
    Jingle::Session::Jingle js( Jingle::ContentRemove, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='content-remove' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, description-info";
    Jingle::Session::Jingle js( Jingle::DescriptionInfo, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='description-info' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, session-accept";
    Jingle::Session::Jingle js( Jingle::SessionAccept, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='session-accept' responder='foo@bar/barfoo' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, session-info";
    Jingle::Session::Jingle js( Jingle::SessionInfo, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='session-info' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, session-initiate";
    Jingle::Session::Jingle js( Jingle::SessionInitiate, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='session-initiate' initiator='foo@bar/barfoo' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, session-terminate";
    Jingle::Session::Jingle js( Jingle::SessionTerminate, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='session-terminate' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, transport-accept";
    Jingle::Session::Jingle js( Jingle::TransportAccept, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='transport-accept' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, transport-info";
    Jingle::Session::Jingle js( Jingle::TransportInfo, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='transport-info' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, transport-reject";
    Jingle::Session::Jingle js( Jingle::TransportReject, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='transport-reject' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "empty Jingle, transport-replace";
    Jingle::Session::Jingle js( Jingle::TransportReplace, jid, jid, 0, "somesid" );
    Tag* t = js.tag();
    if( !t || t->xml() != "<jingle xmlns='" + XMLNS_JINGLE + "' "
      "action='transport-replace' sid='somesid'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }




  // -------
  name = "Jingle::Session::Jingle/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Jingle::Session::Jingle() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "jingle", "xmlns", XMLNS_JINGLE );
  IQ iq( IQ::Get, JID() );
  sef.addExtensions( iq, f );
  const Jingle::Session::Jingle* se = iq.findExtension<Jingle::Session::Jingle>( ExtJingle );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;



  if( fail == 0 )
  {
    printf( "Jingle::Session::Jingle: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Jingle::Session::Jingle: %d test(s) failed\n", fail );
    return 1;
  }

}
