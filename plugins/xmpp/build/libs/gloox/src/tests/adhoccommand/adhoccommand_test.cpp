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
#define ADHOC_COMMANDS_TEST
#include "../../adhoc.h"
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
    name = "execute specific command";
    Adhoc::Command ac( "somecmd", Adhoc::Command::Execute );
    t = ac.tag();
    if( t->xml() != "<command xmlns='" + XMLNS_ADHOC_COMMANDS + "' node='somecmd' action='execute'/>"
        || ac.node() != "somecmd" || ac.action() != Adhoc::Command::Execute )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "execute specific command /w session";
    Adhoc::Command ac( "somecmd", "somesession", Adhoc::Command::Execute );
    t = ac.tag();
    if( t->xml() != "<command xmlns='" + XMLNS_ADHOC_COMMANDS + "' node='somecmd' action='execute' sessionid='somesession'/>"
        || ac.node() != "somecmd" || ac.sessionID() != "somesession"
        || ac.action() != Adhoc::Command::Execute )
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
    t = new Tag( "command" );
    t->setXmlns( XMLNS_ADHOC_COMMANDS );
    t->addAttribute( "node", "somecmd" );
    t->addAttribute( "sessionid", "somesession" );
    t->addAttribute( "action", "execute" );
    Tag* f = new Tag( t, "x" );
    f->setXmlns( XMLNS_X_DATA );
    Adhoc::Command ac( t );
    if( ac.node() != "somecmd" || ac.sessionID() != "somesession"
        || ac.action() != Adhoc::Command::Execute || ac.form() == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  name = "Adhoc::Command/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Adhoc::Command( "foo", Adhoc::Command::Execute) );
  Tag* f = new Tag( "iq" );
  new Tag( f, "command", "xmlns", XMLNS_ADHOC_COMMANDS );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const Adhoc::Command* se = iq.findExtension<Adhoc::Command>( ExtAdhocCommand );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  if( fail == 0 )
  {
    printf( "Adhoc::Command: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Adhoc::Command: %d test(s) failed\n", fail );
    return 1;
  }

}
