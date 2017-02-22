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

#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"
#include "../../iq.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

class SETest : public StanzaExtension
{
  public:
    SETest( const Tag* tag ) : StanzaExtension( ExtUser + 1 ), m_tag( const_cast<Tag*>( tag ) ) {}
    ~SETest() {}

    virtual const std::string& filterString() const
    {
      static const std::string filter = "/foo/bar";
      return filter;
    }

    virtual StanzaExtension* newInstance( const Tag* tag ) const
    { return new SETest( tag ); }

    virtual Tag* tag() const
    { return m_tag; }

    virtual StanzaExtension* clone() const
    { return new SETest( m_tag ? m_tag->clone() : 0 ); }

  private:
    Tag* m_tag;

};

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  StanzaExtensionFactory sef;

  // -------
  name = "SEFactory test";
  SETest* set = new SETest( 0 ); // deleted by StanzaExtensionFactory sef;
  sef.registerExtension( set );
  Tag* f = new Tag( "foo" );
  Tag* b = new Tag( f, "bar", "attr", "value" );
  IQ iq( IQ::Set, JID(), "" );
  sef.addExtensions( iq, f );
  const StanzaExtension* se = iq.findExtension( ExtUser + 1 );
  if( se == 0 || se == set || se->tag() != b )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete f;

  // -------
  name = "remove ext";
  if( !sef.removeExtension( ExtUser + 1 ) )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }


  if( fail == 0 )
  {
    printf( "StanzaExtensionFactory: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "StanzaExtensionFactory: %d test(s) failed\n", fail );
    return 1;
  }

}
