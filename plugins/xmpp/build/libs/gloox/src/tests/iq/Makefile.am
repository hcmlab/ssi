##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = iq_test

iq_test_SOURCES = iq_test.cpp
iq_test_LDADD = ../../tag.o ../../iq.o ../../stanza.o ../../jid.o ../../prep.o ../../gloox.o ../../util.o \
                ../../sha.o ../../base64.o
iq_test_CFLAGS = $(CPPFLAGS)
