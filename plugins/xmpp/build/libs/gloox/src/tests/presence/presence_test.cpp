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
#define PRESENCE_TEST
#include "../../tag.h"
#include "../../presence.h"
#include "../../stanza.h"
#include "../../jid.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *pres = new Tag( "presence" );
  pres->addAttribute( "from", "me@example.net/gloox" );
  pres->addAttribute( "to", "you@example.net/gloox" );
  new Tag( pres, "status", "the status" );
  new Tag( pres, "priority", "10" );
  Tag* s = new Tag( "show" );
  Presence* i = 0;

  // -------
  name = "parse Presence implicit available";
  i = new Presence( pres );
  if( i->subtype() != Presence::Available
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence available";
  pres->addAttribute( "type", "available" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Available
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence unavailable";
  pres->addAttribute( "type", "unavailable" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Unavailable
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence error";
  pres->addAttribute( "type", "error" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Error
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence probe";
  pres->addAttribute( "type", "probe" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Probe
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence available";
  pres->addAttribute( "type", "available" );
  pres->addChild( s );
  s->setCData( "chat" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Chat
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence away";
  pres->addAttribute( "type", "available" );
  s->setCData( "away" );
  i = new Presence( pres );
  if( i->subtype() != Presence::Away
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence dnd";
  pres->addAttribute( "type", "available" );
  s->setCData( "dnd" );
  i = new Presence( pres );
  if( i->subtype() != Presence::DND
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Presence xa";
  pres->addAttribute( "type", "available" );
  s->setCData( "xa" );
  i = new Presence( pres );
  if( i->subtype() != Presence::XA
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" || i->priority() != 10 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  {
    name = "new simple Presence available";
    Presence p( Presence::Available, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" )
        || i->hasAttribute( "type" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence unavailable";
    Presence p( Presence::Unavailable, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "type", "unavailable" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence chat";
    Presence p( Presence::Chat, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" )
        || !i->hasChildWithCData( "show", "chat" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence away";
    Presence p( Presence::Away, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" )
        || !i->hasChildWithCData( "show", "away" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence dnd";
    Presence p( Presence::DND, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" )
        || !i->hasChildWithCData( "show", "dnd" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence xa";
    Presence p( Presence::XA, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" )
        || !i->hasChildWithCData( "show", "xa" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence probe";
    Presence p( Presence::Probe, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "type", "probe" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple Presence error";
    Presence p( Presence::Error, JID( "xyz@example.org/blah" ), "the status",
                          10, "the xmllang" );
    p.setFrom( JID( "foo@bar.com" ) );
    Tag* i = p.tag();
    if( !i->hasAttribute( "type", "error" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" )
        || !i->hasChildWithCData( "priority", "10" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }



  delete pres;
  pres = 0;

  if( fail == 0 )
  {
    printf( "Presence: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Presence: %d test(s) failed\n", fail );
    return 1;
  }

}
