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
#define UNIQUEMUCROOM_TEST
#include "../../uniquemucroom.h"
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

  // -------
  {
    name = "create Tag";
    UniqueMUCRoom::Unique uq;
    t = uq.tag();
    if( !t || t->xml() != "<unique xmlns='" + XMLNS_MUC_UNIQUE + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "parse Tag";
    Tag u( "unique" );
    u.setXmlns( XMLNS_MUC_UNIQUE );
    u.setCData( "foo" );
    UniqueMUCRoom::Unique uq( &u );
    t = uq.tag();
    if( !t || t->xml() != "<unique xmlns='" + XMLNS_MUC_UNIQUE + "'>"
                          "foo</unique>"
       || uq.name() != "foo" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  name = "UniqueMUCRoom::Unique/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new UniqueMUCRoom::Unique() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "unique", "xmlns", XMLNS_MUC_UNIQUE );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const UniqueMUCRoom::Unique* se = iq.findExtension<UniqueMUCRoom::Unique>( ExtMUCUnique );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "UniqueMUCRoom::Unique: " );
  if( !fail )
    printf( "OK\n" );
  else
    fprintf( stderr, "%d test(s) failed\n", fail );

  return fail;
}
