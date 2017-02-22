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
    name = "presence broadcast";
    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_MUC_USER );
    Tag* i = new Tag( x, "item" );
    i->addAttribute( "jid", "foo@bar" );
    i->addAttribute( "role", "participant" );
    i->addAttribute( "affiliation", "member" );
    MUCRoom::MUCUser mu( x );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
                          "<item jid='foo@bar' role='participant' affiliation='member'/>"
                          "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete x;
  }


  // -------
  {
    name = "presence broadcast incl status codes";
    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_MUC_USER );
    Tag* i = new Tag( x, "item" );
    i->addAttribute( "jid", "foo@bar" );
    i->addAttribute( "role", "participant" );
    i->addAttribute( "affiliation", "member" );
    new Tag( i, "actor", "jid", "foojid" );
    new Tag( x, "status", "code", "100" );
    new Tag( x, "status", "code", "101" );
    new Tag( x, "status", "code", "110" );
    new Tag( x, "status", "code", "170" );
    new Tag( x, "status", "code", "201" );
    new Tag( x, "status", "code", "210" );
    new Tag( x, "status", "code", "301" );
    new Tag( x, "status", "code", "303" );
    new Tag( x, "status", "code", "307" );
    new Tag( x, "status", "code", "321" );
    new Tag( x, "status", "code", "322" );
    new Tag( x, "status", "code", "332" );
    MUCRoom::MUCUser mu( x );
    t = mu.tag();
    if( !t || *t != *x )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:\n%s\n%s\n", name.c_str(), x->xml().c_str(), t->xml().c_str() );
    }
    delete t;
    delete x;
  }

  // -------
  {
    name = "inviting someone";
    MUCRoom::MUCUser mu( MUCRoom::OpInviteTo, "foo@bar", "why not?", "somethread" );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<invite to='foo@bar'>"
         "<reason>why not?</reason>"
         "<continue thread='somethread'/>"
         "</invite>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "being invited";
    MUCRoom::MUCUser mu( MUCRoom::OpInviteFrom, "foo@bar", "why not?", "somethread" );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<invite from='foo@bar'>"
         "<reason>why not?</reason>"
         "<continue thread='somethread'/>"
         "</invite>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "parse invitation";
    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_MUC_USER );
    Tag* i = new Tag( x, "invite" );
    i->addAttribute( "to", "foo@bar" );
    new Tag( i, "reason", "why not?" );
    new Tag( i, "continue", "thread", "somethread" );
    new Tag( x, "password", "foopwd" );
    MUCRoom::MUCUser mu( x );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<invite to='foo@bar'>"
         "<reason>why not?</reason>"
         "<continue thread='somethread'/>"
         "</invite>"
         "<password>foopwd</password>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete x;
  }

  // -------
  {
    name = "decline invitation";
    MUCRoom::MUCUser mu( MUCRoom::OpDeclineTo, "bar@foo", "because." );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<decline to='bar@foo'>"
         "<reason>because.</reason>"
         "</decline>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "decline invitation";
    MUCRoom::MUCUser mu( MUCRoom::OpDeclineFrom, "bar@foo", "because." );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<decline from='bar@foo'>"
         "<reason>because.</reason>"
         "</decline>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "parse decline";
    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_MUC_USER );
    Tag* i = new Tag( x, "decline" );
    i->addAttribute( "from", "foo@bar" );
    new Tag( i, "reason", "because." );
    MUCRoom::MUCUser mu( x );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<decline from='foo@bar'>"
         "<reason>because.</reason>"
         "</decline>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete x;
  }

  // -------
  {
    // this is for parsing incoming stanzas only.
    // for actively destroying a room (as the owner), see MUCOwner
    name = "destroying a room";
    Tag* x = new Tag( "x" );
    x->setXmlns( XMLNS_MUC_USER );
    Tag* d = new Tag( x, "destroy" );
    d->addAttribute( "jid", "foo@bar" );
    new Tag( d, "reason", "fooreason" );
    MUCRoom::MUCUser mu( x );
    t = mu.tag();
    if( !t || t->xml() != "<x xmlns='" + XMLNS_MUC_USER + "'>"
         "<destroy jid='foo@bar'>"
         "<reason>fooreason</reason>"
         "</destroy>"
         "</x>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s \n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete x;
  }

  // -------
  {
    name = "MUCRoom::MUCUser/SEFactory test (message)";
    StanzaExtensionFactory sef;
    sef.registerExtension( new MUCRoom::MUCUser() );
    Tag* f = new Tag( "message" );
    new Tag( f, "x", "xmlns", XMLNS_MUC_USER );
    Message msg( Message::Groupchat, JID(), "" );
    sef.addExtensions( msg, f );
    const MUCRoom::MUCUser* se = msg.findExtension<MUCRoom::MUCUser>( ExtMUCUser );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }

  // -------
  {
    name = "MUCRoom::MUCUser/SEFactory test (presence)";
    StanzaExtensionFactory sef;
    sef.registerExtension( new MUCRoom::MUCUser() );
    Tag* f = new Tag( "presence" );
    new Tag( f, "x", "xmlns", XMLNS_MUC_USER );
    Presence pres( Presence::Available, JID(), "" );
    sef.addExtensions( pres, f );
    const MUCRoom::MUCUser* se = pres.findExtension<MUCRoom::MUCUser>( ExtMUCUser );
    if( se == 0 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete f;
  }


  printf( "MUCRoom::MUCUser: " );
  if( !fail )
    printf( "OK\n" );
  else
    fprintf( stderr, "%d test(s) failed\n", fail );

  return fail;
}
