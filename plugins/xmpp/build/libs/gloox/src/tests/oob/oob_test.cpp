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
#include "../../oob.h"
#include "../../iq.h"
#include "../../stanzaextensionfactory.h"
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
  OOB *d;

  // -------
  name = "parsing 0 tag";
  d = new OOB( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------------------
  // jabber:x:oob tests
  // -------------------
  Tag *x = new Tag( "x" );
  x->addAttribute( "xmlns", XMLNS_X_OOB );
  new Tag( x, "url", "invalidurl" );
  new Tag( x, "desc", "description" );


  // -------
  name = "filled object/getters";
  d = new OOB( "invalidurl", "description", false );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/tag()";
  d = new OOB( "invalidurl", "description", false );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
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
  d = new OOB( x );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "from Tag/tag()";
  d = new OOB( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;


  // -------------------
  // jabber:iq:oob tests
  // -------------------

  delete x;
  x  = 0;
  x = new Tag( "query" );
  x->addAttribute( "xmlns", XMLNS_IQ_OOB );
  new Tag( x, "url", "invalidurl" );
  new Tag( x, "desc", "description" );


  // -------
  name = "filled object/getters";
  d = new OOB( "invalidurl", "description", true );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/tag()";
  d = new OOB( "invalidurl", "description", true );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_IQ_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
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
  d = new OOB( x );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "from Tag/tag()";
  d = new OOB( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_IQ_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
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


  StanzaExtensionFactory sef;

  // -------
  name = "OOB/SEFactory test";
  OOB* oob = new OOB( 0 ); // deleted by StanzaExtensionFactory sef;
  sef.registerExtension( oob );
  Tag* f = new Tag( "iq" );
  Tag* b = new Tag( f, "query", "xmlns", XMLNS_IQ_OOB );
  new Tag( b, "url", "url" );
  new Tag( b, "desc", "desc" );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const OOB* se = iq.findExtension<OOB>( ExtOOB );
  if( se == 0 || se->url() != "url" || se->desc() != "desc" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "OOB: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "OOB: %d test(s) failed\n", fail );
    return 1;
  }

}
