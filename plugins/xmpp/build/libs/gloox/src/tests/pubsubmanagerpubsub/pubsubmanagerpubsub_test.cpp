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

#include "../../jid.h"
#include "../../stanzaextensionfactory.h"
#include "../../iq.h"
using namespace gloox;

#include <string>
#include <cstdio> // [s]print[f]

static const JID jid( "aaa@bbb.ccc" );
static const std::string node( "node" );

#define PUBSUBMANAGER_TEST
#include "../../pubsubmanager.cpp"
#include "../../pubsubmanager.h"

JID jid2( "some@jid.com" );

int main()
{
  std::string name;
  int fail = 0;
  // -------
  {
  }

  // -------
  {
    name = "PubSub::Manager::PubSub/SEFactory test";
    StanzaExtensionFactory sef;
    sef.registerExtension( new PubSub::Manager::PubSub() );
    Tag* f = new Tag( "iq" );
    new Tag( f, "pubsub", "xmlns", XMLNS_PUBSUB );
    IQ iq( IQ::Get, JID(), "" );
    sef.addExtensions( iq, f );
    const PubSub::Manager::PubSub* se = iq.findExtension<PubSub::Manager::PubSub>( ExtPubSub );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }





  printf( "PubSub::Manager::PubSub: " );
  if( fail )
    fprintf( stderr, "%d test(s) failed\n", fail );
  else
    printf( "OK\n" );

}

