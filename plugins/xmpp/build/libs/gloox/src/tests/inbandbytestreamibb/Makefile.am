##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = inbandbytestreamibb_test

inbandbytestreamibb_test_SOURCES = inbandbytestreamibb_test.cpp
inbandbytestreamibb_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
			../../iq.o ../../base64.o ../../mutex.o
inbandbytestreamibb_test_CFLAGS = $(CPPFLAGS)
