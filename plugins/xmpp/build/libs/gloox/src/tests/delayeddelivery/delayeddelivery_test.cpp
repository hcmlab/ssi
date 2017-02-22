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
#include "../../delayeddelivery.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;
  DelayedDelivery *d;
  JID j( "abc@def/ghi" );
  Tag *x = new Tag( "delay", "reason" );
  x->addAttribute( "stamp", "invalidstamp" );
  x->addAttribute( "from", j.full() );
  x->addAttribute( "xmlns", XMLNS_DELAY );

  // -------
  name = "parsing 0 tag";
  d = new DelayedDelivery( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/getters";
  d = new DelayedDelivery( j, "invalidstamp", "reason" );
  if( d->reason() != "reason" || d->stamp() != "invalidstamp" || d->from() != j )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;
  t = 0;

  // -------
  name = "filled object/tag()";
  d = new DelayedDelivery( j, "invalidstamp", "reason" );
  t = d->tag();
  if( !t || t->name() != "delay" || !t->hasAttribute( "xmlns", XMLNS_DELAY )
       || !t->hasAttribute( "from", j.full() ) || !t->hasAttribute( "stamp", "invalidstamp" )
       || t->cdata() != "reason" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/getters";
  d = new DelayedDelivery( x );
  if( d->reason() != "reason" || d->stamp() != "invalidstamp" || d->from() != j )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new DelayedDelivery( x );
  t = d->tag();
  if( !t || t->name() != "delay" || !t->hasAttribute( "xmlns", XMLNS_DELAY )
       || !t->hasAttribute( "from", j.full() ) || !t->hasAttribute( "stamp", "invalidstamp" )
       || t->cdata() != "reason" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;



  delete x;
  x = 0;



  if( fail == 0 )
  {
    printf( "DelayedDelivery: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "DelayedDelivery: %d test(s) failed\n", fail );
    return 1;
  }

}
