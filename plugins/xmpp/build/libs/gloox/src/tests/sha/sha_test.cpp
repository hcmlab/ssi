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

#include "../../sha.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  SHA sha;



  // -------
  name = "empty string";
  sha.feed( "" );
  sha.finalize();
  if( sha.hex() != "da39a3ee5e6b4b0d3255bfef95601890afd80709" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  sha.reset();

  // -------
  name = "The quick brown fox jumps over the lazy dog";
  sha.feed( name );
  sha.finalize();
  if( sha.hex() != "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "The quick brown fox jumps over the lazy cog";
  sha.feed( name );
  sha.finalize();
  if( sha.hex() != "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "two-step";
  sha.feed( "The quick brown fox " );
  sha.feed( "jumps over the lazy dog" );
  sha.finalize();
  if( sha.hex() != "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "54byte string";
  sha.feed( std::string( 54, 'x' ) );
  sha.finalize();
  if( sha.hex() != "31045e7bb077ff8d188a776b196b980388735dbb" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "55byte string";
  sha.feed( std::string( 55, 'x' ) );
  sha.finalize();
  if( sha.hex() != "cef734ba81a024479e09eb5a75b6ddae62e6abf1" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "56byte string";
  sha.feed( std::string( 56, 'x' ) );
  sha.finalize();
  if( sha.hex() != "901305367c259952f4e7af8323f480d59f81335b" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "57byte string";
  sha.feed( std::string( 57, 'x' ) );
  sha.finalize();
  if( sha.hex() != "025ecbd5d70f8fb3c5457cd96bab13fda305dc59" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();

  // -------
  name = "many-step";
  sha.feed( "The" );
  sha.feed( " quick bro" );
  sha.feed( "" );
  sha.feed( "wn fox " );
  sha.feed( "jumps over the lazy dog" );
  sha.finalize();
  if( sha.hex() != "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12" )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), sha.hex().c_str() );
  }
  sha.reset();



  if( fail == 0 )
  {
    printf( "SHA: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "SHA: %d test(s) failed\n", fail );
    return 1;
  }


}
