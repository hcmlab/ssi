##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = capabilities_test

capabilities_test_SOURCES = capabilities_test.cpp
capabilities_test_LDADD = ../../tag.o ../../stanza.o ../../prep.o ../../stanzaextensionfactory.o \
			../../gloox.o ../../base64.o ../../util.o ../../sha.o \
                        ../../jid.o ../../iq.o ../../error.o ../../softwareversion.o \
                        ../../dataform.o ../../dataformfieldcontainer.o ../../dataformreported.o \
                        ../../dataformitem.o ../../dataformfield.o ../../mutex.o
capabilities_test_CFLAGS = $(CPPFLAGS)
