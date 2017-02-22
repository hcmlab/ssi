##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = chatstatefilter_test

chatstatefilter_test_SOURCES = chatstatefilter_test.cpp
chatstatefilter_test_LDADD = ../../tag.o ../../stanza.o ../../stanzaextensionfactory.o \
 				../../jid.o ../../prep.o \
				../../message.o ../../util.o \
				../../gloox.o ../../chatstate.o ../../mutex.o
chatstatefilter_test_CPPFLAGS = $(CPPFLAGS)
