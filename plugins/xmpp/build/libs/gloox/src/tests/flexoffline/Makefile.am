##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = flexoffline_test

flexoffline_test_SOURCES = flexoffline_test.cpp
flexoffline_test_LDADD = ../../jid.o ../../tag.o \
                        ../../logsink.o ../../prep.o ../../util.o \
                        ../../gloox.o ../../iq.o ../../stanza.o \
                        ../../error.o ../../dataformfieldcontainer.o \
                        ../../dataform.o ../../dataformfield.o \
                        ../../dataformitem.o ../../softwareversion.o \
                        ../../dataformreported.o
flexoffline_test_CFLAGS = $(CPPFLAGS)
