
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = registrationquery_test

registrationquery_test_SOURCES = registrationquery_test.cpp
registrationquery_test_LDADD = ../../stanza.o ../../jid.o ../../dataform.o ../../dataformfieldcontainer.o \
 		../../dataformreported.o ../../dataformitem.o ../../dataformfield.o ../../tag.o ../../prep.o \
 		../../gloox.o ../../stanzaextensionfactory.o \
		../../iq.o ../../util.o ../../sha.o ../../base64.o \
		../../error.o ../../oob.o ../../mutex.o
registrationquery_test_CFLAGS = $(CPPFLAGS)
