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
#define VCARD_TEST
#include "../../vcard.h"
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
    name = "empty vcard request";
    VCard v;
    t = v.tag();
    if( !t || t->xml() != "<vCard xmlns='" + XMLNS_VCARD_TEMP + "' version='3.0'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  name = "VCard/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new VCard() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "vCard", "xmlns", XMLNS_VCARD_TEMP );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const VCard* se = iq.findExtension<VCard>( ExtVCard );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "VCard: " );
  if( fail == 0 )
  {
    printf( "OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "%d test(s) failed\n", fail );
    return 1;
  }

}
