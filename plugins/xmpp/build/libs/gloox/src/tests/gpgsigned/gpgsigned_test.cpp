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
#include "../../gpgsigned.h"
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
  GPGSigned *d;
  Tag *x = new Tag( "x", "invalidsignature" );
  x->addAttribute( "xmlns", XMLNS_X_GPGSIGNED );

  // -------
  name = "parsing 0 tag";
  d = new GPGSigned( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "filled object/getters";
  d = new GPGSigned( "invalidsignature" );
  if( d->signature() != "invalidsignature" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;
  t = 0;

  // -------
  name = "filled object/tag()";
  d = new GPGSigned( "invalidsignature" );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_GPGSIGNED )
       || t->cdata() != "invalidsignature" )
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
  d = new GPGSigned( x );
  if( d->signature() != "invalidsignature" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new GPGSigned( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_GPGSIGNED )
       || t->cdata() != "invalidsignature" )
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
    printf( "GPGSigned: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "GPGSigned: %d test(s) failed\n", fail );
    return 1;
  }

}
