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

#include "../../nickname.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;


  // -------
  {
    name = "empty nick";
    Nickname n( "" );
    if( !n.nick().empty() || n.tag() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "nick check";
    Nickname n( "foo" );
    Tag* t = n.tag();
    if( n.nick() != "foo" || !t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "tag check";
    Nickname n( "foo" );
    Tag* t = n.tag();
    if( t->xml() != "<nick xmlns='http://jabber.org/protocol/nick'>foo</nick>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }



  if( fail == 0 )
  {
    printf( "Nickname: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Nickname: %d test(s) failed\n", fail );
    return 1;
  }


}
