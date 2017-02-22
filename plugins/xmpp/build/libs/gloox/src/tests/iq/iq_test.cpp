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

#define IQ_TEST
#include "../../tag.h"
#include "../../iq.h"
#include "../../stanza.h"
#include "../../jid.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *iq = new Tag( "iq" );
  iq->addAttribute( "from", "me@example.net/gloox" );
  iq->addAttribute( "to", "you@example.net/gloox" );
  iq->addAttribute( "id", "id1" );
  iq->addAttribute( "type", "set" );
  IQ* i = 0;

  // -------
  name = "parse IQ set";
  i = new IQ( iq );
  if( i->subtype() != IQ::Set || i->from().full() != "me@example.net/gloox"
        || i->to().full() != "you@example.net/gloox" || i->id() != "id1" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse IQ get";
  iq->addAttribute( "type", "get" );
  i = new IQ( iq );
  if( i->subtype() != IQ::Get || i->from().full() != "me@example.net/gloox"
        || i->to().full() != "you@example.net/gloox" || i->id() != "id1" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse IQ error";
  iq->addAttribute( "type", "error" );
  i = new IQ( iq );
  if( i->subtype() != IQ::Error || i->from().full() != "me@example.net/gloox"
        || i->to().full() != "you@example.net/gloox" || i->id() != "id1" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parse IQ result";
  iq->addAttribute( "type", "result" );
  i = new IQ( iq );
  if( i->subtype() != IQ::Result || i->from().full() != "me@example.net/gloox"
        || i->to().full() != "you@example.net/gloox" || i->id() != "id1" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  {
    name = "new simple IQ error";
    IQ iq( IQ::Error, JID( "xyz@example.org/blah" ), "id2" );
    Tag* i = iq.tag();
    if( !i->hasAttribute( "type", "error" ) || !i->hasAttribute( "id", "id2" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple IQ result";
    IQ iq( IQ::Result, JID( "xyz@example.org/blah" ), "id2" );
    Tag* i = iq.tag();
    if( !i->hasAttribute( "type", "result" ) || !i->hasAttribute( "id", "id2" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple IQ get";
    IQ iq( IQ::Get, JID( "xyz@example.org/blah" ), "id2" );
    Tag* i = iq.tag();
    if( !i->hasAttribute( "type", "get" ) || !i->hasAttribute( "id", "id2" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

  // -------
  {
    name = "new simple IQ set 1";
    IQ iq( IQ::Set, JID( "xyz@example.org/blah" ), "id2" );
    Tag* i = iq.tag();
    if( !i->hasAttribute( "type", "set" ) || !i->hasAttribute( "id", "id2" )
        || !i->hasAttribute( "to", "xyz@example.org/blah" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
    }
    delete i;
  }

// FIXME these need to use SEs, as IQ::query() will go away eventually
//   // -------
//   {
//     name = "new simple IQ set 2";
//     IQ iq( IQ::Set, JID( "xyz@example.org/blah" ), "id2", "mynamespace" );
//     Tag* i = iq.tag();
//     if( !i->hasAttribute( "type", "set" ) || !i->hasAttribute( "id", "id2" )
//         || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasChild( "query", "xmlns", "mynamespace" ) )
//     {
//       ++fail;
//       fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
//     }
//     delete i;
//   }
//
//   // -------
//   {
//     name = "new simple IQ set 3";
//     IQ iq( IQ::Set, JID( "xyz@example.org/blah" ), "id2", "mynamespace", "testtag" );
//     Tag* i = iq.tag();
//     if( !i->hasAttribute( "type", "set" ) || !i->hasAttribute( "id", "id2" )
//         || !i->hasAttribute( "to", "xyz@example.org/blah" )
//         || !i->hasChild( "testtag", "xmlns", "mynamespace" ) )
//     {
//       ++fail;
//       fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
//     }
//     delete i;
//   }
//
//   // -------
//   {
//     name = "new simple IQ set 4";
//     IQ iq( IQ::Set, JID( "xyz@example.org/blah" ), "id2", "mynamespace", "testtag",
//                 JID( "blah@example.net/foo" ) );
//     Tag* i = iq.tag();
//     if( !i->hasAttribute( "type", "set" ) || !i->hasAttribute( "id", "id2" )
//         || !i->hasAttribute( "to", "xyz@example.org/blah" ) || !i->hasChild( "testtag", "xmlns", "mynamespace" )
//         || !i->hasAttribute( "from", "blah@example.net/foo" ) )
//     {
//       ++fail;
//       fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
//     }
//     delete i;
//   }

// FIXME fix the following test. how to test private functions, ctors, etc?
//   // -------
//   name = "rip off";
//   i = new IQ( iq );
//   if( !i->hasAttribute( "type", "result" ) || !i->hasAttribute( "id", "id1" )
//        || !i->hasAttribute( "to", "you@example.net/gloox" ) || !i->hasChild( "query", "xmlns", "mynamespace" )
//        || !i->hasAttribute( "from", "me@example.net/gloox" )
//        || iq->children().size() != 0 )
//   {
//     ++fail;
//     fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), i->xml().c_str() );
//   }
//   delete i;
//   i = 0;







  delete iq;
  iq = 0;

  if( fail == 0 )
  {
    printf( "IQ: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "IQ: %d test(s) failed\n", fail );
    return 1;
  }

}
