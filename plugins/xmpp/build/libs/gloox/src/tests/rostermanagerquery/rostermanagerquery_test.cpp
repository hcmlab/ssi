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
#define ROSTERMANAGER_TEST
#include "../../rostermanager.h"
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
    name = "request roster";
    RosterManager::Query rq;
    t = rq.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_ROSTER + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "add/update item";
    StringList sl;
    sl.push_back( "group1" );
    sl.push_back( "group2" );
    RosterManager::Query rq( JID( "foof" ), "name", sl );
    t = rq.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_ROSTER + "'>"
                    "<item jid='foof' name='name'>"
                    "<group>group1</group>"
                    "<group>group2</group>"
                    "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "remove item";
    RosterManager::Query rq( JID( "foof" ) );
    t = rq.tag();
    if( t->xml() != "<query xmlns='" + XMLNS_ROSTER + "'>"
        "<item jid='foof' subscription='remove'/>"
        "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "initial roster";
    Tag* q = new Tag( "query" );
    q->setXmlns( XMLNS_ROSTER );
    Tag* i = new Tag( q, "item", "jid", "foo1" );
    i->addAttribute( "name", "name1" );
    i->addAttribute( "subscription", "from" );
    i = new Tag( q, "item", "jid", "foo2" );
    i->addAttribute( "name", "name2" );
    i->addAttribute( "subscription", "both" );
    RosterManager::Query rq( q );
    t = rq.tag();
    if( *t != *q || rq.roster().size() != 2 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete q;
  }

  // -------
  {
    name = "roster push";
    Tag* q = new Tag( "query" );
    q->setXmlns( XMLNS_ROSTER );
    Tag* i = new Tag( q, "item", "jid", "foo1" );
    i->addAttribute( "name", "name1" );
    i->addAttribute( "subscription", "from" );
    new Tag( i, "group", "group1" );
    new Tag( i, "group", "group2" );
    RosterManager::Query rq( q );
    t = rq.tag();
    if( *t != *q )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
      printf( "     got: %s\n", t->xml().c_str() );
      printf( "expected: %s\n", q->xml().c_str() );
    }
    delete t;
    t = 0;
    delete q;
  }


  // -------
  name = "RosterManager::Query/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new RosterManager::Query() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_ROSTER );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const RosterManager::Query* se = iq.findExtension<RosterManager::Query>( ExtRoster );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "RosterManager::Query: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "RosterManager::Query: %d test(s) failed\n", fail );
    return 1;
  }

}
