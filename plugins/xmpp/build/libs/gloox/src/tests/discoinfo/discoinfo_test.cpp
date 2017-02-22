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
#define DISCO_INFO_TEST
#include "../../disco.h"
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
    name = "empty disco#info request";
    Disco::Info di;
    t = di.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_DISCO_INFO + "'/>"
        || !di.node().empty() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "empty (default) disco#info reply";
    Disco::Info di( EmptyString, true );
    t = di.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_DISCO_INFO + "'>"
                    "<feature var='" + XMLNS_DISCO_INFO + "'/>"
                    "<feature var='" + XMLNS_DISCO_ITEMS + "'/>"
        "</query>"
        || !di.node().empty() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "empty disco#info request + node + default features";
    Disco::Info di( "somenode", true );
    t = di.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_DISCO_INFO + "' node='somenode'>"
                    "<feature var='" + XMLNS_DISCO_INFO + "'/>"
                    "<feature var='" + XMLNS_DISCO_ITEMS + "'/>"
                    "</query>"
        || di.node() != "somenode" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "empty disco#info request + node - default features";
    Disco::Info di( "somenode" );
    t = di.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_DISCO_INFO + "' node='somenode'/>"
        || di.node() != "somenode" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "setnode()";
    Disco::Info di( "somenode" );
    di.setNode( "othernode" );
    if( di.node() != "othernode" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "Tag ctor";
    t = new Tag( "query" );
    t->setXmlns( XMLNS_DISCO_INFO );
    t->addAttribute( "node", "somenode" );
    Tag* i = new Tag( t, "identity", "category", "category1" );
    i->addAttribute( "type", "type1" );
    i->addAttribute( "name", "name1" );
    i = new Tag( t, "identity", "category", "category2" );
    i->addAttribute( "type", "type1" );
    i->addAttribute( "name", "name1" );
    i = new Tag( t, "identity", "category", "category3" );
    i->addAttribute( "type", "type1" );
    i->addAttribute( "name", "name1" );
    new Tag( t, "feature", "var", "foo1" );
    new Tag( t, "feature", "var", "foo2" );
    new Tag( t, "feature", "var", "foo3" );
    Disco::Info di( t );
    if( di.node() != "somenode" || di.identities().size() != 3 || di.features().size() != 3
        || !di.hasFeature( "foo1" ) || !di.hasFeature( "foo2" ) || !di.hasFeature( "foo3" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  name = "Disco::Info/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Disco::Info() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_DISCO_INFO );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const Disco::Info* se = iq.findExtension<Disco::Info>( ExtDiscoInfo );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "Disco::Info: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Disco::Info: %d test(s) failed\n", fail );
    return 1;
  }

}
