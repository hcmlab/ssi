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
#include "../../util.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t = new Tag( "toe" ); t->addAttribute( "foo", "bar" );
  Tag *u = new Tag( t, "uni" ); u->addAttribute( "u3", "3u" );
  Tag *v = new Tag( t, "vie" ); v->addAttribute( "v3", "3v" );
  Tag *v2 = new Tag( t, "vie" ); v->addAttribute( "v32", "3v2" );
  Tag *w = new Tag( u, "who" ); w->addAttribute( "w3", "3w" );
  Tag *x = new Tag( v, "xep" ); x->addAttribute( "x3", "3x" );
  Tag *y = new Tag( u, "yps" ); y->addAttribute( "y3", "3y" );
  Tag *z = new Tag( w, "zoo" ); z->addAttribute( "z3", "3z" );
  Tag *c = 0;
  Tag *d = 0;

  // -------
  name = "simple ctor";
  if( t->name() != "toe" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "cdata ctor";
  c = new Tag( "cod", "foobar" );
  if( c->name() != "cod" || c->cdata() != "foobar" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "clone test 1";
  c = z->clone();
  if( *z != *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "clone test 2";
  c = t->clone();
  if( *t != *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 1";
  c = new Tag( "name" );
  if( *t == *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 2";
  c = new Tag( "test" );
  c->addAttribute( "me", "help" );
  c->addChild( new Tag( "yes" ) );
  if( *t == *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 3";
  c = new Tag( "hello" );
  c->addAttribute( "test", "bacd" );
  c->addChild( new Tag( "hello" ) );
  d = new Tag( "hello" );
  d->addAttribute( "test", "bacd" );
  d->addChild( new Tag( "helloo" ) );
  if( *d == *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete d;
  c = 0;
  d = 0;

  //-------
  name = "operator!= test 1";
  c = new Tag( "hello" );
  c->addAttribute( "test", "bacd" );
  c->addChild( new Tag( "hello" ) );
  d = new Tag( "hello" );
  d->addAttribute( "test", "bacd" );
  d->addChild( new Tag( "hello" ) );
  if( *d != *c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete d;
  c = 0;
  d = 0;

  //-------
  name = "findChildren test";
  TagList l = t->findChildren( "vie" );
  TagList::const_iterator it = l.begin();
  if( l.size() != 2 || (*it) != v || *(++it) != v2 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "util::escape";
  if ( util::escape( "&<>'\"" ) != "&amp;&lt;&gt;&apos;&quot;" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  //-------
  name = "xml() 1";
  if( t->xml() != "<toe foo='bar'><uni u3='3u'><who w3='3w'><zoo z3='3z'/></who><yps y3='3y'/>"
                    "</uni><vie v3='3v' v32='3v2'><xep x3='3x'/></vie><vie/></toe>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "xml() 2";
  t->addAttribute( "test", "bacd" );
  if( t->xml() != "<toe foo='bar' test='bacd'><uni u3='3u'><who w3='3w'><zoo z3='3z'/></who><yps y3='3y'/>"
                    "</uni><vie v3='3v' v32='3v2'><xep x3='3x'/></vie><vie/></toe>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "hasChild 1";
  if( !t->hasChild( "uni" ) || !t->hasChild( "vie" ) || !u->hasChild( "who" ) || !w->hasChild( "zoo" )
      || !u->hasChild( "yps" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "hasAttribute 1";
  if( !t->hasAttribute( "test" ) || !t->hasAttribute( "test", "bacd" )
      || !t->hasAttribute( "foo" ) || !t->hasAttribute( "foo", "bar" ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findAttribute 1";
  if( t->findAttribute( "test" ) != "bacd" || t->findAttribute( "foo" ) != "bar" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 1";
  c = t->findChild( "uni" );
  if( c != u )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 2";
  c = t->findChild( "uni", "u3" );
  if( c != u )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 3";
  c = t->findChild( "uni", "u3", "3u" );
  if( c != u )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChildWithAttrib 1";
  c = t->findChildWithAttrib( "u3" );
  if( c != u )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChildWithAttrib 2";
  c = t->findChildWithAttrib( "u3", "3u" );
  if( c != u )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "attribute order";
  c = new Tag( "abc" );
  c->addAttribute( "abc", "def" );
  c->addAttribute( "xyz", "123" );
  d = c->clone();
  if( *c != *d )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), d->xml().c_str() );
  }
  delete c;
  c = 0;
  delete d;
  d = 0;

  //-------
  name = "mixed content 1";
  c = new Tag( "abc" );
  c->addCData( "cdata1" );
  new Tag( c, "fgh" );
  c->addCData( "cdata2" );
  new Tag( c, "xyz" );
  c->addCData( "cdata3" );
  if( c->xml() != "<abc>cdata1<fgh/>cdata2<xyz/>cdata3</abc>" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), c->xml().c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator bool()";
  Tag tag1( "" );
  if( tag1 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), tag1.xml().c_str() );
  }

  //-------
  name = "bool operator!()";
  Tag tag2( "abc" );
  if( !tag2 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), d->xml().c_str() );
  }

  //-------
  {
    name = "simple xmlns";
    Tag t( "abc" );
    t.setXmlns( "foo" );
    if( t.xml() != "<abc xmlns='foo'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "deep xmlns";
    Tag t( "abc" );
    Tag* f = new Tag( &t, "def" );
    f = new Tag( f, "ghi" );
    t.setXmlns( "foo" );
    if( t.xml() != "<abc xmlns='foo'><def><ghi/></def></abc>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "simple nested xmlns 2";
    Tag t( "abc" );
    t.setXmlns( "foo" );
    Tag* d = new Tag( &t, "def" );
    d->setXmlns( "foobar", "xyz" );
    d->setPrefix( "xyz" );
    if( t.xml() != "<abc xmlns='foo'><xyz:def xmlns:xyz='foobar'/></abc>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "attribute with xmlns";
    Tag t( "abc" );
    t.setXmlns( "foo", "xyz" );
    Tag::Attribute* a = new Tag::Attribute( "foo", "bar", "foo" );
    a->setPrefix( "xyz" );
    t.addAttribute( a );
    if( t.xml() != "<abc xmlns:xyz='foo' xyz:foo='bar'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "escape attribute value";
    Tag t( "foo", "abc", "&amp;" );
    if( t.xml() != "<foo abc='&amp;amp;'/>" )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "remove child 1";
    Tag t( "foo" );
    t.addChild( new Tag( "test", "xmlns", "foo" ) );
    t.addChild( new Tag( "abc", "xmlns", "foobar" ) );
    t.addAttribute( "attr1", "value1" );
    t.addAttribute( "attr2", "value2" );
    t.removeChild( "test" );
    if( t.hasChild( "test" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }

    name = "remove child 2";
    t.removeChild( "abc", "foobar" );
    if( t.hasChild( "abc", "xmlns", "foobar" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }

    name = "remove attrib 1";
    t.removeAttribute( "attr1" );
    if( t.hasAttribute( "attr1", "value1") )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }

    name = "remove attrib 2";
    t.removeAttribute( "attr2", "value2" );
    if( t.hasAttribute( "attr2", "value2") )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "invalid chars 1";
    Tag t( "foo" );
    bool check = t.addAttribute( "nul", std::string( 1, 0x00 ) );
    if( check || t.hasAttribute( "nul" ) )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed:%s\n", name.c_str(), t.xml().c_str() );
    }
  }

  //-------
  {
    name = "invalid chars 2";
    for( int i = 0; i <= 0xff; ++i )
    {
      Tag::Attribute a( "test", std::string( 1, i ) );

      if( ( i < 0x09 || i == 0x0b || i == 0x0c
          || ( i > 0x0d && i < 0x20 ) || i == 0xc0
          || i == 0xc1 || i >= 0xf5 ) && a )
      {
        ++fail;
        fprintf( stderr, "test '%s' (branch 1) failed (i == %02X)\n", name.c_str(), i );
      }
      else if( ( i == 0x09 || i == 0x0a || i == 0x0d
                 || ( i >= 0x20 && i < 0xc0 )
                 || ( i > 0xc1 && i < 0xf5 ) ) && !a )
      {
        ++fail;
        fprintf( stderr, "test '%s' (branch 2) failed (i == %02X)\n", name.c_str(), i );
      }
//       printf( "i: 0x%02X, a: %d, value: %s\n", i, (bool)a, std::string( 1, i ).c_str() );
    }
  }




  delete t;
  t = 0;




















  if( fail == 0 )
  {
    printf( "Tag: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Tag: %d test(s) failed\n", fail );
    return 1;
  }

}
