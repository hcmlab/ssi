##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = simanager_test

simanager_test_SOURCES = simanager_test.cpp
simanager_test_LDADD = ../../jid.o ../../tag.o \
			../../logsink.o ../../prep.o ../../util.o \
			../../gloox.o ../../iq.o ../../stanza.o \
			../../error.o
simanager_test_CFLAGS = $(CPPFLAGS)
