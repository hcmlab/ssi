##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = lastactivityquery_test

lastactivityquery_test_SOURCES = lastactivityquery_test.cpp
lastactivityquery_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
                        ../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
                        ../../iq.o ../../base64.o ../../dataformfieldcontainer.o \
                        ../../dataform.o ../../dataformfield.o \
                        ../../dataformitem.o ../../softwareversion.o \
                        ../../dataformreported.o ../../mutex.o
lastactivityquery_test_CFLAGS = $(CPPFLAGS)
