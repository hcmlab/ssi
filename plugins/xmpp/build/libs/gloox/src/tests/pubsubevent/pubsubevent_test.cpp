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

#include "../../pubsubevent.h"
#include "../../tag.h"

#include <cstdio> // [s]print[f]

static int failed = 0;

using namespace gloox;

int main()
{
  Tag* tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  Tag* t   = new Tag( tag, "items", "node", "princely_musings" );
  new Tag( t, "item", "id", "id" );

  PubSub::Event* pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "items test failed\n" );
  }
  delete tag;
  delete t;
  delete pse;
  pse = 0;
  t = 0;
  tag = 0;

  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "items", "node", "princely_musings" );
  new Tag( t, "item", "id", "id" );
  t = new Tag( tag, "headers", XMLNS, "http://jabber.org/protocol/shim" );
  Tag* t3 = new Tag( t, "header", "name", "pubsub#collection" );
  t3->setCData( "collection" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    if( t )
      printf ( "%s\n", t->xml().c_str() );
      printf ( "%s\n", tag->xml().c_str() );
    fprintf( stderr, "items from collection test failed\n" );
  }
  delete tag;
  delete t;
  delete pse;
  pse = 0;
  t = 0;
  tag = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "items", "node", "princely_musings" );
  t   = new Tag( t, "item", "id", "id" );
  t   = new Tag( t, "entry", XMLNS, "http://www.w3.org" );
  new Tag( t, "title", "Soliloquy" );
  new Tag( t, "summary", "To be or not to be " );
  t3  = new Tag( t, "link", "rel", "alternate" );
  t3->addAttribute( "type", "text/html" );
  t3->addAttribute( "href", "http://denmark.lit/2003/12/13/atom03" );
  new Tag( t, "id", "tag:denmark.lit,2003:entry-32397" );
  new Tag( t, "published", "2003-12-13T18:30:02Z" );
  new Tag( t, "updated", "2003-12-13T18:30:02Z" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "items w/ payload test failed\n" );
  }
  delete tag;
  delete t;
  delete pse;
  pse = 0;
  t = 0;
  tag = 0;

  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "items", "node", "princely_musings" );
  new Tag( t, "item", "id", "id" );
  t = new Tag( tag, "headers", XMLNS, "http://jabber.org/protocol/shim" );
  t3 = new Tag( t, "header", "name", "pubsub#subid" );
  t3->setCData( "123-abc" );
  t3 = new Tag( t, "header", "name", "pubsub#subid" );
  t3->setCData( "004-yyy" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "items w/ subscription id test failed\n" );
  }
  delete tag;
  delete t;
  delete pse;
  pse = 0;
  t = 0;
  tag = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "items", "node", "princely_musings" );
  new Tag( t, "retract", "id", "id" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag || pse->type() != PubSub::EventItemsRetract )
  {
    ++failed;
    fprintf( stderr, "retract test failed\n" );
  }
  delete pse;
  delete t;
  pse = 0;
  t = 0;

  t = new Tag( tag, "headers", XMLNS, "http://jabber.org/protocol/shim" );
  t3 = new Tag( t, "header", "name", "pubsub#subid" );
  t3->setCData( "123-abc" );
  t3 = new Tag( t, "header", "name", "pubsub#subid" );
  t3->setCData( "004-yyy" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "retract w/ subscription id test failed\n" );
  }
  delete pse;
  delete t;
  delete tag;
  pse = 0;
  t = 0;
  tag = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "subscription", "node", "princely_musings" );
  t->addAttribute( "jid", "foo@bar.com" );
  t->addAttribute( "subscription", "subscribed" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag || pse->type() != PubSub::EventSubscription
      || pse->node() != "princely_musings" || pse->jid().full() != "foo@bar.com" || !pse->subscription())
  {
    ++failed;
    printf("t: %s\n", t->xml().c_str() );
    printf("tag: %s\n", tag->xml().c_str() );
    fprintf( stderr, "'subscription successful' test failed\n" );
  }
  delete pse;
  delete t;
  pse = 0;
  t = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t   = new Tag( tag, "subscription", "node", "princely_musings" );
  t->addAttribute( "jid", "foo@bar.com" );
  t->addAttribute( "subscription", "none" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag || pse->type() != PubSub::EventSubscription
      || pse->node() != "princely_musings" || pse->jid().full() != "foo@bar.com" || pse->subscription())
  {
    ++failed;
    fprintf( stderr, "'subscription failed' test failed\n" );
  }
  delete pse;
  delete t;
  pse = 0;
  t = 0;


  Tag* tmp = 0;
  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t = new Tag( tag, "configuration", "node", "princely_musings" );

  pse = new PubSub::Event( tag );
  if( !pse || !(tmp = pse->tag()) || *tmp != *tag )
  {
    ++failed;
    if( tmp )
      printf( "t1: %s\n", tmp->xml().c_str() );
    printf( "t2: %s\n", tag->xml().c_str() );
    fprintf( stderr, "configuration w/o payload failed\n" );
  }
  delete tmp;
  delete pse;
  tmp = 0;
  pse = 0;



  t = new Tag( t, "x", XMLNS, "jabber:x:data" );
  t->addAttribute( "type", "result" );
  t3 = new Tag( t, "field", "var", "FORM_TYPE" );
  t3->addAttribute( "type", "hidden" );
  new Tag( t3, "value", "http://jabber.org/protocol/pubsub#node_config" );
  t3 = new Tag( t, "field", "var", "pubsub#title" );
  new Tag( t3, "value", "Princely Musings (Atom)" );


  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "configuration w/ payload failed\n" );
  }
  delete t;
  delete pse;
  delete tag;
  pse = 0;
  t = 0;
  tag = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t = new Tag( tag, "delete", "node", "princely_musings" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "delete node test failed\n" );
  }
  delete pse;
  delete tag;
  delete t;
  pse = 0;
  tag = 0;
  t = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t = new Tag( tag, "purge", "node", "princely_musings" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "purge node id test failed\n" );
  }
  delete pse;
  delete tag;
  delete t;
  pse = 0;
  tag = 0;
  t = 0;


  tag = new Tag( "event", XMLNS, XMLNS_PUBSUB_EVENT );
  t = new Tag( tag, "collection" );
  t3 = new Tag( t, "node", "id", "princely_musings" );

  pse = new PubSub::Event( tag );
  if( !pse || !(tmp = pse->tag()) || *tmp != *tag )
  {
    ++failed;
    if( tmp )
      printf( "t1: %s\n", tmp->xml().c_str() );
    printf( "t2: %s\n", tag->xml().c_str() );
    fprintf( stderr, "collection test failed\n" );
  }
  delete tmp;
  delete pse;
  tmp = 0;
  pse = 0;



  t = new Tag( t3, "x", XMLNS, "jabber:x:data" );
  t->addAttribute( "type", "result" );
  t3 = new Tag( t, "field", "var", "FORM_TYPE" );
  t3->addAttribute( "type", "hidden" );
  new Tag( t3, "value", "http://jabber.org/protocol/pubsub#meta_data" );
  t3 = new Tag( t, "field", "var", "pubsub#description" );
  new Tag( t3, "value", "Atom feed for my blog" );

  pse = new PubSub::Event( tag );
  if( !pse || !(t = pse->tag()) || *t != *tag )
  {
    ++failed;
    fprintf( stderr, "collection w/ payload failed\n" );
  }
  delete t;
  delete pse;
  delete tag;
  pse = 0;
  t = 0;
  tag = 0;













  if( failed )
  {
    fprintf( stderr, "PubSub::Event: %d test(s) failed\n", failed );
    return 1;
  }
  else
  {
    printf( "PubSub::Event: OK\n" );
    return 0;
  }

}

