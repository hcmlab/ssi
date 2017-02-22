##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = vcard_test

vcard_test_SOURCES = vcard_test.cpp
vcard_test_LDADD = ../../vcard.o ../../gloox.o ../../tag.o ../../util.o ../../iq.o \
                   ../../stanzaextensionfactory.o ../../base64.o ../../stanza.o \
                   ../../jid.o ../../prep.o ../../mutex.o
vcard_test_CFLAGS = $(CPPFLAGS)
