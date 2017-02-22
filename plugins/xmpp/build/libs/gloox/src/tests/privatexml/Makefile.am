##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = privatexml_test

privatexml_test_SOURCES = privatexml_test.cpp
privatexml_test_LDADD = ../../gloox.o ../../tag.o \
                  ../../util.o ../../stanza.o ../../message.o \
                  ../../jid.o ../../prep.o \
                  ../../stanzaextensionfactory.o ../../iq.o ../../mutex.o

privatexml_test_CFLAGS = $(CPPFLAGS)
