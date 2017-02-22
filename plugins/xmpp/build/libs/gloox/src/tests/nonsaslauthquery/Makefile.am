
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = nonsaslauthquery_test

nonsaslauthquery_test_SOURCES = nonsaslauthquery_test.cpp
nonsaslauthquery_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o \
			../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
			../../iq.o ../../base64.o ../../sha.o ../../stanzaextensionfactory.o ../../mutex.o
nonsaslauthquery_test_CFLAGS = $(CPPFLAGS)
