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

#include "../../stanza.h"
#include "../../tag.h"
#include "../../prep.h"
#include "../../gloox.h"
#include "../../jid.h"
#include "../../messageeventhandler.h"
#include "../../message.h"
#include "../../messageevent.h"

#include <stdio.h>
#include <string>
#include <cstdio> // [s]print[f]

namespace gloox
{
  class MessageSession : public MessageEventHandler
  {
    public:
      MessageSession() : m_jid( "abc@example.net/foo" ), m_test( 0 ), m_result( false ) {}
      virtual ~MessageSession() {}
      const JID& target() const { return m_jid; }
      void send( Message& msg )
      {
        const MessageEvent* me = msg.findExtension<MessageEvent>( ExtMessageEvent );
        if( !me )
          return;

        switch( m_test )
        {
          case 0:
            if( me->event() == MessageEventOffline )
              m_result = true;
            break;
          case 1:
            if( me->event() == MessageEventDelivered )
              m_result = true;
            break;
          case 2:
            if( me->event() == MessageEventDisplayed )
              m_result = true;
            break;
          case 3:
            if( me->event() == MessageEventComposing )
              m_result = true;
            break;
          case 4:
            if( me->event() == MessageEventCancel )
              m_result = true;
            break;
          default:
            break;
        }
      }
      void setTest( int test ) { m_test = test; }
      bool ok() { bool ok = m_result; m_result = false; return ok; }
      virtual void handleMessageEvent( const JID& from, MessageEventType event )
      {
        printf( "recved event %d\n", event );
      }
    private:
      JID m_jid;
      int m_test;
      bool m_result;
  };

  class MessageFilter
  {
    public:
      MessageFilter( MessageSession *parent );
      virtual ~MessageFilter();
      void attachTo( MessageSession *session );
      virtual void decorate( Message& msg );
      void send( Message& msg );
    protected:
      MessageSession *m_parent;
  };

  MessageFilter::MessageFilter( MessageSession *parent ) : m_parent( parent ) {}
  MessageFilter::~MessageFilter() { delete m_parent; }
  void MessageFilter::attachTo( MessageSession *session ) {}
  void MessageFilter::decorate( Message& msg ) {}
  void MessageFilter::send( Message& msg ) { m_parent->send( msg ); }
}

#define MESSAGEFILTER_H__
#define MESSAGESESSION_H__
#include "../../messageeventfilter.h"
#include "../../messageeventfilter.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  gloox::MessageEventFilter *f;
  gloox::MessageSession *ms;
  gloox::Tag *x = 0;

  // -------
  {
    name = "simple decorate";
    f = new gloox::MessageEventFilter( new gloox::MessageSession() );
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    f->decorate( m );
    const gloox::MessageEvent* me = m.findExtension<gloox::MessageEvent>( gloox::ExtMessageEvent );
    if( me && me->event() != ( gloox::MessageEventOffline | gloox::MessageEventDelivered
                         | gloox::MessageEventDisplayed | gloox::MessageEventComposing ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
    f = 0;
  }

  // -------
  ms = new gloox::MessageSession();
  f = new gloox::MessageEventFilter( ms );
  f->registerMessageEventHandler( ms );

  gloox::Message m( gloox::Message::Chat, gloox::JID(), "my message" );
  m.addExtension( new gloox::MessageEvent( gloox::MessageEventOffline | gloox::MessageEventDelivered
                                           | gloox::MessageEventDisplayed | gloox::MessageEventComposing
                                           | gloox::MessageEventCancel) );
  f->filter( m );

  name = "raise offline event 1";
  ms->setTest( 0 );
  f->raiseMessageEvent( gloox::MessageEventOffline );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise offline event 2";
  ms->setTest( 0 );
  f->raiseMessageEvent( gloox::MessageEventOffline );
  if( ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise delivered event 1";
  ms->setTest( 1 );
  f->raiseMessageEvent( gloox::MessageEventDelivered );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise delivered event 2";
  ms->setTest( 1 );
  f->raiseMessageEvent( gloox::MessageEventDelivered );
  if( ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise displayed event 1";
  ms->setTest( 2 );
  f->raiseMessageEvent( gloox::MessageEventDisplayed );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise displayed event 2";
  ms->setTest( 2 );
  f->raiseMessageEvent( gloox::MessageEventDisplayed );
  if( ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise composing event 1";
  ms->setTest( 3 );
  f->raiseMessageEvent( gloox::MessageEventComposing );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise composing event 2";
  ms->setTest( 3 );
  f->raiseMessageEvent( gloox::MessageEventComposing );
  if( ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise cancel event 1";
  ms->setTest( 4 );
  f->raiseMessageEvent( gloox::MessageEventCancel );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "raise cancel event 2";
  ms->setTest( 4 );
  f->raiseMessageEvent( gloox::MessageEventCancel );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  delete f;
  f = 0;











  if( fail == 0 )
  {
    printf( "MessageEventFilter: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "MessageEventFilter: %d test(s) failed\n", fail );
    return 1;
  }

}
