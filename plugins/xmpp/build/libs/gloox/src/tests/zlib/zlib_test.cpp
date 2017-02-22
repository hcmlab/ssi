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

#include "../../compressionzlib.h"
#include "../../compressiondatahandler.h"
using namespace gloox;

#include "../../config.h"

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

#ifdef HAVE_ZLIB

class ZlibTest : public CompressionDataHandler
{
  public:
    ZlibTest() : m_zlib( this ) { m_zlib.init(); }
    ~ZlibTest() {}
    virtual void handleCompressedData( const std::string& data );
    virtual void handleDecompressedData( const std::string& data );
    const std::string data() { std::string ret = m_decompressed; m_decompressed = ""; return ret; }
    void compress(  const std::string& data );
  private:
    CompressionZlib m_zlib;
    std::string m_decompressed;
};

void ZlibTest::compress( const std::string& data )
{
  m_zlib.compress( data );
}

void ZlibTest::handleCompressedData( const std::string& data )
{
  m_zlib.decompress( data );
}

void ZlibTest::handleDecompressedData( const std::string& data )
{
  m_decompressed += data;
}

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  ZlibTest t;

  // -------
  name = "short test";
  const std::string a( 10, 'a' );
  t.compress( a );
  if( t.data() != a )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "longer test";
  const std::string b( 1000, 'b' );
  t.compress( b );
  if( t.data() != b )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "long test";
  const std::string c( 100000, 'b' );
  t.compress( c );
  if( t.data() != c )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "large test";
  const std::string d( 10000000, 'b' );
  t.compress( d );
  if( t.data() != d )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "concat test";
  t.compress( a );
  t.compress( b );
  t.compress( c );
  t.compress( d );
  if( t.data() != a + b + c + d )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }










  if( fail == 0 )
  {
    printf( "CompressionZlib: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "CompressionZlib: %d test(s) failed\n", fail );
    return 1;
  }

}

#else
int main( int /*argc*/, char** /*argv*/ )
{
  printf( "Zlib not available. Skipped tests.\n" );
}
#endif // HAVE_ZLIB
