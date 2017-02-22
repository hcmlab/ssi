
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = jinglesessionjingle_test

jinglesessionjingle_test_SOURCES = jinglesessionjingle_test.cpp
jinglesessionjingle_test_LDADD = ../../stanza.o ../../jid.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o \
		../../jinglecontent.o ../../error.o ../../mutex.o \
		../../jinglepluginfactory.o
jinglesessionjingle_test_CFLAGS = $(CPPFLAGS)
