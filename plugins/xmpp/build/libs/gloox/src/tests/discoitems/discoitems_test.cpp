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
#define DISCO_ITEMS_TEST
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
    name = "empty disco#items request";
    Disco::Items di;
    t = di.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_DISCO_ITEMS + "'/>"
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
    name = "empty disco#items request + node";
    Disco::Items di( "somenode" );
    t = di.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_DISCO_ITEMS + "' node='somenode'/>"
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
    name = "Tag ctor";
    t = new Tag( "query" );
    t->setXmlns( XMLNS_DISCO_ITEMS );
    t->addAttribute( "node", "somenode" );
    Tag* i = new Tag( t, "item", "jid", "jid1" );
    i->addAttribute( "node", "node1" );
    i->addAttribute( "name", "name1" );
    i = new Tag( t, "item", "jid", "jid2" );
    i->addAttribute( "node", "node2" );
    i->addAttribute( "name", "name2" );
    i = new Tag( t, "item", "jid", "jid3" );
    i->addAttribute( "node", "node3" );
    i->addAttribute( "name", "name3" );
    Disco::Items di( t );
    Disco::Item* item = 0;
    if( di.node() != "somenode" || di.items().size() != 3 || !( item = *(di.items().begin()) )
        || item->name() != "name1" || item->node() != "node1" || item->jid() != "jid1" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  name = "Disco::Items/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Disco::Items() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_DISCO_ITEMS );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const Disco::Items* se = iq.findExtension<Disco::Items>( ExtDiscoItems );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "Disco::Items: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Disco::Items: %d test(s) failed\n", fail );
    return 1;
  }

}
