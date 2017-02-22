##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = disco_test

disco_test_SOURCES = disco_test.cpp
disco_test_LDADD = ../../tag.o ../../stanza.o \
			../../prep.o \
			../../gloox.o \
			../../iq.o ../../util.o \
			../../error.o ../../jid.o ../../dataform.o \
			../../dataformfieldcontainer.o ../../dataformreported.o ../../dataformitem.o \
			../../dataformfield.o ../../softwareversion.o
disco_test_CFLAGS = $(CPPFLAGS)
