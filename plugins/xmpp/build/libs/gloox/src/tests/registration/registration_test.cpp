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

#define GLOOX_TESTS
#include "../../gloox.h"
#include "../../jid.h"
#include "../../dataform.h"
#include "../../stanza.h"
#include "../../tag.h"
#include "../../iqhandler.h"
#include "../../iq.h"
#include "../../stanzaextension.h"
#include "../../oob.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

const std::string& g_server = "test.server";
const std::string& g_inst = "the instructions";

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
      bool authed() { return true; }
  };

}

#define CAPABILITIES_H__
#define CLIENTBASE_H__
#define REGISTRATION_TEST
#include "../../registration.h"
#include "../../registration.cpp"
#include "../../registrationhandler.h"

class RegistrationTest : public gloox::RegistrationHandler, public gloox::ClientBase
{
  public:
    RegistrationTest();
    ~RegistrationTest();
    virtual void handleRegistrationFields( const gloox::JID& /*from*/, int fields,
                                           std::string instructions )
    {
      if( m_test == 2 && fields & ( gloox::Registration::FieldUsername | gloox::Registration::FieldPassword
                                    | gloox::Registration::FieldEmail ) && instructions == g_inst )
      {
        m_result = true;
      }
      m_test = 0;
    }

    virtual void handleAlreadyRegistered( const gloox::JID& from )
    {
      if( m_test == 3 && from == g_server )
      {
        m_result = true;
      }
      m_test = 0;
    }

    virtual void handleRegistrationResult( const gloox::JID& /*from*/, gloox::RegistrationResult regResult )
    {
      if( ( m_test == 4 || m_test == 6 || m_test == 7 || m_test == 8 )
            && regResult == gloox::RegistrationSuccess )
        m_result = true;

      m_test = 0;
    }

    virtual void handleDataForm( const gloox::JID& /*from*/, const gloox::DataForm& form )
    {
      if( m_test == 5 && form )
        m_result = true;

      m_test = 0;
    }

    virtual void handleOOB( const gloox::JID& /*from*/, const gloox::OOB& /*oob*/ )
    {
    }

    virtual void send( gloox::IQ& iq, gloox::IqHandler* ih, int context )
    {
      m_context = context;
      gloox::Tag* tag = iq.tag();
      if( !tag->hasAttribute( "id" ) )
        tag->addAttribute( "id", "id" );

      switch( m_test )
      {
        case 1:
          if( tag && tag->hasAttribute( "id", "id" ) && tag->hasAttribute( "to", g_server )
               && tag->hasAttribute( "type", "get" ) && tag->hasChild( "query", "xmlns",
                                                                       gloox::XMLNS_REGISTER ) )
            m_result = true;
          m_test = 0;
          break;
        case 4:
        {
          gloox::Tag *t = 0;
          if( tag && tag->hasAttribute( "id", "id" ) && tag->hasAttribute( "to", g_server )
              && tag->hasAttribute( "type", "set" )
              && ( t = tag->findChild( "query", "xmlns", gloox::XMLNS_REGISTER ) ) != 0
              && t->hasChildWithCData( "username", "foobar" )
              && t->hasChildWithCData( "password", "password" )
              && t->hasChildWithCData( "email", "email" ) )
          {
            gloox::IQ re( gloox::IQ::Result, iq.from(), iq.id() );
            ih->handleIqID( re, context );
          }
          break;
        }
        case 6:
        {
          gloox::Tag *t = 0;
          if( tag && tag->hasAttribute( "id", "id" ) && tag->hasAttribute( "to", g_server )
               && tag->hasAttribute( "type", "set" )
               && ( ( t = tag->findChild( "query", "xmlns", gloox::XMLNS_REGISTER ) ) != 0 )
               && t->hasChild( "x", "xmlns", gloox::XMLNS_X_DATA ) )
          {
            gloox::IQ re( gloox::IQ::Result, iq.from(), iq.id() );
            ih->handleIqID( re, context );
          }
          break;
        }
        case 7:
        {
          gloox::Tag *t = 0;
          if( tag && tag->hasAttribute( "id", "id" ) && tag->hasAttribute( "to", g_server )
              && tag->hasAttribute( "type", "set" )
              && ( ( t = tag->findChild( "query", "xmlns", gloox::XMLNS_REGISTER ) ) != 0 )
              && t->hasChild( "remove" ) )
          {
            gloox::IQ re( gloox::IQ::Result, iq.from(), iq.id() );
            ih->handleIqID( re, context );
          }
          break;
        }
        case 8:
        {
          gloox::Tag *t = 0;
          if( tag && tag->hasAttribute( "id", "id" ) && tag->hasAttribute( "to", g_server )
              && tag->hasAttribute( "type", "set" )
              && ( ( t = tag->findChild( "query", "xmlns", gloox::XMLNS_REGISTER ) ) != 0 )
              && t->hasChildWithCData( "username", "foobar" )
              && t->hasChildWithCData( "password", "newpwd" ) )
          {
            gloox::IQ re( gloox::IQ::Result, iq.from(), iq.id() );
            ih->handleIqID( re, context );
          }
          break;
        }
        default:
          break;
      }
      delete tag;
    }
    void setTest( int test ) { m_test = test; }
    void fetchRegistrationFields() { m_reg.fetchRegistrationFields(); }
    void createAccount( int fields, const gloox::RegistrationFields& values )
      { m_reg.createAccount( fields, values ); }
    void createAccount( gloox::DataForm* form )
      { m_reg.createAccount( form ); }
    void removeAccount() { m_reg.removeAccount(); }
    void changePassword( const std::string& username, const std::string& password )
      { m_reg.changePassword( username, password ); }
    bool result() { bool t = m_result; m_result = false; return t; }
    void feed( gloox::IQ& s ) { m_reg.handleIqID( s, m_context ); }
    virtual void trackID( gloox::IqHandler* /*ih*/, const std::string& /*id*/, int /*context*/ ) {}
  private:
    gloox::Registration m_reg;
    int m_test;
    int m_context;
    bool m_result;
};

