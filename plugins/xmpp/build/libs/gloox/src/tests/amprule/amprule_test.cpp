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
#include "../../amp.h"
#include "../../message.h"
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
    name = "create condition 'deliver/direct -- notify' rule";
    AMP::Rule r( AMP::DeliverDirect, AMP::ActionNotify );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='deliver' action='notify' value='direct'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "create condition 'deliver/gateway -- error' rule";
    AMP::Rule r( AMP::DeliverGateway, AMP::ActionError );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='deliver' action='error' value='gateway'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "create condition 'deliver/stored -- drop' rule";
    AMP::Rule r( AMP::DeliverStored, AMP::ActionDrop );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='deliver' action='drop' value='stored'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "create condition 'expire-at/foodate -- notify' rule";
    AMP::Rule r( "foodate", AMP::ActionNotify );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='expire-at' action='notify' value='foodate'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

   // -------
  {
    name = "create condition 'match-resource/exact -- alert' rule";
    AMP::Rule r( AMP::MatchResourceExact, AMP::ActionAlert );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='match-resource' action='alert' value='exact'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "string ctor 1";
    AMP::Rule r( "deliver", "drop", "direct" );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='deliver' action='drop' value='direct'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "string ctor 2";
    AMP::Rule r( "expire-at", "notify", "foodate" );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='expire-at' action='notify' value='foodate'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }

  // -------
  {
    name = "string ctor 3";
    AMP::Rule r( "match-resource", "alert", "other" );
    t = r.tag();
    if( !t || t->xml() != "<rule condition='match-resource' action='alert' value='other'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    t = 0;
  }




  if( fail == 0 )
  {
    printf( "AMP::Rule: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "AMP::Rule: %d test(s) failed\n", fail );
    return 1;
  }

}
