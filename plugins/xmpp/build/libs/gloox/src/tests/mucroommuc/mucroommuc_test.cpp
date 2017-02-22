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
#define MUCROOM_TEST
#include "../../mucroom.h"
#include "../../dataform.h"
#include "../../iq.h"
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
  Tag* t = 0;


  // -------
  {
    name = "joining a room";
    MUCRoom::MUC mu;
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "joining a password-protected room";
    MUCRoom::MUC mu( "foopwd" );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'>"
                          "<password>foopwd</password>"
                          "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "joining room, requesting room history 1";
    MUCRoom::MUC mu( EmptyString, MUCRoom::HistorySince, "foodate" );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'>"
         "<history since='foodate'/>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "joining room, requesting room history 2";
    MUCRoom::MUC mu( EmptyString, MUCRoom::HistoryMaxChars, EmptyString, 100 );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'>"
         "<history maxchars='100'/>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "joining room, requesting room history 3";
    MUCRoom::MUC mu( EmptyString, MUCRoom::HistoryMaxStanzas, EmptyString, 100 );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'>"
         "<history maxstanzas='100'/>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "joining room, requesting room history 4 + password";
    MUCRoom::MUC mu( "foopwd", MUCRoom::HistorySeconds, EmptyString, 100 );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC + "'>"
         "<history seconds='100'/>"
         "<password>foopwd</password>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "MUCRoom::MUC/SEFactory test (presence)";
    StanzaExtensionFactory sef;
    sef.registerExtension( new MUCRoom::MUC() );
    Tag* f = new Tag( "presence" );
    new Tag( f, "x", "xmlns", XMLNS_MUC );
    Presence pres( Presence::Available, JID(), "" );
    sef.addExtensions( pres, f );
    const MUCRoom::MUC* se = pres.findExtension<MUCRoom::MUC>( ExtMUC );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "MUCRoom::MUC: " );
  if( !fail )
    printf( "OK\n" );
  else
    fprintf( stderr, "%d test(s) failed\n", fail );

  return fail;
}
