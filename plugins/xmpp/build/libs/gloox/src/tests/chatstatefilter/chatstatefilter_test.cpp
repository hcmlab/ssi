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

#include "../../message.h"
#include "../../tag.h"
#include "../../prep.h"
#include "../../gloox.h"
#include "../../jid.h"
#include "../../chatstatehandler.h"
#include "../../chatstate.h"

#include <stdio.h>
#include <string>
#include <cstdio> // [s]print[f]

namespace gloox
{
  class ClientBase;

  class MessageSession : public ChatStateHandler
  {
    public:
      MessageSession() : m_jid( "abc@example.net/foo" ), m_test( 0 ), m_result( false ) {}
      MessageSession( ClientBase*, const JID&, bool, int ); /*: m_jid( "abc@example.net/foo" ), m_test( 0 ),
                      m_result( false ) {}*/
      virtual ~MessageSession() {}
      const JID& target() const { return m_jid; }
      void send( Message& msg )
      {
        if( msg.to() != m_jid )
          return;

        const ChatState* cs = msg.findExtension<ChatState>( ExtChatState );
        if( !cs )
          return;

        switch( m_test )
        {
          case 0:
            if( cs->state() == ChatStateGone )
              m_result = true;
            break;
          case 1:
            if( cs->state() == ChatStateInactive )
              m_result = true;
            break;
          case 2:
            if( cs->state() == ChatStateActive )
              m_result = true;
            break;
          case 3:
            if( cs->state() == ChatStateComposing )
              m_result = true;
            break;
          case 4:
            if( cs->state() == ChatStatePaused )
              m_result = true;
            break;
          default:
            break;
        }
      }
      void setTest( int test ) { m_test = test; }
      bool ok() { bool ok = m_result; m_result = false; return ok; }
      virtual void handleChatState( const JID& from, ChatStateType state )
      {
        switch( m_test )
        {
          case 0:
            if( state == ChatStateGone )
              m_result = true;
            break;
          case 1:
            if( state == ChatStateInactive )
              m_result = true;
            break;
          case 2:
            if( state == ChatStateActive )
              m_result = true;
            break;
          case 3:
            if( state == ChatStateComposing )
              m_result = true;
            break;
          case 4:
            if( state == ChatStatePaused )
              m_result = true;
            break;
          default:
            break;
        }
      }
    private:
      JID m_jid;
      int m_test;
      bool m_result;
  };

  MessageSession::MessageSession( ClientBase*, const JID&, bool, int )
    : m_jid( "abc@example.net/foo" ), m_test( 0 ), m_result( false ) {}

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
#include "../../chatstatefilter.h"
#include "../../chatstatefilter.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  gloox::ChatStateFilter *f;
  gloox::MessageSession *ms;
  gloox::Tag *t = 0;
  gloox::Tag *x = 0;
  gloox::Message *s = 0;

  // -------
  {
    name = "simple decorate";
    f = new gloox::ChatStateFilter( new gloox::MessageSession() );
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    f->decorate( m );
    if( m.findExtension<gloox::ChatState>( gloox::ExtChatState )->state()
           != gloox::ChatStateActive )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:s %s\n", name.c_str(), t->xml().c_str() );
    }
    delete f;
    f = 0;
  }
  // -------
  ms = new gloox::MessageSession();
  f = new gloox::ChatStateFilter( ms );
  f->registerChatStateHandler( ms );

  {
    name = "filter gone";
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    m.addExtension( new gloox::ChatState( gloox::ChatStateGone ) );
    ms->setTest( 0 );
    f->filter( m );
    if( !ms->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "filter inactive";
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    m.addExtension( new gloox::ChatState( gloox::ChatStateInactive ) );
    ms->setTest( 1 );
    f->filter( m );
    if( !ms->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "filter active";
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    m.addExtension( new gloox::ChatState( gloox::ChatStateActive ) );
    ms->setTest( 2 );
    f->filter( m );
    if( !ms->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "filter composing";
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    m.addExtension( new gloox::ChatState( gloox::ChatStateComposing ) );
    ms->setTest( 3 );
    f->filter( m );
    if( !ms->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "filter paused";
    gloox::Message m( gloox::Message::Chat, gloox::JID() );
    m.addExtension( new gloox::ChatState( gloox::ChatStatePaused ) );
    ms->setTest( 4 );
    f->filter( m );
    if( !ms->ok() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  name = "set inactive state";
  ms->setTest( 1 );
  f->setChatState( gloox::ChatStateInactive );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set active state";
  ms->setTest( 2 );
  f->setChatState( gloox::ChatStateActive );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set composing state";
  ms->setTest( 3 );
  f->setChatState( gloox::ChatStateComposing );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set paused state";
  ms->setTest( 4 );
  f->setChatState( gloox::ChatStatePaused );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "set gone state";
  ms->setTest( 0 );
  f->setChatState( gloox::ChatStateGone );
  if( !ms->ok() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  delete f;
//   delete s;
  f = 0;
//   s = 0;











  if( fail == 0 )
  {
    printf( "ChatStateFilter: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "ChatStateFilter: %d test(s) failed\n", fail );
    return 1;
  }

}
