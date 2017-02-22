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
#include "../../iq.h"
#include "../../iqhandler.h"
#include "../../messagehandler.h"
#include "../../bytestreamdatahandler.h"
#include "../../stanzaextensionfactory.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

gloox::JID g_jid( "foof" );

namespace gloox
{
  class Disco;

  class ClientBase
  {
    public:
      ClientBase() {}
      virtual ~ClientBase() {}
      const JID& jid() const { return m_jid; }
      const std::string getID();
      virtual void send( IQ& ) = 0;
      virtual void send( const IQ&, IqHandler*, int ) = 0;
      virtual void trackID( IqHandler *ih, const std::string& id, int context ) = 0;
      void removeIqHandler( IqHandler* ih, int exttype );
      void registerIqHandler( IqHandler* ih, int exttype );
      void registerMessageHandler( MessageHandler* ) {}
      void registerStanzaExtension( StanzaExtension* ext );
      void removeStanzaExtension( int ext );
      void removeIDHandler( IqHandler* ) {}
      void removeMessageHandler( MessageHandler* ) {}
    private:
      JID m_jid;
  };
  void ClientBase::removeIqHandler( IqHandler*, int ) {}
  void ClientBase::registerIqHandler( IqHandler*, int ) {}
  void ClientBase::registerStanzaExtension( StanzaExtension* se ) { delete se; }
  void ClientBase::removeStanzaExtension( int ) {}
  const std::string ClientBase::getID() { return "id"; }
}
using namespace gloox;

#define CLIENTBASE_H__
#define INBANDBYTESTREAM_TEST
#include "../../inbandbytestream.h"
#include "../../inbandbytestream.cpp"

class IBBTest : public ClientBase, public BytestreamDataHandler
{
  public:
    IBBTest() : m_result( 0 ) {}
    virtual ~IBBTest() {}
    void setTest( int test ) { m_test = test; }
    virtual void send( IQ& );
    virtual void send( const IQ&, IqHandler*, int );
    virtual void trackID( IqHandler*, const std::string&, int ) {}
    virtual void handleBytestreamData( Bytestream* /*bs*/, const std::string& data )
    {
      if( m_test == 5 && data == "data" )
        m_result++;
    }
    virtual void handleBytestreamError( Bytestream* /*bs*/, const IQ& /*iq*/ ) {}
    virtual void handleBytestreamOpen( Bytestream* /*bs*/ )
    {
      if( m_test == 1 || m_test == 4 )
        m_result++;
    }
    virtual void handleBytestreamClose( Bytestream* /*bs*/ )
    {
      if( m_test == 3 || m_test == 6 )
        m_result++;
      else
        m_result--;
    }
    int checkResult() { int t = m_result; m_result = 0; return t; }
  private:
    int m_test;
    int m_result;
};
void IBBTest::send( IQ& )
{
  if( m_test == 4 || m_test == 5 )
    m_result++;
}
void IBBTest::send( const IQ& iq, IqHandler* ih, int ctx )
{
  if( m_test == 1 )
  {
    const InBandBytestream::IBB* i = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( i && i->type() == InBandBytestream::IBBOpen )
      m_result++;
  }
  else if( m_test == 2 )
  {
    const InBandBytestream::IBB* i = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( i && i->type() == InBandBytestream::IBBData )
      m_result++;
  }
  else if( m_test == 3 )
  {
    const InBandBytestream::IBB* i = iq.findExtension<InBandBytestream::IBB>( ExtIBB );
    if( i && i->type() == InBandBytestream::IBBClose )
      m_result++;
  }

  IQ re( IQ::Result, iq.from(), iq.id() );
  ih->handleIqID( re, ctx );
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  IBBTest* it = new IBBTest();
  LogSink li;
  InBandBytestream ibb( it, li, JID( "foof" ), JID( "toof" ), "sid" );
  ibb.registerBytestreamDataHandler( it );

  // -------
  {
    name = "open ibb";
    it->setTest( 1 );
    ibb.connect();
    if( it->checkResult() != 2 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "send data";
    it->setTest( 2 );
    ibb.send( "data" );
    if( !it->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "close ibb";
    it->setTest( 3 );
    ibb.close();
    if( !it->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "open ibb by tag";
    it->setTest( 4 );
    IQ iq( IQ::Set, JID(), it->getID() );
    iq.addExtension( new InBandBytestream::IBB( "sid", 4096 ) );
    ibb.handleIq( iq );
    if( it->checkResult() != 2 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "receive data ";
    it->setTest( 5 );
    IQ iq( IQ::Set, JID(), it->getID() );
    iq.addExtension( new InBandBytestream::IBB( "sid", 0, "data" ) );
    ibb.handleIq( iq );
    if( it->checkResult() != 2 )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  // -------
  {
    name = "close ibb by tag";
    it->setTest( 6 );
    IQ iq( IQ::Set, JID(), it->getID() );
    iq.addExtension( new InBandBytestream::IBB( "sid" ) );
    ibb.handleIq( iq );
    if( !it->checkResult() )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
  }

  delete it;


  if( fail == 0 )
  {
    printf( "InBandBytestream: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "InBandBytestream: %d test(s) failed\n", fail );
    return 1;
  }

}
