##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = presence_test

presence_test_SOURCES = presence_test.cpp
presence_test_LDADD = ../../tag.o ../../presence.o ../../stanza.o ../../jid.o ../../prep.o ../../gloox.o \
                      ../../util.o ../../sha.o ../../base64.o
presence_test_CFLAGS = $(CPPFLAGS)
