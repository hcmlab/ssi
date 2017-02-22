##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = amprule_test

amprule_test_SOURCES = amprule_test.cpp
amprule_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
			../../amp.o ../../mutex.o
amprule_test_CFLAGS = $(CPPFLAGS)
