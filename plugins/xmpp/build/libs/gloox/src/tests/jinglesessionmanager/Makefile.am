##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = jinglesessionmanager_test

jinglesessionmanager_test_SOURCES = jinglesessionmanager_test.cpp
jinglesessionmanager_test_LDADD = ../../tag.o ../../stanza.o ../../base64.o \
			../../prep.o ../../gloox.o \
			../../iq.o ../../util.o ../../mutex.o \
			../../sha.o ../../error.o ../../jid.o \
			../../jinglecontent.o ../../jinglepluginfactory.o \
			../../stanzaextensionfactory.o ../../jingleiceudp.o ../../jinglefiletransfer.o
jinglesessionmanager_test_CFLAGS = $(CPPFLAGS) -g3
