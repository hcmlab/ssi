##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = amp_test

amp_test_SOURCES = amp_test.cpp
amp_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
			../../amp.o ../../mutex.o
amp_test_CFLAGS = $(CPPFLAGS)
