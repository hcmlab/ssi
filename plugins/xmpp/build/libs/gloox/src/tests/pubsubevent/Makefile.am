##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = pubsubevent_test

pubsubevent_test_SOURCES = pubsubevent_test.cpp
pubsubevent_test_LDADD = ../../gloox.o ../../tag.o ../../jid.o ../../prep.o \
                           ../../util.o ../../error.o ../../pubsubevent.o \
                           ../../dataform.o ../../dataformfield.o \
                           ../../dataformfieldcontainer.o ../../dataformitem.o \
                           ../../dataformreported.o

pubsubevent_test_CFLAGS = $(CPPFLAGS)
