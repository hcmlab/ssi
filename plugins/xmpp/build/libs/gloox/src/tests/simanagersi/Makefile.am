##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = simanagersi_test

simanagersi_test_SOURCES = simanagersi_test.cpp
simanagersi_test_LDADD = ../../jid.o ../../tag.o \
                        ../../logsink.o ../../prep.o ../../util.o \
                        ../../gloox.o ../../iq.o ../../stanza.o \
                        ../../error.o ../../stanzaextensionfactory.o ../../mutex.o
simanagersi_test_CFLAGS = $(CPPFLAGS)
