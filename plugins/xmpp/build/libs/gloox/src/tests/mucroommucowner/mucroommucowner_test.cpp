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
  Tag *t;

  // -------
  {
    name = "request config form";
    MUCRoom::MUCOwner mo( MUCRoom::MUCOwner::TypeRequestConfig );
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "submit config form";
    MUCRoom::MUCOwner mo( MUCRoom::MUCOwner::TypeSendConfig, new DataForm( /*DataForm::*/TypeForm ) );
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'>"
                            "<x xmlns='" + XMLNS_X_DATA + "' type='form'/>"
                          "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "cancel initial room config";
    MUCRoom::MUCOwner mo( MUCRoom::MUCOwner::TypeCancelConfig );
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'>"
                            "<x xmlns='" + XMLNS_X_DATA + "' type='cancel'/>"
                          "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // ------
  {
    name = "request instant room";
    MUCRoom::MUCOwner mo( MUCRoom::MUCOwner::TypeInstantRoom );
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'>"
                            "<x xmlns='" + XMLNS_X_DATA + "' type='submit'/>"
                          "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // ------
  {
    name = "destroy room";
    MUCRoom::MUCOwner mo( JID( "foo" ), "foobar", "foopwd" );
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'>"
                            "<destroy jid='foo'>"
                              "<reason>foobar</reason>"
                              "<password>foopwd</password>"
                            "</destroy></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // ------
  {
    name = "destroy room w/o alternate venue";
    MUCRoom::MUCOwner mo;
    t = mo.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_OWNER + "'>"
         "<destroy/></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // ------
  {
    name = "from Tag: request room config";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_MUC_OWNER );
    MUCRoom::MUCOwner mo( d );
    t = mo.tag();
    if( !t || *t != *d )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }

  // ------
  {
    name = "from Tag: destroy room";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_MUC_OWNER );
    Tag* destroy = new Tag( d, "destroy" );
    destroy->addAttribute( "jid", "alternate" );
    new Tag( destroy, "reason", "reason" );
    new Tag( destroy, "password", "pwd" );
    MUCRoom::MUCOwner mo( d );
    t = mo.tag();
    if( !t || *t != *d )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }

  // -------
  name = "MUCRoom::MUCOwner/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new MUCRoom::MUCOwner() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_MUC_OWNER );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const MUCRoom::MUCOwner* se = iq.findExtension<MUCRoom::MUCOwner>( ExtMUCOwner );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "MUCRoom::MUCOwner: " );
  if( !fail )
    printf( "OK\n" );
  else
    fprintf( stderr, "%d test(s) failed\n", fail );

  return fail;
}
