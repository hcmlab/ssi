##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = pubsubmanager_test

pubsubmanager_test_SOURCES = pubsubmanager_test.cpp
pubsubmanager_test_LDADD = ../../gloox.o ../../tag.o ../../iq.o \
				 ../../jid.o ../../prep.o \
				 ../../stanza.o ../../util.o \
                                 ../../error.o \
				 ../../dataform.o \
                                 ../../dataformfield.o \
				 ../../dataformfieldcontainer.o \
                                 ../../dataformitem.o \
                                 ../../dataformreported.o \
				 ../../pubsubitem.o ../../shim.o \
				 ../../mutex.o

pubsubmanager_test_CFLAGS = $(CPPFLAGS)
