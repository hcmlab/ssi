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

#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

using namespace gloox;


  class ForwardTest : public MessageHandler
  {
    public:
      ForwardTest() : cb( "foo" )
      {
        cb.registerMessageHandler( this );
        cb.registerStanzaExtension( new Forward() );
        cb.registerStanzaExtension( new Attention() );
      }
      ~ForwardTest() {}
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

  Message m( Message::Chat, JID( "a@b/c" ), "body test", "subject" );
  Message* fm = new Message( Message::Headline, JID( "d@e/f" ), "headline test", "headline subject" );
  fm->addExtension( new Attention() );
  DelayedDelivery* d = new DelayedDelivery( JID( "from@me/res" ), "thestamp" );
  Forward* f = new Forward( fm, d );
  m.addExtension( f );

  Tag* tag = m.tag();
  ForwardTest t;
  t.testTag( tag );




  // -------
  name = "create Forward 1";
  if( tag->xml() != t.getXml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed:\n%s\n---\n%s\n", name.c_str(), tag->xml().c_str(), t.getXml().c_str() );
  }









  printf( "Forward: " );
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
