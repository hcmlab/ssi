
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT([gloox], [1.0.14], [js@camaya.net])
AC_CONFIG_SRCDIR(src/gloox.cpp)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h.unix)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

dnl
dnl extra paths
dnl

AC_ARG_WITH(extra_include_path, AC_HELP_STRING([--with-extra-include-path],
                                           [use additional include paths]),
            extra_include_path=$withval)
split_includes="`echo $extra_include_path | sed -e 's/:/ /g'`"
for incpath in $split_includes ; do
    CPPFLAGS="-I$incpath $CPPFLAGS"
done

AC_ARG_WITH(extra_library_path, AC_HELP_STRING([--with-extra-library-path],
                                               [use additional library paths]),
            extra_library_path=$withval)
split_libs="`echo $extra_library_path | sed -e 's/:/ /g'`"
for libpath in $split_libs ; do
    LDFLAGS="-L$libpath $LDFLAGS"
done

dnl Check for GCC atomic builtins
AC_MSG_CHECKING([whether GCC atomic builtins are available])
echo 'int main(int argc, char* argv[]) { int i; __sync_add_and_fetch(&i, 1); __sync_sub_and_fetch(&i, 1); return 0; } ' > check_atomicbuiltins.c
if test -z "`${CC} check_atomicbuiltins.c -o check_atomicbuiltins 2>&1`" ; then
	HAVE_GCC_ATOMIC_BUILTINS=yes
	AC_DEFINE(HAVE_GCC_ATOMIC_BUILTINS, 1, [Define to 1 if GCC atomic builtins are available])
	AC_MSG_RESULT([yes])
	if test $cross_compiling == "yes"; then
		# Some platforms don't seem to be fully happy for libraries using GCC atomic builtins
		#  unless -lgcc and -lgcc_s are explicitly included.
		# (Errors like
		# ld: hidden symbol `__sync_add_and_fetch_4' in .../libgcc.a(linux-atomic.o) is referenced by DSO
		# ld: final link failed: Nonrepresentable section on output)
		LDFLAGS="$LDFLAGS -lgcc -lgcc_s"
	fi
else
	HAVE_GCC_ATOMIC_BUILTINS=no
	AC_MSG_RESULT([no])
fi
rm -f check_atomicbuiltins*
AC_SUBST(HAVE_GCC_ATOMIC_BUILTINS,$HAVE_GCC_ATOMIC_BUILTINS)


dnl Checks for libraries.

dnl Libidn
AC_ARG_WITH(libidn, AC_HELP_STRING([--with-libidn=[DIR]],
                                   [Support IDN (needs GNU Libidn)]),
      libidn=$withval, libidn=yes)
if test "$libidn" != "no"; then
  if test "$libidn" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$libidn/lib"
    CPPFLAGS="${CPPFLAGS} -I$libidn/include"
  fi
  AC_CHECK_HEADER(idna.h,
  AC_CHECK_LIB(idn, stringprep_check_version,
           [libidn=yes LIBS="${LIBS} -lidn"], libidn=no),
           libidn=no)
fi
if test "$libidn" != "no" ; then
  AC_DEFINE(HAVE_LIBIDN, 1, [Define to 1 if you want IDN support.])
else
  AC_MSG_WARN([Libidn not found])
fi
AC_MSG_CHECKING([if Libidn should be used])
AC_MSG_RESULT($libidn)


dnl extra win32 libs
case "$host_os" in
  *mingw*|*cygwin*)
      LIBS="${LIBS} -lwsock32 -lgdi32 -ldnsapi";;
  *)
      CXXFLAGS="$CXXFLAGS -ansi";;
esac


ssl=no

dnl schannel
AC_ARG_WITH(schannel, AC_HELP_STRING([--with-schannel],
      [Support Stream Encryption (win32 only)]),
      schannel=yes, schannel=no)
if test "$schannel" = "yes"; then
  case "$host_os" in
    *mingw*|*cygwin*)
        ssl=yes
        LIBS="${LIBS} -lcrypt32 -lsecur32";;
    *)
        schannel=no
        echo "*"
        echo "* You provided the '--with-schannel' switch. SChannel is only available in MinGW/cygwin. Ignoring."
        echo "*";;
  esac
  AC_MSG_CHECKING([if SChannel should be used])
  AC_MSG_RESULT($schannel)
