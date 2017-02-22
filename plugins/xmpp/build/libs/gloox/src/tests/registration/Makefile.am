##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = registration_test

registration_test_SOURCES = registration_test.cpp
registration_test_LDADD = ../../stanza.o ../../jid.o ../../dataform.o ../../dataformfieldcontainer.o \
 		../../dataformreported.o ../../dataformitem.o ../../dataformfield.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o ../../oob.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o \
		../../error.o ../../mutex.o
registration_test_CFLAGS = $(CPPFLAGS)
