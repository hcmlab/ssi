##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = receipt_test

receipt_test_SOURCES = receipt_test.cpp
receipt_test_LDADD = ../../receipt.o ../../gloox.o ../../tag.o \
                  ../../util.o ../../stanza.o ../../message.o \
                  ../../jid.o ../../prep.o \
                  ../../stanzaextensionfactory.o ../../mutex.o

receipt_test_CFLAGS = $(CPPFLAGS)
