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
#define SUBSCRIPTION_TEST
#include "../../tag.h"
#include "../../subscription.h"
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
  Tag *s10n = new Tag( "presence" );
  s10n->addAttribute( "from", "me@example.net/gloox" );
  s10n->addAttribute( "to", "you@example.net/gloox" );
  new Tag( s10n, "status", "the status" );
  Subscription* i = 0;

  // -------
  name = "parse Subscription subscribe";
  s10n->addAttribute( "type", "subscribe" );
  i = new Subscription( s10n );
  if( i->subtype() != Subscription::Subscribe
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Subscription subscribed";
  s10n->addAttribute( "type", "subscribed" );
  i = new Subscription( s10n );
  if( i->subtype() != Subscription::Subscribed
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Subscription unsubscribe";
  s10n->addAttribute( "type", "unsubscribe" );
  i = new Subscription( s10n );
  if( i->subtype() != Subscription::Unsubscribe
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse Subscription unsubscribed";
  s10n->addAttribute( "type", "unsubscribed" );
  i = new Subscription( s10n );
  if( i->subtype() != Subscription::Unsubscribed
      || i->from().full() != "me@example.net/gloox" || i->to().full() != "you@example.net/gloox"
      || i->status() != "the status" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  {
    name = "new simple Subscription subscribe";
    Subscription s( Subscription::Subscribe, JID( "xyz@example.org/blah" ), "the status",
                          "the xmllang" );
    s.setFrom( JID( "foo@bar.com" ) );
    Tag* i = s.tag();
    if( i->name() != "presence" || !i->hasAttribute( "type", "subscribe" )
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
    name = "new simple Subscription subscribed";
    Subscription s( Subscription::Subscribed, JID( "xyz@example.org/blah" ), "the status",
                          "the xmllang" );
    s.setFrom( JID( "foo@bar.com" ) );
    Tag* i = s.tag();
    if( i->name() != "presence" || !i->hasAttribute( "type", "subscribed" )
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
    name = "new simple Subscription unsubscribe";
    Subscription s( Subscription::Unsubscribe, JID( "xyz@example.org/blah" ), "the status",
                          "the xmllang" );
    s.setFrom( JID( "foo@bar.com" ) );
    Tag* i = s.tag();
    if( i->name() != "presence" || !i->hasAttribute( "type", "unsubscribe" )
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
    name = "new simple Subscription unsubscribed";
    Subscription s( Subscription::Unsubscribed, JID( "xyz@example.org/blah" ), "the status",
                          "the xmllang" );
    s.setFrom( JID( "foo@bar.com" ) );
    Tag* i = s.tag();
    if( i->name() != "presence" || !i->hasAttribute( "type", "unsubscribed" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasAttribute( "from", "foo@bar.com" )
        || !i->hasChildWithCData( "status", "the status" )
        || !i->hasChild( "status", "xml:lang", "the xmllang" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }



















  delete s10n;
  s10n = 0;

  if( fail == 0 )
  {
    printf( "Subscription: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Subscription: %d test(s) failed\n", fail );
    return 1;
  }

}
