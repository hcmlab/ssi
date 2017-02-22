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

#include "../../dataformfield.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  DataFormField *f;

  // -------
  name = "empty field";
  f = new DataFormField();
  if( f->type() != DataFormField::TypeTextSingle )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "TypeBoolean field";
  f = new DataFormField( DataFormField::TypeBoolean );
  if( f->type() != DataFormField::TypeBoolean )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "2nd ctor";
  f = new DataFormField( "fieldName", "fieldValue", "fieldLabel", DataFormField::TypeBoolean );
  if( f->type() != DataFormField::TypeBoolean || f->name() != "fieldName" ||
      f->value() != "fieldValue" || f->label() != "fieldLabel" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse 0";
  f = new DataFormField( 0 );
  if( f->type() != DataFormField::TypeInvalid )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;
  Tag*t;

  // -------
  name = "set name";
  f = new DataFormField();
  f->setName( name );
  if( f->name() != name )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set required";
  f = new DataFormField();
  bool req = true;
  f->setRequired( req );
  if( f->required() != req )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set label";
  f = new DataFormField();
  f->setLabel( name );
  if( f->label() != name )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set value";
  f = new DataFormField();
  f->setValue( name );
  if( f->value() != name )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set values";
  f = new DataFormField();
  StringList val;
  val.push_back( "val 1" );
  val.push_back( "val 2" );
  val.push_back( "val 3" );
  f->setValues( val );
  if( f->values() != val )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set values";
  f = new DataFormField();
  StringMultiMap opt;
  opt.insert( std::make_pair( "lock", "1" ) );
  opt.insert( std::make_pair( "stock", "1" ) );
  opt.insert( std::make_pair( "smoking barrel", "2" ) );
  f->setOptions( opt );
  if( f->options() != opt )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse Tag 1";
  t = new Tag( "field");
  t->addAttribute( "type", "fixed" );
  new Tag( t, "value", "abc" );
  f = new DataFormField( t );
  Tag *ft = f->tag();
  if( *ft != *t )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }
  delete f;
  delete t;
  delete ft;
  f = 0;

  // -------
  t = new Tag( "field");
  t->addAttribute( "type", "list-multi" );
  t->addAttribute( "label", "blabla label" );
  t->addAttribute( "var", "features" );
  Tag *o = new Tag( t, "option" );
  o->addAttribute( "label", "lock" );
  new Tag( o, "value", "lock" );
  o = new Tag( t, "option" );
  o->addAttribute( "label", "stock" );
  new Tag( o, "value", "stock" );
  o = new Tag( t, "option" );
  o->addAttribute( "label", "smoking barrel" );
  new Tag( o, "value", "smoking barrel" );
  new Tag( t, "value", "lock" );
  new Tag( t, "value", "stock" );
  f = new DataFormField( t );
  Tag *r = f->tag();
  name = "parse Tag 2.1";
  if( r->name() != "field" || !r->hasAttribute( "type", "list-multi" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.2";
  if( !r->hasAttribute( "label", "blabla label" ) || !r->hasAttribute( "var", "features" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.3";
  if( !r->hasChild( "option" ) || !r->findChild( "option" )->hasAttribute( "label", "lock" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.4";
  if( !r->hasChild( "option", "label", "stock" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.5";
  if( !r->hasChild( "option", "label", "smoking barrel" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.6";
  if( !r->findChild( "option" )->findChild( "value" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.7";
  if( r->findChild( "option" )->findChild( "value" )->cdata() != "lock" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.8";
  TagList l = r->children();
  TagList::const_iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    if( (*it)->name() == "option" && ( !(*it)->hasChildWithCData( "value", "lock" ) &&
          !(*it)->hasChildWithCData( "value", "stock" ) &&
          !(*it)->hasChildWithCData( "value", "smoking barrel" ) ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
      printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
      printf( "       t: %s\n", t->xml().c_str() );
    }
  }

  name = "parse Tag 2.9";
  if( !r->hasChildWithCData( "value", "lock" ) || !r->hasChildWithCData( "value", "stock" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }
  delete f;
  delete t;
  delete r;
  f = 0;



  // -------
  name = "boolean duplicate <value/>";
  f = new DataFormField( DataFormField::TypeBoolean );
  f->setName( "name" );
  f->setValue( "1" );
  f->setLabel( "label" );
  t = f->tag();
  if( t->children().size() != 1 || t->xml() != "<field type='boolean' var='name' "
                                               "label='label'><value>1</value></field>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }
  delete f;
  delete t;
  f = 0;
  t = 0;









  if( fail == 0 )
  {
    printf( "DataFormField: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "DataFormField: %d test(s) failed\n", fail );
    return 1;
  }

}
