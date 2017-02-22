##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = lastactivity_test

lastactivity_test_SOURCES = lastactivity_test.cpp
lastactivity_test_LDADD = ../../jid.o ../../tag.o \
                        ../../logsink.o ../../prep.o ../../util.o \
                        ../../gloox.o ../../iq.o ../../stanza.o \
                        ../../error.o ../../dataformfieldcontainer.o \
                        ../../dataform.o ../../dataformfield.o \
                        ../../dataformitem.o ../../softwareversion.o \
                        ../../dataformreported.o
lastactivity_test_CFLAGS = $(CPPFLAGS)
