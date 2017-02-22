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
  Tag* t;


  // -------
  {
    name = "info note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Info, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Info || n.content() != "content"
        || t->xml() != "<note type='info'>content</note>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "warning note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Warning, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Warning || n.content() != "content"
        || t->xml() != "<note type='warn'>content</note>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "error note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Error, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Error || n.content() != "content"
        || t->xml() != "<note type='error'>content</note>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "error note";
    Adhoc::Command::Note n( Adhoc::Command::Note::Error, "content" );
    t = n.tag();
    if( n.severity() != Adhoc::Command::Note::Error || n.content() != "content"
        || t->xml() != "<note type='error'>content</note>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }


  // -------
  {
    name = "parse Tag";
    Tag* b = new Tag( "note", "foo" );
    b->addAttribute( "type", "info" );
    Adhoc::Command::Note n( b );
    t = n.tag();
    if( *t != *b )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete b;
    b = 0;
  }

  // -------
  {
    name = "parse Tag w/o type";
    Tag* b = new Tag( "note", "foo" );
    b->addAttribute( "type", "info" );
    Adhoc::Command::Note n( b );
    t = n.tag();
    if( *t != *b )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
    delete b;
    b = 0;
  }



  if( fail == 0 )
  {
    printf( "Adhoc::Command::Note: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Adhoc::Command::Note: %d test(s) failed\n", fail );
    return 1;
  }

}
