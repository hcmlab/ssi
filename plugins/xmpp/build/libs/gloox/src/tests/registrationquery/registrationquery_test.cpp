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

#define REGISTRATION_TEST
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
      void registerIqHandler( IqHandler*, int ) {}
      void removeIqHandler( IqHandler*, int ) {}
      void registerStanzaExtension( StanzaExtension* ext ) { delete ext; }
      void removeStanzaExtension( int ) {}
      ConnectionState state() const { return StateConnected; }
      bool authed() { return false; }
  };
}

#define CLIENTBASE_H__
#include "../../registration.h"
#include "../../registration.cpp"

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;

  // -------
  {
    name = "fetch reg fields";
    Registration::Query sq;
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "receive reg fields";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_REGISTER );
    new Tag( d, "instructions", "foobar" );
    new Tag( d, "username" );
    new Tag( d, "password" );
    new Tag( d, "email" );
    Registration::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
         "<instructions>foobar</instructions>"
         "<username/>"
         "<password/>"
         "<email/>"
         "</query>"
         || sq.instructions() != "foobar"
         || sq.fields() != ( Registration::FieldUsername | Registration::FieldPassword
                             | Registration::FieldEmail ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }

  // -------
  {
    name = "reg by fields";
    RegistrationFields rf;
    rf.username = "foo";
    rf.password = "bar";
    rf.email = "email";
    Registration::Query sq( Registration::FieldUsername | Registration::FieldPassword
                            | Registration::FieldEmail, rf );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
         "<username>foo</username>"
         "<password>bar</password>"
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
    name = "receive search form";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_REGISTER );
    Tag* f = new Tag( d, "x" );
    f->setXmlns( XMLNS_X_DATA );
    f->addAttribute( "type", "form" );
    Registration::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
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
    name = "reg by form";
    DataForm* form = new DataForm( TypeSubmit );
    Registration::Query sq( form );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
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
    name = "remove account";
    Registration::Query sq( true );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
         "<remove/>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "change password";
    RegistrationFields rf;
    rf.username = "foobar";
    rf.password = "newpwd";
    Registration::Query sq( Registration::FieldUsername | Registration::FieldPassword, rf );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
         "<username>foobar</username>"
         "<password>newpwd</password>"
         "</query>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }

  // -------
  {
    name = "redirection";
    Tag* d = new Tag( "query" );
    d->setXmlns( XMLNS_REGISTER );
    new Tag( d, "instructions", "foobar" );
    Tag* x = new Tag( d, "x" );
    x->setXmlns( XMLNS_X_OOB );
    new Tag( x, "url", "http://camaya.net/gloox" );
    Registration::Query sq( d );
    Tag* t = sq.tag();
    if( !t || t->xml() != "<query xmlns='" + XMLNS_REGISTER + "'>"
         "<instructions>foobar</instructions>"
         "<x xmlns='" + XMLNS_X_OOB + "'>"
         "<url>http://camaya.net/gloox</url>"
         "</x>"
         "</query>"
         || sq.instructions() != "foobar"
         || sq.oob()->url() != "http://camaya.net/gloox" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
    delete d;
  }




  // -------
  name = "Registration::Query/SEFactory test";
  StanzaExtensionFactory sef;
  sef.registerExtension( new Registration::Query() );
  Tag* f = new Tag( "iq" );
  new Tag( f, "query", "xmlns", XMLNS_REGISTER );
  IQ iq( IQ::Get, JID() );
  sef.addExtensions( iq, f );
  const Registration::Query* se = iq.findExtension<Registration::Query>( ExtRegistration );
  if( se == 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;



  printf( "Registration::Query: " );
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
