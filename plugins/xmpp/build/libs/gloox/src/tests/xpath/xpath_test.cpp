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
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int fail = 0;

void printResult( const std::string& name, ConstTagList& result )
{
  printf( ">-- %s --------------------------------------\n", name.c_str() );
  int i = 0;
  ConstTagList::const_iterator it = result.begin();
  for( ; it != result.end(); ++it, ++i )
  {
    printf( "tag #%d: %s\n", i, (*it)->xml().c_str() );
  }
  printf( "<-- %s --------------------------------------\n", name.c_str() );
}

// void testLexer( const std::string& name )
// {
//   int len = 0;
//   XPathToken *t = XPath::parse( name, len );
//   if( !t || t->toString() != name )
//   {
//     ++fail;
//     fprintf( stderr, "test 'lexer: %s' failed: %s\n", name.c_str(), t->toString().c_str() );
//   }
//   printf( "str: %s\n", t->toString().c_str() );
//   printf( "xml: %s\n", t->xml().c_str() );
//   delete t;
// }

int main( int /*argc*/, char** /*argv*/ )
{
  std::string name;
  Tag *aaa = new Tag( "aaa" );
  Tag *bbb = new Tag( aaa, "bbb" ); bbb->addAttribute( "name", "b1" );
  Tag *ccc = new Tag( aaa, "ccc" ); ccc->setCData( "abc" );
  Tag *ddd = new Tag( ccc, "ddd" ); ddd->setCData( "bcd" );
  Tag *eee = new Tag( ccc, "eee" );
  Tag *fff = new Tag( aaa, "fff" );
  Tag *ggg = new Tag( fff, "ggg" );
  Tag *hhh = new Tag( bbb, "hhh" ); hhh->addAttribute( "name", "h1" );
  Tag *iii = new Tag( bbb, "bbb" ); iii->addAttribute( "name", "b2" );
  Tag *jjj = new Tag( hhh, "bbb" ); jjj->addAttribute( "name", "b3" );
  ConstTagList result;
  ConstTagList::const_iterator it;
//   XPathToken *t = 0;

// <aaa>
//   <bbb name='b1'>
//     <hhh>
//       <bbb name='b3'/>
//     </hhh>
//     <bbb name='b2'/>
//   </bbb>
//   <ccc>
//     <ddd/>
//     <eee/>
//   </ccc>
//   <fff>
//     <ggg/>
//   </fff>
// </aaa>

  /*
   * Lexer tests
   */

//   // -------
//   name = "/";
//   t = XPath::parse( name );
//   if( t != 0 )
//   {
//     ++fail;
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
//   }
//   delete t;

//   // -------
//   name = "//";
//   t = XPath::parse( name );
//   if( t != 0 )
//   {
//     ++fail;
//     fprintf( stderr, "test 'lexer: %s' failed\n", name.c_str() );
//   }
//   delete t;

  // ------- working
//   testLexer( "/abc" );
//   testLexer( "/abc/def" );
//   testLexer( "/abc//def" );
//
//   testLexer( "/abc/def[//dgh]" );
//
//   testLexer( "count(//dgh)" );
//
//   testLexer( "/*/abc" );
//
//   testLexer( "count(count(//dgh))" );
//
//   testLexer( "*/abc" );
//
//   testLexer( "*" );
//
//   testLexer( "//*" );
//
//   testLexer( "count(count(//dgh[//abc]))" );
//
//   testLexer( "//c[id>count(//aaa|//bbb)]" );
//
//   testLexer( "a/*/b" );
//
//   testLexer( "a<b" );
//
//   testLexer( "/./*" );
//
//   testLexer( "/./../*" );
//
//   testLexer( "/./../../." );
//
//   testLexer( "a=b" );
//
//   testLexer( "/b[@a='b']" );

  // ------- ~working


//   testLexer( "a*b" );

//   testLexer( "a*b" );

//   testLexer( "a*b" );









  //   testLexer( "//c[id>count(//aaa|//bbb*//ccc)]" );





//   testLexer( "//aaa|//bbb*(//ccc+//abc)" );

// //   testLexer( "//a|(//b*//c)+//d" );

//   testLexer( "//a|//b*//c+//d" );

//   testLexer( "//c[id>count(//aaa|//bbb*//ccc)+//abc]" );












  // -- simple paths --

  // -------
  name = "get root: /";
  if( aaa->findTag( "/" ) != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get deeproot: //";
  if( aaa->findTag( "//" ) != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get root tag: aaa";
  if( aaa->findTag( "aaa" ) != aaa )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get root tag: /aaa";
  if( aaa->findTag( "/aaa" ) != aaa )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "simple child: /aaa/bbb";
  if( aaa->findTag( "/aaa/bbb" ) != bbb )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "simple child: /aaa/ccc";
  if( aaa->findTag( "/aaa/ccc" ) != ccc )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "simple child: /aaa/ccc/ddd";
  if( aaa->findTag( "/aaa/ccc/ddd" ) != ddd )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find all: //aaa";
  if( aaa->findTag( "//aaa" ) != aaa )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find all: //eee";
  if( aaa->findTag( "//eee" ) != eee )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find all: //bbb";
  if( aaa->findTag( "//bbb" ) != bbb )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "get root tag from child: /aaa";
  if( bbb->findTag( "/aaa" ) != aaa )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "fail test 1: /abc";
  if( aaa->findTag( "/abc" ) != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "fail test 2: /bbb";
  if( aaa->findTag( "/bbb" ) != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "relative find 1: aaa";
  if( aaa->findTag( "aaa" ) != aaa )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "relative find 2: bbb";
  if( bbb->findTag( "bbb" ) != bbb )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find ConstTagList: //bbb";
  result = aaa->findTagList( "//bbb" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find ConstTagList: //ggg";
  result = aaa->findTagList( "//ggg" );
  it = result.begin();
  if( result.size() != 1 || (*it) != ggg )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find all: //*";
  result = aaa->findTagList( "//*" );
  it = result.begin();
  if( result.size() != 10 || (*it) != aaa || (*++it) != bbb || (*++it) != hhh ||
      (*++it) != jjj || (*++it) != iii || (*++it) != ccc || (*++it) != ddd ||
      (*++it) != eee || (*++it) != fff || (*++it) != ggg )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find first level: /*";
  result = aaa->findTagList( "/*" );
  if( result.size() != 1 || result.front() != aaa )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find second level: /*/*";
  result = aaa->findTagList( "/*/*" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != ccc || (*++it) != fff )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find third level: /*/*/*";
  result = aaa->findTagList( "/*/*/*" );
  it = result.begin();
  if( result.size() != 5 || (*it) != hhh || (*++it) != iii ||
      (*++it) != ddd || (*++it) != eee || (*++it) != ggg )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find fourth level: /*/*/*/*";
  result = aaa->findTagList( "/*/*/*/*" );
  if( result.size() != 1 || result.front() != jjj )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find fith level: /*/*/*/*/*";
  result = aaa->findTagList( "/*/*/*/*/*" );
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find all sub-bbb: /*/*//bbb";
  result = aaa->findTagList( "/*/*//bbb" );
  if( result.size() != 2 || result.front() != jjj || result.back() != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find second level bbb: /*/bbb";
  result = aaa->findTagList( "/*/bbb" );
  if( result.size() != 1 || result.front() != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find second level via self/noop: /*/./*";
  result = aaa->findTagList( "/*/./*" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != ccc || (*++it) != fff )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find second level via repeated self/noop: /*/././*";
  result = aaa->findTagList( "/*/././*" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != ccc || (*++it) != fff )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "find first level via parent: /*/../*";
  result = aaa->findTagList( "/*/../*" );
  it = result.begin();
  if( result.size() != 1 || (*it) != aaa )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "invalid parent: /../*";
  result = aaa->findTagList( "/../*" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch && * combined 1: //fff/*";
  result = aaa->findTagList( "//fff/*" );
  it = result.begin();
  if( result.size() != 1 || (*it) != ggg )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch && .. combined 2: //ggg/..";
  result = aaa->findTagList( "//ggg/.." );
  it = result.begin();
  if( result.size() != 1 || (*it) != fff )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

//   printf( "--------------------------------------------------------------\n" );
//   // -------
//   name = "select non-leaf elements: //..";
//   result = aaa->findTagList( "//.." );
//   it = result.begin();
//   if( result.size() != 5 || (*it) != aaa || (*++it) != bbb ||
//       (*++it) != hhh || (*++it) != ccc || (*++it) != fff )
//   {
//     ++fail;
//     printResult( name, result );
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
//   }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "deepsearch && .. combined 3: //ggg/../../bbb";
  result = aaa->findTagList( "//ggg/../../bbb" );
  it = result.begin();
  if( result.size() != 1 || (*it) != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch && .. combined 4: //ggg/../..//bbb";
  result = aaa->findTagList( "//ggg/../..//bbb" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch && .. && * combined 1: //*/../..//bbb";
  result = aaa->findTagList( "//*/../..//bbb" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch && .. && * combined 2: //*/*/..//*";
  result = aaa->findTagList( "//*/*/..//*" );
  it = result.begin();
  if( result.size() != 9 || (*it) != bbb || (*++it) != hhh || (*++it) != jjj ||
      (*++it) != iii || (*++it) != ccc || (*++it) != ddd || (*++it) != eee ||
      (*++it) != fff || (*++it) != ggg )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "deepsearch: //bbb/hhh/bbb";
  result = aaa->findTagList( "//bbb/hhh/bbb" );
  it = result.begin();
  if( result.size() != 1 || (*it) != jjj )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -- ~simple paths --


  // -- operators --

  // ---- union ----

  // -------
  name = "union 1: //bbb|/aaa";
  result = aaa->findTagList( "//bbb|/aaa" );
  it = result.begin();
  if( result.size() != 4 || (*it) != bbb || (*++it) != jjj || (*++it) != iii || (*++it) != aaa )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "union 1: //bbb|//bbb";
  result = aaa->findTagList( "//bbb|//bbb" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "union 2: /aaa|/aaa";
  result = aaa->findTagList( "/aaa|/aaa" );
  it = result.begin();
  if( result.size() != 1 || (*it) != aaa )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "union 3: /aaa|/aaa|//bbb";
  result = aaa->findTagList( "/aaa|/aaa|//bbb" );
  it = result.begin();
  if( result.size() != 4 || (*it) != aaa || (*++it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );


  // -------
  name = "union + predicates: //bbb[@name='b1']|//hhh[@name='h1']";
  result = aaa->findTagList( "//bbb[@name='b1']|//hhh[@name='h1']" );
  it = result.begin();
  if( result.size() != 2 || (*it) != bbb || (*++it) != hhh )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "empty union 1: /cde|/def";
  result = aaa->findTagList( "/cde|/def" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );


  // ---- ~union ----

  // -- ~operators --



  // -- predicates --

  // -------
  name = "filter 1: //bbb[1]";
  result = aaa->findTagList( "//bbb[1]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "filter 2: //bbb[2]";
  result = aaa->findTagList( "//bbb[2]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != jjj )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "filter 3: //bbb[3]";
  result = aaa->findTagList( "//bbb[3]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "invalid filter 4: //bbb[4]";
  result = aaa->findTagList( "//bbb[4]" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "filter 5: /aaa/bbb[1]";
  result = aaa->findTagList( "/aaa/bbb[1]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "filter 5: /aaa[1]";
  result = aaa->findTagList( "/aaa[1]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != aaa )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "invalid filter 6: /aaa[2]";
  result = aaa->findTagList( "/aaa[2]" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "invalid filter 7: [2]";
  result = aaa->findTagList( "[2]" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "deepsearch + predicate 1: //bbb[@name]";
  result = aaa->findTagList( "//bbb[@name]" );
  it = result.begin();
  if( result.size() != 3 || (*it) != bbb || (*++it) != jjj || (*++it) != iii )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "deepsearch + predicate 2: //bbb[@xyz]";
  result = aaa->findTagList( "//bbb[@xyz]" );
  it = result.begin();
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  // -------
  name = "deepsearch + predicate + literal 1: //bbb[@name='b1']";
  result = aaa->findTagList( "//bbb[@name='b1']" );
  it = result.begin();
  if( result.size() != 1 || (*it) != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  name = "deepsearch + predicate + literal 2: //bbb[@name='test@test2']";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']" );
  it = result.begin();
  if( result.size() != 1 || (*it) != bbb )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "deepsearch + predicate + literal + child: //bbb[@name='b1']/hhh";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']/hhh" );
  it = result.begin();
  if( result.size() != 1 || (*it) != hhh )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "deepsearch + predicate + literal + child + predicate 1: //bbb[@name='b1']/hhh[@name]";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']/hhh[@name]" );
  it = result.begin();
  if( result.size() != 1 || (*it) != hhh )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "deepsearch + predicate + literal + child + predicate 2: //bbb[@name='b1']/hhh[@name1]";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']/hhh[@name1]" );
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "deepsearch + predicate + literal + child + predicate + literal: //bbb[@name='b1']/hhh[@name='h1']";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']/hhh[@name='h1']" );
  it = result.begin();
  if( result.size() != 1 || (*it) != hhh )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "deepsearch + predicate + literal + child + predicate + literal: //bbb[@name='b1']/hhh[@name='h2']";
  bbb->addAttribute( "blah", "test@test2" );
  result = aaa->findTagList( "//bbb[@blah='test@test2']/hhh[@name='h2']" );
  if( result.size() != 0 )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

//   // -------
//   name = "deepsearch + predicate + path 1: //bbb[hhh]";
//   result = aaa->findTagList( "//bbb[hhh]" );
//   it = result.begin();
//   if( result.size() != 1 || (*it) != bbb )
//   {
//     ++fail;
//     printResult( name, result );
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
//   }
//   printf( "--------------------------------------------------------------\n" );

//   // -------
//   name = "filter 1: //bbb[1+2]";
//   result = aaa->findTagList( "//bbb[1+2]" );
//   it = result.begin();
//   if( result.size() != 1 || (*it) != iii )
//   {
//     ++fail;
//     printResult( name, result );
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
//   }
//   printf( "--------------------------------------------------------------\n" );




  // -- predicates --


  name = "cdata: //ccc";
  if( aaa->findCData( "//ccc" ) != "abc" )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );

  name = "cdata: //ddd";
  if( aaa->findCData( "//ddd" ) != "bcd" )
  {
    ++fail;
    printResult( name, result );
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
//   printf( "--------------------------------------------------------------\n" );













//   Tag *c2 = new Tag( ddd, "ccc" );
//   Tag *c3 = new Tag( c2, "ccc" );
//   Tag *c4 = new Tag( eee, "ccc" );
//   Tag *c5 = new Tag( c4, "ccc" );

// <aaa>
//   <bbb name='b1'>
//     <hhh>
//       <bbb name='b3'/>
//     </hhh>
//     <bbb name='b2'/>
//   </bbb>
//   <ccc>
//     <ddd>
//       <ccc>
//         <ccc/>
//       </ccc>
//     </ddd>
//     <eee>
//       <ccc>
//         <ccc/>
//       </ccc>
//     </eee>
//   </ccc>
//   <fff>
//     <ggg/>
//   </fff>
// </aaa>










  delete aaa;

  if( fail == 0 )
  {
    printf( "XPath: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "XPath: %d test(s) failed\n", fail );
    return 1;
  }

}
