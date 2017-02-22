##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = flexofflineoffline_test

flexofflineoffline_test_SOURCES = flexofflineoffline_test.cpp
flexofflineoffline_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
                        ../../gloox.o ../../message.o ../../util.o ../../error.o ../../jid.o \
                        ../../iq.o ../../base64.o ../../dataformfieldcontainer.o \
                        ../../dataform.o ../../dataformfield.o \
                        ../../dataformitem.o ../../softwareversion.o \
                        ../../dataformreported.o ../../mutex.o
flexofflineoffline_test_CFLAGS = $(CPPFLAGS)