RegistrationTest::RegistrationTest() : m_reg( this, g_server ), m_test( 0 ),
                                       m_context( -1 ), m_result( false )
{
  m_reg.registerRegistrationHandler( this );
}

RegistrationTest::~RegistrationTest() {}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  RegistrationTest t;

  // -------
  {
    name = "fetch fields (old-style)";
    t.setTest( 1 );
    t.fetchRegistrationFields();
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "receive fields (old-style)";
    gloox::IQ iq( gloox::IQ::Result, gloox::JID( "regtest" ), "id" );
    gloox::Tag* q = new gloox::Tag( "query" );
    q->setXmlns( gloox::XMLNS_REGISTER );
    new gloox::Tag( q, "instructions", g_inst );
    new gloox::Tag( q, "username", "foo" );
    new gloox::Tag( q, "password", "bar" );
    new gloox::Tag( q, "email", "email" );
    iq.addExtension( new gloox::Registration::Query( q ) );
    iq.setFrom( gloox::JID( g_server ) );
    t.setTest( 2 );
    t.feed( iq );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete q;
  }

  // -------
  {
    name = "already registered";
    gloox::IQ iq( gloox::IQ::Result, gloox::JID( "regtest" ), "id" );
    gloox::Tag* q = new gloox::Tag( "query" );
    q->setXmlns( gloox::XMLNS_REGISTER );
    new gloox::Tag( q, "registered" );
    new gloox::Tag( q, "username" );
    new gloox::Tag( q, "nick" );
    new gloox::Tag( q, "email" );
    iq.addExtension( new gloox::Registration::Query( q ) );
    iq.setFrom( gloox::JID( g_server ) );
    t.setTest( 3 );
    t.feed( iq );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete q;
  }

  // -------
  {
    name = "register (old-style)";
    gloox::RegistrationFields rf;
    rf.username = "foobar";
    rf.password = "password";
    rf.email = "email";
    t.setTest( 4 );
    t.createAccount( gloox::Registration::FieldUsername | gloox::Registration::FieldPassword
                     | gloox::Registration::FieldEmail, rf );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "fetch fields (form)";
    t.setTest( 1 );
    t.fetchRegistrationFields();
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "receive form";
    gloox::IQ iq( gloox::IQ::Result, gloox::JID( "regtest" ), "id" );
    gloox::Tag* q = new gloox::Tag( "query" );
    q->setXmlns( gloox::XMLNS_REGISTER );
    gloox::Tag* x = new gloox::Tag( q, "x" );
    x->setXmlns( gloox::XMLNS_X_DATA );
    x->addAttribute( "type", "form" );
    iq.addExtension( new gloox::Registration::Query( q ) );
    iq.setFrom( gloox::JID( g_server ) );
    t.setTest( 5 );
    t.feed( iq );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete q;
  }

  // -------
  {
    name = "register (form)";
    gloox::DataForm* form = new gloox::DataForm( gloox::TypeSubmit );
    t.setTest( 6 );
    t.createAccount( form );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "remove account";
    t.setTest( 7 );
    t.removeAccount();
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "change password";
    t.setTest( 8 );
    t.changePassword( "foobar", "newpwd" );
    if( !t.result() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }








  printf( "Registration: " );
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
