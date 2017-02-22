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

#include "../../base64.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  std::string b;
  std::string sample;



  // -------
  name = "empty string";
  sample = "";
  b = Base64::encode64( sample );
  if( sample != Base64::decode64( b ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  b = "";
  sample = "";

  // -------
  name = "leviathan test";
  sample = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
  b = Base64::encode64( sample );
  if( sample != Base64::decode64( b ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed: %s\n", name.c_str(), b.c_str() );
  }
  b = "";
  sample = "";




  if( fail == 0 )
  {
    printf( "Base64: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Base64: %d test(s) failed\n", fail );
    return 1;
  }


}
