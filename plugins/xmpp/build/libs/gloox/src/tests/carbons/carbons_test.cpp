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
#include "../../delayeddelivery.h"
#include "../../forward.h"
#include "../../message.h"
#include "../../messagehandler.h"
#include "../../client.h"
#include "../../attention.h"
#include "../../carbons.h"

#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

using namespace gloox;


  class CarbonsTest : public MessageHandler
  {
    public:
      CarbonsTest() : cb( "foo" )
      {
        cb.registerMessageHandler( this );
        cb.registerStanzaExtension( new Forward() );
        cb.registerStanzaExtension( new Carbons() );
        cb.registerStanzaExtension( new Attention() );
      }
      ~CarbonsTest() {}
      void testTag( Tag* tag ) { cb.handleTag( tag ); }
      virtual void handleMessage( const Message& msg, MessageSession* )
      {
        Tag* m = msg.tag();
        m_xml = m->xml();
//         printf( "msg: %s\n", m->xml().c_str() );
        delete m;
      }
      const std::string& getXml() { return m_xml; }
    private:
      Client cb;
      std::string m_xml;
  };


int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  Tag* tag = new Tag( "message", XMLNS, XMLNS_CLIENT );
  tag->addAttribute( "to", "foo@myserver/4444" );
  tag->addAttribute( "from", "foo@myserver" );
  tag->addAttribute( "id", "someid" );
  tag->addAttribute( "type", "chat" );
  Tag* c = new Tag( tag, "received", XMLNS, XMLNS_MESSAGE_CARBONS );
  Tag* f = new Tag( c, "forwarded",  XMLNS, XMLNS_STANZA_FORWARDING );
  Tag* m = new Tag( f, "message", XMLNS, XMLNS_CLIENT );
  m->addAttribute( "to", "foo@myserver/0123" );
  m->addAttribute( "from", "bar@theirserver/home" );
  m->addAttribute( "id", "someotherid" );
  m->addAttribute( "type", "chat" );
  new Tag( m, "body", "a sample message body" );
  new Tag( m, "attention", XMLNS, XMLNS_ATTENTION );

  CarbonsTest t;
  t.testTag( tag );

  // -------
  name = "parse Carbons";
  if( tag->xml() != t.getXml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed:\n%s\n---\n%s\n", name.c_str(), tag->xml().c_str(), t.getXml().c_str() );
  }


  Carbons* cm = 0;
  Tag* temp = 0;

  // -------
  name = "create Carbons::Received";
  cm = new Carbons( Carbons::Received );
  temp = cm->tag();
  if( cm->type() != Carbons::Received || temp->xml() != "<received xmlns='" + XMLNS_MESSAGE_CARBONS + "'/>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: type: %d, tag: %s\n", name.c_str(), cm->type(), temp ? temp->xml().c_str() : "'0'" );
  }
  delete temp;
  delete cm;

  // -------
  name = "create Carbons::Sent";
  cm = new Carbons( Carbons::Sent );
  temp = cm->tag();
  if( cm->type() != Carbons::Sent  || temp->xml() != "<sent xmlns='" + XMLNS_MESSAGE_CARBONS + "'/>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: type: %d, tag: %s\n", name.c_str(), cm->type(), temp ? temp->xml().c_str() : "'0'" );
  }
  delete temp;
  delete cm;

  // -------
  name = "create Carbons::Enable";
  cm = new Carbons( Carbons::Enable );
  temp = cm->tag();
  if( cm->type() != Carbons::Enable || temp->xml() != "<enable xmlns='" + XMLNS_MESSAGE_CARBONS + "'/>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: type: %d, tag: %s\n", name.c_str(), cm->type(), temp ? temp->xml().c_str() : "'0'" );
  }
  delete temp;
  delete cm;

  // -------
  name = "create Carbons::Disable";
  cm = new Carbons( Carbons::Disable );
  temp = cm->tag();
  if( cm->type() != Carbons::Disable || temp->xml() != "<disable xmlns='" + XMLNS_MESSAGE_CARBONS + "'/>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: type: %d, tag: %s\n", name.c_str(), cm->type(), temp ? temp->xml().c_str() : "'0'" );
  }
  delete temp;
  delete cm;

  // -------
  name = "create Carbons::Private";
  cm = new Carbons( Carbons::Private );
  temp = cm->tag();
  if( cm->type() != Carbons::Private || temp->xml() != "<private xmlns='" + XMLNS_MESSAGE_CARBONS + "'/>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: type: %d, tag: %s\n", name.c_str(), cm->type(), temp ? temp->xml().c_str() : "'0'" );
  }
  delete temp;
  delete cm;










  if( fail == 0 )
  {
    printf( "Carbons: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Carbons: %d test(s) failed\n", fail );
    return 1;
  }

}
