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
#include "../../gpgencrypted.h"
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
  GPGEncrypted *d;
  Tag *x = new Tag( "x", "invalidencrypted" );
  x->addAttribute( "xmlns", XMLNS_X_GPGENCRYPTED );

  // -------
  name = "parsing 0 tag";
  d = new GPGEncrypted( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/getters";
  d = new GPGEncrypted( "invalidencrypted" );
  if( d->encrypted() != "invalidencrypted" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/tag()";
  d = new GPGEncrypted( "invalidencrypted" );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_GPGENCRYPTED )
       || t->cdata() != "invalidencrypted" )
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
  d = new GPGEncrypted( x );
  if( d->encrypted() != "invalidencrypted" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new GPGEncrypted( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_GPGENCRYPTED )
       || t->cdata() != "invalidencrypted" )
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
    printf( "GPGEncrypted: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "GPGEncrypted: %d test(s) failed\n", fail );
    return 1;
  }

}
