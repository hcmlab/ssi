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
    name = "set role 'none'";
    MUCRoom::MUCAdmin ma( RoleNone, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' role='none'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set role 'visitor'";
    MUCRoom::MUCAdmin ma( RoleVisitor, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' role='visitor'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set role 'participant'";
    MUCRoom::MUCAdmin ma( RoleParticipant, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' role='participant'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set role 'moderator'";
    MUCRoom::MUCAdmin ma( RoleModerator, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' role='moderator'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set affiliation 'none'";
    MUCRoom::MUCAdmin ma( AffiliationNone, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' affiliation='none'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set affiliation 'outcast'";
    MUCRoom::MUCAdmin ma( AffiliationOutcast, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' affiliation='outcast'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set affiliation 'member'";
    MUCRoom::MUCAdmin ma( AffiliationMember, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' affiliation='member'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set affiliation 'owner'";
    MUCRoom::MUCAdmin ma( AffiliationOwner, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' affiliation='owner'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "set affiliation 'admin'";
    MUCRoom::MUCAdmin ma( AffiliationAdmin, "foo", "fooish" );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item nick='foo' affiliation='admin'>"
         "<reason>fooish</reason>"
         "</item></query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "request role list: 'participant'";
    MUCRoom::MUCAdmin ma( RequestVoiceList );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item role='participant'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "request role list: 'moderator'";
    MUCRoom::MUCAdmin ma( RequestModeratorList );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item role='moderator'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "request affiliation list: 'outcast'";
    MUCRoom::MUCAdmin ma( RequestBanList );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item affiliation='outcast'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "request affiliation list: 'member'";
    MUCRoom::MUCAdmin ma( RequestMemberList );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item affiliation='member'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "store affiliation list: 'member'";
    MUCListItemList list;
    list.push_back( MUCListItem( JID( "foo@bar" ) ) );
    list.push_back( MUCListItem( JID( "bar@foo" ) ) );
    MUCRoom::MUCAdmin ma( StoreMemberList, list );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item jid='foo@bar' affiliation='member'/>"
         "<item jid='bar@foo' affiliation='member'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "store role list: 'participant'";
    MUCListItemList list;
    list.push_back( MUCListItem( JID( "foo@bar" ) ) );
    list.push_back( MUCListItem( JID( "bar@foo" ) ) );
    MUCRoom::MUCAdmin ma( StoreVoiceList, list );
    t = ma.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_MUC_ADMIN + "'>"
         "<item jid='foo@bar' role='participant'/>"
         "<item jid='bar@foo' role='participant'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "parse member list from Tag";
    Tag q( "query" );
    q.setXmlns( XMLNS_MUC_ADMIN );
    Tag* i = new Tag( &q, "item", "jid", "foo@bar" );
    i->addAttribute( "affiliation", "member" );
    i = new Tag( &q, "item", "jid", "bar@foo" );
    i->addAttribute( "affiliation", "member" );
    MUCRoom::MUCAdmin ma( &q );
    Tag* t = ma.tag();
    if( !t || q != *t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
  }

  // -------
  name = "MUCRoom::MUCAdmin/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new MUCRoom::MUCAdmin() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_MUC_ADMIN );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const MUCRoom::MUCAdmin* se = iq.findExtension<MUCRoom::MUCAdmin>( ExtMUCAdmin );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;


  printf( "MUCRoom::MUCAdmin: " );
  if( !fail )
    printf( "OK\n" );
  else
    fprintf( stderr, "%d test(s) failed\n", fail );

  return fail;
}
