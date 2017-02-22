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

#define SEARCH_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../dataform.h"
#include "../../tag.h"
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

namespace gloox
{

  class Disco;
  class Capabilities : public StanzaExtension
  {
    public:
      Capabilities() : StanzaExtension( ExtUser + 1 ) {}
      const std::string& ver() const { return EmptyString; }
      const std::string& node() const { return EmptyString; }
  };

  class ClientBase
  {
    public:
      ClientBase() {}
      virtual ~ClientBase() {}
      const std::string getID() { return "id"; }
      virtual void send( IQ& iq, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIDHandler( IqHandler* ) {}
      void registerStanzaExtension( StanzaExtension* ext ) { delete ext; }
      void removeStanzaExtension( int ) {}
  };

}

#define CLIENTBASE_H__
#include "../../search.h"
#include "../../search.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  {
    name = "fetch fields";
    Search::Query sq;
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "receive search fields";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_SEARCH );
    new Tag( d, "instructions", "foobar" );
    new Tag( d, "first" );
    new Tag( d, "last" );
    new Tag( d, "email" );
    new Tag( d, "nick" );
    Search::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
         "<instructions>foobar</instructions>"
         "<first/>"
         "<last/>"
         "<nick/>"
         "<email/>"
         "</query>"
         || sq.instructions() != "foobar"
         || sq.fields() != ( SearchFieldFirst | SearchFieldLast | SearchFieldNick | SearchFieldEmail ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }

  // -------
  {
    name = "receive search form";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_SEARCH );
    Tag* f = new Tag( d, "x" );
    f->setXmlns( XMLNS_X_DATA );
    f->addAttribute( "type", "form" );
    Search::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
         "<x xmlns='" + XMLNS_X_DATA + "' type='form'/>"
         "</query>"
         || !sq.form() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete d;
  }

  // -------
  {
    name = "search by form";
    DataForm* form = new DataForm( TypeSubmit );
    Search::Query sq( form );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
       "<x xmlns='" + XMLNS_X_DATA + "' type='submit'/>"
       "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "search by fields";
    SearchFieldStruct sfs( "first", "last", "nick", "email" );
    Search::Query sq( SearchFieldFirst | SearchFieldLast | SearchFieldNick | SearchFieldEmail, sfs );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
         "<first>first</first>"
         "<last>last</last>"
         "<nick>nick</nick>"
         "<email>email</email>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "receive form result";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_SEARCH );
    Tag* f = new Tag( d, "x" );
    f->setXmlns( XMLNS_X_DATA );
    f->addAttribute( "type", "result" );
    Search::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
         "<x xmlns='" + XMLNS_X_DATA + "' type='result'/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }

  // -------
  {
    name = "receive fields result";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_SEARCH );
    Tag* i = new Tag( d, "item" );
    i->addAttribute( "jid", "foo@bar" );
    new Tag( i, "first", "first1" );
    new Tag( i, "last", "last1" );
    new Tag( i, "email", "email1" );
    new Tag( i, "nick", "nick1" );
    i = new Tag( d, "item" );
    i->addAttribute( "jid", "foo@bar2" );
    new Tag( i, "first", "first2" );
    new Tag( i, "last", "last2" );
    new Tag( i, "nick", "nick2" );
    new Tag( i, "email", "email2" );
    Search::Query sq( d );
    Tag* t = sq.tag();
    SearchResultList srl = sq.result();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_SEARCH + "'>"
         "<item jid='foo@bar'>"
         "<first>first1</first>"
         "<last>last1</last>"
         "<nick>nick1</nick>"
         "<email>email1</email></item>"
         "<item jid='foo@bar2'>"
         "<first>first2</first>"
         "<last>last2</last>"
         "<nick>nick2</nick>"
         "<email>email2</email></item>"
         "</query>"
       || srl.size() != 2 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
    }
    delete t;
    delete d;
  }




  // -------
  name = "Search::Query/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Search::Query() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_SEARCH );
  IQ iq( IQ::Get, JID() );
  sef.addExtensions( iq, f );
  const Search::Query* se = iq.findExtension<Search::Query>( ExtSearch );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;



  printf( "Search::Query: " );
  if( fail == 0 )
  {
    printf( "OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "%d test(s) failed\n", fail );
    return 1;
  }

}
