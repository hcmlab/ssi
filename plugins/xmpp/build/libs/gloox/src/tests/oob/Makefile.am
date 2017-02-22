##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = oob_test

oob_test_SOURCES = oob_test.cpp
oob_test_LDADD = ../../oob.o ../../tag.o ../../gloox.o ../../iq.o ../../stanzaextensionfactory.o \
                 ../../stanza.o ../../util.o ../../jid.o ../../prep.o ../../mutex.o
oob_test_CFLAGS = $(CPPFLAGS)