fi

dnl GnuTLS
if test "$ssl" = "no"; then
  AC_ARG_WITH(gnutls, AC_HELP_STRING([--with-gnutls=[yes/no]],
                                     [Support Stream Encryption (needs GnuTLS)]),
        gnutls=$withval, gnutls=yes)
  if test "$gnutls" != "no"; then
    PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.12], [gnutls=yes], [gnutls=no])
    if test "$gnutls" = "yes" ; then
      LIBS="${LIBS} ${GNUTLS_LIBS}"
      CPPFLAGS="${CPPFLAGS} ${GNUTLS_CFLAGS}"
      AC_DEFINE(HAVE_GNUTLS, 1, [Define to 1 if you want TLS support (GnuTLS). Undefine HAVE_OPENSSL.])
      AC_DEFINE(HAVE_GNUTLS_SESSION_CHANNEL_BINDING, 1, [Define to 1 if you have GnuTLS 2.12.0 or above.] )
      ssl=yes
    else
      AC_MSG_WARN([GnuTLS not found])
    fi
  fi
  AC_MSG_CHECKING([if GnuTLS should be used])
  AC_MSG_RESULT($gnutls)
fi

dnl OpenSSL
if test "$ssl" = "no"; then
  AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl=[DIR]],
                                      [Support Stream Encryption (needs OpenSSL)]),
        openssl=$withval, openssl=yes)
  if test "$openssl" != "no"; then
    if test "$openssl" != "yes"; then
      LDFLAGS="${LDFLAGS} -L$openssl/lib"
      CPPFLAGS="${CPPFLAGS} -I$openssl/include"
    fi
    LIBS_TMP=${LIBS}
    LIBS="${LIBS} -lssl -lcrypto"
    AC_CHECK_HEADER(openssl/ssl.h,
    AC_CHECK_LIB(ssl, SSL_library_init,
            [openssl=yes], openssl=no),
            [openssl=no LIBS=${LIBS_TMP}])
  fi
  if test "$openssl" != "no" ; then
    AC_DEFINE(HAVE_OPENSSL, 1, [Define to 1 if you want TLS support (OpenSSL). Undefine HAVE_GNUTLS.])
  else
    AC_MSG_WARN([OpenSSL not found])
  fi
  AC_MSG_CHECKING([if OpenSSL should be used])
  AC_MSG_RESULT($openssl)
fi

dnl Zlib
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib=[DIR]],
                                 [Support Stream Compression (needs Zlib)]),
      zlib=$withval, zlib=yes)
if test "$zlib" != "no"; then
  if test "$zlib" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$zlib/lib"
    CPPFLAGS="${CPPFLAGS} -I$zlib/include"
  fi
  AC_CHECK_HEADER(zlib.h,
  AC_CHECK_LIB(z, zlibVersion,
           [zlib=yes LIBS="${LIBS} -lz"], zlib=no),
           zlib=no)
fi
if test "$zlib" != "no" ; then
  AC_DEFINE(HAVE_ZLIB, 1, [Define to 1 if you want Stream Compression support.])
else
  AC_MSG_WARN([Zlib not found])
fi
AC_MSG_CHECKING([if Zlib should be used])
AC_MSG_RESULT($zlib)

dnl pthread
ACX_PTHREAD()
LIBS="$PTHREAD_LIBS $LIBS"
CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS"
LDFLAGS="$LDFLAGS $PTHREAD_CFLAGS"

dnl check for res_querydomain in libc, libbind and libresolv
AC_CHECK_FUNCS(res_querydomain)
if test "x-$ac_cv_func_res_querydomain" = "x-yes" ; then
  have_res_querydomain=yes
