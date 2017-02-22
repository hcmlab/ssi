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
#include "../../iodata.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  IOData *i = 0;
  Tag* t = 0;
  Tag* f = 0;
  Tag* s = new Tag( "iodata" );
  s->setXmlns( XMLNS_IODATA );


  // -------
  name = "parsing 0 tag";
  i = new IOData( 0 );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing 'io-schemata-get' type";
  s->addAttribute( "type", "io-schemata-get" );
  i = new IOData( s );
  if( i->tag()->xml() != s->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing 'input' type";
  t = s->clone();
  t->addAttribute( "type", "input" );
  f = new Tag( t, "in" );
  new Tag( f, "foo" );
  i = new IOData( t );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "parsing 'getStatus' type";
  s->addAttribute( "type", "getStatus" );
  i = new IOData( s );
  if( i->tag()->xml() != s->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing 'getOutput' type";
  s->addAttribute( "type", "getOutput" );
  i = new IOData( s );
  if( i->tag()->xml() != s->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing 'io-schemata-result' type";
  t = s->clone();
  t->addAttribute( "type", "io-schemata-result" );
  f = new Tag( t, "in" );
  new Tag( f, "foo" );
  f = new Tag( t, "out" );
  new Tag( f, "foobar" );
  f = new Tag( t, "desc", "some description" );
  i = new IOData( t );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "parsing 'output' type";
  t = s->clone();
  t->addAttribute( "type", "output" );
  f = new Tag( t, "out" );
  new Tag( f, "foo" );
  i = new IOData( t );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "parsing 'error' type";
  t = s->clone();
  t->addAttribute( "type", "error" );
  f = new Tag( t, "error", "some error description" );
  i = new IOData( t );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "parsing 'status' type";
  t = s->clone();
  t->addAttribute( "type", "status" );
  f = new Tag( t, "status" );
  new Tag( f, "elapsed", "12" );
  new Tag( f, "remaining", "34" );
  new Tag( f, "percentage", "56" );
  new Tag( f, "information", "some information" );
  i = new IOData( t );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'io-schemata-get' type";
  t = s->clone();
  t->addAttribute( "type", "io-schemata-get" );
  i = new IOData( IOData::TypeIoSchemataGet );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'input' type";
  t = s->clone();
  t->addAttribute( "type", "input" );
  f = new Tag( t, "in" );
  f = new Tag( f, "foo" );
  i = new IOData( IOData::TypeInput );
  i->setIn( f->clone() );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'getStatus' type";
  t = s->clone();
  t->addAttribute( "type", "getStatus" );
  i = new IOData( IOData::TypeGetStatus );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'getOutput' type";
  t = s->clone();
  t->addAttribute( "type", "getOutput" );
  i = new IOData( IOData::TypeGetOutput );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'io-schemata-result' type";
  t = s->clone();
  t->addAttribute( "type", "io-schemata-result" );
  i = new IOData( IOData::TypeIoSchemataResult );
  f = new Tag( t, "in" );
  f = new Tag( f, "foo" );
  i->setIn( f->clone() );
  f = new Tag( t, "out" );
  f = new Tag( f, "foobar" );
  i->setOut( f->clone() );
  f = new Tag( t, "desc", "some description" );
  i->setDesc( "some description" );
  if( i->tag()->xml() != t->xml() || i->desc() != "some description" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'output' type";
  t = s->clone();
  t->addAttribute( "type", "output" );
  i = new IOData( IOData::TypeOutput );
  f = new Tag( t, "out" );
  f = new Tag( f, "foobar" );
  i->setOut( f->clone() );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'error' type";
  t = s->clone();
  t->addAttribute( "type", "error" );
  i = new IOData( IOData::TypeError );
  f = new Tag( t, "error" );
  f = new Tag( f, "foo" );
  i->setError( f->clone() );
  if( i->tag()->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;

  // -------
  name = "creating 'status' type";
  t = s->clone();
  t->addAttribute( "type", "status" );
  i = new IOData( IOData::TypeStatus );
  f = new Tag( t, "status" );
  new Tag( f, "elapsed", "12" );
  new Tag( f, "remaining", "34" );
  new Tag( f, "percentage", "56" );
  new Tag( f, "information", "some info" );
  IOData::Status st = { 12, 34, 56, "some info" };
  i->setStatus( st );
  if( i->tag()->xml() != t->xml() || i->status().elapsed != st.elapsed
      || i->status().remaining != st.remaining || i->status().percentage != st.percentage
      || i->status().info != st.info )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\ni: %s\ns: %s\n", name.c_str(), i->tag()->xml().c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;


  // -------
  name = "cloning";
  i = new IOData( IOData::TypeStatus );
  i->setStatus( st );
  i->setOut( new Tag( "foo" ) );
  IOData* j = i->clone();
  if( !j || !j->out() || i->out()->name() != "out" || !j->out()->hasChild( "foo" ) || j->status().elapsed != st.elapsed )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;
  delete j;
  j = 0;







  if( fail == 0 )
  {
    printf( "IOData: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "IOData: %d test(s) failed\n", fail );
    return 1;
  }

}
