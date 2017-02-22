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

#ifndef _WIN32

#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <cstdlib>
#include <string>
#include <cstdio> // [s]print[f]

#include <sys/time.h>

static double divider = 1000000;
static int num = 2500;
static double t;

static Tag *tag;

static void printTime ( const char * testName, struct timeval tv1, struct timeval tv2 )
{
  t = tv2.tv_sec - tv1.tv_sec;
  t +=  ( tv2.tv_usec - tv1.tv_usec ) / divider;
  printf( "%s: %.03f seconds (%.00f/s)\n", testName, t, num / t );
}

static Tag * newTag ( const std::string& str )
{
  Tag *aaa = new Tag( str, str );
  Tag *bbb = new Tag( aaa, str, str ); bbb->addAttribute( str, str );
  Tag *ccc = new Tag( aaa, str, str ); ccc->addAttribute( str, str );
  Tag *ddd = new Tag( ccc, str, str ); ddd->addAttribute( str, str );
  Tag *eee = new Tag( ccc, str, str ); eee->addAttribute( str, str );
  Tag *fff = new Tag( aaa, str, str ); fff->addAttribute( str, str );
  Tag *ggg = new Tag( fff, str, str ); ggg->addAttribute( str, str );
  Tag *hhh = new Tag( bbb, str, str ); hhh->addAttribute( str, str );
  Tag *iii = new Tag( bbb, str, str ); iii->addAttribute( str, str );
  Tag *jjj = new Tag( hhh, str, str ); jjj->addAttribute( str, str );
  return aaa;
}

static const std::string simpleString    = "azzaaaggaaaaqs dfqsdadddaads ";
static const std::string escapableString = ">aa< < <w< w<<<<.' <.<& %)(>>";
static const std::string escapedString   = "&amp;&lt;&gt;&apos;&quot;&#60;&#62;&#39;&#34;&#x3c;&#x3e;&#x3C;"
                                      "&#x3E;&#x27;&#x22;&#X3c;&#X3e;&#X3C;&#X3E;&#X27;&#X22;";

static inline Tag * newSimpleTag ()    { return newTag( simpleString ); }
static inline Tag * newEscapedTag ()   { return newTag( escapedString ); }
static inline Tag * newEscapableTag () { return newTag( escapableString ); }


static const int sz_max = 1000;

static char values[sz_max];

static void randomize( const int size )
{
  srand( time( 0 ) );
  for (int i = 0; i < size-1; ++i)
  {
    values[i] = rand() % 96 + 32;
  }
  values[size-1] = 0;
}

int main( int /*argc*/, char** /*argv*/ )
{
  struct timeval tv1;
  struct timeval tv2;

  printf( "Testing %d...\n", num );

  tag = newSimpleTag();
  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    tag->xml();
  }
  gettimeofday( &tv2, 0 );
  delete tag;
  printTime ("non escaping xml", tv1, tv2);


  // ---------------------------------------------------------------------

  tag = newEscapableTag();
  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    tag->xml();
  }
  gettimeofday( &tv2, 0 );
  delete tag;
  printTime ("escaping xml", tv1, tv2);


  // ---------------------------------------------------------------------

  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    delete newSimpleTag();
  }
  gettimeofday( &tv2, 0 );
  printTime ("non relaxing create/delete", tv1, tv2);


  // -----------------------------------------------------------------------

  gettimeofday( &tv1, 0 );
  for (int i = 0; i < num; ++i)
  {
    randomize( 100 );
    delete newTag( values);
  }
  gettimeofday( &tv2, 0 );
  printTime ("relaxing create/delete (small)", tv1, tv2);


  // -----------------------------------------------------------------------

  gettimeofday( &tv1, 0 );
  for (int i = 0; i < num; ++i)
  {
    randomize( 1000 );
    delete newTag( values );
  }
  gettimeofday( &tv2, 0 );
  printTime ("relaxing create/delete (big)", tv1, tv2);

  // -----------------------------------------------------------------------

  tag = newSimpleTag();

  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    delete tag->clone();
  }
  gettimeofday( &tv2, 0 );
  printTime ("clone/delete", tv1, tv2);




  delete tag;

  return 0;
}
#else
int main( int, char** ) { return 0; }
#endif
