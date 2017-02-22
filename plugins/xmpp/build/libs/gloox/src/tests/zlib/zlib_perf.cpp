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

#include "../../compressionzlib.h"
#include "../../compressiondatahandler.h"
using namespace gloox;

#include "../../config.h"

#include <stdio.h>
#include <locale.h>
#include <cstdlib>
#include <string>
#include <sys/time.h>
#include <cstdio> // [s]print[f]

#ifdef HAVE_ZLIB

class ZlibTest : public CompressionDataHandler
{
  public:
    ZlibTest() : m_zlib( this ) {}
    ~ZlibTest() {}
    void handleCompressedData( const std::string& data )
      { m_zlib.decompress( data ); }
    void handleDecompressedData( const std::string& data )
      { m_decompressed += data; }
    const std::string data()
      { std::string ret = m_decompressed; m_decompressed = ""; return ret; }
    void compress(  const std::string& data )
      { m_zlib.compress( data ); }
  private:
    CompressionZlib m_zlib;
    std::string m_decompressed;
};

static const double divider = 1000000;
static const int num = 250;
static double t;

static void printTime ( const char * testName, struct timeval tv1, struct timeval tv2 )
{
  t = tv2.tv_sec - tv1.tv_sec;
  t +=  ( tv2.tv_usec - tv1.tv_usec ) / divider;
  printf( "%s: %.03f seconds (%.00f/s)\n", testName, t, num / t );
}

static const int sz_max = 1000000;

static char values[sz_max+1];

static void randomize( const int size )
{
  if( size > sz_max )
  {
    printf( "error: randomize size bigger than buffer size\n" );
    exit( 1 );
  }
  srand( time(NULL) );
  for (int i = 0; i < size; ++i)
  {
    values[i] = rand() % 96 + 32;
  }
  values[size] = 0;
}

int main( int, char** )
{
//   int fail = 0;
  std::string name;
  ZlibTest t;

  struct timeval tv1;
  struct timeval tv2;

  printf("testing %d run of 10^{4,5,6}...\n", num);

  // -------
  randomize( 10000 );
  std::string s (values);
  gettimeofday( &tv1, 0 );
  for (int x=0; x<num; ++x)
  {
    t.compress( s );
  }
  gettimeofday( &tv2, 0 );
  printTime( "small", tv1, tv2 );

  // -------
  randomize( 100000 );
  s = values;
  gettimeofday( &tv1, 0 );
  for (int x=0; x<num; ++x)
  {
    t.compress( s );
  }
  gettimeofday( &tv2, 0 );
  printTime( "medium", tv1, tv2 );

  // -------
  randomize( 1000000 );
  s = values;
  gettimeofday( &tv1, 0 );
  for (int x=0; x<num; ++x)
  {
    t.compress( s );
  }
  gettimeofday( &tv2, 0 );
  printTime( "large", tv1, tv2 );

}

#else
int main( int /*argc*/, char** /*argv*/ )
{
  printf( "Zlib not available. Skipped tests.\n" );
}
#endif // HAVE_ZLIB

#else // _WIN32
int main( int, char** ) { return 0; }
#endif // !_WIN32
