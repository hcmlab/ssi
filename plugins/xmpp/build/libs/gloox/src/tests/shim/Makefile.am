##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = shim_test

shim_test_SOURCES = shim_test.cpp
shim_test_LDADD = ../../shim.o ../../gloox.o ../../tag.o \
                  ../../util.o ../../stanza.o ../../message.o \
                  ../../jid.o ../../prep.o \
                  ../../stanzaextensionfactory.o ../../mutex.o

shim_test_CFLAGS = $(CPPFLAGS)