else
  AC_CHECK_LIB(resolv, res_querydomain)
  if test "x-$ac_cv_lib_resolv_res_querydomain" = "x-yes" ; then
    have_res_querydomain=yes
    AC_DEFINE(HAVE_RES_QUERYDOMAIN)
  else
    AC_CHECK_LIB(bind, res_querydomain)
    if test "x-$ac_cv_lib_bind_res_querydomain" = "x-yes" ; then
      have_res_querydomain=yes
      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
    else
      AC_MSG_CHECKING([for res_querydomain in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_querydomain(0,0,0,0,0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
                      have_res_querydomain=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for dn_skipname in libc, libbind and libresolv
AC_CHECK_FUNCS(dn_skipname)
if test "x-$ac_cv_func_dn_skipname" = "x-yes" ; then
  have_dn_skipname=yes
else
  AC_CHECK_LIB(resolv, dn_skipname)
  if test "x-$ac_cv_lib_resolv_dn_skipname" = "x-yes" ; then
    have_dn_skipname=yes
    AC_DEFINE(HAVE_DN_SKIPNAME)
  else
    AC_CHECK_LIB(bind, dn_skipname)
    if test "x-$ac_cv_lib_bind_dn_skipname" = "x-yes" ; then
      have_dn_skipname=yes
      AC_DEFINE(HAVE_DN_SKIPNAME)
    else
      AC_MSG_CHECKING([for dn_skipname in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <netinet/in.h>
 #include <resolv.h>]], [[dn_skipname(0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_DN_SKIPNAME)
                      have_dn_skipname=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for res_query in libc, libbind and libresolv
AC_CHECK_FUNCS(res_query)
if test "x-$ac_cv_func_res_query" = "x-yes" ; then
  have_res_query=yes
else
  AC_CHECK_LIB(resolv, res_query)
  if test "x-$ac_cv_lib_resolv_res_query" = "x-yes" ; then
    have_res_query=yes
    AC_DEFINE(HAVE_RES_QUERY)
  else
    AC_CHECK_LIB(bind, res_query)
    if test "x-$ac_cv_lib_bind_res_query" = "x-yes" ; then
      have_res_query=yes
      AC_DEFINE(HAVE_RES_QUERY)
    else
      AC_MSG_CHECKING([for res_query in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_query(0,0,0,0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERY)
                      have_res_query=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi


#  AC_SEARCH_LIBS(res_query,resolv bind,,
#                 AC_SEARCH_LIBS(__res_query,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(dn_expand,resolv bind,,
#                 AC_SEARCH_LIBS(__dn_expand,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(dn_skipname,resolv bind,,
#                 AC_SEARCH_LIBS(__dn_skipname,resolv bind,,have_resolver=no))
#  AC_SEARCH_LIBS(res_querydomain,resolv bind,,
#                 AC_SEARCH_LIBS(__res_querydomain,resolv bind,,have_resolver=no))


dnl Debug
debug="no"
AC_ARG_ENABLE( debug,
               [AC_HELP_STRING([--enable-debug],
                               [turn on debugging [default=no]])],
               [debug="yes"] )
if test "x$debug" = "xyes"; then
      CPPFLAGS="$CPPFLAGS -g3 -fno-inline -DDEBUG"
fi
AC_MSG_CHECKING([whether to enable debug])
AC_MSG_RESULT($debug)

dnl getaddrinfo
getaddrinfo="no"
AC_ARG_ENABLE( getaddrinfo,
               [AC_HELP_STRING([--enable-getaddrinfo],
                               [use getaddrinfo for dns lookups [default=no]])],
               [getaddrinfo="yes"] )
if test "x$getaddrinfo" = "xyes"; then
    AC_CHECK_FUNCS(getaddrinfo,,getaddrinfo="no")
fi
AC_MSG_CHECKING([whether to use getaddrinfo])
AC_MSG_RESULT($getaddrinfo)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h strings.h errno.h arpa/nameser.h)
AC_CHECK_FUNCS(setsockopt,,[AC_CHECK_LIB(socket,setsockopt)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl Want mdns support?
AC_ARG_ENABLE([mdns],
              [AC_HELP_STRING([--enable-mdns],
                              [use mdns [default=no]])],
              [enable_mdns=yes],
              [enable_mdns=no])
if test "$enable_mdns" = yes; then
  AC_DEFINE([HAVE_MDNS], [], [enable mdns support])
  AC_CHECK_HEADER(dns_sd.h,
                  AC_CHECK_LIB(dns_sd, DNSServiceCreateConnection,
                               [LIBS="${LIBS} -ldns_sd"], enable_mdns=no),
                  enable_mdns=no)
else
  MAYBE_LIB_mdns=
fi
AC_SUBST(MAYBE_LIB_mdns)
AC_MSG_CHECKING([if mDNS support should be built])
AC_MSG_RESULT($enable_mdns)

dnl build tests?
want_tests=yes
AC_ARG_WITH(tests, AC_HELP_STRING([--with-tests],
      [Build unit tests (default=yes]),
      want_tests=$withval, want_tests=yes)
AC_MSG_CHECKING([if unit tests should be built])
AC_MSG_RESULT($want_tests)

if test "$want_tests" = yes;
then
  MAYBE_TESTS=tests
else
  MAYBE_TESTS=
fi
AC_SUBST([MAYBE_TESTS])

dnl build examples?
want_examples=yes
AC_ARG_WITH(examples, AC_HELP_STRING([--with-examples],
      [Build example programs (default=yes)]),
      want_examples=$withval, want_examples=yes)
AC_MSG_CHECKING([if example programs should be built])
AC_MSG_RESULT($want_examples)

if test "$want_examples" = yes;
then
  MAYBE_EXAMPLES=examples
else
  MAYBE_EXAMPLES=
fi
AC_SUBST([MAYBE_EXAMPLES])


dnl Generating makefiles.
AC_CONFIG_FILES([
Makefile
gloox.pc
gloox-config
src/Makefile
src/examples/Makefile
src/tests/Makefile
src/tests/adhoc/Makefile
src/tests/adhoccommand/Makefile
src/tests/adhoccommandnote/Makefile
src/tests/amprule/Makefile
src/tests/amp/Makefile
src/tests/base64/Makefile
src/tests/capabilities/Makefile
src/tests/carbons/Makefile
src/tests/chatstatefilter/Makefile
src/tests/client/Makefile
src/tests/clientbase/Makefile
src/tests/connectionbosh/Makefile
src/tests/dataform/Makefile
src/tests/dataformfield/Makefile
src/tests/dataformitem/Makefile
src/tests/dataformreported/Makefile
src/tests/delayeddelivery/Makefile
src/tests/discoinfo/Makefile
src/tests/discoitems/Makefile
src/tests/disco/Makefile
src/tests/error/Makefile
src/tests/featureneg/Makefile
src/tests/flexofflineoffline/Makefile
src/tests/flexoffline/Makefile
src/tests/forward/Makefile
src/tests/gpgencrypted/Makefile
src/tests/gpgsigned/Makefile
src/tests/inbandbytestreamibb/Makefile
src/tests/inbandbytestream/Makefile
src/tests/iodata/Makefile
src/tests/iq/Makefile
src/tests/jid/Makefile
src/tests/jingleiceudp/Makefile
src/tests/jinglesession/Makefile
src/tests/jinglesessionjingle/Makefile
src/tests/jinglesessionmanager/Makefile
src/tests/lastactivityquery/Makefile
src/tests/lastactivity/Makefile
src/tests/md5/Makefile
src/tests/message/Makefile
src/tests/messageeventfilter/Makefile
src/tests/mucroommuc/Makefile
src/tests/mucroommucadmin/Makefile
src/tests/mucroommucowner/Makefile
src/tests/mucroommucuser/Makefile
src/tests/nickname/Makefile
src/tests/nonsaslauthquery/Makefile
src/tests/nonsaslauth/Makefile
src/tests/oob/Makefile
src/tests/parser/Makefile
src/tests/prep/Makefile
src/tests/presence/Makefile
src/tests/privacymanagerquery/Makefile
src/tests/privacymanager/Makefile
src/tests/privatexml/Makefile
src/tests/pubsubmanagerpubsub/Makefile
src/tests/pubsubmanager/Makefile
src/tests/pubsubevent/Makefile
src/tests/receipt/Makefile
src/tests/registrationquery/Makefile
src/tests/registration/Makefile
src/tests/rostermanagerquery/Makefile
src/tests/rostermanager/Makefile
src/tests/searchquery/Makefile
src/tests/search/Makefile
src/tests/sha/Makefile
src/tests/shim/Makefile
src/tests/simanager/Makefile
src/tests/simanagersi/Makefile
src/tests/stanzaextensionfactory/Makefile
src/tests/subscription/Makefile
src/tests/tag/Makefile
src/tests/tlsgnutls/Makefile
src/tests/uniquemucroomunique/Makefile
src/tests/util/Makefile
src/tests/vcard/Makefile
src/tests/vcardupdate/Makefile
src/tests/xpath/Makefile
src/tests/zlib/Makefile
])
AC_OUTPUT
