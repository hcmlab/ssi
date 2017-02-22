
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = nonsaslauth_test

nonsaslauth_test_SOURCES = nonsaslauth_test.cpp
nonsaslauth_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o \
			../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
			../../iq.o ../../base64.o ../../sha.o
nonsaslauth_test_CFLAGS = $(CPPFLAGS)
